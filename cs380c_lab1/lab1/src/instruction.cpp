#include "ir.h"
Instruction::Instruction(const string& s):is_block_leader(false),predecessor_labels({}) {
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
    if(this->predecessor_labels.size()>0)
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
            context.push_back(operands[0].ccode());
            return tmp.str();
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

bool Instruction::is_branch() {
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

long long Instruction::branch_target_label(){
    assert(this->is_branch());
    return operands.back().inst_label;
}
