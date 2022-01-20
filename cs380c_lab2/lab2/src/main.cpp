#include <iostream>
#include <string>

#include "ir.h"

int main(int argc, char** argv) {
    std::vector<std::string> all_args;
    if (argc > 1) {
        all_args.assign(argv + 1, argv + argc);
    }
    bool do_dse = false;
    bool do_scp = false;
    bool do_rep = false;
    string backend;
    for (auto& s : all_args) {
        if (s.find("dse") != string::npos)
            do_dse = true;
        if (s.find("scp") != string::npos)
            do_scp = true;
        if (s.find("backend") != string::npos) {
            backend = s.substr(s.find('=') + 1);
        }
    }
    if (backend.find("rep") != string::npos) {
        do_rep = true;
    }

    vector<Instruction> instructions;
    for (std::string line; std::getline(std::cin, line);) {
        if (line.find("instr") != string::npos)
            instructions.emplace_back(line);
    }
    auto program = Program(instructions);
    if (do_scp) {
        program.scp();
        if (do_rep) program.scp_report();
    }
    if (do_dse) {
        program.dse();
        if(do_rep) program.dse_report();
    }
    if(backend[0]=='c'&&backend.size()==1)
        std::cout << program.ccode();
    else if(backend.find("cfg")!=string::npos)
        std::cout << program.cfg();
    else if(backend.find("3addr")!=string::npos)
        std::cout<<program.icode()<<std::endl;
    
    return 0;
}