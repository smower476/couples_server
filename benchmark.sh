#!/bin/bash

# Configuration
THREADS="2"          # Number of threads (clients)
CONNECTIONS="4"     # Number of connections shared between threads
TIME="3s"            # Duration of the test

#IP_ADDRESS=129.158.234.85:8080
IP_ADDRESS="localhost:8080"

# User credentials
LOGIN1="testtestuser"
PASSWORD="secretsecret"

# Add user
curl -X POST "http://$IP_ADDRESS/add-user" -d "username=$LOGIN1&password=$PASSWORD"

# Login to get JWT
JWT1=$(curl -s -X POST "http://$IP_ADDRESS/login" -d "username=$LOGIN1&password=$PASSWORD")
echo "JWT Token: $JWT1"
printf "\n\n"

# Getting the link code
ADDRESS="http://$IP_ADDRESS/get-link-code?token=$JWT1"

# Running WRK benchmark
wrk -t"$THREADS" -c"$CONNECTIONS" -d"$TIME" "$ADDRESS"
