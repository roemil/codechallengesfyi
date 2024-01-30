#pragma once

#include <string>
#include <memory>
#include <queue>
#include <vector>
#include <map>

namespace json {
    struct ParseResult {
        std::vector<ParseResult> array_{};
        std::map<std::string, ParseResult> map_{};
        std::string val_{};
    };
    struct parser
    {
        void isValidJson(const std::string& filename);

        unsigned validateArray(const std::vector<std::string>& tokens, unsigned index);
        unsigned validateObject(const std::vector<std::string>& tokens, unsigned index);
        int depth{};
    };
}