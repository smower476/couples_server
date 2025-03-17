#include "../include/db.h"
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>  

pqxx::connection conn("dbname=couples_db user=postgres host=localhost port=5432");

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
    create_table(tables::create_quizz_table);
    create_table(tables::create_quizz_user_answer_table);
    create_table(tables::create_quizz_content_table);
    create_table(tables::create_quizz_answer_content_table);
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


int add_user(const std::string& username, const std::string& password){
    try {
        std::string hashed_password = hash_password(password);
        pqxx::work txn(conn);
        std::string query = "INSERT INTO users (username, password) VALUES ($1, $2)";
        
        txn.exec_params(query, username, hashed_password);
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

