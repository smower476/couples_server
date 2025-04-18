#include "../include/db.h"
#include "../include/token.h"
#include <cstdint>
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <array>
#include <sstream>
#include <vector>

struct PipeResult {
    std::string output;
    std::string error;
    int exit_code;
};

PipeResult exec_pipe(const std::string& cmd, const std::string& /* input */) {
    PipeResult result;
    result.exit_code = -1;

#define POPEN popen
#define PCLOSE pclose

struct PipeCloser {
    void operator()(FILE* pipe) const {
        if (pipe) {
            PCLOSE(pipe);
        }
    }
};

    std::unique_ptr<FILE, PipeCloser> pipe(popen(cmd.c_str(), "r"));

    if (!pipe) {
        int error_code = errno;
        result.error = "popen() failed! errno: " + std::to_string(error_code) + " (" + strerror(error_code) + ")";
        return result;
    }

    std::array<char, 256> buffer;
    result.output = "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result.output += buffer.data();
    }
    FILE* pipe_ptr = pipe.release();
    if (pipe_ptr) {
         result.exit_code = PCLOSE(pipe_ptr);
    } else {
         // If pipe creation failed initially, result.exit_code remains -1
         // If release happened but PCLOSE fails, exit_code might be inaccurate.
         // Consider more robust error checking if PCLOSE failure is critical.
    }

    return result;
}


const char * pqxx_connection = std::getenv("PQXX_CONNECTION");
pqxx::connection conn(pqxx_connection);
//pqxx::connection conn("dbname=couples_db user=postgres host=localhost port=5432");

int create_table(const std::string query) {
    try {
        pqxx::work txn(conn);

        txn.exec(query);
        txn.commit();
        return 1;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Failed query: " << e.query() << std::endl;
        return -1;
    } catch (const std::exception &e) {
        std::cerr << "Error: failed to process DB request" << e.what() << std::endl;
        return -2;
    }
}

void create_tables() {
    create_table(tables::create_users_table);
    create_table(tables::create_token_table);
    create_table(tables::create_quiz_table);
    create_table(tables::create_quiz_user_answer_table);
    create_table(tables::create_quiz_content_table);
    create_table(tables::create_quiz_answer_content_table);
}

