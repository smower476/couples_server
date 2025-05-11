#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN=test1test
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
# printf "Get daily question\n"
# curl -X POST http://$ADDRESS/get-daily-question -d "token=$JWT"
# printf "\n\n"

# Get question content
# printf "Get question content\n"
# curl -X POST http://$ADDRESS/get-question-content -d "token=$JWT&quiz_id=1065019527751499777"
# printf "\n\n"

# Set question answer
printf "Set question answer\n"
curl -X POST http://$ADDRESS/answer-daily-question -d "token=$JWT&question_id=1&answer=answer"
printf "\n\n"

# Get question user answer
# printf "Get question user answer\n"
# curl -X POST http://$ADDRESS/get-question-user-answer -d "token=$JWT&quiz_id=1065019527751499777"
# printf "\n\n"

# Get answered questiones
# printf "Get answered questionzes\n"
# curl -X POST http://$ADDRESS/get-answered-questiones -d "token=$JWT"
# printf "\n\n"

# Get unanswered questionzes
# printf "Get unanswered questionzes\n"
# curl -X POST http://$ADDRESS/get-unanswered-questiones -d "token=$JWT"
# printf "\n\n"

# Get unanswered questionzes
# printf "Get unanswered questionzes for pair\n"
# curl -X POST http://$ADDRESS/get-unanswered-questionzes-for-pair -d "token=$JWT"
# printf "\n\n"


