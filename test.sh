#!/bin/bash
ADDRESS=localhost:8080
LOGIN=mmmmmax14
PASSWORD=secretsecret14
NEW_PASSWORD=newsecret14

# Create user
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN&password=$PASSWORD"
printf "\n"

# Get JWT token
JWT=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN&password=$PASSWORD")
echo $JWT
printf "\n"

# Get link code
LINK_CODE=$(curl -X POST http://$ADDRESS/get-link-code -d "token=$JWT")
echo $LINK_CODE
printf "\n"

