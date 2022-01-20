#include <algorithm>

#include "ir.h"
void Program::scan_global_variables(vector<Instruction>& instrs) {
    // Scan all instructions in turn,
    // and save the global variables that appear in the instructions to the vector,
    // so that the addresses are arranged from low to high*/
    for (const auto& inst : instrs) {
        if (inst.operands.size() == 2 && inst.operands[1].type == Operand::Type::GP) {
            assert(inst.opcode.type == Opcode::Type::ADD);
            assert(inst.operands[0].type == Operand::Type::GLOBAL_ADDR);
            global_variables.emplace_back(inst.operands[0].variable_name, inst.operands[0].offset);
        }
    }

    // unique
    std::sort(global_variables.begin(), global_variables.end());
    auto iter = std::unique(global_variables.begin(), global_variables.end());
    global_variables = vector<Variable>(global_variables.begin(), iter);

    // calculate the variable size
    if (!global_variables.empty()) {
        int i = 0;
        for (; i < global_variables.size() - 1; i++) {
            global_variables[i].size = global_variables[i + 1].address - global_variables[i].address;
        }
        global_variables[i].size = 32768 - global_variables[i].address;
    }

    // Reverse the vector so that the elements are in the order they are declared
    std::reverse(global_variables.begin(), global_variables.end());
}

Program::Program(vector<Instruction>& insts) : instruction_cnt(insts.size()), global_variables({}), functions({}) {
    // scan for global variables
    this->scan_global_variables(insts);
    // Divide the entire program into several functions for further processing
    bool _is_main = false;
    vector<Instruction> tmp = {};
    for (const auto& inst : insts) {
        if (inst.opcode.type == Opcode::Type::ENTRYPC) {
            _is_main = true;
            continue;
        }
        if (tmp.empty() && inst.opcode.type == Opcode::Type::NOP)
            continue;
        tmp.push_back(inst);
        if (inst.opcode.type == Opcode::RET) {
            functions.emplace_back(tmp, _is_main);
            _is_main = false;
            tmp = {};
        }
    }
#ifdef PROGRAM_DEBUG
    std::cout << "program" << std::endl;
    std::cout << "--global variables---" << std::endl;
    for (const auto& v : this->global_variables) {
        std::cout << v.variable_name << " " << v.size << " " << std::endl;
    }
#endif
}

string Program::ccode()const {
    std::stringstream tmp;
    tmp << "#include <stdio.h>" << std::endl;
    tmp << "#define long long long" << std::endl;
    tmp << "#define WriteLine() printf(\"\\n\");" << std::endl;
    tmp << "#define WriteLong(x) printf(\" %lld\", (long)x);" << std::endl;
    tmp << "#define ReadLong(a) if (fscanf(stdin, \"%lld\", &a) != 1) a = 0;" << std::endl;
    tmp << "long REG[" << this->instruction_cnt + 4 << "];" << std::endl;

    for (auto& v : global_variables) {
        tmp << "long " << v.variable_name;
        if (v.size > 8)
            tmp << "[" << v.size / 8 << "]";
        tmp << ";";
        tmp << std::endl;
    }
    for (auto& f : functions) {
        tmp << f.ccode() << std::endl;
    }
    return tmp.str();
}
string Program::icode() const{
    std::stringstream tmp;
    for (auto& func : functions) {
        tmp << func.icode();
    }
    return tmp.str();
}
string Program::cfg() const{
    std::stringstream tmp;
    for (auto& func : functions) {
        tmp << func.cfg();
    }
    return tmp.str();
}
void Program::scp(){
    for(auto & func:functions){
        func.scp_peephole();
    }
}
void Program::dse(){
    for(auto &func:functions){
        func.dse();
    }
}
void Program::scp_report()const{
    for (const auto & func:functions){
        std::cout<<"Function: "<<func.id<<std::endl;
        std::cout<<"Number of constants propagated: "<<func.constant_propagated_cnt<<std::endl;
    }
}
void Program::dse_report()const{
    for (const auto & func:functions){
        std::cout<<"Function: "<<func.id<<std::endl;
        std::cout<<"Number of statements eliminated: "<<func.statement_eliminated_cnt<<std::endl;
    }
}