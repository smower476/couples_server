#include <iostream>
#include "include/routes.h"
#include "include/db.h"
#include "include/db_pool.h"
#include <httpserver.hpp>
#include <pqxx/pqxx>

#define PORT 8080

using namespace httpserver;

int main() {
    //const char* pqxx_connection = "dbname=couples_db user=postgres host=localhost port=5432";
    const char* pqxx_connection = std::getenv("PQXX_TEST_CONNECTION"); 
    // const char* pqxx_connection = std::getenv("PQXX_CONNECTION"); 

    if (!pqxx_connection) {
        std::cerr << "Environment variable PQXX_CONNECTION not set" << std::endl;
        return EXIT_FAILURE;
    }

    conn_pool = std::make_shared<ConnectionPool>(pqxx_connection, 4 /* amount of connections */);

    create_tables();
    webserver ws = create_webserver(PORT)
        .start_method(httpserver::http::http_utils::INTERNAL_SELECT)
        .max_connections(100); 

//    validate_resource validate_res;
    add_user_resource add_user_res;
    login_resource login_res;
    get_link_code_resource get_link_code_res;
    link_users_resource link_users_res;
    unlink_users_resource unlink_users_res;
    get_daily_quiz_resource get_daily_quiz_res;
    get_quiz_content_resource get_quiz_content_res;
    answer_quiz_resource answer_quiz_res;
    get_quiz_user_answer_resource get_quiz_user_answer_res;
    get_answered_quizes_resource get_answered_quizes_res;
    get_unanswered_quizes_resource get_unanswered_quizes_res;
    get_unanswered_quizzes_for_pair_resource get_unanswered_quizzes_for_pair_res;
    set_user_info_resource set_user_info_res;
    get_user_info_resource get_user_info_res;
    get_partner_info_resource get_partner_info_res;
    
    answer_daily_question_resource answer_daily_question_res; 
    get_unanswered_questions_resource get_unanswered_questions_res;
    get_daily_question_answer_resource get_daily_question_answer_res;

    ws.register_resource("/login", &login_res); // Login endpoint
//    ws.register_resource("/validate-token", &validate_res);  // Token validation endpoint
    ws.register_resource("/add-user", &add_user_res);        // User creation endpoint
    ws.register_resource("/get-link-code", &get_link_code_res);        // User creation endpoint
    ws.register_resource("/link-users", &link_users_res);
    ws.register_resource("/unlink-users", &unlink_users_res);
    ws.register_resource("/get-daily-quiz", &get_daily_quiz_res);
    ws.register_resource("/get-quiz-content", &get_quiz_content_res);
    ws.register_resource("/answer-quiz", &answer_quiz_res);
    ws.register_resource("/get-quiz-user-answer", &get_quiz_user_answer_res);
    ws.register_resource("/get-answered-quizes", &get_answered_quizes_res);
    ws.register_resource("/get-unanswered-quizes", &get_unanswered_quizes_res);
    ws.register_resource("/get-unanswered-quizzes-for-pair", &get_unanswered_quizzes_for_pair_res);
    ws.register_resource("/set-user-info", &set_user_info_res); 
    ws.register_resource("/get-user-info", &get_user_info_res); 
    ws.register_resource("/get-partner-info", &get_partner_info_res); 
    ws.register_resource("/answer-daily-question", &answer_daily_question_res); 
    ws.register_resource("/get-unanswered-questions", &get_unanswered_questions_res);
    ws.register_resource("/get-daily-question-answer", &get_daily_question_answer_res);

    std::cout << "Server running on http://localhost:" << PORT << "\n";

    ws.start(true);


    return 0;
}
