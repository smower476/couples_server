#include "../include/routes.h"
#include "../include/token.h"
#include "../include/db.h"
#include "../include/regex.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

//  Add User
std::shared_ptr<http_response> add_user_resource::render(const http_request& req) {
    std::string username = req.get_arg("username");
    std::string password = req.get_arg("password");
    if (!is_valid_login(username) || !is_valid_password(password)){
        return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
    }

    if (add_user(username, password) == 0) {
        return std::make_shared<string_response>("User created successfully", 201, "text/plain");
    }

    return std::make_shared<string_response>("User creation failed", 400, "text/plain");
}

// Login (returns JWT)
std::shared_ptr<http_response> login_resource::render(const http_request& req) {
    std::string username = req.get_arg("username");
    std::string password = req.get_arg("password");
    
    if (!is_valid_login(username) || !is_valid_password(password)){
        return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
    }

    if (validate_user(username, password)) {
        std::string token = create_jwt(username);
        return std::make_shared<string_response>(token, 200, "application/json");
    }

    return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
} 

std::shared_ptr<http_response> set_user_info_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    std::string mood_scale = req.get_arg("mood_scale");
    std::string mood_status = req.get_arg("mood_status");

    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    
    if (!mood_scale.empty()) {
        try {
            set_mood_scale(id, std::stol(mood_scale));
        }  catch (const pqxx::sql_error &e) {
            return std::make_shared<string_response>(e.what(), 500, "text/plain");
        } catch (const std::exception &e) {   
           return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
        }
    }
    
    if (!mood_status.empty()) {
        try {
            set_mood_status(id, mood_status);
        }  catch (const pqxx::sql_error &e) {
            return std::make_shared<string_response>(e.what(), 500, "text/plain");
        } catch (const std::exception &e) {   
           return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
        }
    }

    if (!mood_status.empty() && !mood_scale.empty()) return std::make_shared<string_response>("No changes", 409, "text/plain");



    return std::make_shared<string_response>("Success", 200, "text/plain");
} 

std::shared_ptr<http_response> get_user_info_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");

    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");

    std::string user_info;
    try {
        user_info = get_info(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }

    return std::make_shared<string_response>(user_info, 200, "text/plain");
}

std::shared_ptr<http_response> get_partner_info_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");

    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");

    std::string partner_info;
    try {
        partner_info = get_partner_info(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }

    return std::make_shared<string_response>(partner_info, 200, "text/plain");
}


