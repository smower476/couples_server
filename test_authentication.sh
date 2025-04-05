#!/bin/bash

#ADDRESS=129.18.234.85:8081
ADDRESS=localhost:8080
LOGIN=mmmmmax1
PASSWORD=secretsecret1
NEW_PASSWORD=newsecret1

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

