#ifndef MESSAGEFILTER_HPP
#define MESSAGEFILTER_HPP

#include <string>
#include <optional>
#include <memory>
#include "message.hpp"

class MessageFilter {
private:
    std::optional<std::string> ip;
    std::optional<uint16_t> port;
    std::optional<uint8_t> flags;
    std::optional<uint32_t> seqNum;
    std::optional<uint32_t> ackNum;
    std::unique_ptr<uint8_t[]> payload;
    size_t payloadSize = 0;

    // Helper to copy payload
    void copyPayload(const uint8_t* source, size_t size);

public:
    // Constructors and Destructor
    MessageFilter(
        const std::optional<std::string>& ip = std::nullopt,
        const std::optional<uint16_t>& port = std::nullopt,
        const std::optional<uint8_t>& flags = std::nullopt,
        const std::optional<uint32_t>& seqNum = std::nullopt,
        const std::optional<uint32_t>& ackNum = std::nullopt,
        const uint8_t* payload = nullptr,
        size_t payloadSize = 0);
    MessageFilter(const MessageFilter& other);             // Copy constructor
    MessageFilter(MessageFilter&& other) noexcept;         // Move constructor
    MessageFilter& operator=(const MessageFilter& other);  // Copy assignment
    MessageFilter& operator=(MessageFilter&& other) noexcept; // Move assignment
    ~MessageFilter();

    // Builder Methods
    MessageFilter withIP(const std::string& ip) const;
    MessageFilter withPort(uint16_t port) const;
    MessageFilter withSeqNum(uint32_t seqNum) const;
    MessageFilter withFlags(uint8_t flags) const;
    MessageFilter withAckNum(uint32_t ackNum) const;
    MessageFilter withPayloads(const uint8_t* payload, size_t payloadSize) const;

    // Validation
    bool validate(const Message& message) const;
};

#endif
