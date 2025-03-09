#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN=bigbon__5
PASSWORD=King22401825

# Create user
printf "Create user\n"
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN&password=$PASSWORD"
printf "\n\n"

# Get JWT token
printf "Get JWT token\n"
JWT=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN&password=$PASSWORD")
echo $JWT
printf "\n\n"

# Get most recent available qiuz data
printf "Get daily quiz\n"
curl -X POST http://$ADDRESS/get-daily-quiz -d "token=$JWT"
printf "\n\n"

# Get quiz content
printf "Get quiz content\n"
curl -X POST http://$ADDRESS/get-quiz-content -d "token=$JWT&quiz_id=1065019527751499777"
printf "\n\n"

# Set quiz answer
printf "Set quiz answer\n"
curl -X POST http://$ADDRESS/answer-quiz -d "token=$JWT&quiz_id=1065019527751499777&answer=123456789"
printf "\n\n"

# Get quiz user answer
printf "Get quiz user answer\n"
curl -X POST http://$ADDRESS/get-quiz-user-answer -d "token=$JWT&quiz_id=1065019527751499777"
printf "\n\n"

# Get answered quizes
printf "Get answered quizzes\n"
curl -X POST http://$ADDRESS/get-answered-quizes -d "token=$JWT"
printf "\n\n"

#Get unanswered quizzes
printf "Get unanswered quizzes\n"
curl -X POST http://$ADDRESS/get-unanswered-quizes -d "token=$JWT"
printf "\n\n"

#Get unanswered quizzes
printf "Get unanswered quizzes for pair\n"
curl -X POST http://$ADDRESS/get-unanswered-quizzes-for-pair -d "token=$JWT"
printf "\n\n"

