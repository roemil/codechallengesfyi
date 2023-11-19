#pragma once

#include <string>
#include <memory>
#include <vector>

namespace json
{
class Lexer
{
    public:
        Lexer(const std::string& filename);
        void lex(const std::string& str);

        bool isValid();
        std::unique_ptr<std::vector<std::string>> getTokens();

    private:
        std::unique_ptr<std::vector<std::string>> tokens{nullptr};
        bool isValidJson{};
};
}