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
    : is_main(_is_main), id(0), constant_propagated_cnt(0), statement_eliminated_cnt(0) {
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
    for (const auto& bb : basic_blocks) {
        for (auto s : bb.successor_labels) {
            basic_blocks[idx_of_bb[s]].predecessor_labels.push_back(bb.first_label());
        }
    }
    for (auto& bb : basic_blocks) {
        sort(bb.predecessor_labels.begin(), bb.predecessor_labels.end());
        auto iter = unique(bb.predecessor_labels.begin(), bb.predecessor_labels.end());
        bb.predecessor_labels.erase(iter, bb.predecessor_labels.end());
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

    // gens,kills,globals set of basicblocks;(inst label)
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
    /*
    for(int i=0;i<basic_blocks.size();i++){
        std::cout<<"gen of bb "<<i<<": ";
        for(auto j:gens[i]){
            std::cout<<" "<<j;
        }
        std::cout<<std::endl;

        std::cout<<"kill of bb "<<i<<": ";
        for(auto j:kills[i]){
            std::cout<<" "<<j;
        }
        std::cout<<std::endl;
    }
*/
    auto bb_cnt = basic_blocks.size();
    vector<unordered_set<int>> ins(bb_cnt), outs(bb_cnt);  // IN set and OUT set of basic blocks
    deque<int> changed;                                    //blocks changed in the last iteration
    for (auto i = 0; i < bb_cnt; i++) {
        changed.push_back(i);
    }

    while (!changed.empty()) {
        auto n = changed.front();
        changed.pop_front();
        ins[n].clear();
        //std::cout<<"predecessors of bb "<<n<<" :"<<basic_blocks[n].predecessor_labels.size();
        for (auto pre_label : basic_blocks[n].predecessor_labels) {
            ins[n].insert(outs[idx_of_bb[pre_label]].begin(), outs[idx_of_bb[pre_label]].end());
            // std::cout<<" "<<pre_label;
        }
        //std::cout<<std::endl;

        auto out_n_copy = outs[n];
        outs[n].clear();
        // out = gen-global
        for (auto i : gens[n]) {
            if (globals[n].count(i) == 0)
                outs[n].insert(i);
        }
        // out = out \cup (in-kill)
        for (auto i : ins[n]) {
            if (kills[n].count(i) == 0)
                outs[n].insert(i);
        }
        /*
        std::cout << "size of in " << n <<" :"<< ins[n].size() << std::endl;
        std::cout << "size of out " << n <<" :"<< outs[n].size() << std::endl;
        */
        // if out changed, successor basic blocks' out need to be recomputed
        if (out_n_copy != outs[n]) {
            for (auto s : basic_blocks[n].successor_labels) {
                changed.push_back(idx_of_bb[s]);
            }
            sort(changed.begin(), changed.end());
            auto iter = unique(changed.begin(), changed.end());
            changed.erase(iter, changed.end());
        }
    }
    unordered_map<long long, long long> const_val_of_def;
    for (const auto& bb : basic_blocks) {
        for (const auto& inst : bb.instructions) {
            if (inst.is_constant_def()) {
                const_val_of_def[inst.label] = inst.const_def_val();
                // std::cout << "const val of def " << inst.label << " is " << inst.const_def_val() << std::endl;
            }
        }
    }
    for (int i = 0; i < basic_blocks.size(); i++) {
        const auto& in = ins[i];
        unordered_set<string> non_constant_variable;
        unordered_map<string, long long> constant_variable;
        for (auto j : in) {
            const auto& variable_name = object_def_by_inst[j - label_0];  //the variable name defed by definition i
            assert(variable_name.size() > 0);
            if (non_constant_variable.count(variable_name) > 0)
                continue;
            if (const_val_of_def.count(j) == 0) {  //this def does not generate constant value
                non_constant_variable.insert(variable_name);
                constant_variable.erase(variable_name);
            } else {
                auto constant_value = const_val_of_def[j];  //the definition's const value
                if (constant_variable.count(variable_name) == 0) {
                    constant_variable[variable_name] = constant_value;
                } else {
                    if (constant_variable[variable_name] != constant_value) {
                        constant_variable.erase(variable_name);
                        non_constant_variable.insert(variable_name);
                    }
                }
            }
        }

        /*
        for (auto& [key, value] : constant_variable) {
            std::cout << "bb " << i << " " << key << " has value " << value << std::endl;
        }*/

        for (auto& inst : basic_blocks[i].instructions) {
            if (!inst.is_arithmetic()) {
                continue;
            }
            for (auto& operand : inst.operands) {
                auto op_variable_name = operand.icode();
                if (constant_variable.count(op_variable_name) > 0) {
                    //std::cout << "//" << inst.icode() << std::endl;
                    operand.type = Operand::Type::CONSTANT;
                    operand.constant = constant_variable[op_variable_name];
                    constant_propagated_cnt++;
                    //std::cout << "//" << inst.icode() << std::endl;
                }
            }
            if (inst.is_def()) {
                auto tmp_name = object_def_by_inst[inst.label - label_0];
                assert(tmp_name == inst.get_def());
                if (constant_variable.count(tmp_name) != 0) {
                    if (!inst.is_constant_def()) {
                        constant_variable.erase(tmp_name);
                        //std::cout << "//erased " << tmp_name << std::endl;
                    } else {
                        if (constant_variable.count(inst.get_def()) > 0) {
                            if (inst.const_def_val() != constant_variable[inst.get_def()]) {
                                constant_variable.erase(tmp_name);
                                //std::cout << "//erased " << tmp_name << std::endl;
                            }
                        }
                    }
                } else if (inst.is_constant_def()) {
                    constant_variable[inst.get_def()] = inst.const_def_val();
                }
            }
        }
    }
}

void Function::scp_peephole() {
    bool flag = false;
    while (true) {
        for (auto& bb : basic_blocks) {
            for (auto& inst : bb.instructions) {
                inst.peephole2();
                inst.peephole3();
            }
        }
        if (flag)
            break;
        auto old_cnt = constant_propagated_cnt;
        scp();
        if (constant_propagated_cnt == old_cnt)
            flag = true;
    }
}

// Only consider local variables and virtual registers
void Function::dse() {
    vector<unordered_set<string>> defs, uses;
    for (const auto& bb : basic_blocks) {
        defs.emplace_back();
        uses.emplace_back();
        auto& def = defs.back();
        auto& use = uses.back();
        for (const auto& inst : bb.instructions) {
            for (auto& u : inst.get_use_dse()) {
                if (u.size() == 0)
                    continue;
                if (def.count(u) == 0)
                    use.insert(u);
            }
            auto d = inst.get_def_dse();
            if (d.size() == 0)
                continue;
            if (use.count(d) == 0)
                def.insert(d);
        }
    }

    auto bb_cnt = basic_blocks.size();
    vector<unordered_set<string>> ins(bb_cnt), outs(bb_cnt);
    unordered_set<int> changed;
    for (int i = 0; i < bb_cnt; i++) {
        changed.insert(i);
    }
    while (!changed.empty()) {
        auto n = *changed.begin();
        changed.erase(n);
        outs[n].clear();
        for (auto s : basic_blocks[n].successor_labels) {
            int idx_s = idx_of_bb[s];
            outs[n].insert(ins[idx_s].begin(), ins[idx_s].end());
        }
        auto ins_n_copy = ins[n];
        ins[n].clear();

        //IN = use \cup (Out -def)
        ins[n].insert(uses[n].begin(), uses[n].end());

        for (auto& s : outs[n]) {
            if (defs[n].count(s) == 0)
                ins[n].insert(s);
        }
        if (ins[n] != ins_n_copy) {
            for (auto p : basic_blocks[n].predecessor_labels) {
                auto idx_p = idx_of_bb[p];
                changed.insert(idx_p);
            }
        }
    }

    for (int i = 0; i < basic_blocks.size(); i++) {
        auto& bb = basic_blocks[i];
        auto live = outs[i];
        for (auto iter = bb.instructions.rbegin(); iter != bb.instructions.rend(); ++iter) {
            auto& inst = *iter;
            for (auto& u : inst.get_use_dse()) {
                if (u.size() > 0)
                    live.insert(u);
            }
            auto def_of_inst = inst.get_def_dse();
            if (def_of_inst.size() == 0)
                continue;
            if(live.count(def_of_inst)==0){
                inst.to_nop();
                statement_eliminated_cnt++;
            }
        }
    }
}