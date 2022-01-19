#include "ir.h"
Instruction::Instruction(const string& s) : is_block_leader(false), predecessor_labels({}) {
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

deque<string> Instruction::context = {};

string Instruction::ccode() const {
    std::stringstream tmp;
    if (this->predecessor_labels.size() > 0)
        tmp << "inst_" << this->label << ":";
    switch (this->opcode.type) {
        case Opcode::Type::ADD:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " + " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::SUB:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " - " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::MUL:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " * " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::DIV:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " / " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::MOD:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " % " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::NEG:
            tmp << "REG[" << this->label << "] = -" << operands[0].ccode() << " ; ";
            return tmp.str();
        case Opcode::Type::CMPEQ:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " == " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::CMPLE:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " <= " << operands[1].ccode() << ";";
            return tmp.str();
        case Opcode::Type::CMPLT:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << " < " << operands[1].ccode() << ";";
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
            tmp << "REG[" << this->label << "] = "
                << "*((long *)" << operands[0].ccode() << ");";
            return tmp.str();
        case Opcode::Type::STORE:
            tmp << "*( (long *)" << operands[1].ccode() << ") = " << operands[0].ccode() << ";";
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
            Instruction::context.push_back(operands[0].ccode());
            return tmp.str();
        case Opcode::Type::ENTER:
            return "";
        case Opcode::Type::ENTRYPC:
            return "";
        case Opcode::Type::CALL:
            tmp << operands[0].ccode();
            tmp << "(";
            while (!Instruction::context.empty()) {
                tmp << Instruction::context.front();
                Instruction::context.pop_front();
                if (!Instruction::context.empty())
                    tmp << ",";
            }
            tmp << ");";
            return tmp.str();
        case Opcode::Type::RET:
            tmp << "return ;";
            return tmp.str();
        case Opcode::Type::NOP:
            return tmp.str();  // Can't return empty string directly, because jump labels may need to be output
        case Opcode::Type::ASSIGN:
            tmp << "REG[" << this->label << "] = " << operands[0].ccode() << ";";
            return tmp.str();
    }
    return tmp.str();
}

bool Instruction::is_branch() const {
    switch (this->opcode.type) {
        case Opcode::Type::BR:
        case Opcode::Type::BLBC:
        case Opcode::Type::BLBS:
            return true;
        default:
            return false;
    }
    return false;
}

long long Instruction::branch_target_label() const {
    assert(this->is_branch());
    return operands.back().inst_label;
}

string Instruction::icode() const {
    std::stringstream tmp;
    tmp << "    instr " << this->label << ": " << Opcode::opcode_name[this->opcode.type];
    for (auto& op : operands) {
        tmp << " " << op.icode();
    }
    tmp << std::endl;
    return tmp.str();
}
void Instruction::to_nop() {
    this->opcode.type = Opcode::Type::NOP;
    this->operands.clear();
}
string Instruction::get_def() const {
    switch (this->opcode.type) {
        case Opcode::Type::MOVE:
            return operands.back().icode();

        case Opcode::Type::BLBC:
        case Opcode::Type::BLBS:
        case Opcode::Type::BR:
        case Opcode::Type::CALL:
        case Opcode::Type::END:
        case Opcode::Type::ENTER:
        case Opcode::Type::ENTRYPC:
        case Opcode::Type::INVALID:
        case Opcode::Type::NOP:
        case Opcode::Type::PARAM:
        case Opcode::Type::RET:
        case Opcode::Type::STORE:
        case Opcode::Type::WRITE:
        case Opcode::Type::WRL:
            return "";

        case Opcode::Type::ADD:
        case Opcode::Type::ASSIGN:
        case Opcode::Type::CMPEQ:
        case Opcode::Type::CMPLE:
        case Opcode::Type::CMPLT:
        case Opcode::Type::DIV:
        case Opcode::Type::LOAD:
        case Opcode::Type::MOD:
        case Opcode::Type::MUL:
        case Opcode::Type::NEG:
        case Opcode::Type::READ:
        case Opcode::Type::SUB:
            std::stringstream tmp;
            tmp << "(" << this->label << ")";
            return tmp.str();
    }
    return "";
}

bool Instruction::is_def() const {
    return this->get_def().size() > 0;
}

bool Instruction::is_constant_def() const {
    if (opcode.type == Opcode::Type::MOVE || opcode.type == Opcode::Type::ASSIGN) {
        if (operands.front().type == Operand::Type::CONSTANT) {  //the operand is a immediate number
            return true;
        }
    }
    return false;
}

void Instruction::peephole2() {
    if (operands.size() != 2)
        return;
    for (auto& operand : operands) {
        if (operand.type != Operand::Type::CONSTANT)
            return;
    }
    long long n_val = 0;
    switch (opcode.type) {
        case Opcode::Type::ADD:
            n_val = operands[0].constant + operands[1].constant;
            break;
        case Opcode::Type::SUB:
            n_val = operands[0].constant - operands[1].constant;
            break;
        case Opcode::Type::MUL:
            n_val = operands[0].constant * operands[1].constant;
            break;
        case Opcode::Type::DIV:
            n_val = operands[0].constant / operands[1].constant;
            break;
        case Opcode::Type::CMPLT:
            n_val = operands[0].constant < operands[1].constant;
            break;
        case Opcode::Type::CMPEQ:
            n_val = operands[0].constant == operands[1].constant;
            break;
        case Opcode::Type::CMPLE:
            n_val = operands[0].constant <= operands[1].constant;
            break;

        default:
            return;
    }
    opcode.type = Opcode::Type::ASSIGN;
    operands[0].constant = n_val;
    operands.resize(1);
}