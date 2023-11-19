#pragma once

#include <string>
#include <memory>
#include <queue>

namespace json {
    struct parser
    {
        //void readFile(const std::string&);
        void isValidJson(const std::string& filename) const;

        std::unique_ptr<std::queue<std::string>> content = nullptr;
    };
}