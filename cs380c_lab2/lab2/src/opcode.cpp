#include <algorithm>

#include "ir.h"
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
    {END, "end"},
    {ASSIGN,"assign"}};

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
    {END, 0},
    {ASSIGN,1}};

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
