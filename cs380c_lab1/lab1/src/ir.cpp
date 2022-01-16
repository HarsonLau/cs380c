#include "ir.h"

#include <algorithm>
map<Opcode::Type, string> Opcode::opcode_name = {
    {INVALID, "invalid"},
    {ADD, "add"},
    {SUB, "sub"},
    {MUL, "mul"},
    {DIV, "div"},
    {MOD, "mod"},
    {NEG, "neg"},
    {CMPEQ, "cmpeq"},
    {CMPLE, "cmple"},
    {CMPLT, "cmplt"},
    {BR, "br"},
    {BLBC, "blbc"},
    {BLBS, "blbs"},
    {LOAD, "load"},
    {STORE, "store"},
    {MOVE, "move"},
    {READ, "read"},
    {WRITE, "write"},
    {WRL, "wrl"},
    {PARAM, "param"},
    {ENTER, "enter"},
    {ENTRYPC, "entrypc"},
    {CALL, "call"},
    {RET, "ret"},
    {NOP, "nop"},
    {END, "end"}};

map<Opcode::Type, int> Opcode::operand_cnt = {
    {INVALID, 0},
    {ADD, 2},
    {SUB, 2},
    {MUL, 2},
    {DIV, 2},
    {MOD, 2},
    {NEG, 1},
    {CMPEQ, 2},
    {CMPLE, 2},
    {CMPLT, 2},
    {BR, 1},
    {BLBC, 2},
    {BLBS, 2},
    {LOAD, 1},
    {STORE, 2},
    {MOVE, 2},
    {READ, 0},
    {WRITE, 1},
    {WRL, 0},
    {PARAM, 1},
    {ENTER, 1},
    {ENTRYPC, 0},
    {CALL, 1},
    {RET, 1},
    {NOP, 0},
    {END, 0}};

map<Operand::Type, string> Operand::type_name = {
    {INVALID, "invalid"},
    {GP, "GP"},
    {FP, "FP"},
    {CONSTANT, "constant"},
    {ADDR_OFFSET, "address offset"},
    {FIELD_OFFSET, "field offset"},
    {LOCAL_VARIABLE, "local variable"},
    {LOCAL_ADDR, "local variable address"},
    {REG, "register"},
    {LABEL, "instruction label"},
    {FUNCTION, "function"},
    {END, "end"},
    {GLOBAL_ADDR, "global variable address"},
    {PARAMETER, "parameter"}};

Operand::Operand(const string& s, bool is_function) : type(INVALID), constant(0), variable_name("") {
    if (s.find('(') != string::npos) {
        assert(s[0] == '(' && s[s.size() - 1] == ')');
        this->type = Operand::Type::REG;

        // Convert the string inside ( ) to longlong
        this->reg_name = atoll(s.substr(1, s.size() - 2).c_str());
    } else if (s.find('[') != string::npos) {
        assert(s[0] == '[' && s[s.size() - 1] == ']');
        if (is_function) {
            this->type = Operand::Type::FUNCTION;
            this->function_id = atoll(s.substr(1, s.size() - 2).c_str());
        } else {
            this->type = Operand::Type::LABEL;
            this->inst_label = atoll(s.substr(1, s.size() - 2).c_str());
        }
    } else if (s.find("base") != string::npos) {
        auto sharp_idx = s.find('#');
        assert(sharp_idx != string::npos);
        //this->type = Operand::Type::ADDR_OFFSET;
        this->offset = atoll(s.substr(sharp_idx + 1).c_str());
        if (this->offset > 8192)
            this->type = Operand::Type::GLOBAL_ADDR;
        else if (this->offset < 0)
            this->type = Operand::Type::LOCAL_ADDR;
        auto base_idx = s.find("_base");
        this->variable_name = s.substr(0, base_idx);
    } else if (s.find("offset") != string::npos) {
        auto sharp_idx = s.find('#');
        assert(sharp_idx != string::npos);
        this->type = Operand::Type::FIELD_OFFSET;
        this->offset = atoll(s.substr(sharp_idx + 1).c_str());
        auto offset_idex = s.find("_offset");
        this->variable_name = s.substr(0, offset_idex);
    } else if (s.find('#') != string::npos) {
        auto sharp_idx = s.find('#');
        //this->type = Operand::Type::LOCAL_VARIABLE;
        this->offset = atoll(s.substr(sharp_idx + 1).c_str());
        if (this->offset < 0)
            this->type = Operand::Type::LOCAL_VARIABLE;
        else
            this->type = Operand::Type::PARAMETER;
        this->variable_name = s.substr(0, sharp_idx);
    } else if (s.find("GP") != string::npos) {
        this->type = Operand::Type::GP;
    } else if (s.find("FP") != string::npos) {
        this->type = Operand::Type::FP;
    } else {
        // Does not contain #, [, (, so it must be a constant
        this->type = Operand::Type::CONSTANT;
        this->constant = atoll(s.c_str());
    }
#ifdef OPERAND_DEBUG
    std::cout << "  "
              << "handling operand " << s;
    std::cout << "  " << Operand::type_name[this->type] << " " << this->offset << "   " << this->variable_name << std::endl;
#endif
}

