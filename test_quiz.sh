#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN=testuser
PASSWORD=testpassword

# # Create users
# curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN&password=$PASSWORD"
# printf "\n"

# Get JWT token
JWT=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN&password=$PASSWORD")
echo $JWT
printf "\n"

# Get most recent available quiz ID
echo "Fetching daily quiz ID..."
# Use -s for silent mode to only output the body (the ID)
QUIZ_ID=$(curl -s -X POST http://$ADDRESS/get-daily-quiz -d "token=$JWT")
echo "Daily Quiz ID: $QUIZ_ID"
printf "\n"

# Get quiz content using the fetched ID
echo "Fetching content for Quiz ID: $QUIZ_ID..."
curl -X POST http://$ADDRESS/get-quiz-content -d "token=$JWT&quiz_id=$QUIZ_ID"
