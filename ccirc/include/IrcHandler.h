#pragma once

#include "IrcParser.h"

#include <stdexcept>
#include <string_view>

class IrcHandler
{
    public:
        static auto parse(const std::string_view data) {
            const auto delimiterPos = data.find(":");
            if(delimiterPos == std::string_view::npos) {
                throw std::invalid_argument{"Delimiter missing"};
            }

            const auto crlfPos = data.find("\r\n");
            if(crlfPos == std::string_view::npos) {
                throw std::invalid_argument{"CRLF missing"};
            }

            const auto parseResult = IrcParser::parse(data);
            return IrcParser::createMessage(parseResult, data);
        }
};