string Operand::ccode() {
    std::stringstream tmp;
    switch (this->type) {
        case Operand::Type::FP:
        case Operand::Type::GP:
            return "0";
        case Operand::Type::REG:
            tmp << "r[" << this->reg_name << "]";
            return tmp.str();
        case Operand::Type::LOCAL_VARIABLE:
        case Operand::Type::PARAMETER:
            tmp << this->variable_name;
            return tmp.str();
        case Operand::Type::GLOBAL_ADDR:
        case Operand::Type::LOCAL_ADDR:
            tmp << "(long long)(&" << this->variable_name << ")";
            return tmp.str();
        case Operand::Type::FUNCTION:
            tmp << "function_" << this->function_id;
            return tmp.str();
        case Operand::Type::FIELD_OFFSET:
        case Operand::Type::CONSTANT:
            tmp << this->constant;
            return tmp.str();
        case Operand::Type::LABEL: {
            tmp << "inst_" << this->constant;
            return tmp.str();
        }
    }
    return tmp.str();
}

//match input with opcode to determine type
Opcode::Opcode(const string& s) : type(Opcode::Type::INVALID) {
    for (int i = 0; i < Opcode::Type::END; i++) {
        if (s.find(Opcode::opcode_name[Opcode::Type(i)]) != string::npos) {
            this->type = Opcode::Type(i);
            break;
        }
    }
    assert(s.find(Opcode::opcode_name[this->type]) != string::npos);
#ifdef OPCODE_DEBUG
    std::cout << "handling opcode " << s << " ";
    std::cout << Opcode::opcode_name[this->type] << std::endl;
#endif
}

Instruction::Instruction(const string& s) {
    //instr 33:   add   global_array_base#32576   GP
    //      1 2   3  4  6                      8  75
    auto idx1 = s.find_first_of("0123456789");
    auto idx2 = s.find_first_of(':');
    auto idx3 = s.find_first_not_of(' ', idx2 + 1);
    assert(idx3 != string::npos && idx3 < s.size());
    assert(idx1 != string::npos && idx2 != string::npos && idx1 != idx2);
    this->label = atoll(s.substr(idx1, idx2 - idx1).c_str());
    this->opcode = Opcode(s);
    if (Opcode::operand_cnt.at(opcode.type) > 0) {
        auto idx4 = s.find_first_of(' ', idx3);
        assert(idx4 != string::npos && idx4 < s.size());
        auto idx6 = s.find_first_not_of(' ', idx4);
        auto is_function = (s.find("call") != string::npos);
        if (Opcode::operand_cnt.at(opcode.type) == 1) {
            operands.emplace_back(s.substr(idx6), is_function);
        } else {
            auto idx8 = s.find_first_of(' ', idx6);
            operands.emplace_back(s.substr(idx6, idx8 - idx6));
            auto idx7 = s.find_first_not_of(' ', idx8);
            auto idx5 = s.find_last_not_of(' ');
            operands.emplace_back(s.substr(idx7, idx5 - idx7 + 1));
        }
    }
    assert(operands.size() == Opcode::operand_cnt.at(opcode.type));
}

