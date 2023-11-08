#pragma once

#include <string>
#include <fstream>
#include <memory>

struct Json
{
    void readFile(const std::string&);
    bool isValidJson() const;

    std::string str{};
    std::unique_ptr<std::string> content = nullptr;
};