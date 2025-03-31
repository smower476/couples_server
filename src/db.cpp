#include "../include/db.h"
#include "../include/token.h"
#include <cstdint>
#include <pqxx/pqxx>
#include <iostream>
#include <sodium.h>  
#include <cstdlib>


// pqxx::connection conn("dbname=couples_db user=postgres host=localhost port=5432");
pqxx::connection conn("dbname=defaultdb user= password= host=mackson-5039.jxf.gcp-us-east1.cockroachlabs.cloud port= sslmode=require");

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
        
        std::cout<< "\n sizeof " << sizeof(int) <<  "\n";
        std::srand(std::time(NULL));
        int link_code =100000 + std::rand() % 899999;
        pqxx::work txn(conn);
        std::string query = "INSERT INTO token (user_id, link_token) VALUES ($1, $2)";
        
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
