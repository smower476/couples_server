# test_parse_quiz.ps1
$QUIZ_JSON = @'
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
'@

# Remove newlines and carriage returns from the JSON string
$QUIZ_JSON = $QUIZ_JSON -replace "`r", "" -replace "`n", ""

# Remove newlines and carriage returns from the JSON string
$QUIZ_JSON = $QUIZ_JSON -replace "`r", "" -replace "`n", ""

# Replace the degree symbol with a standard degree symbol
$QUIZ_JSON = $QUIZ_JSON -replace "Â°", "°"

# Escape the JSON string for command line
$escapedJson = $QUIZ_JSON -replace '"', '\"'

# Call the Python script with the JSON string as an argument
py scripts/parse_quiz.py "$escapedJson"