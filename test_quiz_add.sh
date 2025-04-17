#!/bin/bash

# Fixed username and password for the test user
USERNAME="testuser"
PASSWORD="testpassword"
BASE_URL="http://localhost:8080"

# Create a new user
echo "Creating a new user..."
CREATE_RESPONSE=$(curl -s -X POST "$BASE_URL/add-user?username=$USERNAME&password=$PASSWORD")
echo "Create User Response: $CREATE_RESPONSE"

# Login and get the JWT token
echo "Logging in and getting JWT token..."
# Curl returns the token directly in the body for this endpoint based on the ps1 script
TOKEN=$(curl -s -X GET "$BASE_URL/login?username=$USERNAME&password=$PASSWORD")
echo "Login Response (Token): $TOKEN"

# Check if token is empty
if [ -z "$TOKEN" ]; then
    echo "Error: Failed to retrieve JWT token. Aborting."
    exit 1
fi

echo "JWT Token: $TOKEN"

# The Quiz JSON payload
# Using a heredoc for multiline JSON
read -r -d '' QUIZ_JSON << EOM
{
    "quizName": "Quiz Example",
    "quizId": "12345",
    "numberOfQuestions": 10,
    "questions": {
        "1": {
            "question": "What is the capital of France?",
            "questionOptions": {
                "A": { "optionText": "Berlin" },
                "B": { "optionText": "Madrid" },
                "C": { "optionText": "Paris" },
                "D": { "optionText": "Rome" }
            },
            "questionAnswer": "C"
        },
        "2": {
            "question": "What is the largest planet in our solar system?",
            "questionOptions": {
                "A": { "optionText": "Earth" },
                "B": { "optionText": "Jupiter" },
                "C": { "optionText": "Mars" },
                "D": { "optionText": "Saturn" }
            },
            "questionAnswer": "B"
        },
        "3": {
            "question": "What is the chemical symbol for water?",
            "questionOptions": {
                "A": { "optionText": "H2O" },
                "B": { "optionText": "O2" },
                "C": { "optionText": "CO2" },
                "D": { "optionText": "NaCl" }
            },
            "questionAnswer": "A"
        },
        "4": {
            "question": "Who wrote 'Romeo and Juliet'?",
            "questionOptions": {
                "A": { "optionText": "Charles Dickens" },
                "B": { "optionText": "William Shakespeare" },
                "C": { "optionText": "Mark Twain" },
                "D": { "optionText": "Jane Austen" }
            },
            "questionAnswer": "B"
        },
        "5": {
            "question": "What is the speed of light?",
            "questionOptions": {
                "A": { "optionText": "300,000 km/s" },
                "B": { "optionText": "150,000 km/s" },
                "C": { "optionText": "450,000 km/s" },
                "D": { "optionText": "600,000 km/s" }
            },
            "questionAnswer": "A"
        },
        "6": {
            "question": "What is the smallest prime number?",
            "questionOptions": {
                "A": { "optionText": "1" },
                "B": { "optionText": "2" },
                "C": { "optionText": "3" },
                "D": { "optionText": "5" }
            },
            "questionAnswer": "B"
        },
        "7": {
            "question": "What is the boiling point of water at sea level?",
            "questionOptions": {
                "A": { "optionText": "90°C" },
                "B": { "optionText": "100°C" },
                "C": { "optionText": "110°C" },
                "D": { "optionText": "120°C" }
            },
            "questionAnswer": "B"
        },
        "8": {
            "question": "Who painted the Mona Lisa?",
            "questionOptions": {
                "A": { "optionText": "Vincent van Gogh" },
                "B": { "optionText": "Pablo Picasso" },
                "C": { "optionText": "Leonardo da Vinci" },
                "D": { "optionText": "Claude Monet" }
            },
            "questionAnswer": "C"
        },
        "9": {
            "question": "What is the square root of 64?",
            "questionOptions": {
                "A": { "optionText": "6" },
                "B": { "optionText": "7" },
                "C": { "optionText": "8" },
                "D": { "optionText": "9" }
            },
            "questionAnswer": "C"
        },
        "10": {
            "question": "What is the largest ocean on Earth?",
            "questionOptions": {
                "A": { "optionText": "Atlantic Ocean" },
                "B": { "optionText": "Indian Ocean" },
                "C": { "optionText": "Arctic Ocean" },
                "D": { "optionText": "Pacific Ocean" }
            },
            "questionAnswer": "D"
        }
    }
}
EOM

# Replace the degree symbol using sed (matches the ps1 script's logic)
# Note: The specific character 'Â°' might depend on file encoding issues.
# Using the standard degree symbol '°' directly in the heredoc is often safer.
# However, to match the ps1 exactly, we perform the replacement.
PROCESSED_QUIZ_JSON=$(echo "$QUIZ_JSON" | sed 's/Â°/°/g')

echo "Attempting to add quiz..."

# Send the POST request using curl
ADD_RESPONSE=$(curl -s -X POST "$BASE_URL/add-quiz?token=$TOKEN" \
     -H "Content-Type: application/json" \
     -d "$PROCESSED_QUIZ_JSON")

echo # Add a newline for cleaner output
echo "Add Quiz Response: $ADD_RESPONSE"
echo "Request sent."