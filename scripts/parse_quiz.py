import sys
import json
import argparse

def main():
    parser = argparse.ArgumentParser(description='Parse quiz JSON from stdin.')
    # No arguments needed as we read from stdin
    args = parser.parse_args()

    try:
        quiz_json_str = sys.stdin.read()
        if not quiz_json_str:
            print("Error: No JSON input received.", file=sys.stderr)
            sys.exit(1)

        # Print the received JSON string to stderr for debugging
        print("Received JSON:", quiz_json_str, file=sys.stderr)

        quiz_data = json.loads(quiz_json_str)

        # --- Output format for C++ ---
        # Line 1: Quiz Name
        # Line 2: Number of Questions
        # For each question:
        #   Line: Question Text
        #   Line: Number of Options
        #   For each option:
        #       Line: Option Text
        # -----------------------------

        quiz_name = quiz_data.get("quizName", "Unnamed Quiz")
        questions = quiz_data.get("questions", {})
        num_questions = len(questions)

        print(quiz_name)
        print(num_questions)

        # Ensure questions are processed in order if keys are numeric strings
        question_keys = sorted(questions.keys(), key=lambda x: int(x) if x.isdigit() else float('inf'))

        for q_key in question_keys:
            q_data = questions[q_key]
            question_text = q_data.get("question", "No question text")
            options = q_data.get("questionOptions", {})
            num_options = len(options)

            print(question_text)
            print(num_options)

            # Ensure options are processed in order (A, B, C, D...)
            option_keys = sorted(options.keys())
            for opt_key in option_keys:
                opt_data = options[opt_key]
                option_text = opt_data.get("optionText", "No option text")
                print(option_text)

        sys.exit(0) # Success

    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format - {e}", file=sys.stderr)
        sys.exit(2)
    except KeyError as e:
        print(f"Error: Missing expected key in JSON - {e}", file=sys.stderr)
        sys.exit(3)
    except Exception as e:
        print(f"Error: An unexpected error occurred - {e}", file=sys.stderr)
        sys.exit(4)

if __name__ == "__main__":
    main()