#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <nlohmann/json.hpp>

#include "../client.h"
#include "../http_handling/get.h"
#include "../utility/get_time.h"
#include "../utility/colours.h"

using json = nlohmann::json;

namespace stoat::users {

    class GetUser {

        public:

            std::string fetch_id(StoatClient &client , const std::string &user_id) {

                /*
                Return a users Stoat ID
                */

                json object = fetch_user(client , user_id);
                id_ = object["_id"].get<std::string>();

                if (id_.empty()) {
                    std::ostringstream oss;
                    oss << MAG << "[ " << get_current_time() << " ] " << RED << "[ ERROR ] The returned ID is null." << std::endl;
                    throw std::runtime_error(oss.str());
                }

                return id_;

            }

        private:

            std::string id_;
            std::string username_;
            int discriminator_;
            std::string avatar_id_;
            std::string avatar_tag_;
            std::string avatar_filename_;
            std::string avatar_filetype_;
            std::string avatar_height_;
            std::string avatar_width_;
            std::string avatar_extension_;
            std::string avatar_filesize_;
            std::string status_text_;
            std::string online_status_;

            json fetch_user(StoatClient &client , const std::string &user_id) {

                auto &ioc = client.ioc();
                auto &ssl_ctx = client.ssl_ctx();
                auto &host = client.host();
                auto &token = client.token();                
                auto &rate_limiter = client.rate_limiter();

                int backoff_base = 100; // This allows for exponential backoff upon weird response codes

                std::string path = "/users/" + user_id;
                int attempts = 0;

                while (attempts < 20) { // The rate limit for /users I think... I confused myself
                    if (!rate_limiter.can_request()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Make this wait 0.5 seconds / attempt
                        continue;
                    }

                    rate_limiter.consume();
                    auto response = http_get(client , path);

                    rate_limiter.update(response.rate_limit_remaining , response.rate_limit_reset);

                    if (response.status_code == 429) {
                        std::cout << MAG << "[ " << get_current_time() << " ] " << YEL << "[ WARNING ] You are being rate limited!" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(response.retry_after_ms));
                        continue;
                    }

                    if (response.status_code >= 500) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(backoff_base * std::min(1 << attempts , 1024)));
                        ++attempts;
                        continue;
                    }

                    if (response.status_code != 200) {
                        std::ostringstream oss;
                        oss << MAG << "[ " << get_current_time() << " ] " << RED << "[ ERROR ] HTTP error: " << std::to_string(response.status_code);
                        throw std::runtime_error(oss.str());
                    }
                    
                    try{
                        return json::parse(response.body);
                    }
                    catch (std::exception &error) {
                        std::ostringstream oss;
                        oss << MAG << "[ " << get_current_time() << " ] " << RED << "[ ERROR ] An error was caught in the parsing of the returned JSON body: " << error.what() << std::to_string(response.status_code);
                        throw std::runtime_error(oss.str());                        
                    }

                }

                std::ostringstream oss;
                oss << MAG << "[ " << get_current_time() << " ] " << RED << "Request failed after retries!";
                throw std::runtime_error(oss.str());

            }

    };

};