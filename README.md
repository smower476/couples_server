## ToDo List

**Refinements & Enhancements**

- [ ] Authentication & Security:
    - [x] Implement robust 401 response for `get-link-code` endpoint.
    - [ ] Introduce `expire_at` claim to JWT and `link_token` for enhanced security and token management.

**Daily Quiz Functionality**

- [x] Quiz Retrieval:
    - [x] Develop `get-daily-quiz` route to fetch and deliver the daily quiz.
    - [x] Structure quiz data into a JSON response for the client.

- [x] Answer Processing:
    - [x] Create `submit-quiz-answer` [POST] route for quiz submissions.
    - [x] Design a data structure to receive and process quiz answers:
        - [x] Accept `jwt-token`, `quiz-id`, and `int64 answers`.
        - [x] Encode answers efficiently using a 64-bit integer, allocating 2 bits per answer.

**Quiz History & Review**

- [x] Paged Quiz History:
    - [x] Implement `/get-quiz-history` endpoint to return a paged list of all completed quizzes, excluding answers.

- [ ] Detailed Quiz Review:
    - [ ] Enhance `/get-quiz-history` to support a `quiz-id` parameter:
        - [ ] `/get-quiz-history?quiz-id={quiz-id}` should return a specific quiz along with the user's answers.
