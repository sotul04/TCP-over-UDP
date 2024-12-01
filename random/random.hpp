#include <fstream>
#include <iostream>
#include <cstdint>
#include <stdexcept>

uint32_t randomNumber() {
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom) {
        throw std::runtime_error("Failed to open /dev/urandom");
    }

    uint32_t randomNumber;
    urandom.read(reinterpret_cast<char*>(&randomNumber), sizeof(randomNumber));
    if (!urandom) {
        throw std::runtime_error("Failed to read from /dev/urandom");
    }

    return randomNumber;
}
