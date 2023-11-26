#pragma once

#include <string>
#include <memory>
#include <queue>

namespace json {
    struct parser
    {
        void isValidJson(const std::string& filename);

        unsigned validateArray(const std::vector<std::string>& tokens, unsigned index);
        unsigned validateObject(const std::vector<std::string>& tokens, unsigned index);
        int depth{};
    };
}