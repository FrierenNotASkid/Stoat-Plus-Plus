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

    class FetchUser {

        public:

            struct User {

                std::string id;
                std::string username;
                std::string avatar_id;
                std::string avatar_filename;
                std::string avatar_filetype;
                std::string avatar_extension;
                std::string avatar_filesize;
                std::string status_text;
                bool online_status;
                int discriminator;
                int avatar_height;
                int avatar_width;

            };

            static User fetch(StoatClient &client , const std::string &user_id) {
                json data = fetch_user_json(client , user_id);
                return json_to_user(data);
            }

        private:

            static json fetch_user_json(StoatClient &client , const std::string &user_id) {

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
                    
                    try {
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

            static User json_to_user(const json &json) {

                User user;

                user.id = json.at("_id").get<std::string>();
                user.username = json.at("username").get<std::string>();
                user.discriminator = json.at("discriminator").get<int>();
                user.online_status = json.at("online").get<bool>();

                if (json.contains("avatar")) {
                    const auto &avatar = json.at("avatar");
                    user.avatar_id = avatar.value("_id" , "Null");
                    user.avatar_filename = avatar.value("filename" , "Null");
                    user.avatar_extension = avatar.value("content_type" , "Null");
                    user.avatar_filesize = avatar.value("size" , 0);
                }

                if (json.contains("metadata")) {
                    const auto &metadata = json.at("metadata");
                    user.avatar_filetype = metadata.value("type" , "Null");
                    user.avatar_width = metadata.value("width" , 0);
                    user.avatar_height = metadata.value("height" , 0);
                }

                if (json.contains("status")) {
                    const auto &status = json.at("status");
                    user.status_text = status.value("text" , "Null");
                }

                return user;

            }

    };

};