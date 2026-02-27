#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "get.h"
#include "../client.h"
#include "../utility/get_time.h"
#include "../utility/colours.h"

using json = nlohmann::json;

std::string fetch_gateway_url(StoatClient &client) {

    std::string path = "/v1/gateway";
    HTTPResponse response = http_get(client , path);

    auto json = json::parse(response.body);
    std::string gateway_url = json["url"];
    if (gateway_url.empty()) {
        std::ostringstream oss;
        oss << MAG << "[ " << get_current_time() << " ] " << RED << "[ ERROR ] The returned gateway URL is null." << std::endl;
        throw std::runtime_error(oss.str());
    }
    
    return gateway_url;

}