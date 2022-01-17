#include "ir.h"

BasicBlock::BasicBlock(vector<Instruction>& instrs) : instructions(instrs), predecessor_labels(instrs.front().predecessor_labels), successor_labels({instrs.back().label + 1}) {
    if (instrs.back().is_branch()) {
        this->successor_labels.push_back(instrs.back().label + 1);
    }
}

string BasicBlock::ccode() {
    std::stringstream tmp;
    for (auto& inst : instructions) {
        tmp << "  " << inst.ccode() << std::endl;
    }
    return tmp.str();
}