std::shared_ptr<http_response> get_link_code_resource::render(const http_request& req) {
    std::string username = req.get_arg("token");
    int64_t id = get_user_id(username);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    int code = generate_link_code(id); 
    if (code == -3) return std::make_shared<string_response>("User have been already linked", 409, "text/plain");
    if ( 100000 > code || code > 999999 || id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    return std::make_shared<string_response>(std::to_string(code), 200, "text/plain");
}

std::shared_ptr<http_response> link_users_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    std::string link_code = req.get_arg("link_code");

    int response = link_user(std::stoll(link_code), jwt);

    if (response == -3) return std::make_shared<string_response>("Invalid link code", 404, "text/plain");
    if (response == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (response == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    return std::make_shared<string_response>("Success", 200, "text/plain");
}

std::shared_ptr<http_response> unlink_users_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("invalid jwt token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("internal server error", 500, "text/plain");
    try {
        unlink_users(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("internal server error", 500, "text/plain");
    }
    return std::make_shared<string_response>("Success", 200, "text/plain");
}

std::shared_ptr<http_response> get_daily_quiz_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");

    std::string daily_quiz = get_daily_quiz( id);
    
    if (daily_quiz.empty()) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");


    return std::make_shared<string_response>(daily_quiz, 200, "text/plain");
}

std::shared_ptr<http_response> get_quiz_content_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    std::string quiz_id = req.get_arg("quiz_id");

    int64_t user_id = get_user_id(jwt);
    if (user_id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (user_id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string quiz_content;
    try {
        quiz_content = get_quiz_content(std::stoll(quiz_id), user_id);
        if (quiz_content.empty()) {
                return std::make_shared<string_response>(
                    R"({"error": "Quiz content not found"})", 404, "application/json");
        }
    } catch (const std::exception& e) {
            std::cerr << "Error fetching quiz content: " << e.what() << std::endl;
            
            return std::make_shared<string_response>(
                R"({"error": "Failed to retrieve quiz content"})",
                500,  // Internal Server Error
                "application/json"
            );}
    return std::make_shared<string_response>(quiz_content, 200, "text/plain");
}

std::shared_ptr<http_response> answer_quiz_resource::render(const http_request& req) {
    int64_t answer;
    int64_t quiz_id;
    quiz_id = std::stoll(req.get_arg("quiz_id"));
    answer = std::stoll(req.get_arg("answer"));
    std::string jwt = req.get_arg("token");
    int64_t user_id = get_user_id(jwt);     
    if (user_id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (user_id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    try {
        answer_quiz(quiz_id, user_id, answer);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>("Success", 200, "text/plain");
}

std::shared_ptr<http_response> get_quiz_user_answer_resource::render(const http_request& req) {
    int64_t quiz_id;
    quiz_id = std::stoll(req.get_arg("quiz_id"));
    std::string jwt = req.get_arg("token");
    int64_t user_id = get_user_id(jwt);     

    if (user_id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (user_id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");

    std::string user_answer_json;
    try {
        user_answer_json = get_user_quiz_answer(quiz_id, user_id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }

    return std::make_shared<string_response>(user_answer_json, 200, "text/plain");
}

std::shared_ptr<http_response> get_answered_quizes_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string answered_quizes;
    try {
        answered_quizes = get_answered_quizes(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(answered_quizes, 200, "text/plain");
}

std::shared_ptr<http_response> get_unanswered_quizes_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string unanswered_quizes;
    try {
        unanswered_quizes = get_unanswered_quizes(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(unanswered_quizes, 200, "text/plain");
}

std::shared_ptr<http_response> get_unanswered_quizzes_for_pair_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string unanswered_quizes;
    try {
        unanswered_quizes = get_unanswered_quizes(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(unanswered_quizes, 200, "text/plain");
}

std::shared_ptr<http_response> answer_daily_question_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t question_id = std::stoll(req.get_arg("question_id"));
    std::string answer = req.get_arg("answer");
    if (answer.empty()) return std::make_shared<string_response>("Invalid answer", 401, "text/plain");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    

    try {
        answer_daily_question(id, question_id, answer);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>("Success", 200, "text/plain");
}

std::shared_ptr<http_response> get_unanswered_questions_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string unanswered_questions;
    try {
        unanswered_questions = get_unanswered_questions_for_pair(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(unanswered_questions, 200, "text/plain");
}

std::shared_ptr<http_response> get_daily_question_answer_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t daily_question_id = std::stoll(req.get_arg("daily_question_id"));

    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string daily_question_answer;
    try {
        daily_question_answer = get_daily_question_answer(id, daily_question_id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(daily_question_answer, 200, "text/plain");
}

std::shared_ptr<http_response> get_answered_questions_resource::render(const http_request& req) {
    std::string jwt = req.get_arg("token");
    int64_t id = get_user_id(jwt);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (id == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    std::string answered_questions;
    try {
        answered_questions = get_answered_questions_for_pair(id);
    }  catch (const pqxx::sql_error &e) {
        return std::make_shared<string_response>(e.what(), 500, "text/plain");
    } catch (const std::exception &e) {   
       return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    }
    return std::make_shared<string_response>(answered_questions, 200, "text/plain");
}


/*
// Validate JWT Token
std::shared_ptr<http_response> validate_resource::render(const http_request& req) {
    std::string auth_header = std::string(req.get_header("Authorization"));

    if (auth_header.find("Bearer ") == 0) {
        std::string token = auth_header.substr(7); // Remove "Bearer "

        std::string username;
        if (validate_jwt(token, username)) {
            return std::make_shared<string_response>("Token is valid for user: " + username, 200, "text/plain");
        }
    }

    return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
}
*/
