#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <asio.hpp>
#include <asio/ssl.hpp>

#include "http_handling/rate_limiting.h"

class StoatClient {

    public:

        StoatClient(const std::string &token) : host_("api.stoat.chat") , token_(token) , ioc_() , ssl_ctx_(asio::ssl::context::tlsv12_client) {}        

        const std::string &host() const { return host_; }
        const std::string &token() const { return token_; }
        const asio::io_context &ioc() const { return ioc_; }
        const asio::ssl::context &ssl_ctx() const { return ssl_ctx_; }
        const RateLimiter &rate_limiter() const { return rate_limiter_; }

        std::string &host() { return host_; }
        std::string &token() { return token_; }
        asio::io_context &ioc() { return ioc_; }
        asio::ssl::context &ssl_ctx() { return ssl_ctx_; }
        RateLimiter &rate_limiter() { return rate_limiter_; }

    private:

        std::string host_;
        std::string token_;
        asio::io_context ioc_;
        asio::ssl::context ssl_ctx_;
        RateLimiter rate_limiter_;

};

#endif