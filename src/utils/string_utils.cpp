#include "utils/string_utils.hpp"
#include <cctype>
#include <charconv>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
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

std::optional<int> string_utils::to_int(const std::string& input) {
    if (input.empty()) {
        return std::nullopt;
    }

    int value;

    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), value);

    if (ec != std::errc{} || ptr != input.data() + input.size()) {
        return std::nullopt;
    }

    return value;
}

std::string string_utils::surround_with_double_quotes(const std::string& input) {
    return std::format("\"{}\"", input);
}

std::optional<std::pair<std::string, std::string>>
string_utils::split_variable_name_and_value(const std::string& input) {
    if (input.empty()) {
        return std::nullopt;
    }

    auto equal_sing_position = input.find_first_of('=');

    if (equal_sing_position == std::string::npos) {
        return std::nullopt;
    }

    std::pair<std::string, std::string> result;

    result.first = input.substr(0, equal_sing_position);
    result.second = input.substr(equal_sing_position + 1);

    return result;
}

bool string_utils::validate_variable_name_string(const std::string& input) {
    if (input.empty()) {
        return false;
    }
    for (size_t i{0}; i < input.size(); ++i) {
        unsigned char character = static_cast<unsigned char>(input[i]);

        if (i == 0) {
            if (character != '_' && !std::isalpha(character)) {
                return false;
            }
            continue;
        }

        if (character != '_' && !std::isalnum(character)) {
            return false;
        }
    }

    return true;
}
