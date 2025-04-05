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
#endif
