#include <string>
#include <print>
#include <iostream>

int main() {
    bool shouldRun = true;
    while (shouldRun) {
        std::print("$ ");
        std::string command;
        std::getline(std::cin, command);
        std::println("{}: command not found", command);    
    }
    
}
