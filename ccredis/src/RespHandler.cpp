#include "RespHandler.h"

#include <cassert>
#include <charconv>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
// #include <ranges> todo upgrade compiler

void RespHandler::appendChars(const std::string_view str)
{
    for (const auto& c : str) {
        buffer.push_back(static_cast<char>(c));
    }
}

void RespHandler::appendCRLF()
{
    buffer.push_back('\r');
    buffer.push_back('\n');
}

void RespHandler::appendSimpleString(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::SIMPLE_STRING));
    for (const auto& c : str) {
        if (c == '\r' || c == '\n') {
            throw std::invalid_argument("String must not contain CR or LF!");
        }
        buffer.push_back(static_cast<char>(c));
    }
    appendCRLF();
}

void RespHandler::appendBulkstring(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::BULK_STRING));
    buffer.push_back(char(str.length()) + '0');
    appendCRLF();
    // TODO: Should probably do something like buf.writeu8() or writeu32() to handle wide chars
    appendChars(str);
    appendCRLF();
}

void RespHandler::appendNull()
{
    buffer.push_back(static_cast<char>(Prefix::BULK_STRING));
    buffer.push_back(-1);
    appendCRLF();
}

void RespHandler::appendError(const std::string_view str)
{
    buffer.push_back(static_cast<char>(Prefix::ERROR));
    for (const auto& c : str) {
        buffer.push_back(static_cast<char>(c));
    }
    appendCRLF();
}

void RespHandler::appendInt(const std::string_view n)
{
    buffer.push_back(static_cast<char>(Prefix::INTEGER));
    appendChars(n);
    appendCRLF();
}

void RespHandler::beginArray(const unsigned numElements)
{
    buffer.push_back(static_cast<char>(Prefix::ARRAY));
    buffer.push_back(char(numElements) + '0');
    appendCRLF();
}

void RespHandler::beginMap(const unsigned numElements)
{
    buffer.push_back(static_cast<char>(Prefix::MAP));
    buffer.push_back(char(numElements) + '0');
    appendCRLF();
}

void RespHandler::appendKV(const std::string_view key, const std::string_view val)
{
    appendBulkstring(key);
    appendBulkstring(val);
}
void RespHandler::appendKV(const std::string_view key, const int val)
{
    appendBulkstring(key);
    appendInt(std::to_string(val));
}

std::pair<size_t, RedisRespRes> RespHandler::decodeSimpleString(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");

    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    return { crlfPos, RedisRespRes { .string_ = str.substr(1, crlfPos - 1) } };
}

std::pair<size_t, RedisRespRes> RespHandler::decodeBulkString(const std::string_view str)
{
    const auto lengthDelim = str.find_first_of("\r\n");
    if (lengthDelim == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    if (str.substr(1, lengthDelim - 1) == "-1") {
        return { lengthDelim, RedisRespRes { .string_ = "null" } };
    }

    int length {};
    std::from_chars_result result = std::from_chars(str.data() + 1, str.data() + lengthDelim, length);
    if (result.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[lengthDelim] } };
    }

    const auto endDelim = lengthDelim + 2 + length;
    if (str[endDelim] != '\r') {
        throw std::invalid_argument { "Invalid format. End delimiter should come after the string. Length mismatch?" };
    }
    if (endDelim == lengthDelim) {
        throw std::invalid_argument { "Invalid format. End delimiter same as length delimiter" };
    }
    return { endDelim, RedisRespRes { .string_ = str.substr(lengthDelim + 2, length) } };
}

std::pair<size_t, RedisRespRes> RespHandler::decodeError(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }

    return { crlfPos, RedisRespRes { .error_ = str.substr(1, crlfPos - 1) } };
}

std::pair<size_t, RedisRespRes> RespHandler::decodeInt(const std::string_view str)
{
    const auto crlfPos = str.find("\r\n");
    if (crlfPos == std::string::npos) {
        throw std::invalid_argument { "Missing CRLF in Simple String" };
    }
    std::string_view decodedInt = str.substr(1, crlfPos - 1);
    // TODO: Verify value is integer with regex
    return { crlfPos, RedisRespRes { .integer_ = std::stoi(decodedInt.data()) } };
}

