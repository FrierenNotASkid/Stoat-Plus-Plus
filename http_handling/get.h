#ifndef GET_H
#define GET_H

#include <string>
#include "../client.h"
#include "http_init.h"

HTTPResponse http_get(StoatClient &client , std::string &path);

#endif