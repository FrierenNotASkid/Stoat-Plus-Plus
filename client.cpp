#include <iostream>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <string>
#include <nlohmann/json.hpp>

#include "http_handling/http_init.h"
#include "HTTP_handling/rate_limiting.cpp"

using json = nlohmann::json;

class StoatClient {

    public:

        StoatClient(const std::string &token) : host_("api.stoat.chat") , token_(token) , ioc_() , ssl_ctx_(asio::ssl::context::tlsv12_client) {}

    private:

        std::string host_;
        std::string token_;
        asio::io_context ioc_;
        asio::ssl::context ssl_ctx_;
        RateLimiter rate_limiter_;

};