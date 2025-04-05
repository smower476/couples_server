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

    if (response == -2) return std::make_shared<string_response>("Invalid JWT token", 401, "text/plain");
    if (response == -1) return std::make_shared<string_response>("Internal Server Error", 500, "text/plain");
    return std::make_shared<string_response>("Success", 200, "text/plain");
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
