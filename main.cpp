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
    create_tables();
    // signal(SIGINT, signal_callback_handler); // Ctrl + C handler
    
    
    webserver ws = create_webserver(PORT);
//    validate_resource validate_res;
    add_user_resource add_user_res;
    login_resource login_res;
    get_link_code_resource get_link_code_res;
    link_users_resource link_users_res;


    ws.register_resource("/login", &login_res); // Login endpoint
//    ws.register_resource("/validate-token", &validate_res);  // Token validation endpoint
    ws.register_resource("/add-user", &add_user_res);        // User creation endpoint
    ws.register_resource("/get-link-code", &get_link_code_res);        // User creation endpoint
    ws.register_resource("/link_users", &link_users_res);
    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}


