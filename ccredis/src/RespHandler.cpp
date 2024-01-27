#include "RespHandler.h"

#include <_types/_uint8_t.h>
#include <cassert>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

void RespHandler::appendChars(const std::string_view str)
{
    for (const auto& c : str) {
        buffer.push_back(static_cast<uint8_t>(c));
    }
}

void RespHandler::appendCRLF()
{
    buffer.push_back('\r');
    buffer.push_back('\n');
}

void RespHandler::appendSimpleString(const std::string_view str)
{
    buffer.push_back(static_cast<uint8_t>(Prefix::SIMPLE_STRING));
    for (const auto& c : str) {
        if (c == '\r' || c == '\n') {
            throw std::invalid_argument("String must not contain CR or LF!");
        }
        buffer.push_back(static_cast<uint8_t>(c));
    }
    appendCRLF();
}

void RespHandler::appendBulkstring(const std::string_view str)
{
    buffer.push_back(static_cast<uint8_t>(Prefix::BULK_STRING));
    buffer.push_back(str.length());
    appendCRLF();
    // TODO: Should probably do something like buf.writeu8() or writeu32() to handle wide chars
    appendChars(str);
    appendCRLF();
}

void RespHandler::appendNull()
{
    buffer.push_back(static_cast<uint8_t>(Prefix::BULK_STRING));
    buffer.push_back(-1);
    appendCRLF();
}

void RespHandler::appendError(const std::string_view str)
{
    buffer.push_back(static_cast<uint8_t>(Prefix::ERROR));
    for (const auto& c : str) {
        buffer.push_back(static_cast<uint8_t>(c));
    }
    appendCRLF();
}

void RespHandler::appendInt(const std::string_view n)
{
    buffer.push_back(static_cast<uint8_t>(Prefix::INTEGER));
    appendChars(n);
    appendCRLF();
}

void RespHandler::beginArray(const unsigned numElements)
{
    buffer.push_back(static_cast<uint8_t>(Prefix::ARRAY));
    buffer.push_back(static_cast<uint8_t>(numElements));
    appendCRLF();
}

constexpr std::string_view RespHandler::decodeSimpleString(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    return str.substr(1, crlfPos - 1);
}

constexpr std::string_view RespHandler::decodeBulkString(const std::string_view str)
{
    const auto lengthDelim = str.find_first_of("\r\n");
    if (lengthDelim == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    if (str.substr(1, lengthDelim - 1) == "-1") {
        return "null";
    }

    int length {};
    std::from_chars_result result = std::from_chars(str.data() + 1, str.data() + lengthDelim, length);
    if (result.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[lengthDelim] } };
    }

    const auto endDelim = str.find_last_of("\r\n");
    if (endDelim == lengthDelim) {
        throw std::invalid_argument { "Invalid format. End delimiter same as length delimiter" };
    }
    return str.substr(lengthDelim + 2, endDelim - 1 - (lengthDelim + 2));
}

constexpr std::string_view RespHandler::decodeError(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    return str.substr(1, crlfPos - 1);
}

constexpr std::string_view RespHandler::decodeInt(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    std::string_view decodedInt = str.substr(1, crlfPos - 1);
    // TODO: Verify value is integer with regex
    return decodedInt;
}

constexpr std::string_view RespHandler::decodeArray(const std::string_view str)
{
    const auto arrLenDel = str.find_first_of("\r\n");
    int arrLen {};
    std::from_chars_result convertArrLenRes = std::from_chars(str.data() + 1, str.data() + arrLenDel, arrLen);
    if (convertArrLenRes.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[arrLenDel] } };
    }

    assert(arrLen == 1); // TODO: Accept arrays longer than 1 :)
    const auto endDelim = str.find_last_of("\r\n");
    return decode(str.substr(arrLenDel + 2, endDelim - 1 - (endDelim + 2)));
}

std::string_view RespHandler::decode(const std::string_view str)
{
    const auto prefix = static_cast<Prefix>(str[0]);
    switch (prefix) {
    case Prefix::SIMPLE_STRING:
        return decodeSimpleString(str);
    case Prefix::ERROR:
        return decodeError(str);
    case Prefix::INTEGER:
        return decodeInt(str);
    case Prefix::BULK_STRING:
        return decodeBulkString(str);
    case Prefix::ARRAY:
        return decodeArray(str);
    }
    return "";
}

const std::vector<uint8_t>& RespHandler::getBuffer() const { return buffer; }