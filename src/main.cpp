#include <cstdlib>
#include <cwctype>
#include <iostream>
#include <optional>
#include <print>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

std::string ltrim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) {
        return "";
    }

    return str.substr(start);
}

std::vector<std::string> split(const std::string& input) {
    std::stringstream ss(input);
    std::vector<std::string> parts;
    std::string word;

    while (ss >> word) {
        parts.push_back(word);
    }

    return parts;
}

std::vector<std::string> get_path_directories() {
    auto dirs = std::vector<std::string>();
    const char* path = std::getenv("PATH");
    if (path == nullptr) {
        return dirs;
    }

    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        dirs.push_back(dir);
    }

    return dirs;
}

std::optional<std::string> find_executable(const std::string& command) {
    for (const auto& dir : get_path_directories()) {
        std::string candidate = dir + "/" + command;

        if (access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
    }

    return std::nullopt;
}

void run_executable(const std::string& path, const std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execv(path.c_str(), argv.data());
        std::exit(1);
    }

    waitpid(pid, nullptr, 0);
}

int main() {

    const auto shell_builtin_commands = std::unordered_set<std::string>{"exit", "echo", "type"};

    while (true) {
        std::print("$ ");
        std::string input;
        std::getline(std::cin, input);

        auto trimmed_input = ltrim(input);

        if (trimmed_input.empty()) {
            continue;
        }

        auto whitespace_pos = input.find_first_of(" \t\n\r\f\v");

        auto parts = split(trimmed_input);

        auto cmd = parts.at(0);

        if (cmd == "exit") {
            break;
        }

        if (cmd == "echo") {
            auto rest = trimmed_input.substr(trimmed_input.find("echo") + 4);
            std::println("{}", rest);
        } else if (cmd == "type") {
            auto program_name = parts.at(1);
            if (shell_builtin_commands.contains(program_name)) {
                std::println("{} is a shell builtin", program_name);
            } else {
                auto path = find_executable(program_name);
                if (path.has_value()) {
                    std::println("{} is {}", program_name, path.value());
                } else {
                    std::println("{}: not found", program_name);
                }
            }
        } else {
            auto program = find_executable(cmd);
            if (program.has_value()) {
                run_executable(program.value(), parts);
            } else {
                std::println("{}: command not found", cmd);
            }
        }
    }
}
