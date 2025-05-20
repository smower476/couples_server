#!/bin/bash

#ADDRESS=129.158.234.85:8080
ADDRESS=localhost:8080
LOGIN2=22testuser1111_6
LOGIN1=11testuser22_6
PASSWORD=secretsecret1

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

# Unlink users
curl -X POST http://$ADDRESS/unlink-users -d "token=$JWT2"
printf "\n\n"

