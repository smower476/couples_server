e# Database Functions

## create_table
```cpp
int create_table(const std::string query);
```

Creates a table in the database using the provided SQL query.

**Parameters:**

*   `query`: The SQL query to create the table.

**Returns:**

*   `1`: If the table was created successfully.
*   `-1`: If there was an SQL error.
*   `-2`: If there was another error.

## create_tables
```cpp
void create_tables();
```

Creates all the tables required for the application.

## add_user
```cpp
int add_user(const std::string& username, const std::string& password);
```

Adds a new user to the database.

**Parameters:**

*   `username`: The username of the new user.
*   `password`: The password of the new user.

**Returns:**

*   `0`: If the user was added successfully.
*   `-1`: If there was an SQL error.
*   `-2`: If there was another error.
*   `-3`: If the username already exists.

## verify_password
```cpp
bool verify_password(const std::string& password, const std::string& hashed_password);
```

Verifies that the provided password matches the hashed password stored in the database.

**Parameters:**

*   `password`: The password to verify.
*   `hashed_password`: The hashed password to compare against.

**Returns:**

*   `true`: If the password is valid.
*   `false`: If the password is invalid.

## validate_user
```cpp
bool validate_user(const std::string& username, const std::string& password);
```

Validates that the provided username and password are valid.

**Parameters:**

*   `username`: The username to validate.
*   `password`: The password to validate.

**Returns:**

*   `true`: If the username and password are valid.
*   `false`: If the username and password are invalid.

## get_user_id
```cpp
std::int64_t get_user_id(const std::string& jwt);
```

Gets the user ID from the provided JWT.

**Parameters:**

*   `jwt`: The JWT to decode.

**Returns:**

*   The user ID.
*   `-1`: If there was an SQL error.
*   `-2`: If there was another error.

## generate_link_code
```cpp
int64_t generate_link_code(const int64_t id);
```

Generates a link code for the provided user ID.

**Parameters:**

*   `id`: The user ID to generate the link code for.

**Returns:**

*   The link code.
*   `-1`: If there was an SQL error.
*   `-2`: If there was another error.

# Regex Functions

## is_valid_login
```cpp
bool is_valid_login(const std::string& login);
```

Checks if the provided login is valid.

**Parameters:**

*   `login`: The login to check.

**Returns:**

*   `true`: If the login is valid.
*   `false`: If the login is invalid.

## is_valid_password
```cpp
bool is_valid_password(const std::string& password);
```

Checks if the provided password is valid.

**Parameters:**

*   `password`: The password to check.

**Returns:**

*   `true`: If the password is valid.
*   `false`: If the password is invalid.

## is_valid_jwt
```cpp
bool is_valid_jwt(const std::string& token);
```

Checks if the provided JWT is valid.

**Parameters:**

*   `token`: The JWT to check.

**Returns:**

*   `true`: If the JWT is valid.
*   `false`: If the JWT is invalid.

# Route Functions

## add_user_resource::render
```cpp
std::shared_ptr<http_response> add_user_resource::render(const http_request& req);
```

Handles the request to add a new user.

**Parameters:**

*   `req`: The HTTP request.

**Returns:**

*   A HTTP response.

## login_resource::render
```cpp
std::shared_ptr<http_response> login_resource::render(const http_request& req);
```

Handles the request to log in a user.

**Parameters:**

*   `req`: The HTTP request.

**Returns:**

*   A HTTP response.

## get_link_code_resource::render
```cpp
std::shared_ptr<http_response> get_link_code_resource::render(const http_request& req);
```

Handles the request to get a link code for a user.

**Parameters:**

*   `req`: The HTTP request.

**Returns:**

*   A HTTP response.

# Token Functions

## create_jwt
```cpp
std::string create_jwt(const std::string& username);
```

Creates a new JWT for the provided username.

**Parameters:**

*   `username`: The username to create the JWT for.

**Returns:**

*   The JWT.

## decode_jwt
```cpp
std::string decode_jwt(const std::string& token);
```

Decodes the provided JWT.

**Parameters:**

*   `token`: The JWT to decode.

**Returns:**

*   The username.

# HTTP Request Documentation

## Add User

*   **Endpoint:** `/add_user`
*   **Method:** `POST`
*   **Parameters:**
    *   `username`: The username of the new user (3-20 alphanumeric characters)
    *   `password`: The password of the new user (8-32 characters, special chars allowed)
*   **Responses:**
    *   `201 Created`: "User created successfully"
    *   `400 Bad Request`: "User creation failed"
    *   `401 Unauthorized`: "Invalid username or password"

## Login

*   **Endpoint:** `/login`
*   **Method:** `POST`
*   **Parameters:**
    *   `username`: The username of the user
    *   `password`: The password of the user
*   **Responses:**
    *   `200 OK`: Returns a JWT token (application/json)
    *   `401 Unauthorized`: "Invalid username or password"

## Get Link Code

*   **Endpoint:** `/get_link_code`
*   **Method:** `GET`
*   **Parameters:**
    *   `token`: Valid JWT token
*   **Responses:**
    *   `200 OK`: Returns 6-digit link code (text/plain)
