#include <iostream>
#include <string>

#include "ir.h"

int main() {
    vector<Instruction> instructions;
    for (std::string line; std::getline(std::cin, line);) {
        if (line.find("instr") != string::npos)
            instructions.emplace_back(line);
    }
    auto program = Program(instructions);
    std::cout << program.cfg() << std::endl;

    return 0;
}