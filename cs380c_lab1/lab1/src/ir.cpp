#include "ir.h"
const string Opcode::opcode_name[Opcode::Type::END + 1] = {
    "invalid",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "neg",
    "cmpeq",
    "cmple",
    "cmplt",
    "br",
    "blbc",
    "blbs",
    "load",
    "store",
    "read",
    "write",
    "wrl",
    "param",
    "enter",
    "entrypc",
    "call",
    "ret",
    "nop",
    "end"};

const string Operand::type_name[Operand::Type::END + 1] = {
    "invalid",
    "GP",
    "FP",
    "constant",
    "address offset",
    "field offset",
    "local variable",
    "register",
    "instruction label",
    "end"};
Operand::Operand(const string& s) : type_(INVALID), constant_(0), variable_name_("") {
    if (s.find('(') != string::npos) {
        assert(s[0] == '(' && s[s.size() - 1] == ')');
        this->type_ = Operand::Type::REG;

        // Convert the string inside ( ) to longlong
        this->reg_name_ = atoll(s.substr(1, s.size() - 2).c_str());
    } else if (s.find('[') != string::npos) {
        assert(s[0] == '[' && s[s.size() - 1] == ']');
        this->type_ = Operand::Type::LABEL;

        // Convert the string inside [ ]  to longlong
        this->inst_label_ = atoll(s.substr(1, s.size() - 2).c_str());
    } else if (s.find("base") != string::npos) {
        auto sharp_idx = s.find('#');
        assert(sharp_idx != string::npos);
        this->type_ = Operand::Type::ADDR_OFFSET;
        this->offset_ = atoll(s.substr(sharp_idx + 1).c_str());
        this->variable_name_ = s.substr(0, sharp_idx);
    } else if (s.find("offset") != string::npos) {
        auto sharp_idx = s.find('#');
        assert(sharp_idx != string::npos);
        this->type_ = Operand::Type::FIELD_OFFSET;
        this->offset_ = atoll(s.substr(sharp_idx + 1).c_str());
        this->variable_name_ = s.substr(0, sharp_idx);
    } else if (s.find('#') != string::npos) {
        auto sharp_idx = s.find('#');
        this->type_ = Operand::Type::LOCAL_VARIABLE;
        this->offset_ = atoll(s.substr(sharp_idx + 1).c_str());
        this->variable_name_ = s.substr(0, sharp_idx);
    } else if (s.find("GP") != string::npos) {
        this->type_ = Operand::Type::GP;
    } else if (s.find("FP") != string::npos) {
        this->type_ = Operand::Type::FP;
    } else {
        // Does not contain #, [, (, so it must be a constant
        this->type_ = Operand::Type::CONSTANT;
        this->constant_ = atoll(s.c_str());
    }
#ifdef OPERAND_DEBUG
    std::cout << s << "  " << Operand::type_name[this->type_] << " " << this->offset_ << "   " << this->variable_name_ << std::endl;
#endif
}

//match input with opcode to determine type
Opcode::Opcode(const string& s) : type_(Opcode::Type::INVALID) {
    for (int i = 0; i < Opcode::Type::END; i++) {
        if (s.find(Opcode::opcode_name[i]) != string::npos) {
            this->type_ = Opcode::Type(i);
            break;
        }
    }
    if (this->type_ == Opcode::Type::INVALID) {
        std::cerr << s << " is an illegal opcode " << std::endl;
    }
}