std::string hash_password(const std::string& password) {
    char hashed_password[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hashed_password,
            password.c_str(),
            password.length(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hashed_password);
}

bool verify_password(const std::string& password, const std::string& hashed_password) {
    return crypto_pwhash_str_verify(hashed_password.c_str(), password.c_str(), password.length()) == 0;
}

int add_user(const std::string& username, const std::string& password) {
    try {
        std::string hashed_password = hash_password(password);
        pqxx::work txn(conn);
        std::string query = "insert into users (username, password) values ($1, $2)";

        std::cout << "executing query: " << query << " with username: " << username << std::endl;

        txn.exec_params(query, username, hashed_password);
        txn.commit();

        std::cout << "user added successfully." << std::endl;
        return 0;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;

        // check for duplicate key violation (error code 23505)
        if (std::string(e.what()).find("23505") != std::string::npos) {
            std::cerr << "error: username already exists." << std::endl;
            return -3;
        }

        return -1;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        return -2;
    }
}

bool validate_user(const std::string& username, const std::string& password) {
    try {
        pqxx::work txn(conn);

        const std::string query = "SELECT password FROM users WHERE username = $1;";
        pqxx::result result = txn.exec_params(query, username);

        if (result.empty()) {
            std::cerr << "Error: Username not found." << std::endl;
            return false;
        }

        std::string hashed_password = result[0][0].as<std::string>();

        return verify_password(password, hashed_password);
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Failed query: " << e.query() << std::endl;
        throw std::runtime_error("Database error occurred");
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw std::runtime_error("An error occurred during user validation");
        return false;
    }
}
std::int64_t get_user_id(const std::string& jwt){
    try {
        std::string username = decode_jwt(jwt); // Can throw std::exception
        pqxx::read_transaction txn(conn); // Use read transaction for SELECT
        std::string query = "SELECT id FROM users WHERE username = $1;";

        std::cout << "executing query: " << query << " with username: " << username << std::endl;

        pqxx::row result = txn.exec_params1(query, username); // Expects exactly one user
        int64_t id = result[0].as<std::int64_t>();
        // No commit needed for read_transaction
        std::cout<<"\n USER ID " << id << "\n";
        return id;
    } catch (const pqxx::unexpected_rows &) {
        // Username not found in database (or multiple found, which is unlikely)
        std::cerr << "error: user not found for provided token (username: " << decode_jwt(jwt) << ")" << std::endl; // Decode again for logging, consider efficiency
        return -3; // Specific error code for user not found
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        return -1; // SQL error
    } catch (const std::exception &e) {
        // Could be JWT decode error or other std::exception
        std::cerr << "error: failed to process db request (get_user_id): " << e.what() << std::endl;
        return -2; // Other errors (e.g., JWT decode)
    }
}
int64_t generate_link_code(const int64_t id){
    try {
        std::srand(std::time(NULL));
        int link_code =100000 + std::rand() % 899999;
        pqxx::work txn(conn);
        // INSERT INTO token (user_id, link_token) VALUES (1, 123456) ON CONFLICT (user_id) DO UPDATE SET link_token = 3;
        // std::string query = "INSERT INTO token (user_id, link_token) VALUES ($1, $2)";
        std::string query = "INSERT INTO token (user_id, link_token, expired_at) VALUES ($1, $2, NOW() + INTERVAL '10 minutes') ON CONFLICT (user_id) DO UPDATE SET link_token=$2, expired_at=(NOW() + INTERVAL '10 minutes')";

        std::cout << "executing query: " << query << " with id: " << id << " link code: " << link_code << std::endl;

        txn.exec_params(query, id, link_code);

        txn.commit();

        std::cout << "link code genereted successfully." << std::endl;
        return link_code;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        return -1;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        return -2;
    }
}

int link_user(const int link_token, const std::string &jwt){
    try {
        int64_t id = get_user_id(jwt);
        if (id < 0) return id;

        pqxx::work txn(conn);
        std::string query =  R"(
            WITH token_owner AS (
                SELECT u.id
                FROM users u
                JOIN token t ON u.id = t.user_id
                WHERE t.link_token = $1 AND t.expired_at > NOW()
                LIMIT 1
            )
            UPDATE users
            SET linked_user = CASE
                WHEN users.id = $2 THEN (SELECT id FROM token_owner)
                WHEN users.id = (SELECT id FROM token_owner) THEN $2
                ELSE linked_user
            END
            WHERE users.id IN ($2, (SELECT id FROM token_owner))
            AND EXISTS (SELECT 1 FROM token WHERE link_token = $1 AND expired_at > NOW())
        RETURNING 1
        )";

        std::cout << "executing query: " << query << " with id: " << id << " link code: " << link_token << std::endl;

        auto result = txn.exec_params(query, link_token, id);
        txn.commit();

        if (result.empty()) {
            return -3;
        }

        std::cout << "User linked successfully!" << std::endl;
        return id;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        return -1;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        return -2;
    }
}

int64_t get_daily_quiz(const int64_t id){
    try {
        // Use a read-only transaction for SELECT queries
        pqxx::read_transaction txn(conn);
        // Select only the ID of the latest quiz (public or user's)
        std::string query = R"(
            SELECT id FROM quiz
            WHERE belongs_to = 0 OR belongs_to = $1
            ORDER BY created_at DESC
            LIMIT 1
        )";

        std::cout << "executing query: " << query << " with id: " << id << std::endl;

        // Use exec_params1 which expects exactly one row
        pqxx::row result = txn.exec_params1(query, id);
        // No explicit commit needed for read_transaction

        int64_t quiz_id = result[0].as<int64_t>();
        std::cout << "\n Daily quiz ID: " << quiz_id << "\n";
        return quiz_id;

    } catch (const pqxx::unexpected_rows &) {
        // exec_params1 throws this if 0 or >1 rows are returned.
        // Treat this as "not found" for the daily quiz.
        std::cout << "\n No daily quiz found for user id: " << id << " (unexpected_rows exception)\n";
        return -1; // Indicate quiz not found
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        return -2; // Indicate SQL error
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        return -3; // Indicate other exception
    }
}

