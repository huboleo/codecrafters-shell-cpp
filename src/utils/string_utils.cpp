#include "utils/string_utils.hpp"
#include <cctype>
#include <format>
#include <sstream>
#include <string>
#include <vector>

std::string string_utils::ltrim(const std::string& input) {
    auto start = input.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) {
        return "";
    }

    return input.substr(start);
}

std::string string_utils::rtrim(const std::string& input) {
    auto end = input.find_last_not_of(" \t\n\r\f\v");

    if (end == std::string::npos) {
        return "";
    }

    return input.substr(0, end + 1);
}

std::vector<std::string> string_utils::split_whitespace(const std::string& input) {
    std::stringstream ss(input);
    std::vector<std::string> parts;
    std::string word;

    while (ss >> word) {
        parts.push_back(word);
    }

    return parts;
}

std::string string_utils::surround_with_single_quotes(const std::string& input) {
    return std::format("'{}'", input);
}
