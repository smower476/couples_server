#include <iostream>
#include "include/routes.h"
#include "include/db.h"
#include <httpserver.hpp>
#include <pqxx/pqxx>
//#include <signal.h>

#define PORT 8080

using namespace httpserver;

/*
void signal_callback_handler(int signum) {
   // db_close
    std::cout << "\n" << "DB Closed " << "\n";
    exit(signum);
}
*/

int main() {
    pqxx::connection conn("dbname=couples_db user=postgres host=localhost port=5432");
    create_tables(conn);
    // signal(SIGINT, signal_callback_handler); // Ctrl + C handler
    
    
    webserver ws = create_webserver(PORT);

    login_resource login_res;
    ws.register_resource("/login", &login_res); // Login endpoint


    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}


