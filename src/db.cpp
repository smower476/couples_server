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

int add_user(const std::string& username, const std::string& password) {
    try {
        std::string hashed_password = hash_password(password);
        pqxx::work txn(conn);
        std::string query = "INSERT INTO users (username, password) VALUES ($1, $2)";
        
        std::cout << "Executing query: " << query << " with username: " << username << std::endl;
        
        txn.exec_params(query, username, hashed_password);
        txn.commit();
        
        std::cout << "User added successfully." << std::endl;
        return 0; 
    } catch (const pqxx::sql_error &e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Failed query: " << e.query() << std::endl;

        // Check for duplicate key violation (error code 23505)
        if (std::string(e.what()).find("23505") != std::string::npos) {
            std::cerr << "Error: Username already exists." << std::endl;
            return -3; 
        }

        return -1; 
    } catch (const std::exception &e) {
        std::cerr << "Error: failed to process DB request: " << e.what() << std::endl;
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
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw std::runtime_error("An error occurred during user validation");
    }
}
