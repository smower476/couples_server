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
    get_daily_quiz_resource get_daily_quiz_res;
    get_quiz_content_resource get_quiz_content_res;
    answer_quiz_resource answer_quiz_res;
    get_quiz_user_answer_resource get_quiz_user_answer_res;
    get_answered_quizes_resource get_answered_quizes_res;
    get_unanswered_quizes_resource get_unanswered_quizes_res;
    
    ws.register_resource("/login", &login_res); // Login endpoint
//    ws.register_resource("/validate-token", &validate_res);  // Token validation endpoint
    ws.register_resource("/add-user", &add_user_res);        // User creation endpoint
    ws.register_resource("/get-link-code", &get_link_code_res);        // User creation endpoint
    ws.register_resource("/link-users", &link_users_res);
    ws.register_resource("/get-daily-quiz", &get_daily_quiz_res);
    ws.register_resource("/get-quiz-content", &get_quiz_content_res);
    ws.register_resource("/answer-quiz", &answer_quiz_res);
    ws.register_resource("/get-quiz-user-answer", &get_quiz_user_answer_res);
    ws.register_resource("/get-answered-quizes", &get_answered_quizes_res);
    ws.register_resource("/get-unanswered-quizes", &get_unanswered_quizes_res);
    
    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}


