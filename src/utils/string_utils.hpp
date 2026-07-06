#pragma once

#include <string>
#include <vector>

namespace string_utils {
std::string ltrim(const std::string& input);

std::vector<std::string> split_whitespace(const std::string& input);

} // namespace string_utils
