#include <algorithm>

#include "ir.h"
BasicBlock::BasicBlock(vector<Instruction>& instrs) : instructions(instrs), predecessor_labels(instrs.front().predecessor_labels), successor_labels({}) {
    if (instrs.back().is_branch()) {
        this->successor_labels.push_back(instrs.back().operands.back().inst_label);
    }
    if (instrs.back().opcode.type != Opcode::Type::BR && instrs.back().opcode.type != Opcode::Type::RET) {
        this->successor_labels.push_back(instrs.back().label + 1);
    }
    sort(successor_labels.begin(), successor_labels.end());
    auto iter = std::unique(successor_labels.begin(), successor_labels.end());
    successor_labels = vector<long long>(successor_labels.begin(), iter);
    peephole();
    assert(last_label()-first_label()==size()-1);
}

string BasicBlock::ccode() const {
    std::stringstream tmp;
    for (auto& inst : instructions) {
        tmp << "  " << inst.ccode() << std::endl;
    }
    return tmp.str();
}
string BasicBlock::icode() const {
    std::stringstream tmp;
    for (auto& inst : instructions) {
        tmp << inst.icode();
    }
    return tmp.str();
}
string BasicBlock::cfg() const {
    std::stringstream tmp;
    tmp << this->instructions.front().label << " ->";
    for (auto suc : successor_labels) {
        tmp << " " << suc;
    }
    tmp << std::endl;
    return tmp.str();
}
/*
from:
    instr 70: add max_base#32728 GP
    instr 71: load (70)
to:
    instr 70: nop
    instr 71: assign max

from:
    instr 68: add maxi_base#32744 GP
    instr 69: store (67) (68)
to:
    instr 68: nop 
    instr 69: move (67) maxi
*/
void BasicBlock::peephole() {
    if (instructions.size() < 2)
        return;
    for (auto iter = instructions.begin(); iter != instructions.end() - 1; ++iter) {
        auto next_iter = iter + 1;
        if (iter->opcode.type == Opcode::Type::ADD && (iter->operands[1].type == Operand::Type::FP || iter->operands[1].type == Operand::Type::GP)) {
            if (next_iter->operands.back().type == Operand::Type::REG && next_iter->operands.back().reg_name == iter->label) {
                if (next_iter->opcode.type == Opcode::Type::LOAD) {
                    next_iter->opcode.type = Opcode::Type::ASSIGN;
                    next_iter->operands[0] = iter->operands[0];
                    assert(iter->operands[0].type == Operand::Type::GLOBAL_ADDR);
                    next_iter->operands[0].type = Operand::Type::GLOBAL_VARIABLE;
                    iter->to_nop();
                } else if (next_iter->opcode.type == Opcode::Type::STORE) {
                    next_iter->opcode.type = Opcode::Type::MOVE;
                    next_iter->operands[1] = iter->operands[0];
                    assert(iter->operands[0].type == Operand::Type::GLOBAL_ADDR);
                    next_iter->operands[1].type = Operand::Type::GLOBAL_VARIABLE;
                    iter->to_nop();
                }
            }
        }
    }
}

long long BasicBlock::last_label() const {
    return instructions.back().label;
}

long long BasicBlock::first_label() const {
    return instructions.front().label;
}

long long BasicBlock::size() const {
    return instructions.size();
}

void BasicBlock::peephole2(){
    for(auto & inst:instructions){
        inst.peephole2();
    }
}