string Instruction::ccode(deque<string>& context) {
    std::stringstream tmp;
    tmp << "inst_" << this->label << ":";
    switch (this->opcode.type) {
        case Opcode::Type::ADD:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " + " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::SUB:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " - " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::MUL:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " * " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::DIV:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " / " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::MOD:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " % " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::NEG:
            tmp << "r[" << this->label << "] = -" << operands[0].ccode() << " ; ";
            return tmp.str();
        case Opcode::Type::CMPEQ:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " == " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::CMPLE:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " <= " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::CMPLT:
            tmp << "r[" << this->label << "] = " << operands[0].ccode() << " < " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::BR:
            tmp << "goto " << operands[0].ccode() << ";";
            return tmp.str();
        case Opcode::Type::BLBC:
            tmp << "if(" << operands[0].ccode() << " == 0) goto " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::BLBS:
            tmp << "if(" << operands[0].ccode() << " !=0) goto " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::LOAD:
            tmp << "r[" << this->label << "] = "
                << "*((long long *)" << operands[0].ccode() << ");";
            return tmp.str();
        case Opcode::Type::STORE:
            tmp << "*( (long long *)" << operands[1].ccode() << ") = " << operands[0].ccode() << ";";
            return tmp.str();
        case Opcode::Type::MOVE:
            tmp << operands[1].ccode() << " = " << operands[0].ccode() << ";";
            return tmp.str();
        case Opcode::Type::READ:
            tmp << "ReadLong(" << operands[0].ccode() << ");";
            return tmp.str();
        case Opcode::Type::WRITE:
            tmp << "WriteLong(" << operands[0].ccode() << ");";
            return tmp.str();
        case Opcode::Type::WRL:
            tmp << "WriteLine();";
            return tmp.str();
        case Opcode::Type::PARAM:
            context.push_back(operands[0].variable_name);
            return "";
        case Opcode::Type::ENTER:
            return "";
        case Opcode::Type::ENTRYPC:
            return "";
        case Opcode::Type::CALL:
            tmp << operands[0].ccode();
            tmp << "(";
            while (!context.empty()) {
                tmp << context.front();
                context.pop_front();
                if (!context.empty())
                    tmp << ",";
            }
            tmp << ");";
            return tmp.str();
        case Opcode::Type::RET:
            tmp << "return ;";
            return tmp.str();
        case Opcode::Type::NOP:
            return tmp.str();
    }
    return tmp.str();
}

void Program::ScanGlobalVariable() {
}

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
    tmp << "void function_" << id << "(";
    for (int i = 0; i < params.size(); i++) {
        tmp << "long long " << params[i].variable_name;
        if (params.size() > 1 && i < params.size() - 1) {
            tmp << ",";
        }
    }
    tmp << "){";
    tmp << std::endl;

    for (auto& v : local_variables) {
        tmp << "  long long " << v.variable_name;
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

Program::Program(const vector<Instruction>& insts) : instructions(insts), global_variables({}), functions({}) {
    // Scan all instructions in turn,
    // and save the global variables that appear in the instructions to the vector,
    // so that the addresses are arranged from low to high*/
    for (const auto& inst : this->instructions) {
        if (inst.operands.size() == 2 && inst.operands[1].type == Operand::Type::GP) {
            assert(inst.opcode.type == Opcode::Type::ADD);
            assert(inst.operands[0].type == Operand::Type::GLOBAL_ADDR);
            global_variables.emplace_back(inst.operands[0].variable_name, inst.operands[0].offset);
        }
    }

    // unique
    std::sort(global_variables.begin(), global_variables.end());
    auto iter = std::unique(global_variables.begin(), global_variables.end());
    global_variables = vector<Variable>(global_variables.begin(), iter);

    // calculate the variable size
    if (!global_variables.empty()) {
        int i = 0;
        for (; i < global_variables.size() - 1; i++) {
            global_variables[i].size = global_variables[i + 1].address - global_variables[i].address;
        }
        global_variables[i].size = 32768 - global_variables[i].address;
    }

    // Reverse the vector so that the elements are in the order they are declared
    std::reverse(global_variables.begin(), global_variables.end());

    // Divide the entire program into several functions for processing
    bool _is_main = false;
    vector<Instruction> tmp = {};
    for (const auto& inst : instructions) {
        if (inst.opcode.type == Opcode::Type::ENTRYPC) {
            _is_main = true;
        }
        tmp.push_back(inst);
        if (inst.opcode.type == Opcode::RET) {
            functions.emplace_back(tmp, _is_main);
            _is_main = false;
            tmp = {};
        }
    }
#ifdef PROGRAM_DEBUG
    std::cout << "program" << std::endl;
    std::cout << "--global variables---" << std::endl;
    for (const auto& v : this->global_variables) {
        std::cout << v.variable_name << " " << v.size << " " << std::endl;
    }
#endif
}