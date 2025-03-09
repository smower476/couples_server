#include <iostream>
#include "include/routes.h"
#include <httpserver.hpp>
#include <signal.h>
//#include <pqxx/pqxx>

#define PORT 8080

using namespace httpserver;

void signal_callback_handler(int signum) {
   // db_close
   std::cout << "\n" << "DB Closed " << "\n";
	
   exit(signum);
}

int main() {
    signal(SIGINT, signal_callback_handler); // Ctrl + C handler

    webserver ws = create_webserver(PORT);

    login_resource login_res;
    ws.register_resource("/login", &login_res);              // Login endpoint


    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}


