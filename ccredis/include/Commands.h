#pragma once

#include <string_view>
#include <variant>

struct CommandUnknown { };
struct CommandInvalid { };
struct CommandPing { };
struct CommandHello {
    constexpr CommandHello() = default;
    std::string_view version_ {};
};
struct CommandSet {
    std::string_view key_;
    std::string_view value_ {}; // TODO: Allow multiple values
};
struct CommandGet {
    std::string_view key_;
};

using CommandVariant = std::variant<CommandUnknown, CommandInvalid, CommandPing, CommandHello, CommandSet, CommandGet>;