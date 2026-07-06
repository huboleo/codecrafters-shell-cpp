#pragma once

#include <string>

enum class RedirectMode {
    Truncate,
    Append,
};

struct RedirectTarget {
    std::string path;
    RedirectMode mode;
};
