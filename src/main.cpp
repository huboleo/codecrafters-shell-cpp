#include <string>
#include <print>
#include <iostream>

int main() {
    std::print("$ ");
    std::string command;
    std::getline(std::cin, command);
    std::println("{}: command not found", command);    
}
