#include <iostream>
#include "include/routes.h"
#include "include/db.h"
#include <httpserver.hpp>
#include <pqxx/pqxx>
#include <csignal> // Use <csignal> for C++ style

#define PORT 8080

using namespace httpserver;

// Signal handler function
void signal_handler(int signum) {
   std::cout << "\nstopping server.." << std::endl;
   // Perform any necessary cleanup here if needed (e.g., closing DB connections gracefully)
   // The webserver might have a stop method, but exit() provides immediate termination.
   exit(signum);
}


int main() {
    create_tables();
    signal(SIGINT, signal_handler); // Register Ctrl+C handler
    
    
    webserver ws = create_webserver(PORT);
//    validate_resource validate_res;
    add_user_resource add_user_res;
    login_resource login_res;
    get_link_code_resource get_link_code_res;
    link_users_resource link_users_res;
    get_daily_quiz_resource get_daily_quiz_res;
    get_quiz_content_resource get_quiz_content_res;
    add_quiz_resource add_quiz_res; // Instantiate the new resource

    ws.register_resource("/login", &login_res); // Login endpoint
//    ws.register_resource("/validate-token", &validate_res);  // Token validation endpoint
    ws.register_resource("/add-user", &add_user_res);        // User creation endpoint
    ws.register_resource("/get-link-code", &get_link_code_res);        // User creation endpoint
    ws.register_resource("/link-users", &link_users_res);
    ws.register_resource("/get-daily-quiz", &get_daily_quiz_res);
    ws.register_resource("/get-quiz-content", &get_quiz_content_res);
    ws.register_resource("/add-quiz", &add_quiz_res); // Register the new endpoint
    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}


