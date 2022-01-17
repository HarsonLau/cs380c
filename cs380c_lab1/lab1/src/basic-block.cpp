#include "ir.h"

Basic_block::Basic_block(vector<Instruction>& instrs) : instructions(instrs), predecessor_labels(instrs.front().predecessor_labels), successor_labels({instrs.back().label + 1}) {
    if(instrs.back().is_branch()){
        this->successor_labels.push_back(instrs.back().label+1);
    }
}

string Basic_block::ccode(){
    return "";
}