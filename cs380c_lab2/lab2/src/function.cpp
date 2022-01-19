#include <algorithm>

#include "ir.h"
void Function::scan_local_variables(vector<Instruction>& instrs) {
    for (const auto& inst : instrs) {
        for (const auto& operand : inst.operands) {
            if (operand.type == Operand::Type::LOCAL_VARIABLE) {
                this->local_variables.emplace_back(operand.variable_name, operand.offset);
            } else if (operand.type == Operand::Type::LOCAL_ADDR) {
                this->local_variables.emplace_back(operand.variable_name, operand.offset);
            }
        }
    }

    //sort and unique
    sort(local_variables.begin(), local_variables.end());
    auto iter1 = std::unique(local_variables.begin(), local_variables.end());
    local_variables = vector<Variable>(local_variables.begin(), iter1);

    //calculate the size of local variables
    if (!local_variables.empty()) {
        int i = 0;
        for (; i < local_variables.size() - 1; ++i) {
            local_variables[i].size = local_variables[i + 1].address - local_variables[i].address;
        }
        local_variables[i].size = -local_variables[i].address;
    }

    std::reverse(local_variables.begin(), local_variables.end());
}

void Function::scan_parameters(vector<Instruction>& instrs) {
    for (const auto& inst : instrs) {
        for (const auto& operand : inst.operands) {
            if (operand.type == Operand::Type::PARAMETER) {
                this->params.emplace_back(operand.variable_name, operand.offset);
            }
        }
    }
    //sort and unique
    sort(params.begin(), params.end());
    auto iter2 = std::unique(params.begin(), params.end());
    params = vector<Variable>(params.begin(), iter2);

    //parameter's size is 8 == default value of type Variable's size

    // Reverse the vector so that the elements are in the order they are declared
    std::reverse(params.begin(), params.end());
}

Function::Function(vector<Instruction>& instrs, bool _is_main)
    : is_main(_is_main), id(0) {
    assert(instrs[0].opcode.type == Opcode::Type::ENTER);
    this->local_var_size = instrs[0].operands[0].constant;
    this->id = instrs[0].label;
    assert(instrs.back().opcode.type == Opcode::Type::RET);
    this->param_size = instrs.back().operands[0].constant;
    this->scan_local_variables(instrs);
    this->scan_parameters(instrs);
    this->scan_block_leaders(instrs);

    vector<Instruction> tmp = {};
    for (auto& inst : instrs) {
        if (inst.is_block_leader && !tmp.empty()) {
            basic_blocks.emplace_back(tmp);
            tmp = {};
        }
        tmp.push_back(inst);
    }
    if (!tmp.empty())
        basic_blocks.emplace_back(tmp);

    for (int i = 0; i < basic_blocks.size(); i++) {
        idx_of_bb[basic_blocks[i].first_label()] = i;
    }
    //fixme
    //this->instructions = instrs;

#ifdef FUNCTION_DEBUG
    std::cout << "function" << this->id << std::endl;
    std::cout << "--params---" << std::endl;
    for (const auto& v : this->params) {
        std::cout << v.variable_name << " " << v.size << " " << std::endl;
    }
    std::cout << "--local variables---" << std::endl;
    for (const auto& v : this->local_variables) {
        std::cout << v.variable_name << " " << v.size << " " << std::endl;
    }
#endif
}

string Function::ccode() const {
    std::stringstream tmp;
    if (is_main) {
        tmp << "void main(";
    } else {
        tmp << "void function_" << id << "(";
    }
    // function signature
    for (int i = 0; i < params.size(); i++) {
        tmp << "long " << params[i].variable_name;
        if (params.size() > 1 && i < params.size() - 1) {
            tmp << ",";
        }
    }
    tmp << "){";
    tmp << std::endl;
    // declare local variables
    for (auto& v : local_variables) {
        tmp << "  long " << v.variable_name;
        if (v.size > 8)
            tmp << "[" << v.size / 8 << "]";
        tmp << ";";
        tmp << std::endl;
    }

    for (auto& bb : basic_blocks) {
        tmp << bb.ccode() << std::endl;
    }

    tmp << "}";
    return tmp.str();
}

