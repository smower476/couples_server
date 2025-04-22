#include "../include/routes.h"
#include "../include/token.h"
#include "../include/db.h"
#include "../include/regex.h"
#include <cstdint>
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


std::shared_ptr<http_response> get_link_code_resource::render(const http_request& req) {
    std::string username = req.get_arg("token");
    int64_t id = get_user_id(username);
    if (id == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    int code = generate_link_code(id);  
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

std::shared_ptr<http_response> get_answer_quiz_resource::render(const http_request& req) {
    int64_t answer;
    int64_t quiz_id;
    quiz_id = std::stoll(req.get_arg("quiz_id"));
    answer = std::stoll(req.get_arg("answer"));
    std::string jwt = req.get_arg("token");
    int64_t user_id = get_user_id(jwt);     
    // TODO: add try/catch
    answer_quiz(quiz_id, user_id, answer);

    return std::make_shared<string_response>("Success", 200, "text/plain");
}

std::shared_ptr<http_response> get_quiz_user_answer_resource::render(const http_request& req) {
    int64_t quiz_id;
    quiz_id = std::stoll(req.get_arg("quiz_id"));
    std::string jwt = req.get_arg("token");
    int64_t user_id = get_user_id(jwt);     
    // TODO: add try/catch
    std::string user_answer_json= get_user_quiz_answer(quiz_id, user_id);

    return std::make_shared<string_response>(user_answer_json, 200, "text/plain");
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
