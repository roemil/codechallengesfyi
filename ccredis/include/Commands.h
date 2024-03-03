#pragma once

#include <string_view>
#include <variant>
#include <string>
#include <optional>

template <typename Cmd>
struct CommandBase
{
    constexpr bool isValid() {return isValid_;}

    bool isValid_{};
    std::string errorString{}; 
};

struct CommandUnknown : CommandBase<CommandUnknown> { };
struct CommandInvalid : CommandBase<CommandInvalid> { };
struct CommandPing : CommandBase<CommandPing> { };
struct CommandHello : CommandBase<CommandHello> {
    constexpr CommandHello() = default;
    std::string_view version_ {};
};
enum class CommandState
{
    WaitingForValue,
    Done
};
enum class ExpireTimeResolution
{
    Seconds,
    Milliseconds,
    Unix,
    UnixMilliseconds
};
struct CommandSet : CommandBase<CommandSet> {
    std::string_view key_;
    std::string_view value_ {}; // TODO: Allow multiple values
    std::optional<int> expire{};
    CommandState state_;
    ExpireTimeResolution resolution_;
};
struct CommandGet : CommandBase<CommandGet> {
    std::string_view key_;
};
struct CommandExists : CommandBase<CommandExists> {
    std::string_view key_;
};

using CommandVariant = std::variant<CommandUnknown, CommandInvalid, CommandPing, CommandHello, CommandSet, CommandGet, CommandExists>;