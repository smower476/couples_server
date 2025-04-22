#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN=testuser
PASSWORD=testpassword

# Create users
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN&password=$PASSWORD"
printf "\n"

# Get JWT token
JWT=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN&password=$PASSWORD")
echo $JWT
printf "\n"

# Get most recent available qiuz data
curl -X POST http://$ADDRESS/get-daily-quiz -d "token=$JWT"
printf "\n"

# Get quiz content
curl -X POST http://$ADDRESS/get-quiz-content -d "token=$JWT&quiz_id=1065019527751499777"
printf "\n"

# Set quiz answer
curl -X POST http://$ADDRESS/answer-quiz -d "token=$JWT&quiz_id=1065019527751499777&answer=123456789"

