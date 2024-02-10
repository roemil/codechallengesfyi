#pragma once

#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

enum class Prefix : char {
    SIMPLE_STRING = '+',
    ERROR = '-',
    INTEGER = ':',
    BULK_STRING = '$',
    ARRAY = '*',
    MAP = '%'
};

enum class CommandKind {
    UNKNOWN_COMMAND,
    INVALID_COMMAND_PARSE,
    PING,
    HELLO,
    SET,
    GET
};

struct CommandUnknown{};
struct CommandInvalid{};
struct CommandPing{};
struct CommandHello{
    constexpr CommandHello() = default;
    std::string_view version_{};
};
struct CommandSet{
    std::string_view key_;
    std::string_view value_{}; // TODO: Allow multiple values
};
struct CommandGet{
    std::string_view key_;
};


using CommandVariant = std::variant<CommandUnknown, CommandInvalid, CommandPing, CommandHello, CommandSet, CommandGet>;

using PayloadT = std::variant<int, std::string_view>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct Command {
    CommandKind kind_;
    std::optional<std::vector<PayloadT>> payload_ {};

    friend bool operator==(const Command& lhs, const Command& rhs)
    {
        const bool equalKind = lhs.kind_ == rhs.kind_;
        if (!equalKind) {
            return false;
        }
        // TODO: Fix comparison
        // if(lhs.payload_.has_value() && rhs.payload_.has_value())
        // {
        //     return lhs.payload_ == rhs.payload_;
        // }else {
        //     return false;
        // }
        return true;
    }
    friend std::ostream& operator<<(std::ostream& os, const Command& cmd)
    {
        os << "Kind: " << static_cast<int>(cmd.kind_) << "\n";
        os << "Payload? " << std::boolalpha << cmd.payload_.has_value() << "\n";
        if (cmd.payload_.has_value()) {
            for (const auto& elem : cmd.payload_.value()) {
                std::visit(overloaded {
                               [&os](int i) {
                                   os << "Payload(int): " << i << "\n";
                               },
                               [&os](std::string_view str) {
                                   os << "Payload(str): " << str << "/n";
                               } },
                    elem);
            }
        }
        return os;
    }
};

struct RedisRespRes {
    std::optional<int> integer_ {};
    std::optional<std::string_view> string_ {};
    std::optional<std::string_view> error_ {};
    std::optional<std::vector<RedisRespRes>> array_ {};
    std::optional<std::map<std::string_view, RedisRespRes>> map_ {};

    friend bool operator==(const RedisRespRes& lhs, const RedisRespRes& rhs)
    {
        return lhs.integer_ == rhs.integer_ && lhs.string_ == rhs.string_ && lhs.error_ == rhs.error_ && lhs.array_ == rhs.array_;
    }

    friend std::ostream& operator<<(std::ostream& os, const RedisRespRes& resp)
    {
        os << "Int: " << resp.integer_.value_or(-1) << "\n";
        os << "String: " << resp.string_.value_or("") << "\n";
        os << "Error: " << resp.error_.value_or("") << "\n";
        if (resp.array_.has_value()) {
            for (const auto& elem : resp.array_.value()) {
                os << elem;
            }
        }
        return os;
    }
};

struct ParsePayload {
    ParsePayload(const RedisRespRes& resp) : resp_(resp) {}
    const RedisRespRes& resp_{};

    ParsePayload(const ParsePayload&) = delete;
    ParsePayload operator=(const ParsePayload&) = delete;

    void operator()(CommandUnknown&);
    void operator()(CommandInvalid&);
    void operator()(CommandPing&);
    void operator()(CommandHello&);
    void operator()(CommandSet&);
    void operator()(CommandGet&);
};

class RespHandler {
public:
    void appendSimpleString(const std::string_view str);
    void appendError(const std::string_view str);
    void appendInt(const std::string_view n);
    void appendBulkstring(const std::string_view str);
    void appendNull();
    void beginArray(const unsigned numElements);
    void beginMap(const unsigned numElements);
    void appendKV(const std::string_view key, const std::string_view val);
    void appendKV(const std::string_view key, const int val);

    [[nodiscard]] static std::pair<size_t, RedisRespRes> decode(const std::string_view str);

    static std::vector<CommandVariant> convertToCommands(const RedisRespRes& rawCommands);

    const std::vector<char>& getBuffer() const;

private:
    void appendCRLF();
    void appendChars(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeSimpleString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeError(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeInt(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeBulkString(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeArray(const std::string_view str);
    static std::pair<size_t, RedisRespRes> decodeMap(const std::string_view str);
    // TODO: Replace with a output stringstream?
    std::vector<char> buffer {};

    static CommandVariant parseRawCommand(const std::string_view rawCommand);
    static CommandVariant parseRawArrayCommands(const std::vector<RedisRespRes>& commandArray);
};

struct HandleCommand {
    HandleCommand(RespHandler& rh) : rh_(rh) {}
    HandleCommand(const HandleCommand&) = delete;
    HandleCommand operator=(const HandleCommand&) = delete;

    RespHandler& rh_;
    void operator()(const CommandUnknown&) const;
    void operator()(const CommandInvalid&) const;
    void operator()(const CommandPing&) const;
    void operator()(const CommandHello&) const;
    void operator()(const CommandSet&) const;
    void operator()(const CommandGet&) const;
};