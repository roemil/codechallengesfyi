#pragma once

#include <string>
#include <memory>
#include <vector>
#include <regex>

namespace json
{
class Lexer
{
    public:
        Lexer(const std::string& filename);
        void lex(const std::string& str);

        bool isValid();
        std::unique_ptr<std::vector<std::string>> getTokens();

        static bool inline isNumber(const std::string& input)
        {
            // Regular expression to match valid numbers
            std::regex numberPattern("^[-+]?(0|([1-9]\\d*\\.?\\d*)|0?\\.\\d+)([eE][-+]?\\d+)?$");

            // Check if the input matches the pattern
            return std::regex_match(input, numberPattern);
        }

    private:
        std::unique_ptr<std::vector<std::string>> tokens{nullptr};
        bool isValidJson{};
};
}