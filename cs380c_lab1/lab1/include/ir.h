#ifndef IR_H
#define IR_H
#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using std::array;
using std::map;
using std::set;
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
        FUNCTION,
        END
    };

    Type type;
    union {
        long long constant;    // 19.Constants: For example, 24
        long long offset;      // 20.Address offsets 21.Field offsets 22.Local variables (scalars) offsets
        long long reg_name;    // 23.Register names
        long long inst_label;  // 24.Instruction labels
        long long function_id;
    };
    string variable_name;
    static map<Operand::Type, string> type_name;
    Operand() : type(Operand::Type::INVALID){};

    string ccode();
    // Read information from a string and build an IR representation
    // Assume that the input string does not contain spaces
    Operand(const string& s, bool is_function = false);
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
    static map<Opcode::Type, int> operand_cnt;

    static map<Opcode::Type, string> opcode_name;
    Type type;
    Opcode() : type(Opcode::Type::INVALID){};

    // Read information from a string and build an IR representation
    // Assume that the input string does not contain spaces
    Opcode(const string& s);
};

class Instruction {
   public:
    Opcode opcode;
    vector<Operand> operands;
    long long label;
    Instruction() = delete;
    Instruction(const string& s);
    string ccode();
};

class Variable {
   public:
    string variable_name;  //global_array_base#32576 's name should be global_array
    long long address;     // global variable's address relative to GP
                           // the value of address should >0
    long long size;        // size in bytes
    Variable(const string& name, long long addr) : variable_name(name), address(addr){};
    bool operator<(const Variable& gv) const {
        return this->address < gv.address;
    };
    bool operator==(const Variable& gv) const {
        return this->address == gv.address && this->variable_name == gv.variable_name;
    }
};

class Basic_block {
   public:
};

class Function {
   public:
    bool is_main;
    vector<Variable> local_variables;
    vector<Variable> params;
    vector<Instruction> instructions;
};

class Program {
   public:
    vector<Instruction> instructions;
    vector<Variable> global_variables;
    Program(const vector<Instruction>& insts) : instructions(insts){};

    // Scan all instructions in turn,
    // and save the global variables that appear in the instructions to the vector,
    // so that the addresses are arranged from low to high
    void ScanGlobalVariable();
};
#endif  //IR_H