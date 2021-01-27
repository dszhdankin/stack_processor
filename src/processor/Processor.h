//
// Created by dszhdankin on 24.01.2021.
//

#ifndef STACK_PROCESSOR_PROCESSOR_H
#define STACK_PROCESSOR_PROCESSOR_H
#include "../utils.h"
#include <vector>
#include <memory>
#include <string>

class RAM {
public:
    static constexpr int MEM_SIZE = 1024 * 50;
    static bool isValidAddr(int addr);
private:
    char mem[MEM_SIZE];
public:
    void store(double val, int address);

    double load(int address);
};

class Processor {
private:
    char *_ip;
    char *_start;
    DoubleUll _reg[4];
    int _operations_size;

    std::unique_ptr<RAM> _ram;

    std::vector<double> _data_stack;
    std::vector<char *> _call_stack;

    static bool isNoArgsOperation(char prefixCode);

    static bool isHalt(char prefixCode);

    static bool isJump(char prefixCode);

    //Need to ensure buf contains enough bytes
    static double getDouble(char *buf);

    //Need to ensure buf contains enough bytes
    static int getInt(char *buf);

    static bool isCommand(int prefixCode);

    //Command length in bytes
    static int getCommandLength(char prefixCode);

    //Need to ensure prefixCode is a prefix code of jump operation
    ProcessorStatus executeJumpOperation(char prefixCode);

    ProcessorStatus executeNoArgsNonJump(char prefixCode);

    ProcessorStatus executeRegArg(char prefixCode);

    ProcessorStatus executeExactAddr(char prefixCode);

    ProcessorStatus executeUnaryNonJump(char prefixCode);

public:

    Processor();

    ProcessorStatus executeOperations(char *start, int size);

    static std::string statusToStr(ProcessorStatus status);

};


#endif //STACK_PROCESSOR_PROCESSOR_H
