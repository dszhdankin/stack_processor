//
// Created by dszhdankin on 24.01.2021.
//

#ifndef STACK_PROCESSOR_UTILS_H
#define STACK_PROCESSOR_UTILS_H

enum OperationPrefixCode {
    IN = 0b00000001, //Reads from stdin and pushes to stack
    OUT = 0b00000010, //Pops from stack and writes to stdout
    ADD = 0b00000011,
    SUB = 0b00000100,
    MUL = 0b00000101,
    DIV = 0b00000110,
    SIN = 0b00000111,
    COS = 0b00001000,
    SQRT = 0b00001001,
    RET_ABS = 0b00001010,
    HALT = 0b00001011,
    POP = 0b00001100,
    PUSH_REG_VAL = 0b00001101,
    PUSH_EXACT_VAL = 0b00001110,
    PUSH_REG_ADDR = 0b00001111,
    PUSH_EXACT_ADDR = 0b00010000,
    POP_REG_VAL = 0b00010001,
    POP_EXACT_ADDR = 0b00010010,
    POP_REG_ADDR = 0b00010011,
    JMP_OFFSET_EXACT_VAL = 0b00010100,
    JE_OFFSET_EXACT_VAL = 0b00010101,
    JNE_OFFSET_EXACT_VAL = 0b00010110,
    JA_OFFSET_EXACT_VAL = 0b00010111,
    JAE_OFFSET_EXACT_VAL = 0b00011000,
    JB_OFFSET_EXACT_VAL = 0b00011001,
    JBE_OFFSET_EXACT_VAL = 0b00011010,
    CALL_OFFSET_EXACT_VAL = 0b00011011
};

enum RegisterCode{
    AX = 0b00000000,
    BX = 0b00000001,
    CX = 0b00000010,
    DX = 0b00000011,
};

enum ProcessorStatus {
    SUCCESS = 0,
    UNRECOGNIZED_COMMAND,
    COMMAND_ARG_ERROR,
    CALL_STACK_UNDERFLOW,
    DATA_STACK_UNDERFLOW,
    INVALID_INSTRUCTION_POINTER,
    INVALID_RAM_ADDRESS
};

union DoubleChars {
    double db_val = 0.0;
    char chr_val[sizeof (double)];
};

union IntChars {
    int itn_val = 0;
    char chr_val[sizeof (int)];
};

union DoubleUll {
    unsigned long long ull_val;
    double db_val;
};

constexpr double PROCESSOR_EPSILON = 1e-9;

#endif //STACK_PROCESSOR_UTILS_H
