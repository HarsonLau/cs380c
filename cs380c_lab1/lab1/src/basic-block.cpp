#include "ir.h"
#include <algorithm>
BasicBlock::BasicBlock(vector<Instruction>& instrs) : instructions(instrs), predecessor_labels(instrs.front().predecessor_labels), successor_labels({}) {
    if (instrs.back().is_branch()) {
        this->successor_labels.push_back(instrs.back().operands.back().inst_label);
    }
    if(instrs.back().opcode.type!=Opcode::Type::BR&&instrs.back().opcode.type!=Opcode::Type::RET){
        this->successor_labels.push_back(instrs.back().label+1);
    }
    sort(successor_labels.begin(),successor_labels.end());
    auto iter = std::unique(successor_labels.begin(),successor_labels.end());
    successor_labels=vector<long long>(successor_labels.begin(),iter);

}

string BasicBlock::ccode() {
    std::stringstream tmp;
    for (auto& inst : instructions) {
        tmp << "  " << inst.ccode() << std::endl;
    }
    return tmp.str();
}
string BasicBlock::icode(){
    std::stringstream tmp;
    for(auto & inst:instructions){
        tmp<<inst.icode();
    }
    return tmp.str();
}
string BasicBlock::cfg(){
    std::stringstream tmp;
    tmp<<this->instructions.front().label<<" ->";
    for(auto suc:successor_labels){
        tmp<<" "<<suc;
    }
    tmp<<std::endl;
    return tmp.str();
}