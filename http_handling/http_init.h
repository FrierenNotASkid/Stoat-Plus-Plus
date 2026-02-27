#ifndef HTTP_INIT_H
#define HTTP_INIT_H

#include <iostream>
#include <string>

struct HTTPResponse {
    
    int status_code;
    std::string body;
    int rate_limit_remaining;
    int rate_limit_reset;
    int retry_after_ms;

};

#endif