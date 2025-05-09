#include "../include/db.h"
#include "../include/token.h"
#include "../include/db_pool.h"
#include <cstdint>
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>  
#include <cstdlib>
#include <string>


//const char * pqxx_connection = std::getenv("PQXX_CONNECTION");
//pqxx::connection conn(pqxx_connection);
// pqxx::connection conn("dbname=couples_db user=postgres host=localhost port=5432");

std::shared_ptr<ConnectionPool> conn_pool;

int create_table(const std::string query) {
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());
        

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
    create_table(tables::create_mood_enum);
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
void set_mood_status(const int64_t user_id, std::string status){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());       
        
        std::string query = R"(
            UPDATE users SET mood_status = $2 WHERE id = $1
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << std::endl;
        txn.exec_params(query, user_id, status);
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

void set_mood_scale(const int64_t user_id, int mood_scale){
    try {
          ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());      
        
        std::string query = R"( 
        UPDATE users
        SET mood_scale = $2
        WHERE id = $1;
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " mood scale: " << mood_scale <<std::endl;
        txn.exec_params(query, user_id, mood_scale);
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

std::string get_info(const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());     
        
        std::string query = R"(       
        SELECT json_build_object(
            'id', id,
            'username', username,
            'linked_user', linked_user,
            'mood_scale', mood_scale,
            'mood_status', mood_status
        )
        FROM users
        WHERE id = $1;
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " mood scale: " <<std::endl;
        pqxx::result result = txn.exec_params(query, user_id);
        txn.commit();
        

        if (result.empty()) {
            return "{\"error\": \"No linked user found\"}";
        }

        return result[0][0].as<std::string>();
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

std::string get_partner_info(const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());       
        
        std::string query = R"(SELECT json_build_object(
            'id', id,
            'username', username,
            'linked_user', linked_user,
            'mood_scale', mood_scale,
            'mood_status', mood_status
        )
        FROM users
        WHERE id = (SELECT linked_user FROM users WHERE id = $1);
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " mood scale: " <<std::endl;
        pqxx::result result = txn.exec_params(query, user_id);
        
        
        txn.commit();

        return result[0][0].as<std::string>();
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

int add_user(const std::string& username, const std::string& password) {
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());
        std::string hashed_password = hash_password(password);
        
        
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
         ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());       
        

        const std::string query = "SELECT password FROM users WHERE username = $1;";
        pqxx::result result = txn.exec_params(query, username);
        txn.commit();
        

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
         ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());       

        
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
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());
        std::srand(std::time(NULL));
        int link_code = 100000 + std::rand() % 899999;

        
        

        std::string query = R"(
            WITH user_check AS (
                SELECT id FROM users WHERE id = $1 AND linked_user IS NULL
            )
            INSERT INTO token (user_id, link_token, expired_at)
            SELECT id, $2, NOW() + INTERVAL '10 minutes'
            FROM user_check
            ON CONFLICT (user_id)
            DO UPDATE SET link_token = $2, expired_at = NOW() + INTERVAL '10 minutes'
            RETURNING user_id
        )";

        std::cout << "executing query with id: " << id << " link code: " << link_code << std::endl;

        pqxx::result result = txn.exec_params(query, id, link_code);
        txn.commit();
        
        
        if (result.empty()) {
            std::cerr << "error: user does not exist or is already linked." << std::endl;
            return -3;
        }

        std::cout << "link code generated successfully." << std::endl;
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
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());
        int64_t id = get_user_id(jwt);
        if (id < 0) return id;

        
        
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

