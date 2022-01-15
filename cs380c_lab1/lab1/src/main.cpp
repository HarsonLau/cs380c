#include <iostream>
#include <string>

#include "ir.h"
int test(int a, int b) {
    int* ptmp = &a;
    *ptmp = b;
    return a;
}
int main() {
    /*
    string ops[10]={"GP","FP","24","global_array_base#32576"," y_offset#8"," a#24","(12)","[48]"};
    for(int i=0;ops[i].size()>0;i++){
        auto operand=Operand(ops[i]);
    }
*/
    vector<string> raw_insts = {
        "instr 35: add x#-8 y#-16",
        "instr 33: add global_array_base#10024 GP",
        "instr 49: mul (48) 8",
        "instr 46: add p_base#32560 FP",
        "instr 43: load (42)",
        "instr 59: sub height#16 1",
        "instr 5: move a#24 c#-8",
        "instr 8: add a_base#32728 GP",
        "instr 9: add (8) x_offset#8"};
    for (const auto ri : raw_insts) {
        auto inst = Instruction(ri);
        std::cout << ri << std::endl;
        std::cout << inst.ccode() << std::endl;
        std::cout << "---" << std::endl;
    }
    /*
    vector<Instruction> instructions;
    for (std::string line; std::getline(std::cin, line);) {
        if (line.find("instr") != string::npos)
            instructions.emplace_back(line);
    }
    auto program = Program(instructions);
    int cnt=0;
    for(const auto & func: program.functions){
        for(const auto &inst:func.instructions){
            std::cout<<Opcode::opcode_name[inst.opcode.type]<<std::endl;
            cnt++;
        }
    }
    std::cout<<"cnt="<<cnt<<" program inst cnt="<<program.instructions.size()<<std::endl;
    */
    return 0;
}