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

        std::println("{}: command not found", input);
    }
}
