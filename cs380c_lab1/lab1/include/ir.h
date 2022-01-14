#ifndef IR_H
#define IR_H
#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using std::array;
using std::map;
using std::string;
using std::vector;
#define OPERAND_DEBUG
#define OPCODE_DEBUG
class Operand {
   public:
    enum Type {
        INVALID,
        GP,
        FP,
        CONSTANT,
        ADDR_OFFSET,
        FIELD_OFFSET,
        LOCAL_VARIABLE,
        REG,
        LABEL,
        END
    };

    Type type_;
    union {
        long long constant_;    // 19.Constants: For example, 24
        long long offset_;      // 20.Address offsets 21.Field offsets 22.Local variables (scalars) offsets
        long long reg_name_;    // 23.Register names
        long long inst_label_;  // 24.Instruction labels
    };
    string variable_name_;
    static map<Operand::Type,string> type_name;
    Operand() : type_(Operand::Type::INVALID){};

    // Read information from a string and build an IR representation
    // Assume that the input string does not contain spaces
    Operand(const string& s);
};

class Opcode {
   public:
    enum Type {
        INVALID = 0,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        NEG,
        CMPEQ,
        CMPLE,
        CMPLT,
        BR,
        BLBC,
        BLBS,
        LOAD,
        STORE,
        MOVE,
        READ,
        WRITE,
        WRL,
        PARAM,
        ENTER,
        ENTRYPC,
        CALL,
        RET,
        NOP,
        END
    };
    static map<Opcode::Type,int> operand_cnt;

    static map<Opcode::Type,string>opcode_name;
    Type type_;
    Opcode() : type_(Opcode::Type::INVALID){};

    // Read information from a string and build an IR representation
    // Assume that the input string does not contain spaces
    Opcode(const string& s);
};

class Instruction {
   public:
    Opcode opcode;
    vector<Operand> operand_;
    long long label_;
    Instruction() = delete;
    Instruction(const string& s);
};

class Basic_block {
   public:
};

class Function {
   public:
};
#endif  //IR_H