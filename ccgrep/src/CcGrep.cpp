#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <string_view>
#include <regex>
#include <cassert>
#include <filesystem>
#include <ranges>
#include <algorithm>

std::vector<std::string> readFromStdin()
{
    std::vector<std::string> result{};
    std::string line{};
    while (std::getline(std::cin, line))
    {
        result.push_back(line);
    }
    return result;
}

std::vector<std::string> readFromInput(const std::filesystem::path& file)
{
    std::vector<std::string> result{};
    std::ifstream infile{file};
    std::string line{};
    if(!file.empty())
    {
        assert(infile.is_open() && "Filepath must point to a valid file.");
    }
    if(!infile.is_open())
    {
        return readFromStdin();
    }
    while (std::getline(infile, line))
    {
        result.push_back(line);
    }
    return result;
}

std::string grep(const std::string& regexp, const std::filesystem::path& file)
{
    const std::regex r {regexp.empty() ? std::string{".*"} : regexp};

    std::string result{};
    std::smatch pieces_match;
    std::string line;
    for(const auto& line : readFromInput(file))
    {
        if(std::regex_search(line, pieces_match, r))
        {
            result += line + '\n';
        }
        
    }
    return result;
}

std::string grep_recursive(const std::string& regexp, const std::filesystem::path& file)
{
    const std::regex r {regexp.empty() ? std::string{".*"} : regexp};

    std::string result{};
    std::smatch pieces_match;
    std::string line;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator{file})
    {
        std::ifstream infile{dir_entry.path().generic_string()};
        assert(infile.is_open());
        while (std::getline(infile, line))
        {
        if(std::regex_search(line, pieces_match, r))
            {
                result += std::string{dir_entry.path()} + ":" + line + '\n';
            }
            
        }
    }
    return result;
}

std::string grep_inverted(const std::string& regexp, const std::filesystem::path& file)
{
    const std::regex r {regexp.empty() ? std::string{".*"} : regexp};

    std::string result{};
    std::smatch pieces_match;
    std::string line;
    for(const auto& line : readFromInput(file))
    {
        if(!std::regex_search(line, pieces_match, r))
        {
            result += line + '\n';
        }
        
    }
    return result;
}

std::string grep_caseinsensitive(const std::string& regexp, const std::filesystem::path& file)
{
    const std::regex r {regexp.empty() ? std::string{".*"} : regexp, std::regex_constants::icase};

    std::string result{};
    std::smatch pieces_match;
    std::string line;
    for(const auto& line : readFromInput(file))
    {
        if(std::regex_search(line, pieces_match, r))
        {
            result += line + '\n';
        }
        
    }
    return result;
}

int main(int argc, const char** argv)
{
    assert(argc > 1 && "Too few input parameters.");
    // TODO: Support path = *
    if(argc == 4)
    {
        auto flag = std::string{argv[1]};
        if(flag == "-r")
        {
            auto keyword = argv[2];
            auto path = std::string{argv[3]};
            const auto result = grep_recursive(keyword, path);
            if(result.empty())
            {
                return 1;
            }
            std::cout << result;
            return 0;
        }
        else if(flag == "-v")
        {
            auto keyword = argv[2];
            auto path = std::string{argv[3]};
            const auto result = grep_inverted(keyword, path);

            if(result.empty())
            {
                return 1;
            }
            std::cout << result;
            return 0;
        }
        else if(flag == "-i")
        {
            auto keyword = argv[2];
            auto path = std::string{argv[3]};
            const auto result = grep_caseinsensitive(keyword, path);

            if(result.empty())
            {
                return 1;
            }
            std::cout << result;
            return 0;
        }
        else
        {
            assert(false && "flag must be eithr -i, -v or -r");
        }
    }

    auto keyword = std::string{argv[1]};
    std::string flag;
    std::string path;
    if(std::string{argv[1]} == "-v")
    {
        flag = std::string{argv[1]};
        keyword = std::string{argv[2]};
    }
    else
    {
        if(argc == 3)
        {
            path = std::string{argv[2]};
        }
    }
    std::string result;
    if(flag == "-v")
    {
        result = grep_inverted(keyword, path);
    }
    else 
    {
        assert(flag.empty());
        result = grep(keyword, path);
    }
        
    if(result.empty())
    {
        return 1;
    }
    std::cout << result;
    return 0;

}
