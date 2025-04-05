# API Documentation

## HTTP Endpoints

### Add User
* **Endpoint:** `/add-user`
* **Method:** `POST`
* **Parameters:**
  * `username`: The username of the new user (3-20 alphanumeric characters)
  * `password`: The password of the new user (8-32 characters, special chars allowed)
* **Responses:**
  * `201 Created`: "User created successfully"
  * `400 Bad Request`: "User creation failed"
  * `401 Unauthorized`: "Invalid username or password"

### Login
* **Endpoint:** `/login`
* **Method:** `POST`
* **Parameters:**
  * `username`: The username of the user
  * `password`: The password of the user
* **Responses:**
  * `200 OK`: Returns a JWT token (application/json)
  * `401 Unauthorized`: "Invalid username or password"

### Get Link Code
* **Endpoint:** `/get-link-code`
* **Method:** `GET`
* **Parameters:**
  * `token`: Valid JWT token
* **Responses:**
  * `200 OK`: Returns 6-digit link code (text/plain)
  * `500 Internal Server Error`: Internal Server Error
  * `401 Unauthorized`: "Invalid token"
