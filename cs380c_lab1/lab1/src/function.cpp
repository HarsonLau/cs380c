#include"ir.h"
#include<algorithm>
Function::Function(const vector<Instruction>& instrs, bool _is_main)
    : instructions(instrs), is_main(_is_main), id(0) {
    // Scan all operands for local variables and function parameters
    for (const auto& inst : this->instructions) {
        if (inst.opcode.type == Opcode::Type::ENTER) {
            this->local_var_size = inst.operands[0].constant;
            this->id = inst.label;
        } else if (inst.opcode.type == Opcode::Type::RET) {
            this->param_size = inst.operands[0].constant;
        }

        for (const auto& operand : inst.operands) {
            if (operand.type == Operand::Type::LOCAL_VARIABLE) {
                this->local_variables.emplace_back(operand.variable_name, operand.offset);
            } else if (operand.type == Operand::Type::LOCAL_ADDR) {
                this->local_variables.emplace_back(operand.variable_name, operand.offset);
            } else if (operand.type == Operand::Type::PARAMETER) {
                this->params.emplace_back(operand.variable_name, operand.offset);
            }
        }
    }

    //sort and unique
    sort(local_variables.begin(), local_variables.end());
    auto iter1 = std::unique(local_variables.begin(), local_variables.end());
    local_variables = vector<Variable>(local_variables.begin(), iter1);

    //calculate the size of local variables
    //parameter's size is 8 == default value of type Variable's size
    if (!local_variables.empty()) {
        int i = 0;
        for (; i < local_variables.size() - 1; ++i) {
            local_variables[i].size = local_variables[i + 1].address - local_variables[i].address;
        }
        local_variables[i].size = -local_variables[i].address;
    }

    //sort and unique
    sort(params.begin(), params.end());
    auto iter2 = std::unique(params.begin(), params.end());
    params = vector<Variable>(params.begin(), iter2);

    // Reverse the vector so that the elements are in the order they are declared
    std::reverse(params.begin(), params.end());
    std::reverse(local_variables.begin(), local_variables.end());
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

string Function::ccode() {
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
    for (auto& inst : instructions) {
        tmp << "  " << inst.ccode(context) << std::endl;
    }

    tmp << "}";
    return tmp.str();
}
