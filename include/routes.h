#ifndef ROUTES_H
#define ROUTES_H

#include <httpserver.hpp>
using namespace httpserver;

/*
class validate_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};
*/

class add_user_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class login_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_link_code_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class link_users_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class set_user_info_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_user_info_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_partner_info_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};


class unlink_users_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_daily_quiz_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_quiz_content_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class answer_quiz_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_quiz_user_answer_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_answered_quizes_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_unanswered_quizes_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

class get_unanswered_quizzes_for_pair_resource : public http_resource {
public:
    std::shared_ptr<http_response> render(const http_request& req) override;
};

#endif
