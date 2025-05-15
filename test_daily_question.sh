#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN1=test1test
LOGIN2=test2test
PASSWORD=King22401825

# Create users
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN1&password=$PASSWORD"
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN2&password=$PASSWORD"
printf "\n\n"

# Get JWT token
JWT1=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN1&password=$PASSWORD")
JWT2=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN2&password=$PASSWORD")
echo $JWT1
printf "\n\n"
echo $JWT2
printf "\n\n"

# Get link code
LINK_CODE=$(curl -X POST http://$ADDRESS/get-link-code -d "token=$JWT1")
echo $LINK_CODE
printf "\n\n"

# Link users
curl -X POST http://$ADDRESS/link-users -d "token=$JWT2&link_code=$LINK_CODE"
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
curl -X POST http://$ADDRESS/answer-daily-question -d "token=$JWT1&question_id=1&answer=answer2"
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
printf "Get unanswered questions\n"
curl -X POST http://$ADDRESS/get-unanswered-questions -d "token=$JWT1"
printf "\n\n"

# Get unanswered questionzes
# printf "Get unanswered questionzes for pair\n"
# curl -X POST http://$ADDRESS/get-unanswered-questionzes-for-pair -d "token=$JWT"
# printf "\n\n"