void Function::scan_block_leaders(vector<Instruction>& instrs) {
    const int n = instrs.size();
    instrs.front().is_block_leader = true;
    for (int i = 0; i < n; i++) {
        if (instrs[i].is_branch()) {
            // a branch instruction cannot be the last instruction of a function
            assert(i + 1 < n);
            instrs[i + 1].is_block_leader = true;
            long long target_label = instrs[i].branch_target_label();
            long long own_label = instrs[i].label;
            long long target_index = target_label - own_label + i;
            //the target instruction must be in the same function
            assert(target_index >= 0 && target_index < n);
            assert(instrs[target_index].label == target_label);
            instrs[target_index].predecessor_labels.push_back(own_label);
            instrs[target_index].is_block_leader = true;
        } else if (instrs[i].opcode.type == Opcode::Type::CALL) {
            // the last instruction must be a ret
            // so a call instruction must not be the last one
            assert(i + 1 < n);
            instrs[i + 1].is_block_leader = true;
        }
    }
#ifdef BASIC_LEADER_DEBUG
    std::cout << "basic block leaders:" << std::endl;
    std::cout << "      ";
    for (auto& inst : instrs) {
        if (inst.is_block_leader)
            std::cout << inst.label << " ";
    }
    std::cout << std::endl;
#endif
}
string Function::icode() const {
    std::stringstream tmp;
    for (auto& bb : basic_blocks) {
        tmp << bb.icode();
    }
    return tmp.str();
}
string Function::cfg() const {
    std::stringstream tmp;
    tmp << "Function: " << this->id << std::endl;
    tmp << "Basic blocks:";
    for (auto& bb : basic_blocks) {
        tmp << " " << bb.instructions.front().label;
    }
    tmp << std::endl;
    tmp << "CFG:" << std::endl;
    for (auto& bb : basic_blocks) {
        tmp << bb.cfg();
    }
    return tmp.str();
}
void Function::scp() {
    vector<string> object_def_by_inst{};
    // The index of all instructions in object_def_by_inst : label - label_0
    const auto label_0 = basic_blocks.front().first_label();
    for (const auto& bb : basic_blocks) {
        for (const auto& inst : bb.instructions) {
            object_def_by_inst.push_back(inst.get_def());
        }
    }

    // gens,kills,globals set of basicblocks;
    // If there is a function call inside the basic block
    // F(x) = (GEN(B) - GLOBALS(B)) U (x - KILL(B))
    // else global will be empty
    vector<unordered_set<int>> gens, kills, globals;
    for (auto& bb : basic_blocks) {
        gens.emplace_back();
        auto& gen = gens.back();
        kills.emplace_back();
        auto& kill = kills.back();
        globals.emplace_back();
        auto& global = globals.back();
        bool consider_global = bb.instructions.back().opcode.type == Opcode::Type::CALL;
        for (auto& inst : bb.instructions) {
            if (inst.is_def()) {
                gen.insert(inst.label);
                auto object_def = inst.get_def();
                auto own_idx = inst.label - label_0;  // the index of this instruction in object_def_by_inst
                for (int i = 0; i < object_def_by_inst.size(); i++) {
                    if (i == own_idx)
                        continue;
                    if (object_def_by_inst[i] == object_def)
                        kill.insert(i + label_0);
                }
                // Only the move instruction will def global variables
                if (consider_global && inst.opcode.type == Opcode::Type::MOVE && inst.operands.back().type == Operand::Type::GLOBAL_VARIABLE) {
                    global.insert(inst.label);
                }
            }
        }
    }

    auto bb_cnt = basic_blocks.size();
    vector<unordered_set<int>> in(bb_cnt), out(bb_cnt);  // IN set and OUT set of basic blocks
    deque<int> changed;                                  //blocks changed in the last iteration
    for (auto i = 0; i < bb_cnt; i++) {
        changed.push_back(i);
    }

    while (!changed.empty()) {
        auto n = changed.front();
        changed.pop_front();
        in[n].clear();

        for (auto pre_label : basic_blocks[n].predecessor_labels) {
            in[n].insert(out[idx_of_bb[pre_label]].begin(), out[idx_of_bb[pre_label]].end());
        }

        auto out_n_copy = out[n];
        out[n].clear();
        // out = gen-global
        for (auto i : gens[n]) {
            if (globals[n].count(i) == 0)
                out[n].insert(i);
        }
        // out = out \cup (in-kill)
        for (auto i : in[n]) {
            if (kills[n].count(i) == 0)
                out[n].insert(i);
        }
        // if out changed, successor basic blocks' out need to be recomputed
        if (out_n_copy != out[n]) {
            for (auto s : basic_blocks[n].successor_labels) {
                changed.push_back(idx_of_bb[s]);
            }
            sort(changed.begin(), changed.end());
            auto iter = unique(changed.begin(), changed.end());
            changed.erase(iter, changed.end());
        }
    }
    
}