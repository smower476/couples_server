#include "../include/db.h"
#include <pqxx/pqxx>
#include <iostream>

void db_request(pqxx::connection &conn, const std::string content) {
    try {
        pqxx::work txn(conn);
        
        txn.exec(content);
        txn.commit();
    } catch (const std::exception &e) {
        std::cerr << "Error: cannot process request" << e.what() << std::endl;
    }
}


void create_tables(pqxx::connection &conn) {
    db_request(conn, tables::create_users_table);
    db_request(conn, tables::create_token_table);
    db_request(conn, tables::create_quizz_table);
    db_request(conn, tables::create_quizz_user_answer_table);
    db_request(conn, tables::create_quizz_content_table);
    db_request(conn, tables::create_quizz_answer_content_table);
}