std::string get_quiz_content(const int64_t quiz_id, const int64_t user_id){
    try {
        // quiz_id , user_id (jwt)
        if (user_id < 0) throw std::runtime_error("Invalid JWT or user not found");;

        pqxx::work txn(conn);
        std::string query = R"(
            SELECT json_build_object(
            'quiz_content', json_agg(
                json_build_object(
                    'content_id', quiz_content.id,
                    'content_data', quiz_content.question,
                    'answers', (
                        SELECT json_agg(
                            json_build_object(
                                'answer_id', qac.id,
                                'answer_content', qac.content
                            )
                        )
                        FROM quiz_answer_content qac
                        WHERE qac.question_id = quiz_content.id
                    )
                )
            )
            ) as json_result
            FROM quiz
            RIGHT JOIN quiz_content ON quiz.id = quiz_content.quiz_id
            WHERE quiz.id = $1 AND (quiz.belongs_to = $2 OR quiz.belongs_to = 0)
            GROUP BY quiz.id;
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " and quiz_id " << quiz_id << std::endl;
        pqxx::result result = txn.exec_params(query, quiz_id, user_id);

        // Check if the query returned any rows and if the result is not NULL
        if (result.empty() || result[0][0].is_null()) {
            txn.commit(); // Commit even if no content found
            std::cout << "\n No quiz content found or result is NULL for quiz_id: " << quiz_id << ", user_id: " << user_id << "\n";
            return "{}"; // Return empty JSON object
        }

        std::string daily_quiz_json = result[0][0].as<std::string>();
        txn.commit();
        std::cout<<"\n quiz content " << daily_quiz_json << "\n";

        return daily_quiz_json;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

#include <fstream>
#include <filesystem>

// Add Quiz Data using Python script for parsing
int add_quiz(int64_t user_id, const std::string& quiz_json_str) {
    std::cout << "Entering add_quiz for user_id: " << user_id << std::endl;
    // Escape the JSON string for safe command-line argument passing
    // Replace single quotes with '\'' (end quote, escaped quote, start quote) and wrap in single quotes.
    std::string escaped_json_str = "'"; // Start with single quote
    for (char c : quiz_json_str) {
        if (c == '\'') {
            escaped_json_str += "'\\''";
        } else {
            escaped_json_str += c;
        }
    }
    escaped_json_str += "'"; // End with single quote

    // Construct the command to execute the Python script with the JSON string as an argument
    std::string command = "python3 scripts/parse_quiz.py " + escaped_json_str;
    // Log a simplified version for clarity, avoiding potentially huge JSON strings in logs
    std::cerr << "Executing Python command: python3 scripts/parse_quiz.py '<JSON_DATA>'" << std::endl;

    std::cout << "Calling Python script to parse quiz JSON..." << std::endl;
    PipeResult script_result = exec_pipe(command, ""); // Pass empty input string as before

    std::cerr << "Python script output (combined):\n" << script_result.output << std::endl;
    std::cerr << "Python script error:\n" << script_result.error << std::endl;
    std::cerr << "Python script exit code: " << script_result.exit_code << std::endl;
    std::cout << "Python script finished with exit code: " << script_result.exit_code << std::endl;

   if (script_result.exit_code != 0) {
        std::cerr << "Python script execution failed. Exit code: " << script_result.exit_code << std::endl;
        std::cerr << "Script output/error: " << script_result.output << std::endl;
        // No temporary file to remove
        std::cout << "Exiting add_quiz with return code: -6" << std::endl;
        return -6; // Indicate script execution failure
    }

    std::stringstream ss(script_result.output);
    std::string line;
    std::string quiz_name;
    int num_questions = 0;
    std::vector<std::pair<std::string, std::vector<std::string>>> questions_data;

    try {
        // Helper function to extract value after the last tab
        auto extract_value = [](const std::string& line) -> std::string {
            size_t tab_pos = line.rfind('\t');
            if (tab_pos == std::string::npos) {
                throw std::runtime_error("Expected tab delimiter not found in line: " + line);
            }
            return line.substr(tab_pos + 1);
        };

        // Line 1: Quiz Name
        if (!std::getline(ss, line)) throw std::runtime_error("Failed to read quiz name line");
        quiz_name = extract_value(line);
        std::cerr << "Parsed quiz_name: " << quiz_name << std::endl;

        // Line 2: Number of Questions
        if (!std::getline(ss, line)) throw std::runtime_error("Failed to read number of questions line");
        num_questions = std::stoi(extract_value(line));
         std::cerr << "Parsed num_questions: " << num_questions << std::endl;

        for (int i = 0; i < num_questions; ++i) {
            std::string question_text;
            int num_options = 0;
            std::vector<std::string> options_text;

            // Question Text Line (e.g., "QUESTION\t1\tWhat is your favorite color?")
            if (!std::getline(ss, line)) throw std::runtime_error("Failed to read question text line for question " + std::to_string(i+1));
            question_text = extract_value(line); // Value is after the last tab
             std::cerr << "Parsed question_text(" << i+1 << "): " << question_text << std::endl;

            // Number of Options Line (e.g., "NUM_OPTIONS\t1\t4")
            if (!std::getline(ss, line)) throw std::runtime_error("Failed to read number of options line for question " + std::to_string(i+1));
            num_options = std::stoi(extract_value(line)); // Value is after the last tab
             std::cerr << "Parsed num_options(" << i+1 << "): " << num_options << std::endl;

            for (int j = 0; j < num_options; ++j) {
                std::string option_text;
                // Option Text Line (e.g., "OPTION\t1\tA\tBlue")
                if (!std::getline(ss, line)) throw std::runtime_error("Failed to read option text line " + std::to_string(j+1) + " for question " + std::to_string(i+1));
                option_text = extract_value(line); // Value is after the last tab
                 std::cerr << "Parsed option_text(" << i+1 << "," << j+1 << "): " << option_text << std::endl;
                options_text.push_back(option_text);
            }

            // Parse the Answer Line (e.g., "ANSWER\t1\tC")
            std::string answer_text;
            if (!std::getline(ss, line)) throw std::runtime_error("Failed to read answer line for question " + std::to_string(i+1));
            answer_text = extract_value(line); // Value is after the last tab
            std::cerr << "Parsed answer_text(" << i+1 << "): " << answer_text << " (Note: Not currently stored in DB)" << std::endl;

            questions_data.push_back({question_text, options_text}); // Storing only question and options for now
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Python script output: " << e.what() << std::endl;
        std::cerr << "Script output was:\n" << script_result.output << std::endl;
        // No temporary file to remove
        std::cout << "Exiting add_quiz with return code: -7" << std::endl;
        return -7; // Indicate parsing error
    }

    // Now, perform database operations with parsed data
    std::cout << "Starting database transaction to insert quiz data..." << std::endl;
    try {
        pqxx::work txn(conn);

        // 1. Insert into quiz table
        std::string insert_quiz_query = R"(
            INSERT INTO quiz (quiz_name, created_at, belongs_to)
            VALUES ($1, NOW(), $2)
            RETURNING id
        )";
        std::cerr << "Executing query: " << insert_quiz_query << " with quiz_name=" << quiz_name << " and user_id=" << user_id << std::endl;
        pqxx::row quiz_result = txn.exec_params1(insert_quiz_query, quiz_name, user_id);
        int64_t new_quiz_id = quiz_result[0].as<int64_t>();
        std::cerr << "Inserted quiz, new quiz ID: " << new_quiz_id << std::endl;

        // 2. Insert into quiz_content and quiz_answer_content
        std::string insert_question_query = R"(
            INSERT INTO quiz_content (quiz_id, question)
            VALUES ($1, $2)
            RETURNING id
        )";
        std::string insert_option_query = R"(
            INSERT INTO quiz_answer_content (content, question_id)
            VALUES ($1, $2)
        )";

        for (const auto& q_data : questions_data) {
            const std::string& question_text = q_data.first;
            const std::vector<std::string>& options = q_data.second;

            std::cerr << "Inserting question: " << question_text << " for quiz ID: " << new_quiz_id << std::endl;
            // Insert question
            pqxx::row question_result = txn.exec_params1(insert_question_query, new_quiz_id, question_text);
            int64_t new_question_id = question_result[0].as<int64_t>();
            std::cerr << "Inserted question, new question ID: " << new_question_id << std::endl;

            // Insert options
            for (const auto& option_text : options) {
                std::cerr << "Inserting option: " << option_text << " for question ID: " << new_question_id << std::endl;
                txn.exec_params(insert_option_query, option_text, new_question_id);
            }
        }

        txn.commit();
        std::cout << "Database transaction committed." << std::endl;
        std::cout << "Quiz '" << quiz_name << "' (ID: " << new_quiz_id << ") added successfully for user " << user_id << "." << std::endl;
        // No temporary file to remove
        std::cout << "Exiting add_quiz with return code: 0" << std::endl;
        return 0; // Success
    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL error in add_quiz (after script): " << e.what() << std::endl;
        std::cerr << "Failed query: " << e.query() << std::endl;
        // No temporary file to remove
        std::cout << "Exiting add_quiz with return code: -1 (SQL error)" << std::endl;
        return -1; // Generic SQL error
    } catch (const std::exception &e) {
        std::cerr << "Error in add_quiz (after script): " << e.what() << std::endl;
        // No temporary file to remove
        std::cout << "Exiting add_quiz with return code: -1 (Exception)" << std::endl;
        return -1; // Return error code
    }
    // The misplaced cleanup and return 0 are removed; the function's closing brace was already correct at line 477.
}
