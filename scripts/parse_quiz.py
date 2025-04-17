import sys
import json

def main():
    # Expect the JSON string as the first command-line argument
    if len(sys.argv) < 2:
        print("Error: No JSON data provided as argument.", file=sys.stderr)
        sys.exit(1) # Exit code for missing argument

    quiz_json_str = sys.argv[1]

    try:
        # Strip potential BOM (Byte Order Mark) just in case
        if quiz_json_str.startswith('\ufeff'):
            quiz_json_str = quiz_json_str.lstrip('\ufeff')

        # Load the JSON data from the argument string
        quiz_data = json.loads(quiz_json_str)

        # --- Outputting in structured, tab-delimited format ---

        # Helper function to sanitize text for output
        def sanitize(text):
            return str(text).replace('\t', ' ').replace('\n', ' ')

        quiz_name = quiz_data.get("quizName", "Unnamed Quiz")
        print(f"QUIZ_NAME\t{sanitize(quiz_name)}")

        questions = quiz_data.get("questions", {})
        if not isinstance(questions, dict):
             print(f"Error: 'questions' field is not a valid dictionary.", file=sys.stderr)
             sys.exit(3) # Specific exit code for structure error

        num_questions = len(questions)
        print(f"NUM_QUESTIONS\t{num_questions}")

        # Sort question keys numerically if possible, otherwise alphanumerically
        try:
            # Assuming keys are '1', '2', etc.
            question_keys = sorted(questions.keys(), key=int)
        except ValueError:
             # Fallback if keys are not purely numeric strings (e.g., 'q1', 'intro')
             question_keys = sorted(questions.keys())

        # Iterate through sorted questions
        for q_key in question_keys:
            q_data = questions[q_key]
            if not isinstance(q_data, dict):
                print(f"Error: Question data for key '{q_key}' is not a valid dictionary.", file=sys.stderr)
                continue # Skip malformed question

            question_text = q_data.get("question", "No question text")
            print(f"QUESTION\t{sanitize(q_key)}\t{sanitize(question_text)}")

            options = q_data.get("questionOptions", {})
            if not isinstance(options, dict):
                 print(f"Error: 'questionOptions' for question '{q_key}' is not a valid dictionary.", file=sys.stderr)
                 # Still print NUM_OPTIONS as 0, but skip OPTION lines
                 print(f"NUM_OPTIONS\t{sanitize(q_key)}\t0")
                 continue

            num_options = len(options)
            print(f"NUM_OPTIONS\t{sanitize(q_key)}\t{num_options}")

            option_keys = sorted(options.keys()) # Sort options A, B, C, D
            for opt_key in option_keys:
                opt_data = options[opt_key]
                if not isinstance(opt_data, dict):
                     print(f"Error: Option data for key '{opt_key}' in question '{q_key}' is not a valid dictionary.", file=sys.stderr)
                     continue # Skip malformed option

                option_text = opt_data.get("optionText", "No option text")
                print(f"OPTION\t{sanitize(q_key)}\t{sanitize(opt_key)}\t{sanitize(option_text)}")

            answer = q_data.get("questionAnswer", "No answer provided")
            print(f"ANSWER\t{sanitize(q_key)}\t{sanitize(answer)}")

        # Indicate successful completion to the calling C++ program
        sys.exit(0) # Success

    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format - {e}", file=sys.stderr)
        sys.exit(2) # Exit code for JSON error
    except KeyError as e:
        print(f"Error: Missing expected key in JSON - {e}", file=sys.stderr)
        sys.exit(3) # Exit code for structure error
    except Exception as e:
        # Catch any other unexpected error
        print(f"Error: An unexpected error occurred ({type(e).__name__}) - {e}", file=sys.stderr)
        sys.exit(4) # General error exit code

if __name__ == "__main__":
    main()