#ifndef TIMEOUT_EXCEPTION_HPP
#define TIMEOUT_EXCEPTION_HPP

#include <exception>
#include <string>

class TimeoutException: public std::exception {
private:
    std::string message;
public:
    explicit TimeoutException(const std::string& msg): message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

#endif