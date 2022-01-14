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
    auto i1=Instruction("instr 33:   add   global_array_base#10024   GP");
    std::cout<<"---"<<std::endl;
    auto i2=Instruction("instr 49: mul (48) 8");
    std::cout<<"---"<<std::endl;
    auto i3=Instruction("	instr 46: add p_base#32560 GP");
    std::cout<<"---"<<std::endl;
    auto i4 = Instruction("instr 43: load (42)");
    std::cout<<"---"<<std::endl;
    auto i5 = Instruction("fadf 231:   nop ");
    return 0;
}