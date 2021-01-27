//
// Created by dszhdankin on 22.01.2021.
//

#ifndef STACK_PROCESSOR_ASSEMBLER_H
#define STACK_PROCESSOR_ASSEMBLER_H

#include <unordered_map>
#include <vector>
#include "../utils.h"

enum InstructionStatus {
        OK = 0,
        FAILED
};

class Instruction {
protected:
    InstructionStatus _status;

public:
    Instruction();

    InstructionStatus getStatus();

    //Not for labels
    virtual bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress = 0);

    virtual int getOperationSize();

    //For labels and jumps only
    virtual std::string getIdentifier();

    virtual ~Instruction() {}
};

class JumpInstruction : public Instruction {
private:
    std::unordered_map<std::string, int> &_identifiersTable;
    std::string _argumentIdentifier;
    OperationPrefixCode _prefixCode;

    friend class InstructionFactory;

public:
    JumpInstruction(OperationPrefixCode prefixCode, std::string &argumentIdentifier,
                    std::unordered_map<std::string, int> &identifiersTable);

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override;

    int getOperationSize() override;

    std::string getIdentifier() override;

    virtual ~JumpInstruction() {}
};

class LabelInstruction : public Instruction {
private:
    std::string _identifier;

    friend class InstructionFactory;

public:

    LabelInstruction(const std::string &identifier);

    std::string getIdentifier();

    virtual ~LabelInstruction() {}

};

class NoArgsInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;
public:
    NoArgsInstruction(OperationPrefixCode prefixCode);

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override;

    int getOperationSize() override;

    virtual ~NoArgsInstruction() {}
};

class UnaryInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;
    char _operationArgument[8];
    int _argumentSize;

public:
    UnaryInstruction(OperationPrefixCode prefixCode, char *operationArgument, int argumentSize);

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override;

    int getOperationSize() override;

    virtual ~UnaryInstruction() {}

};

class InstructionParser {
private:

    static int getRegCodeByName(const std::string &name);

    static int getNoArgsOperationCodeByName(const std::string &name);

    static int getJumpOperationPrefixCodeByName(const std::string &name);

    static Instruction *getNoArgsInstructionByName(const std::string &name);

    static Instruction *getJumpInstruction(std::istream &in, std::ostream &logsStream, std::string keyword,
                                           std::unordered_map<std::string, int> &identifiersTable);

    static Instruction *getAddrInstruction(const std::string &keyword, std::string argument,
                                           std::ostream &logsStream);

    static Instruction *getValInstruction(const std::string &keyword, const std::string &argument,
                                          std::ostream &logsStream);

    static Instruction *getUnaryInstruction(std::istream &in, std::ostream &logsStream, std::string keyword);

    static bool isLabel(const std::string &identifier);

    static Instruction *getLabelInstruction(std::ostream &logsStream, std::string identifier);

public:

    //Returns pointer. It is needed to perform delete on this pointer since you don't need it anymore
    static Instruction *getInstruction(std::istream &in, std::unordered_map<std::string, int> &identifiersTable,
                                       std::ostream &logsStream);

};

class Assembler {
private:
    std::istream &_in;
    std::ostream &_out;
    std::ostream &_assemblerLogsStream;
    std::unordered_map<std::string, int> _identifiersTable;
    std::vector<Instruction *> _instructions;

    void freeInstructions();

    void prepareLabels();

public:
    Assembler(std::istream &in, std::ostream &out, std::ostream &logs);

    bool assembleAll();

};


#endif //STACK_PROCESSOR_ASSEMBLER_H
