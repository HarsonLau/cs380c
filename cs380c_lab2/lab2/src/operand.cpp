#include "ir.h"
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
    {PARAMETER, "parameter"},
    {OPT_VARIABLE,"variable produced by optimization"}};

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
            tmp << "REG[" << this->reg_name << "]";
            return tmp.str();
        case Operand::Type::OPT_VARIABLE:
        case Operand::Type::LOCAL_VARIABLE:
        case Operand::Type::PARAMETER:
            tmp << this->variable_name;
            return tmp.str();
        case Operand::Type::GLOBAL_ADDR:
        case Operand::Type::LOCAL_ADDR:
            tmp << "(long)(&" << this->variable_name << ")";
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

string Operand::icode() {
    std::stringstream tmp;
    switch (this->type) {
        case Operand::Type::FP:
            return "FP";
        case Operand::Type::GP:
            return "GP";
        case Operand::Type::REG:
            tmp << "(" << this->reg_name << ")";
            return tmp.str();
        case Operand::Type::OPT_VARIABLE:
        case Operand::Type::LOCAL_VARIABLE:
        case Operand::Type::PARAMETER:
            tmp << this->variable_name << "#" << this->offset;
            return tmp.str();
        case Operand::Type::GLOBAL_ADDR:
        case Operand::Type::LOCAL_ADDR:
            tmp << this->variable_name << "_base#" << this->offset;
            return tmp.str();
        case Operand::Type::FUNCTION:
            tmp << "[" << this->function_id << "]";
            return tmp.str();
        case Operand::Type::FIELD_OFFSET:
            tmp << this->variable_name << "_offset#" << this->offset;
            return tmp.str();
        case Operand::Type::CONSTANT:
            tmp << this->constant;
            return tmp.str();
        case Operand::Type::LABEL:
            tmp << "[" << this->inst_label << "]";
            return tmp.str();
    }
    return tmp.str();
}