void unlink_users(const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        

        
        std::string query = R"( 
        UPDATE users SET linked_user = NULL WHERE id = $1 OR id = (SELECT linked_user FROM users WHERE id = $1);
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << std::endl;
        txn.exec_params(query, user_id);
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


std::string get_daily_quiz(const int64_t id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        

        
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
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        
        // quiz_id , user_id (jwt)
        if (user_id < 0) throw std::runtime_error("Invalid JWT or user not found");;
        
        
        
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
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        
        
        
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

std::string get_user_quiz_answer(const int64_t quiz_id, const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        
        
        
        std::string query = R"( 
        WITH user_info AS (
            SELECT id AS self_id, linked_user
            FROM users
            WHERE id = $1
        ),
        quiz_info AS (
            SELECT q.id AS quiz_id, q.belongs_to, u.self_id, u.linked_user
            FROM quiz q
            JOIN user_info u ON q.id = $2
            WHERE q.belongs_to = 0 OR q.belongs_to = u.self_id OR q.belongs_to = u.linked_user
        ),
        self_answer AS (
            SELECT qua.id, qua.quiz_answer, qua.answered_at
            FROM quiz_user_answer qua
            JOIN user_info u ON qua.user_id = u.self_id
            WHERE qua.quiz_id = $2
        ),
        linked_answer AS (
            SELECT qua.id, qua.quiz_answer, qua.answered_at
            FROM quiz_user_answer qua
            JOIN user_info u ON qua.user_id = u.linked_user
            WHERE u.linked_user IS NOT NULL AND u.linked_user != u.self_id AND qua.quiz_id = $2
        )
        SELECT json_build_object(
            'self', json_build_object(
                'id', sa.id,
                'quiz_answer', sa.quiz_answer,
                'answered_at', sa.answered_at
            ),
            'linked', CASE 
                WHEN la.id IS NOT NULL THEN json_build_object(
                    'id', la.id,
                    'quiz_answer', la.quiz_answer,
                    'answered_at', la.answered_at
                )
                ELSE NULL
            END
        ) AS result
        FROM quiz_info qi
        LEFT JOIN self_answer sa ON TRUE
        LEFT JOIN linked_answer la ON TRUE;
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << " quiz_id " << quiz_id << std::endl;
        pqxx::result result = txn.exec_params(query, user_id, quiz_id);
        if (result.empty()) throw std::runtime_error("No valid quiz found, or user doesn't have permission to answer.");
        txn.commit();
        

        std::string user_answer_json = result[0][0].as<std::string>();

        return user_answer_json;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

std::string get_unanswered_quizes(const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        
        
        
        std::string query = R"( 
        SELECT COALESCE(json_agg(q_item), '[]') AS quizzes
          FROM (
            SELECT
              q.id,
              q.quiz_name,
              q.created_at,
              q.belongs_to
            FROM quiz q
            LEFT JOIN quiz_user_answer ua
              ON ua.quiz_id = q.id
             AND ua.user_id = $1
            WHERE (q.belongs_to = 0 OR q.belongs_to = $1)
              AND ua.id IS NULL
            ORDER BY q.created_at DESC
          ) AS q_item
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << std::endl;
        pqxx::result result = txn.exec_params(query, user_id);
        txn.commit();
        

        std::string user_answer_json = result[0][0].as<std::string>();

        return user_answer_json;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

std::string get_answered_quizes(const int64_t user_id){
    try {
        
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());
        
        std::string query = R"( 
WITH user_pair AS (
            SELECT 
                $1 AS user_id,
                linked_user AS partner_id
            FROM users
            WHERE id = $1
        ),
        user_answers AS (
            SELECT
                q.id,
                q.quiz_name,
                q.created_at,
                q.belongs_to,
                ua_self.quiz_answer AS self_answer,
                ua_self.answered_at AS self_answered_at,
                ua_partner.quiz_answer AS partner_answer,
                ua_partner.answered_at AS partner_answered_at
            FROM quiz q
            JOIN user_pair up ON TRUE
            JOIN quiz_user_answer ua_self 
                ON ua_self.quiz_id = q.id AND ua_self.user_id = up.user_id
            LEFT JOIN quiz_user_answer ua_partner
                ON ua_partner.quiz_id = q.id AND ua_partner.user_id = up.partner_id
            WHERE (q.belongs_to = 0 OR q.belongs_to = up.user_id OR q.belongs_to = up.partner_id)
        )
        SELECT COALESCE(json_agg(
            json_build_object(
                'id', ua.id,
                'quiz_name', ua.quiz_name,
                'created_at', ua.created_at,
                'belongs_to', ua.belongs_to,
                'self_answer', ua.self_answer,
                'self_answered_at', ua.self_answered_at,
                'partner_answer', ua.partner_answer,
                'partner_answered_at', ua.partner_answered_at
            )
        ), '[]') AS quizzes
        FROM (
            SELECT *
            FROM user_answers
            ORDER BY created_at DESC
        ) ua;   
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << std::endl;
        pqxx::result result = txn.exec_params(query, user_id);
        

        txn.commit();

        std::string user_answer_json = result[0][0].as<std::string>();

        return user_answer_json;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

std::string get_unanswered_quizzes_for_pair(const int64_t user_id){
    try {
        ConnectionHandle handle(*conn_pool);
        pqxx::work txn(*handle.get());        

        
        std::string query = R"( 
            WITH user_pair AS (
            SELECT 
                $1 AS user_id,
                linked_user AS partner_id
            FROM users
            WHERE id = $1
        )
        SELECT COALESCE(json_agg(q_item), '[]') AS quizzes
        FROM (
            SELECT
                q.id,
                q.quiz_name,
                q.created_at,
                q.belongs_to
            FROM quiz q
            JOIN user_pair up ON TRUE
            WHERE (
                q.belongs_to = 0 OR 
                q.belongs_to = up.user_id OR 
                q.belongs_to = up.partner_id
            )
            AND NOT EXISTS (
                SELECT 1
                FROM quiz_user_answer ua1
                JOIN quiz_user_answer ua2 ON ua1.quiz_id = ua2.quiz_id
                WHERE ua1.quiz_id = q.id 
                  AND ua1.user_id = up.user_id
                  AND ua2.user_id = up.partner_id
            )
            AND NOT EXISTS (
                SELECT 1
                FROM quiz_user_answer ua
                WHERE ua.quiz_id = q.id
                  AND ua.user_id IN (up.user_id, up.partner_id)
                  AND ua.answered_at < NOW() - INTERVAL '1 day'
            )
            ORDER BY q.created_at DESC
        ) AS q_item   
        )";

        std::cout << "executing query: " << query << " with user id: " << user_id << std::endl;
        pqxx::result result = txn.exec_params(query, user_id);
        txn.commit();
        

        std::string user_answer_json = result[0][0].as<std::string>();

        return user_answer_json;
    } catch (const pqxx::sql_error &e) {
        std::cerr << "sql error: " << e.what() << std::endl;
        std::cerr << "failed query: " << e.query() << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "error: failed to process db request: " << e.what() << std::endl;
        throw;
    }
}

