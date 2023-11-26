#pragma once

#include <string>
#include <memory>
#include <queue>

namespace json {
    struct parser
    {
        //void readFile(const std::string&);
        void isValidJson(const std::string& filename);

        std::unique_ptr<std::queue<std::string>> content = nullptr;
        unsigned validateArray(const std::vector<std::string>& tokens, unsigned index);
        unsigned validateObject(const std::vector<std::string>& tokens, unsigned index);
        int depth{};
    };
}