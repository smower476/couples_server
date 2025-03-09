#include "../include/routes.h"
#include "../include/token.h"

/*
//  Add User
std::shared_ptr<http_response> add_user_resource::render(const http_request& req) {
    std::string username = req.get_arg("username");
    std::string password = req.get_arg("password");
    if (!is_valid_login(username) || !is_valid_password(password)){
        return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
    }

    if (add_user(username, password)) {
        return std::make_shared<string_response>("User created successfully", 201, "text/plain");
    }

    return std::make_shared<string_response>("User creation failed", 400, "text/plain");
}
*/


// Login
std::shared_ptr<http_response> login_resource::render(const http_request& req) {
    std::string username = req.get_arg("username");
    std::string password = req.get_arg("password");
    /*
    if (!is_valid_login(username) || !is_valid_password(password)){
        return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
    }

    if (validate_user(username, password)) {
        std::string token = create_jwt(username);
        return std::make_shared<string_response>(token, 200, "application/json");
    }
*/
    return std::make_shared<string_response>("Invalid username or password", 401, "text/plain");
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

    return std::make_shared<string_response>("Invalid token", 401, "text/plain");
}
*/
