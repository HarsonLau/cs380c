#ifndef IR_H
#define IR_H
#include <array>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
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
    static const string type_name[];
    Operand():type_(Operand::Type::INVALID){};

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
    static constexpr int operand_cnt[] = {
        [INVALID] = 0,
        [ADD] = 2,
        [SUB] = 2,
        [MUL] = 2,
        [DIV] = 2,
        [MOD] = 2,
        [NEG] = 1,
        [CMPEQ] = 2,
        [CMPLE] = 2,
        [CMPLT] = 2,
        [BR] = 1,
        [BLBC] = 2,
        [BLBS] = 2,
        [LOAD] = 1,
        [STORE] = 1,
        [READ] = 0,
        [WRITE] = 1,
        [WRL] = 0,
        [PARAM] = 1,
        [ENTER] = 1,
        [ENTRYPC] = 0,
        [CALL] = 1,
        [RET] = 0,
        [NOP] = 0,
        [END] = 0};
    static const string opcode_name[];
    Type type_;
    Opcode() :type_(Opcode::Type::INVALID){};

    // Read information from a string and build an IR representation
    // Assume that the input string does not contain spaces
    Opcode(const string& s);
};

class Instruction {
   public:
    Opcode op_;
    vector<Operand> operand_;
    long long label_;
    Instruction()=delete;
    Instruction(const string& s);
};

class Basic_block {
   public:
};

class Function {
   public:
};
#endif  //IR_H