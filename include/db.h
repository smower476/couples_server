#ifndef DB_H
#define DB_H
#include <pqxx/pqxx>

namespace tables{
    inline std::string create_users_table = R"(CREATE TABLE IF NOT EXISTS users (
        id SERIAL PRIMARY KEY, 
        username VARCHAR(50) UNIQUE NOT NULL, 
        password VARCHAR(255) NOT NULL,
        linked_user INT REFERENCES users(id),
        mood_scale INT,
        mood_status CHAR
        ))";
    
    inline std::string create_token_table = R"(CREATE TABLE IF NOT EXISTS token (
        id SERIAL PRIMARY KEY,
        user_id INT UNIQUE NOT NULL REFERENCES users(id) ON DELETE CASCADE,
        link_token INT UNIQUE NOT NULL
        ))";        

    inline std::string create_quizz_table = R"(CREATE TABLE IF NOT EXISTS quizz (
        id SERIAL PRIMARY KEY,
        quizz_name VARCHAR(255) NOT NULL,
        created_at TIMESTAMP NOT NULL,
        belongs_to INT NOT NULL
        ))";

    inline std::string create_quizz_user_answer_table = R"(CREATE TABLE IF NOT EXISTS quizz_user_answer (
        id SERIAL PRIMARY KEY,
        quizz_answer BIGINT NOT NULL,
        user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE,
        quizz_id INT NOT NULL REFERENCES quizz(id) ON DELETE CASCADE,
        answered_at TIMESTAMP NOT NULL
        ))";

    inline std::string create_quizz_content_table = R"(CREATE TABLE IF NOT EXISTS quizz_content (
        id SERIAL PRIMARY KEY,
        quizz_id INT NOT NULL REFERENCES quizz(id) ON DELETE CASCADE,
        question VARCHAR(255) NOT NULL 
        ))";

    inline std::string create_quizz_answer_content_table = R"(CREATE TABLE IF NOT EXISTS quizz_answer_content (
        id SERIAL PRIMARY KEY,
        content VARCHAR(50) NOT NULL,
        question_id INT NOT NULL REFERENCES quizz_content(id) ON DELETE CASCADE
        ))";
}
extern pqxx::connection conn;
int create_table(const std::string content);
void create_tables();
int add_user(const std::string& username, const std::string& password);
bool verify_password(const std::string& password, const std::string& hashed_password);
bool validate_user(const std::string& username, const std::string& password);
int get_user_id(const std::string& jwt);
int generate_link_code(const int& id);
#endif

