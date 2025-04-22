#include "../include/db.h"
#include "../include/token.h"
#include <cstdint>
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>  
#include <cstdlib>


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
            AND $2 != (SELECT id FROM token_owner)
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

void answer_quiz(const int64_t quiz_id, const int64_t user_id, const int64_t answer){
    try {
        
        pqxx::work txn(conn);
        std::string query = R"( 
        WITH valid_quiz AS (
            SELECT id FROM quiz WHERE id = $1 AND (belongs_to = $2 OR belongs_to = 0)
        )
        INSERT INTO quiz_user_answer (quiz_answer, user_id, quiz_id, answered_at)
        SELECT $3, $2, id, NOW() FROM valid_quiz
        ON CONFLICT (user_id, quiz_id)
        DO UPDATE SET quiz_answer = EXCLUDED.quiz_answer, answered_at = EXCLUDED.answered_at
        RETURNING 1
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " quiz_id " << quiz_id << " answer " << answer << std::endl;
        pqxx::result res = txn.exec_params(query, quiz_id, user_id, answer);
        if (res.empty()) throw std::runtime_error("No valid quiz found, or user doesn't have permission to answer.");
        txn.commit();
    
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}
