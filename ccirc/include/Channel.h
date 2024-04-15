#pragma once

#include <string>

namespace Irc {
    enum class ChannelType {
            Unknown,
            Public,
            Private,
            Secret
        };


    struct Channel{
        ChannelType type = ChannelType::Unknown;
        std::string name{};
    };
}