#ifndef DB_H
#define DB_H
#include <pqxx/pqxx>
void create_tables(pqxx::connection &conn);

#endif

