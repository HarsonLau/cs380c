#include <iostream>
#include <string>

#include "ir.h"

int main() {
    /*
    string ops[10]={"GP","FP","24","global_array_base#32576"," y_offset#8"," a#24","(12)","[48]"};
    for(int i=0;ops[i].size()>0;i++){
        auto operand=Operand(ops[i]);
    }
*/
   /* vector<string> raw_insts = {
        "instr 2: enter 8",
        "instr 3: cmpeq b#16 0",
        "instr 4: blbs (3) [14]",
        "instr 5: move a#24 c#-8",
        "instr 6: move b#16 a#24",
        "instr 7: mod c#-8 b#16",
        "instr 8: move (7) b#16",
        "instr 9: write c#-8",
        "instr 10: write a#24",
        "instr 11: write b#16",
        "instr 12: wrl",
        "instr 13: br [3]",
        "instr 14: add res_base#32744 GP",
        "instr 15: store a#24 (14)",
        "instr 11: add m1_base#-96 FP",
        "instr 17: nop",
        "instr 16: ret 16"};
    vector<Instruction> insts = {};
    for (const auto ri : raw_insts) {
        auto inst = Instruction(ri);
        std::cout << ri << std::endl;        
        insts.push_back(inst);
    }
    auto f1 = Function(insts);
    std::cout<<f1.ccode()<<std::endl;
    */
    /*
    for(const auto & v:f1.local_variables){
        std::cout<< v.variable_name<<" "<<v.size<<" "<<std::endl;
    }
    std::cout<<"-----"<<std::endl;
    for(const auto & v:f1.params){
        std::cout<< v.variable_name<<" "<<v.size<<" "<<std::endl;
    }*/


    vector<Instruction> instructions;
    for (std::string line; std::getline(std::cin, line);) {
        if (line.find("instr") != string::npos)
            instructions.emplace_back(line);
    }
    auto program = Program(instructions);
    std::cout<<program.ccode()<<std::endl;

    return 0;
}