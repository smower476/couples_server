import sys
import json
import argparse

def main():
    parser = argparse.ArgumentParser(description='Parse quiz JSON from a file.')
    parser.add_argument('quiz_json_file', help='Path to the JSON file containing quiz data.')
    args = parser.parse_args()

    try:
        quiz_json_file_path = args.quiz_json_file
        try:
            with open(quiz_json_file_path, 'r') as f:
                quiz_json_str = f.read()
        except FileNotFoundError:
            print(f"Error: File not found at path: {quiz_json_file_path}", file=sys.stderr)
            sys.exit(1)

        quiz_data = json.loads(quiz_json_str)

        quiz_name = quiz_data.get("quizName", "Unnamed Quiz")
        questions = quiz_data.get("questions", {})
        num_questions = len(questions)

        print(quiz_name)
        print(num_questions)

        question_keys = sorted(questions.keys(), key=lambda x: int(x) if x.isdigit() else float('inf'))

        for q_key in question_keys:
            q_data = questions[q_key]
            question_text = q_data.get("question", "No question text")
            options = q_data.get("questionOptions", {})
            num_options = len(options)

            print(question_text)
            print(num_options)

            option_keys = sorted(options.keys())
            for opt_key in option_keys:
                opt_data = options[opt_key]
                option_text = opt_data.get("optionText", "No option text")
                print(option_text)

        sys.exit(0)

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