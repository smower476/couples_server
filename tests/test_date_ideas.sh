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
printf "Get link code\n"
LINK_CODE=$(curl -X POST http://$ADDRESS/get-link-code -d "token=$JWT1")
echo $LINK_CODE
printf "\n\n"

# Link users
printf "Link users\n"
curl -X POST http://$ADDRESS/link-users -d "token=$JWT2&link_code=$LINK_CODE"
printf "\n\n"

# Get date ideas
curl -X POST http://$ADDRESS/get-date-ideas -d "token=$JWT1"
printf "\n\n"
