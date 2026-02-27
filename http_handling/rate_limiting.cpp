#include <iostream>
#include <chrono>

class RateLimiter {

    public:

        RateLimiter() : remaining_(1) , reset_(std::chrono::system_clock::now()) {

            // The 1 in remaining_(1) acts as a placeholder.
            // the update() function will update it dynamically.
            
        }

        bool can_request() {
            return remaining_ > 0 || std::chrono::system_clock::now() >= reset_;
        }

        void consume() {
            if (remaining_ > 0) {
                --remaining_;
            }
        }

        void update(int remaining , int reset_seconds) {
            remaining_ = remaining;
            reset_ = std::chrono::system_clock::now() + std::chrono::seconds(reset_seconds);
        }

    private:

        int remaining_;
        std::chrono::system_clock::time_point reset_;
};