#include <iostream>
#include <string>
#include <asio.hpp>
#include <asio/ssl.hpp>

#include "http_init.h"
#include "../client.h"

HTTPResponse http_get(StoatClient &client , std::string &path) {

    auto &ioc = client.ioc();
    auto &ssl_ctx = client.ssl_ctx();
    auto &host = client.host();
    auto &token = client.token();

    asio::ssl::stream<asio::ip::tcp::socket> stream(ioc , ssl_ctx);

    asio::ip::tcp::resolver resolver(ioc);
    auto endpoints = resolver.resolve(host , "443");
    asio::connect(stream.next_layer() , endpoints);
    stream.handshake(asio::ssl::stream_base::client);

    std::string request = 
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Authorization: Bot " + token + "\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n\r\n";

    asio::write(stream , asio::buffer(request));

    std::string response;
    char buffer[1024];
    asio::error_code error_code;
    while (size_t length = stream.read_some(asio::buffer(buffer) , error_code)) {
        response.append(buffer , length);
    }

    size_t header_end = response.find("\r\n\r\n");
    std::string header_string = response.substr(0 , header_end);
    std::string body = response.substr(header_end + 4);

    int status = 0;
    int remaining = 1;
    int reset = 0;
    int retry_after = 0;

    size_t status_start = header_string.find("HTTP/1.1 ");
    if (status_start != std::string::npos) {
        status = std::stoi(header_string.substr(status_start + 9 , 3));
    }

    auto find_header = [&](const std::string &key) -> std::string {
        size_t position = header_string.find(key);
        if (position == std::string::npos) {
            return "";
        }
        size_t end = header_string.find("\r\n" , position);
        return header_string.substr(position + key.size() + 2 , end - (position + key.size() + 2));
    };

    std::string remaining_string = find_header("X-RateLimit-Remaining");
    if (!remaining_string.empty()) {
        remaining = std::stoi(remaining_string);
    };
    std::string reset_string = find_header("X-RateLimit-Reset");
    if (!reset_string.empty()) {
        reset = std::stoi(reset_string);
    }
    std::string retry_after_string = find_header("Retry-After");
    if (!retry_after_string.empty()) {
        retry_after = std::stoi(retry_after_string) * 1000;
    }

    return {status , body , remaining , reset , retry_after};

}