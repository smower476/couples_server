# C++ Function Documentation

## Database Functions

### create_table
```cpp
int create_table(const std::string query);
```
Creates a table in the database using the provided SQL query.

### create_tables
```cpp
void create_tables();
```
Creates all the tables required for the application.

### add_user
```cpp
int add_user(const std::string& username, const std::string& password);
```
Adds a new user to the database.

### verify_password
```cpp
bool verify_password(const std::string& password, const std::string& hashed_password);
```
Verifies that the provided password matches the hashed password stored in the database.

### validate_user
```cpp
bool validate_user(const std::string& username, const std::string& password);
```
Validates that the provided username and password are valid.

### get_user_id
```cpp
std::int64_t get_user_id(const std::string& jwt);
```
Gets the user ID from the provided JWT.

### generate_link_code
```cpp
int64_t generate_link_code(const int64_t id);
```
Generates a link code for the provided user ID.

## Regex Functions

### is_valid_login
```cpp
bool is_valid_login(const std::string& login);
```
Checks if the provided login is valid.

### is_valid_password
```cpp
bool is_valid_password(const std::string& password);
```
Checks if the provided password is valid.

### is_valid_jwt
```cpp
bool is_valid_jwt(const std::string& token);
```
Checks if the provided JWT is valid.

## Token Functions

### create_jwt
```cpp
std::string create_jwt(const std::string& username);
```
Creates a new JWT for the provided username.

### decode_jwt
```cpp
std::string decode_jwt(const std::string& token);
```
Decodes the provided JWT.