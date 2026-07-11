#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace string_utils {
std::string ltrim(const std::string& input);

std::string rtrim(const std::string& input);

std::vector<std::string> split_whitespace(const std::string& input);

std::string surround_with_single_quotes(const std::string& input);

std::string surround_with_double_quotes(const std::string& input);

std::optional<std::pair<std::string, std::string>>
split_variable_name_and_value(const std::string& input);

std::optional<int> to_int(const std::string& input);

bool validate_variable_name_string(const std::string& input);

} // namespace string_utils
