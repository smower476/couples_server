#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN1=testuser1_6
LOGIN2=testuser2_6
PASSWORD=secretsecret1

# Create users
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN1&password=$PASSWORD"
curl -X POST http://$ADDRESS/add-user -d "username=$LOGIN2&password=$PASSWORD"
printf "\n"

# Get JWT token
JWT1=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN1&password=$PASSWORD")
JWT2=$(curl -X POST http://$ADDRESS/login -d "username=$LOGIN2&password=$PASSWORD")
echo $JWT1
printf "\n"
echo $JWT2
printf "\n"

# Get link code
LINK_CODE=$(curl -X POST http://$ADDRESS/get-link-code -d "token=$JWT1")
echo $LINK_CODE
printf "\n"

# Link users
curl -X POST http://$ADDRESS/link-users -d "token=$JWT2&link_code=$LINK_CODE"