std::pair<size_t, RedisRespRes> RespHandler::decodeArray(const std::string_view str)
{
    const auto arrLenDel = str.find_first_of("\r\n");
    int arrLen {};
    std::from_chars_result convertArrLenRes = std::from_chars(str.data() + 1, str.data() + arrLenDel, arrLen);
    if (convertArrLenRes.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[arrLenDel] } };
    }

    auto startPos = arrLenDel + 2;
    std::vector<RedisRespRes> result {};
    for (int i = 0; i < arrLen; ++i) {
        const auto decodedVal = decode(str.substr(startPos));
        result.push_back(decodedVal.second);
        startPos += decodedVal.first + 2;
    }
    return { startPos, RedisRespRes { .array_ = result } };
}

std::pair<size_t, RedisRespRes> RespHandler::decodeMap(const std::string_view str)
{
    const auto mapLenDel = str.find_first_of("\r\n");
    int mapLen {};
    std::from_chars_result convertArrLenRes = std::from_chars(str.data() + 1, str.data() + mapLenDel, mapLen);
    if (convertArrLenRes.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "Invalid length of bulk string. lengthDelim: " + std::string { str[mapLenDel] } };
    }

    auto startPos = mapLenDel + 2;
    std::map<std::string_view, RedisRespRes> result {};
    for (int i = 0; i < mapLen; ++i) {
        const auto decodedKey = decode(str.substr(startPos));
        std::string_view key {};
        if (decodedKey.second.string_.has_value()) {
            key = decodedKey.second.string_.value();
        } else {
            std::cout << "Expected key as string, got: " << decodedKey.second;
            assert(false);
        }
        startPos += decodedKey.first + 2;
        const auto decodedVal = decode(str.substr(startPos));
        result[key] = decodedVal.second;
    }
    return { startPos, RedisRespRes { .map_ = result } };
}

std::pair<size_t, RedisRespRes> RespHandler::decode(const std::string_view str)
{
    // TODO: Implement sets
    std::cout << __PRETTY_FUNCTION__ << " str= " << str;
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
    case Prefix::MAP:
        return decodeMap(str);
    }
    return { -1, RedisRespRes {} };
}

const std::vector<char>& RespHandler::getBuffer() const { return buffer; }

Command RespHandler::parseRawCommand(const std::string_view rawCommand)
{
    std::cout << "RawCommand: " << rawCommand << ".\n";
    if (rawCommand == "PING") {
        return Command { .kind_ = CommandKind::PING };
    }

    if (rawCommand == "HELLO") {
        return Command { .kind_ = CommandKind::HELLO };
    }

    // TODO Use std::expected??
    return Command { .kind_ = CommandKind::INVALID_COMMAND_PARSE };
}

namespace {
PayloadT parsePayload(const RedisRespRes& payload)
{
    if (payload.string_.has_value()) {
        return payload.string_.value();
    } else if (payload.integer_.has_value()) {
        return payload.integer_.value();
    }
    // TODO: Use std::expected
    return 0;
}
}

Command RespHandler::parseRawArrayCommands(const std::vector<RedisRespRes>& commandArray)
{
    Command cmd {};
    const auto rawKind = commandArray[0].string_;
    if (rawKind == "HELLO") {
        cmd.kind_ = CommandKind::HELLO;
    } else if (rawKind == "SET") {
        cmd.kind_ = CommandKind::SET;
    } else if (rawKind == "GET") {
        cmd.kind_ = CommandKind::GET;
    } else if(rawKind == "PING")
    {
        cmd.kind_ = CommandKind::PING;
    }
    else {
        cmd.kind_ = CommandKind::INVALID_COMMAND_PARSE;
    }
    cmd.payload_ = std::vector<PayloadT> {};
    for (auto it = commandArray.begin() + 1; it != commandArray.end(); ++it)
    // for(const auto& rawCommand : commandArray | std::views::drop(1))
    {
        cmd.payload_->push_back(parsePayload(*it));
    }
    return cmd;
}

std::vector<Command> RespHandler::convertToCommands(const RedisRespRes& rawCommands)
{
    std::vector<Command> commands {};
    if (rawCommands.string_.has_value()) {
        commands.push_back(parseRawCommand(rawCommands.string_.value()));
    } else if (rawCommands.array_.has_value()) {
        commands.push_back(parseRawArrayCommands(rawCommands.array_.value()));
    } else {
        commands.push_back(Command { .kind_ = CommandKind::UNKNOWN_COMMAND });
    }
    return commands;
}