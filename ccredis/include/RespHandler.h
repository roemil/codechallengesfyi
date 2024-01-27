#pragma once

#include <string_view>
#include <vector>

enum class Prefix : char {
    SIMPLE_STRING = '+',
    ERROR = '-',
    INTEGER = ':',
    BULK_STRING = '$'
};

class RespHandler {
public:
    void appendSimpeString(const std::string_view str);
    void appendError(const std::string_view str);
    void appendInt(const std::string_view n);

    [[nodiscard]] static std::string_view decode(const std::string_view str);

    const std::vector<char>& getBuffer() const;

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static constexpr std::string_view decodeSimpleString(const std::string_view str);
    static constexpr std::string_view decodeError(const std::string_view str);
    static constexpr std::string_view decodeInt(const std::string_view str);
    static constexpr std::string_view decodeBulkString(const std::string_view str);
    // TODO: Replace with a output stringstream?
    std::vector<char> buffer {};
};