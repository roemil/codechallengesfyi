#include "RespHandler.h"

#include <_types/_uint8_t.h>
#include <cassert>
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

std::pair<size_t, RedisRespRes> RespHandler::decodeSimpleString(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

    const auto crlfPos = str.find("\r\n");

    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    return {crlfPos, RedisRespRes{.simpleString_ = str.substr(1, crlfPos - 1)}};
}

std::pair<size_t, RedisRespRes> RespHandler::decodeBulkString(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

    const auto lengthDelim = str.find_first_of("\r\n");
    if (lengthDelim == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    if (str.substr(1, lengthDelim - 1) == "-1") {
        return {lengthDelim, RedisRespRes{.bulkString_ = "null"}};
    }

    int length {};
    std::from_chars_result result = std::from_chars(str.data() + 1, str.data() + lengthDelim, length);
    if (result.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[lengthDelim] } };
    }

    std::cout << "[INFO]: Length: " << std::to_string(length) << "\n";

    //const auto endDelim = str.find_last_of("\r\n");
    //const auto endDelim = lengthDelim+2+length;//str.find("\r\n", lengthDelim+3);
    const auto endDelim = str.find("\r\n", lengthDelim+3);
    if (endDelim == lengthDelim) {
        throw std::invalid_argument { "Invalid format. End delimiter same as length delimiter" };
    }
    std::cout << "[INFO]: lengthDelim: " << std::to_string(lengthDelim) << "\n";
    std::cout << "[INFO]: endDelim: " << std::to_string(endDelim) << "\n";
    return {endDelim, RedisRespRes{.bulkString_ = str.substr(lengthDelim + 2, length)}};
}

std::pair<size_t, RedisRespRes> RespHandler::decodeError(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    return {crlfPos, RedisRespRes{.error_ = str.substr(1, crlfPos - 1)}};
}

std::pair<size_t, RedisRespRes> RespHandler::decodeInt(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    std::string_view decodedInt = str.substr(1, crlfPos - 1);
    // TODO: Verify value is integer with regex
    return {crlfPos, RedisRespRes{.integer_ = decodedInt}};
}

std::pair<size_t, RedisRespRes> RespHandler::decodeArray(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

    const auto arrLenDel = str.find_first_of("\r\n");
    int arrLen {};
    std::from_chars_result convertArrLenRes = std::from_chars(str.data() + 1, str.data() + arrLenDel, arrLen);
    if (convertArrLenRes.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[arrLenDel] } };
    }


    auto startPos = arrLenDel + 2;
    std::vector<RedisRespRes> result{};
    for(int i = 0; i < arrLen; ++i){
        const auto decodedVal = decode(str.substr(startPos));
        result.push_back(decodedVal.second);
        startPos+=decodedVal.first+2;
        std::cout << "Startpos: " << startPos << " " << str[startPos] << "\n";
    }
    return {startPos, RedisRespRes{.array_ = result}};
}

std::pair<size_t, RedisRespRes> RespHandler::decode(const std::string_view str)
{
    std::cout << __PRETTY_FUNCTION__ << "str: " << str << "\n";

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
    return {-1, RedisRespRes{}};
}

const std::vector<uint8_t>& RespHandler::getBuffer() const { return buffer; }