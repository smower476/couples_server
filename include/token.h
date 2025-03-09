#ifndef TOKEN_H
#define TOKEN_H

#include <string>

std::string create_jwt(const std::string& username);

// bool validate_jwt(const std::string& token, std::string& username_out);

std::string decode_jwt(const std::string& token);
#endif
