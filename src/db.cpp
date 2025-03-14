#include "../include/db.h"
#include <pqxx/pqxx>
#include <iostream>


void create_tables(pqxx::connection &conn) {
    try {
        pqxx::work txn(conn);
        
        std::string create_users_table = R"(CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY, 
            username VARCHAR(50) UNIQUE NOT NULL, 
            password VARCHAR(255) NOT NULL,
            linked_user INT REFERENCES users(id),
            mood_scale INT,
            mood_status CHAR
            ))";

        txn.exec(create_users_table);
        txn.commit();
    } catch (const std::exception &e) {
        std::cerr << "Error: cannot create users table" << e.what() << std::endl;
    }
}

