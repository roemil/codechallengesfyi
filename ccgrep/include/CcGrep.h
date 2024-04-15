#include <string_view>
#include <string>
#include <filesystem>

std::string grep(const std::string& regexp, const std::filesystem::path& file);
std::string grep_recursive(const std::string& regexp, const std::filesystem::path& file);
std::string grep_inverted(const std::string& regexp, const std::filesystem::path& file);