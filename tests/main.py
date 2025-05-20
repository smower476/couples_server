import unittest
import requests
import json
import datetime

# Assuming the server is running locally on port 8080
BASE_URL = "http://129.158.234.85:8081"

class TestAPIEndpoints(unittest.TestCase):
    user1_username = "bigbon__7"
    user1_password = "King22401825"
    user2_username = "bigbon__8"
    user2_password = "King22401825"
    def test_login_success(self):
        """Test successful user login."""
        # First, ensure a user exists to log in with
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload) # Assuming user creation is idempotent or cleanup is handled elsewhere

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        response = requests.post(login_url, data=login_payload)
        self.assertEqual(response.status_code, 200)
        self.assertIsInstance(response.text, str)
        self.assertTrue(len(response.text) > 0)

    def test_login_invalid_credentials(self):
        """Test login with invalid username or password."""
        url = f"{BASE_URL}/login"
        payload = {"username": "nonexistentuser", "password": "wrongpassword"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid username or password")

    def test_get_link_code_success(self):
        """Test getting a link code with a valid token."""
        # First, create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user2_username, "password": self.user2_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user2_username, "password": self.user2_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # Unlink user first to ensure a clean state for getting a new link code
        unlink_users_url = f"{BASE_URL}/unlink-users"
        requests.post(unlink_users_url, data={"token": token})

        get_link_code_url = f"{BASE_URL}/get-link-code"
        payload = {"token": token}
        response = requests.post(get_link_code_url, data=payload)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(len(response.text), 6)
        self.assertTrue(response.text.isdigit())

    def test_get_link_code_unauthorized(self):
        """Test getting a link code with an invalid token."""
        url = f"{BASE_URL}/get-link-code"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_link_users_success(self):
        """Test successfully linking two users."""
        # Create and login the first user to get a token and link code
        user1_add_url = f"{BASE_URL}/add-user"
        user1_add_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(user1_add_url, data=user1_add_payload)

        user1_login_url = f"{BASE_URL}/login"
        user1_login_payload = {"username": self.user1_username, "password": self.user1_password}
        user1_login_response = requests.post(user1_login_url, data=user1_login_payload)
        user1_token = user1_login_response.text

        # Create and login the second user
        user2_add_url = f"{BASE_URL}/add-user"
        user2_add_payload = {"username": self.user2_username, "password": self.user2_password}
        requests.post(user2_add_url, data=user2_add_payload)

        user2_login_url = f"{BASE_URL}/login"
        user2_login_payload = {"username": self.user2_username, "password": self.user2_password}
        user2_login_response = requests.post(user2_login_url, data=user2_login_payload)
        user2_token = user2_login_response.text

        # Unlink users first to ensure a clean state for linking
        unlink_users_url = f"{BASE_URL}/unlink-users"
        requests.post(unlink_users_url, data={"token": user1_token})
        # requests.post(unlink_users_url, data={"token": user2_token})

        get_link_code_url = f"{BASE_URL}/get-link-code"
        payload = {"token": user1_token}
        link_code_response = requests.post(get_link_code_url, data=payload)
        link_code = link_code_response.text

        # Link the users using the second user's token and the first user's link code
        link_users_url = f"{BASE_URL}/link-users"
        link_users_payload = {"token": user2_token, "link_code": link_code}
        response = requests.post(link_users_url, data=link_users_payload)


        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Success")

        requests.post(unlink_users_url, data={"token": user1_token})

    def test_link_users_unauthorized(self):
        """Test linking users with an invalid token."""
        url = f"{BASE_URL}/link-users"
        payload = {"link_code": "123456"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_set_user_info_success(self):
        """Test setting user information with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        set_user_info_url = f"{BASE_URL}/set-user-info"
        # Test setting only mood_status
        payload_status_only = {"token": token, "mood_status": "sad"}
        response_status_only = requests.post(set_user_info_url, data=payload_status_only)
        self.assertEqual(response_status_only.status_code, 200)
        self.assertEqual(response_status_only.headers['Content-Type'], 'text/plain')
        self.assertEqual(response_status_only.text, "Success")


    def test_set_user_info_unauthorized(self):
        """Test setting user information with an invalid token."""
        url = f"{BASE_URL}/set-user-info"
        payload = {"mood_scale": 5, "mood_status": "Happy"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_user_info_success(self):
        """Test getting user information with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        get_user_info_url = f"{BASE_URL}/get-user-info"
        payload = {"token": token}
        response = requests.post(get_user_info_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, dict)
            self.assertIn("username", data)
            # Assuming mood_scale and mood_status might be returned based on set-user-info
            # self.assertIn("mood_scale", data)
            # self.assertIn("mood_status", data)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_user_info_unauthorized(self):
        """Test getting user information with an invalid token."""
        url = f"{BASE_URL}/get-user-info"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_partner_info_success(self):
        """Test getting partner information with a valid token."""
        # Create and link two users
        user1_add_url = f"{BASE_URL}/add-user"
        user1_add_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(user1_add_url, data=user1_add_payload)

        user1_login_url = f"{BASE_URL}/login"
        user1_login_payload = {"username": self.user1_username, "password": self.user1_password}
        user1_login_response = requests.post(user1_login_url, data=user1_login_payload)
        user1_token = user1_login_response.text

        get_link_code_url = f"{BASE_URL}/get-link-code"
        payload = {"token": user1_token}
        link_code_response = requests.post(get_link_code_url, data=payload)
        link_code = link_code_response.text

        user2_add_url = f"{BASE_URL}/add-user"
        user2_add_payload = {"username": self.user2_username, "password": self.user2_password}
        requests.post(user2_add_url, data=user2_add_payload)

        user2_login_url = f"{BASE_URL}/login"
        user2_login_payload = {"username": self.user2_username, "password": self.user2_password}
        user2_login_response = requests.post(user2_login_url, data=user2_login_payload)
        user2_token = user2_login_response.text

        link_users_url = f"{BASE_URL}/link-users"
        link_users_payload = {"link_code": link_code, "token": user2_token}
        requests.post(link_users_url, data=link_users_payload).text
        

        # Get partner info for user 1
        get_partner_info_url = f"{BASE_URL}/get-partner-info"
        payload = {"token": user1_token}
        response = requests.post(get_partner_info_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, dict)
            self.assertIn("username", data)
            self.assertIn("mood_status", data)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")


    def test_get_partner_info_unauthorized(self):
        """Test getting partner information with an invalid token."""
        url = f"{BASE_URL}/get-partner-info"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_daily_quiz_success(self):
        """Test getting the daily quiz with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        get_daily_quiz_url = f"{BASE_URL}/get-daily-quiz"
        payload = {"token": token}
        response = requests.post(get_daily_quiz_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, list)
            self.assertIsInstance(data[0], dict)
            self.assertTrue(data[0].get("id") is not None)
            self.assertTrue(isinstance(data[0].get("quiz_name"), str))
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_daily_quiz_unauthorized(self):
        """Test getting the daily quiz with an invalid token."""
        url = f"{BASE_URL}/get-daily-quiz"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_quiz_content_success(self):
        """Test getting quiz content with a valid token and quiz ID."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # Assuming a valid quiz_id exists (replace with a real one if possible)
        # First, get a daily quiz to get a valid quiz_id
        get_daily_quiz_url = f"{BASE_URL}/get-daily-quiz"
        payload_daily_quiz = {"token": token}
        daily_quiz_response = requests.post(get_daily_quiz_url, data=payload_daily_quiz)
        daily_quiz_data = daily_quiz_response.json()
        valid_quiz_id = daily_quiz_data[0]["id"]

        get_quiz_content_url = f"{BASE_URL}/get-quiz-content"
        payload = {"token": token, "quiz_id": valid_quiz_id}
        response = requests.post(get_quiz_content_url, data=payload)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        try:
            data = response.json()
            self.assertIsInstance(data, dict)
            # Assuming quiz content will have keys like 'question', 'options'
            # self.assertIn("question", data)
            # self.assertIn("options", data)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_quiz_content_unauthorized(self):
        """Test getting quiz content with an invalid token."""
        url = f"{BASE_URL}/get-quiz-content"
        payload = {"token": "invalidtoken", "quiz_id": 1}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_quiz_content_not_found(self):
        """Test getting quiz content with a valid token but invalid quiz ID."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # Assuming an invalid quiz_id
        invalid_quiz_id = 9999

        get_quiz_content_url = f"{BASE_URL}/get-quiz-content"
        payload = {"token": token, "quiz_id": invalid_quiz_id}
        response = requests.post(get_quiz_content_url, data=payload)

        self.assertEqual(response.status_code, 500)
        self.assertEqual(response.headers['Content-Type'], 'application/json')
        self.assertIn("error", response.text)
        
        try:
            data = response.json()
            self.assertIn("error", data)
            self.assertEqual(data["error"], "Failed to retrieve quiz content")
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_answer_quiz_success(self):
        """Test answering a quiz with a valid token, quiz ID, and answer."""
        # Create and login a user to get a valid token
        today_date = datetime.datetime.now().strftime("%Y%m%d")
        dynamic_username = f"user{today_date}"

        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": dynamic_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": dynamic_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # Assuming a valid quiz_id and answer format
        # First, get a quiz to answer
        get_daily_quiz_url = f"{BASE_URL}/get-daily-quiz"
        quiz_response = requests.post(get_daily_quiz_url, data={"token": token})
        self.assertEqual(quiz_response.status_code, 200)
        quiz_data = quiz_response.json()
        valid_quiz_id = quiz_data[0]["id"] # Or however the quiz ID is structured
        valid_answer = 1234121 # Ensure this answer format is valid for the quiz

        answer_quiz_url = f"{BASE_URL}/answer-quiz"
        payload = {"token": token, "quiz_id": valid_quiz_id, "answer": valid_answer}
        response = requests.post(answer_quiz_url, data=payload)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Success")


    def test_answer_quiz_unauthorized(self):
        """Test answering a quiz with an invalid token."""
        url = f"{BASE_URL}/answer-quiz"
        payload = {"quiz_id": 1, "answer": 123412}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_quiz_user_answer_success(self):
        """Test getting a user's quiz answer with a valid token and quiz ID."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # First, get a daily quiz to get a valid quiz_id
        get_daily_quiz_url = f"{BASE_URL}/get-daily-quiz"
        payload_daily_quiz = {"token": token}
        daily_quiz_response = requests.post(get_daily_quiz_url, data=payload_daily_quiz)
        daily_quiz_data = daily_quiz_response.json()
        valid_quiz_id = daily_quiz_data[0]["id"]

        # Optionally, answer the quiz first to ensure there's an answer to retrieve
        answer_quiz_url = f"{BASE_URL}/answer-quiz"
        # Use a placeholder answer or a specific one if known
        answer_payload = {"token": token, "quiz_id": valid_quiz_id, "answer": 12345}
        answer_response = requests.post(answer_quiz_url, data=answer_payload)
        self.assertEqual(answer_response.status_code, 200, "Failed to answer the quiz before retrieving answer.")

        get_quiz_user_answer_url = f"{BASE_URL}/get-quiz-user-answer"
        payload = {"token": token, "quiz_id": valid_quiz_id}
        response = requests.post(get_quiz_user_answer_url, data=payload)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        try:
            data = response.json()
            self.assertIsInstance(data, dict)
            # Assuming the response will contain the user's answer, e.g., {"answer": "..."}
            # self.assertIn("answer", data)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_quiz_user_answer_unauthorized(self):
        """Test getting a user's quiz answer with an invalid token."""
        url = f"{BASE_URL}/get-quiz-user-answer"
        payload = {"token": "invalidtoken", "quiz_id": 1}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_answered_quizzes_success(self):
        """Test getting answered quizzes with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # Note: To make this test more robust, you would ideally answer at least one quiz first
        # using the answer_quiz endpoint before attempting to retrieve the list of answered quizzes.

        get_answered_quizzes_url = f"{BASE_URL}/get-answered-quizes"
        payload = {"token": token}
        response = requests.post(get_answered_quizzes_url, data=payload)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        try:
            data = response.json()
            self.assertIsInstance(data, list)
            # Assuming each item in the list is a dictionary representing a quiz
            # if data:
            #     self.assertIsInstance(data[0], dict)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_answered_quizzes_unauthorized(self):
        """Test getting answered quizzes with an invalid token."""
        url = f"{BASE_URL}/get-answered-quizes"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_unanswered_quizzes_success(self):
        """Test getting unanswered quizzes with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        get_unanswered_quizzes_url = f"{BASE_URL}/get-unanswered-quizes"
        payload = {"token": token}
        response = requests.post(get_unanswered_quizzes_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, list)
            # Assuming each item in the list is a dictionary representing a quiz
            # if data:
            #     self.assertIsInstance(data[0], dict)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_get_unanswered_quizzes_unauthorized(self):
        """Test getting unanswered quizzes with an invalid token."""
        url = f"{BASE_URL}/get-unanswered-quizes"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_unanswered_quizzes_for_pair_success(self):
        """Test getting unanswered quizzes for a linked pair with a valid token."""
        # Create and link two users
        user1_add_url = f"{BASE_URL}/add-user"
        user1_add_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(user1_add_url, data=user1_add_payload)

        user1_login_url = f"{BASE_URL}/login"
        user1_login_payload = {"username": self.user1_username, "password": self.user1_password}
        user1_login_response = requests.post(user1_login_url, data=user1_login_payload)
        user1_token = user1_login_response.text

        get_link_code_url = f"{BASE_URL}/get-link-code"
        user1_headers = {"Authorization": f"Bearer {user1_token}"}
        link_code_response = requests.get(get_link_code_url, headers=user1_headers)
        link_code = link_code_response.text

        user2_add_url = f"{BASE_URL}/add-user"
        user2_add_payload = {"username": self.user2_username, "password": self.user2_password}
        requests.post(user2_add_url, data=user2_add_payload)

        user2_login_url = f"{BASE_URL}/login"
        user2_login_payload = {"username": self.user2_username, "password": self.user2_password}
        user2_login_response = requests.post(user2_login_url, data=user2_login_payload)
        user2_token = user2_login_response.text

        link_users_url = f"{BASE_URL}/link-users"
        link_users_payload = {"link_code": link_code}
        requests.post(link_users_url, data=link_users_payload)

        # Get unanswered quizzes for the pair using user 1's token
        get_unanswered_quizzes_for_pair_url = f"{BASE_URL}/get-unanswered-quizzes-for-pair"
        payload = {"token": user1_token}
        response = requests.post(get_unanswered_quizzes_for_pair_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, list)
            # Assuming each item in the list is a dictionary representing a quiz
            # if data:
            #     self.assertIsInstance(data[0], dict)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")


    def test_get_unanswered_quizzes_for_pair_unauthorized(self):
        """Test getting unanswered quizzes for a linked pair with an invalid token."""
        url = f"{BASE_URL}/get-unanswered-quizzes-for-pair"
        payload = {"token": "invalidtoken"}
        response = requests.post(url, data=payload)

        self.assertEqual(response.status_code, 401)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Invalid JWT token")

    def test_get_unanswered_questions_success(self):
        """Test getting unanswered questions with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        get_unanswered_daily_question_url = f"{BASE_URL}/get-unanswered-questions"
        payload = {"token": token}
        response = requests.post(get_unanswered_daily_question_url, data=payload)

        self.assertEqual(response.status_code, 200)
        try:
            data = response.json()
            self.assertIsInstance(data, list)
            self.assertIsInstance(data[0], dict)
            self.assertIn("question_content", data[0])
            self.assertIn("id", data[0])
            self.assertIn("question_name", data[0])
            # Assuming the response will contain the daily question
            # self.assertIn("question", data)
        except json.JSONDecodeError:
            self.fail("Response is not valid JSON")

    def test_answer_daily_question(self):
        """Test answering a daily question with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        # First, get the unanswered questions to get a valid question_id
        get_unanswered_daily_question_url = f"{BASE_URL}/get-unanswered-questions"
        payload_daily_question = {"token": token}
        daily_question_response = requests.post(get_unanswered_daily_question_url, data=payload_daily_question)
        daily_question_data = daily_question_response.json()
        valid_question_id = daily_question_data[0]["id"]

        answer_daily_question_url = f"{BASE_URL}/answer-daily-question"
        payload_answer = {"token": token, "question_id": valid_question_id, "answer": "Test Answer"}
        response_answer = requests.post(answer_daily_question_url, data=payload_answer)

        self.assertEqual(response_answer.status_code, 200)
        self.assertEqual(response_answer.headers['Content-Type'], 'text/plain')
        self.assertEqual(response_answer.text, "Success")

    def test_daily_question_logic(self):
        """Test daily question logic with a valid token."""
        # Create and login a user to get a valid token
        add_user_url = f"{BASE_URL}/add-user"
        add_user_payload = {"username": self.user1_username, "password": self.user1_password}
        requests.post(add_user_url, data=add_user_payload)

        login_url = f"{BASE_URL}/login"
        login_payload = {"username": self.user1_username, "password": self.user1_password}
        login_response = requests.post(login_url, data=login_payload)
        token = login_response.text

        add_user2_url = f"{BASE_URL}/add-user"
        add_user2_payload = {"username": self.user2_username, "password": self.user2_password}
        requests.post(add_user2_url, data=add_user2_payload)
        login_url2 = f"{BASE_URL}/login"
        login_payload2 = {"username": self.user2_username, "password": self.user2_password}
        login_response2 = requests.post(login_url2, data=login_payload2)
        token2 = login_response2.text

        # Get link code for user 1
        get_link_code_url = f"{BASE_URL}/get-link-code"
        payload = {"token": token}
        link_code_response = requests.post(get_link_code_url, data=payload)
        link_code = link_code_response.text
        # Link users
        link_users_url = f"{BASE_URL}/link-users"
        link_users_payload = {"link_code": link_code, "token": token2}
        requests.post(link_users_url, data=link_users_payload)

        # First, get the unanswered questions to get a valid question_id
        get_unanswered_daily_question_url = f"{BASE_URL}/get-unanswered-questions"
        payload_daily_question = {"token": token}
        daily_question_response = requests.post(get_unanswered_daily_question_url, data=payload_daily_question)
        daily_question_data = daily_question_response.json()
        valid_question_id = daily_question_data[0]["id"]

        # Get all answered questions
        get_answered_questions_url = f"{BASE_URL}/get-answered-questions"
        payload_answered = {"token": token}
        response_answered = requests.post(get_answered_questions_url, data=payload_answered)
        self.assertEqual(response_answered.status_code, 200)
        self.assertTrue(str(valid_question_id) not in response_answered.text)

        answer_daily_question_url = f"{BASE_URL}/answer-daily-question"
        payload_answer = {"token": token, "question_id": valid_question_id, "answer": "Test Answer"}
        response_answer = requests.post(answer_daily_question_url, data=payload_answer)

        self.assertEqual(response_answer.status_code, 200)
        self.assertEqual(response_answer.headers['Content-Type'], 'text/plain')
        self.assertEqual(response_answer.text, "Success")

        # Get all answered questions
        get_answered_questions_url = f"{BASE_URL}/get-answered-questions"
        payload_answered = {"token": token}
        response_answered = requests.post(get_answered_questions_url, data=payload_answered)
        self.assertEqual(response_answered.status_code, 200)
        self.assertTrue(str(valid_question_id) in response_answered.text)
        data = response_answered.json()
        self.assertIn("linked", response_answered.text)
        self.assertTrue(data[0]["linked"] == None)
        self.assertIsInstance(data[0]["self"], dict)

        #Second user answers the question
        answer_daily_question_url2 = f"{BASE_URL}/answer-daily-question"
        payload_answer2 = {"token": token2, "question_id": valid_question_id, "answer": "Test Answer"}
        response_answer2 = requests.post(answer_daily_question_url2, data=payload_answer2)
        self.assertEqual(response_answer2.status_code, 200)
        self.assertEqual(response_answer2.headers['Content-Type'], 'text/plain')

        # Get all answered questions for the second user
        get_answered_questions_url2 = f"{BASE_URL}/get-answered-questions"
        payload_answered2 = {"token": token2}
        response_answered2 = requests.post(get_answered_questions_url2, data=payload_answered2)
        self.assertEqual(response_answered2.status_code, 200)
        self.assertTrue(response_answered2.json() != [])
        self.assertTrue(str(valid_question_id) in response_answered2.text)
        data2 = response_answered2.json()
        self.assertIn("linked", response_answered2.text)
        self.assertIsInstance(data2[0]["linked"], dict)
        self.assertIsInstance(data2[0]["self"], dict)

    # Note: Testing for 401 Unauthorized for /add-user might require a different setup
    # if it's related to existing users or other authorization mechanisms not clear from docs.
    # For now, focusing on documented responses for invalid input.


if __name__ == '__main__':
    unittest.main()