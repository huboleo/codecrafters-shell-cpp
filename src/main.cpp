#include <iostream>
#include <print>
#include <string>

int main() {
    while (true) {
        std::print("$ ");
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (input.substr(0, 5) == "echo ") {
            std::println("{}", input.substr(5));
            continue;
        }

        std::println("{}: command not found", input);
    }
}
