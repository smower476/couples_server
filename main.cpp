#include <iostream>
#include <string>
//#include "include/routes.h"
#include <httpserver.hpp>

#define PORT 8080

using namespace httpserver;


int main() {
/*
 if (!open_db("store.db")) {
        std::cerr << "Can't open database!" << std::endl;
        return 1;
    }

    create_tables();
*/
    webserver ws = create_webserver(PORT);

//    login_resource login_res;

//    ws.register_resource("/login", &login_res);              // Login endpoint
    std::cout << "Server running on http://localhost:" << PORT << std::endl;

    ws.start(true);

//    close_db();

    return 0;
}


