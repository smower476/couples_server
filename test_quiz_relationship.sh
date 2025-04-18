#!/bin/bash

# Fixed username and password for the test user
USERNAME="testuser"
PASSWORD="testpassword"
BASE_URL="http://129.158.234.85:8080"

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
    "quizName": "Relationship Quiz",
    "quizId": "relationship123",
    "numberOfQuestions": 10,
    "questions": {
        "1": {
            "question": "What is your partner's favorite way to unwind after a long day?",
            "questionOptions": {
                "A": { "optionText": "Reading a book" },
                "B": { "optionText": "Watching a movie" },
                "C": { "optionText": "Taking a bath" },
                "D": { "optionText": "Going for a walk" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "2": {
            "question": "What is one thing your partner is most proud of accomplishing?",
            "questionOptions": {
                "A": { "optionText": "Their career achievements" },
                "B": { "optionText": "Their personal growth" },
                "C": { "optionText": "Their relationships with family and friends" },
                "D": { "optionText": "Their hobbies and interests" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "3": {
            "question": "What is your partner's love language?",
            "questionOptions": {
                "A": { "optionText": "Words of Affirmation" },
                "B": { "optionText": "Acts of Service" },
                "C": { "optionText": "Receiving Gifts" },
                "D": { "optionText": "Quality Time" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "4": {
            "question": "What is your partner's biggest pet peeve?",
            "questionOptions": {
                "A": { "optionText": "Loud chewing" },
                "B": { "optionText": "Being late" },
                "C": { "optionText": "Disorganization" },
                "D": { "optionText": "Interrupting" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "5": {
            "question": "What is your partner's favorite food?",
            "questionOptions": {
                "A": { "optionText": "Pizza" },
                "B": { "optionText": "Sushi" },
                "C": { "optionText": "Pasta" },
                "D": { "optionText": "Tacos" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "6": {
            "question": "What is your partner's dream vacation?",
            "questionOptions": {
                "A": { "optionText": "Tropical beach" },
                "B": { "optionText": "European city" },
                "C": { "optionText": "Mountain retreat" },
                "D": { "optionText": "Adventure travel" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "7": {
            "question": "What is your partner's favorite holiday?",
            "questionOptions": {
                "A": { "optionText": "Christmas" },
                "B": { "optionText": "Thanksgiving" },
                "C": { "optionText": "Halloween" },
                "D": { "optionText": "Valentine's Day" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "8": {
            "question": "What is your partner's favorite movie genre?",
            "questionOptions": {
                "A": { "optionText": "Comedy" },
                "B": { "optionText": "Action" },
                "C": { "optionText": "Romance" },
                "D": { "optionText": "Horror" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "9": {
            "question": "What is your partner's favorite book?",
            "questionOptions": {
                "A": { "optionText": "To Kill a Mockingbird" },
                "B": { "optionText": "Pride and Prejudice" },
                "C": { "optionText": "1984" },
                "D": { "optionText": "The Lord of the Rings" }
            },
            "questionAnswer": "The answer depends on the partner"
        },
        "10": {
            "question": "What is your partner's favorite song?",
            "questionOptions": {
                "A": { "optionText": "Bohemian Rhapsody" },
                "B": { "optionText": "Imagine" },
                "C": { "optionText": "Like a Rolling Stone" },
                "D": { "optionText": "Hey Jude" }
            },
            "questionAnswer": "The answer depends on the partner"
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