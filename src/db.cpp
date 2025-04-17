#include "../include/db.h"
#include "../include/token.h"
#include <cstdint>
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>  
#include <cstdlib>
#include <cstdio> // For popen, pclose, FILE
#include <memory> // For unique_ptr
#include <stdexcept> // For runtime_error
#include <array> // For buffer
#include <sstream> // For stringstream
#include <vector> // For storing parsed data


// Helper function to execute a command and capture its output/error
struct PipeResult {
    std::string output;
    std::string error;
    int exit_code;
};

PipeResult exec_pipe(const std::string& cmd, const std::string& input) {
    PipeResult result;
    result.exit_code = -1; // Default error code

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

// Custom deleter for FILE* from popen - Defined *after* PCLOSE macro
struct PipeCloser {
    void operator()(FILE* pipe) const {
        if (pipe) {
            // PCLOSE is defined by the preprocessor block above
            PCLOSE(pipe);
        }
    }
};

    std::array<char, 128> buffer;
    // Use the custom deleter PipeCloser instead of decltype(&PCLOSE)
    std::unique_ptr<FILE, PipeCloser> pipe(POPEN((cmd + " 2>&1").c_str(), "w+"));

    if (!pipe) {
        result.error = "popen() failed!";
        return result;
    }

    // Write input to the script's stdin
    if (fwrite(input.c_str(), 1, input.size(), pipe.get()) != input.size()) {
         result.error = "Failed to write to pipe stdin";
         // Should probably close pipe here, but unique_ptr handles it
         return result;
    }
    // Indicate end of input for stdin (important!)
    fflush(pipe.get());
     // Close the write end of the pipe to signal EOF to the script
    // This might require platform-specific handling or separate pipes if bidirectional communication is needed.
    // For simple stdin->stdout, closing write handle might be complex with popen.
    // Let's rely on the script reading until EOF.

    // Read the script's output from stdout
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result.output += buffer.data();
    }

    // unique_ptr with custom deleter handles closing the pipe automatically when it goes out of scope.
    // We need to capture the exit code *before* the unique_ptr is destroyed.
    // This is tricky with popen as closing the pipe gives the exit status.
    // Let's capture the exit code after reading is done, but before unique_ptr destructs.
    // Note: Releasing and manually closing might be needed if exit code is strictly required *before* potential exceptions during output processing.
    // However, for this flow, letting unique_ptr manage closure is cleaner.
    // The exit code might be retrieved differently depending on exact needs and error handling strategy.
    // A simple approach is to close manually *after* reading.
    FILE* pipe_ptr = pipe.release(); // Release ownership from unique_ptr
    if (pipe_ptr) {
         result.exit_code = PCLOSE(pipe_ptr);
    } else {
         // If pipe creation failed initially, result.exit_code remains -1
         // If release happened but PCLOSE fails, exit_code might be inaccurate.
         // Consider more robust error checking if PCLOSE failure is critical.
    }


    // Note: A more robust solution might use platform-specific APIs
    // (CreateProcess on Windows, fork/exec/pipe on Linux) for better control
    // over stdin/stdout/stderr redirection and process management.
    // This popen approach is simpler but has limitations.
    // We are also combining stdout and stderr here for simplicity. A real implementation
    // might want to capture them separately.

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
        std::string username = decode_jwt(jwt);
        pqxx::work txn(conn);
        std::string query = "SELECT id FROM users WHERE username = $1;";
        
        std::cout << "executing query: " << query << " with username: " << username << std::endl;
        
        //pqxx::result result = txn.exec_params(query, username);
        pqxx::row result = txn.exec_params1(query, username);
        int64_t id = result[0].as<std::int64_t>();   
        txn.commit();
        std::cout<<"\n USER ID " << id << "\n";
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
int64_t generate_link_code(const int64_t id){
    // INSERT INTO token (user_id, ) VALUES (123456, 789012);
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

std::string get_daily_quiz(const int64_t id){
    try {
        pqxx::work txn(conn);
        std::string query = R"(SELECT json_agg(q) FROM (
            SELECT * FROM quiz 
            WHERE belongs_to = 0 OR belongs_to = $1
            ORDER BY created_at DESC
            LIMIT 1) q
        )";
        
        std::cout << "executing query: " << query << " with id: " << id << std::endl;
        pqxx::result result = txn.exec_params(query, id);

std::string daily_quiz_json = result[0][0].as<std::string>();

        txn.commit();
        std::cout<<"\n daily quiz json " << daily_quiz_json << "\n";
    
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

// Add Quiz Data using Python script for parsing
int add_quiz(int64_t user_id, const std::string& quiz_json_str) {
    std::string command = "python scripts/parse_quiz.py"; // Ensure python is in PATH and script path is correct
    PipeResult script_result = exec_pipe(command, quiz_json_str);

    // Check script execution result
    if (script_result.exit_code != 0) {
        std::cerr << "Python script execution failed. Exit code: " << script_result.exit_code << std::endl;
        std::cerr << "Script output/error: " << script_result.output << std::endl; // Combined output/error
        return -6; // Indicate script error
    }

    // Parse the output from the script
    std::stringstream ss(script_result.output);
    std::string line;
    std::string quiz_name;
    int num_questions = 0;
    std::vector<std::pair<std::string, std::vector<std::string>>> questions_data;

    try {
        // Line 1: Quiz Name
        if (!std::getline(ss, quiz_name)) throw std::runtime_error("Failed to read quiz name");

        // Line 2: Number of Questions
        if (!std::getline(ss, line)) throw std::runtime_error("Failed to read number of questions");
        num_questions = std::stoi(line);

        for (int i = 0; i < num_questions; ++i) {
            std::string question_text;
            int num_options = 0;
            std::vector<std::string> options_text;

            // Question Text
            if (!std::getline(ss, question_text)) throw std::runtime_error("Failed to read question text for question " + std::to_string(i+1));

            // Number of Options
            if (!std::getline(ss, line)) throw std::runtime_error("Failed to read number of options for question " + std::to_string(i+1));
            num_options = std::stoi(line);

            for (int j = 0; j < num_options; ++j) {
                std::string option_text;
                if (!std::getline(ss, option_text)) throw std::runtime_error("Failed to read option text " + std::to_string(j+1) + " for question " + std::to_string(i+1));
                options_text.push_back(option_text);
            }
            questions_data.push_back({question_text, options_text});
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing Python script output: " << e.what() << std::endl;
        std::cerr << "Script output was:\n" << script_result.output << std::endl;
        return -7; // Indicate parsing error
    }

    // Now, perform database operations with parsed data
    try {
        pqxx::work txn(conn);

        // 1. Insert into quiz table
        std::string insert_quiz_query = R"(
            INSERT INTO quiz (quiz_name, created_at, belongs_to)
            VALUES ($1, NOW(), $2)
            RETURNING id
        )";
        pqxx::row quiz_result = txn.exec_params1(insert_quiz_query, quiz_name, user_id);
        int64_t new_quiz_id = quiz_result[0].as<int64_t>();

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

            // Insert question
            pqxx::row question_result = txn.exec_params1(insert_question_query, new_quiz_id, question_text);
            int64_t new_question_id = question_result[0].as<int64_t>();

            // Insert options
            for (const auto& option_text : options) {
                txn.exec_params(insert_option_query, option_text, new_question_id);
            }
        }

        txn.commit();
        std::cout << "Quiz '" << quiz_name << "' (ID: " << new_quiz_id << ") added successfully for user " << user_id << " via Python script." << std::endl;
        return 0; // Success

    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL error in add_quiz (after script): " << e.what() << std::endl;
        std::cerr << "Failed query: " << e.query() << std::endl;
        return -1; // Generic SQL error
    } catch (const std::exception &e) {
        std::cerr << "Error in add_quiz (after script): " << e.what() << std::endl;
        return -2; // Generic internal error
    }
}

