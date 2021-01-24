//
// Created by dszhdankin on 22.01.2021.
//

#ifndef STACK_PROCESSOR_ASSEMBLER_H
#define STACK_PROCESSOR_ASSEMBLER_H

#include <iostream>
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <exception>
#include <cmath>
#include <limits>

enum ProcessorLimits {
    MEM_SIZE = 1024 * 1024
};

class Assembler {
private:
    std::istream &_in;
    std::ostream &_out;
    std::ostream &_assemblerLogsStream;

    friend class InstructionFactory;

public:
    Assembler(std::istream &in, std::ostream &out, std::ostream &logs): _in(in), _out(out), _assemblerLogsStream(logs){

    }

    bool assembleAll() {
        return false;
    }



};

enum InstructionStatus {
    SUCCESS = 0,
    FAILED
};

enum OperationPrefixCode {
    IN = 1, //Reads from stdin and pushes to stack
    OUT, //Pops from stack and writes to stdout
    PUSH_REG_VAL,
    PUSH_EXACT_VAL,
    PUSH_REG_ADDR,
    PUSH_EXACT_ADDR,
    POP_REG_VAL,
    POP_EXACT_ADDR,
    POP_REG_ADDR,
    ADD,
    SUB,
    MUL,
    DIV,
    SIN,
    COS,
    SQRT,
    JMP_OFFSET_EXACT_VAL,
    JE_OFFSET_EXACT_VAL,
    JNE_OFFSET_EXACT_VAL,
    JA_OFFSET_EXACT_VAL,
    JAE_OFFSET_EXACT_VAL,
    JB_OFFSET_EXACT_VAL,
    JBE_OFFSET_EXACT_VAL,
    CALL_OFFSET_EXACT_VAL,
    JMP_OFFSET_REG,
    JE_OFFSET_REG,
    JNE_OFFSET_REG,
    JA_OFFSET_REG,
    JAE_OFFSET_REG,
    JB_OFFSET_REG,
    JBE_OFFSET_REG,
    CALL_OFFSET_REG,
    RET_OFFSET,
    HALT
};

enum RegisterCode {
    AX = 1,
    BX,
    CX,
    DX,
    IP, //instruction pointer
    DB, //data base (it's value and values above stand for addressing data)
    DP  //data pointer (stands for storing pointers to data)
};

class Instruction {
protected:
    InstructionStatus _status;

    friend class InstructionFactory;

public:
    Instruction() {
        _status = InstructionStatus::FAILED;
    }


    InstructionStatus getStatus() { return _status; }

    //Not for labels
    virtual bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress = 0) { return false; }

    virtual int getOperationSize() { return 0; }

    //For labels and jumps only
    virtual std::string getIdentifier() { return ""; }

    virtual ~Instruction() {}
};

class IOInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;

    friend class InstructionFactory;

public:
    IOInstruction(OperationPrefixCode prefixCode) {
        _prefixCode = prefixCode;
        _status = InstructionStatus::SUCCESS;
    }

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override {
        if (bufSize < 1)
            return false;
        buf[0] = static_cast<char>(_prefixCode);
        return true;
    }

    int getOperationSize() override {
        return 1;
    }

    virtual ~IOInstruction() {}
};

class StackOperationInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;
    char _operationArgument[8];
    int _argumentSize;

    friend class InstructionFactory;

public:
    StackOperationInstruction(OperationPrefixCode prefixCode, char *operationArgument, int argumentSize) {
        assert(argumentSize > 0 && argumentSize <= 8);
        _prefixCode = prefixCode;
        std::memcpy(_operationArgument, operationArgument, argumentSize);
        _argumentSize = argumentSize;
        _status = InstructionStatus::SUCCESS;
    }

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override {
        if (bufSize < 1 + _argumentSize)
            return false;
        buf[0] = static_cast<char>(_prefixCode);
        std::memcpy(buf + 1, _operationArgument, _argumentSize);
        return true;
    }

    int getOperationSize() override {
        return 1 + _argumentSize;
    }

    virtual ~StackOperationInstruction() {}
};

class MathOperationInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;

    friend class InstructionFactory;

public:
    MathOperationInstruction(OperationPrefixCode prefixCode) {
        _prefixCode = prefixCode;
        _status = InstructionStatus::SUCCESS;
    }

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override {
        if (bufSize < 1)
            return false;
        buf[0] = static_cast<char>(_prefixCode);
        return true;
    }

    int getOperationSize() override {
        return 1;
    }

    virtual ~MathOperationInstruction() {}
};

class JumpInstruction : public Instruction {
private:
    std::unordered_map<std::string, int> &_identifiersTable;
    std::string _argumentIdentifier;
    OperationPrefixCode _prefixCode;

    friend class InstructionFactory;

public:
    JumpInstruction(OperationPrefixCode prefixCode, std::string &argumentIdentifier,
                    std::unordered_map<std::string, int> &identifiersTable):
    _identifiersTable(identifiersTable), _argumentIdentifier(argumentIdentifier){
        _prefixCode = prefixCode;
        _status = InstructionStatus::SUCCESS;
    }

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) override {
        if (bufSize < 1 + sizeof (int))
            return false;
        if (_identifiersTable.find(_argumentIdentifier) == _identifiersTable.end())
            return false;
        int labelAddress = _identifiersTable[_argumentIdentifier];
        int offset = labelAddress - instructionAddress;
        buf[0] = _prefixCode;
        memcpy(buf + 1, &offset, sizeof (int));
        return true;
    }

    int getOperationSize() override {
        return 1 + sizeof (int);
    }

    std::string getIdentifier() override {
        return _argumentIdentifier;
    }

    virtual ~JumpInstruction() {}
};

class LabelInstruction : public Instruction {
private:
    std::string _identifier;

    friend class InstructionFactory;

public:

    LabelInstruction(std::string &identifier): _identifier(identifier) {
        _status = InstructionStatus::SUCCESS;
    }
    LabelInstruction(std::string &&identifier): _identifier(identifier) {
        _status = InstructionStatus::SUCCESS;
    }

    std::string getIdentifier() {

        return _identifier;
    }

    virtual ~LabelInstruction() {}

};

class TerminateInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;

    friend class InstructionFactory;

public:

    TerminateInstruction(OperationPrefixCode prefixCode) {
        _prefixCode = prefixCode;
        _status = InstructionStatus::SUCCESS;
    }

    int getOperationSize() {
        return 1;
    }

    bool tryGetOperationCode(char *buf, int bufSize, int instructionAddress) {
        if (bufSize < 1)
            return false;
        buf[0] = static_cast<char>(_prefixCode);
        return true;
    }

};

//TODO implement instruction factory
class InstructionFactory {
private:

    int getRegCodeByName(const std::string &name) {
        if (name == "ax")
            return RegisterCode::AX;
        else if (name == "bx")
            return RegisterCode::BX;
        else if (name == "cx")
            return RegisterCode::CX;
        else if (name == "dx")
            return RegisterCode::DX;
        else if (name == "ip")
            return RegisterCode::IP;
        else if (name == "dp")
            return RegisterCode::DP;
        else if (name == "db")
            return RegisterCode::DB;
        else
            return -1;
    }

    Instruction *getStackAddressInstruction(std::istream &in, std::ostream &logsStream, const std::string &keyword,
        std::string argument) {
        argument = argument.substr(1, argument.size() - 2);
        int address = -1;
        try {
            address = std::stoi(argument);
            if (address < 0 || address >= ProcessorLimits::MEM_SIZE) {
                logsStream << "Invalid argument of " << keyword << " command!" << std::endl;
                Instruction *instruction = new Instruction;
                return instruction;
            }
        } catch (std::invalid_argument &e) {

        } catch (std::out_of_range &e) {
            //will never occur in our test examples
            logsStream << "Invalid argument of " << keyword << " command!" << std::endl;
            Instruction *instruction = new Instruction;
            return instruction;
        }

        if (address > -1) {
            OperationPrefixCode prefixCode = OperationPrefixCode::POP_EXACT_ADDR;
            if (keyword == "push")
                prefixCode = OperationPrefixCode::PUSH_EXACT_ADDR;
            Instruction *instruction =
                    new StackOperationInstruction(prefixCode, reinterpret_cast<char *>(&address), sizeof (int));
            return instruction;
        }

        int registerCode = getRegCodeByName(argument);
        if (registerCode < 1) {
            logsStream << "Invalid register in " << keyword << " command!" << std::endl;
            return new Instruction;
        }

        OperationPrefixCode prefixCode = OperationPrefixCode::POP_REG_ADDR;
        if (keyword == "push")
            prefixCode = OperationPrefixCode::PUSH_REG_ADDR;

        Instruction *instruction =
                new StackOperationInstruction(prefixCode, reinterpret_cast<char *>(&registerCode), 1);
        return instruction;
    }

    Instruction *getStackValInstruction(std::istream &in, std::ostream &logsStream, const std::string &keyword,
                                        const std::string &argument) {
        if (keyword == "push") {
            double val = std::numeric_limits<double>::quiet_NaN();
            bool valInit = false;
            try {
                val = std::stod(argument);
                valInit = true;
            } catch (std::invalid_argument &e) {

            } catch (std::out_of_range &e) {
                //Will never occur in our test examples
                logsStream << "Invalid argument in " << keyword << " command!" << std::endl;
                return new Instruction;
            }

            if (valInit) {
                Instruction *instruction =
                        new StackOperationInstruction(OperationPrefixCode::PUSH_EXACT_VAL,
                                                      reinterpret_cast<char*>(&val), sizeof (double ));
                return instruction;
            }
        }

        int regCode = getRegCodeByName(argument);
        if (regCode < 1) {
            logsStream << "Invalid register in " << keyword << " command!" << std::endl;
            return new Instruction;
        }

        OperationPrefixCode prefixCode = OperationPrefixCode::PUSH_REG_VAL;
        if (keyword == "pop")
            prefixCode = OperationPrefixCode::POP_REG_VAL;
        return new StackOperationInstruction(prefixCode, reinterpret_cast<char*>(&regCode), 1);
    }

    Instruction *getStackOperationInstruction(std::istream &in, std::ostream &logsStream, const std::string &keyword) {
        std::string argument;

        in >> argument;
        if (argument.empty()) {
            Instruction *instruction = new Instruction;
            logsStream << "Invalid argument of " << keyword << " command!" << std::endl;
            return instruction;
        }

        if (argument[0] == '[' && argument.back() == ']') {
            return getStackAddressInstruction(in, logsStream, keyword, argument);
        } else {
            return getStackValInstruction(in, logsStream, keyword, argument);
        }
    }

    OperationPrefixCode getUnaryOperationCodeByName(const std::string &name) {
        if (name == "add")
            return OperationPrefixCode::ADD;
        else if (name == "sub")
            return OperationPrefixCode::SUB;
        else if (name == "mul")
            return OperationPrefixCode::MUL;
        else if (name == "div")
            return OperationPrefixCode::DIV;
        else if (name == "sin")
            return OperationPrefixCode::SIN;
        else if (name == "cos")
            return OperationPrefixCode::COS;
        else if (name == "sqrt")
            return OperationPrefixCode::SQRT;
        else if (name == "in")
            return OperationPrefixCode::IN;
        else if (name == "out")
            return OperationPrefixCode::OUT;
        else if (name == "ret")
            return OperationPrefixCode::RET_OFFSET;
        else
            return OperationPrefixCode::HALT;
    }

    OperationPrefixCode getJumpOperationPrefixCodeByName(const std::string &name) {
        if (name == "jmp")
            return OperationPrefixCode::JMP_OFFSET_EXACT_VAL;
        else if (name == "je")
            return OperationPrefixCode::JE_OFFSET_EXACT_VAL;
        else if (name == "jne")
            return OperationPrefixCode::JNE_OFFSET_EXACT_VAL;
        else if (name == "ja")
            return OperationPrefixCode::JA_OFFSET_EXACT_VAL;
        else if (name == "jae")
            return OperationPrefixCode::JAE_OFFSET_EXACT_VAL;
        else if (name == "jb")
            return OperationPrefixCode::JB_OFFSET_EXACT_VAL;
        else if (name == "jbe")
            return OperationPrefixCode::JBE_OFFSET_EXACT_VAL;
        else
            return OperationPrefixCode::CALL_OFFSET_EXACT_VAL;
    }

public:

    //Returns pointer. It is needed to perform delete on this pointer since you don't need it anymore
    Instruction *getInstruction(std::istream &in, std::unordered_map<std::string, int> &identifiersTable,
                                std::ostream &logsStream) {
        std::string keyword;
        in >> keyword;
        if (keyword == "in" || keyword == "out") {
            OperationPrefixCode prefixCode = getUnaryOperationCodeByName(keyword);
            return new IOInstruction(prefixCode);
        } else if (keyword == "push" || keyword == "pop") {
            return getStackOperationInstruction(in, logsStream, keyword);
        } else if (keyword == "add" || keyword == "sub" || keyword == "mul" || keyword == "div" ||
                    keyword == "sin" || keyword == "cos" || keyword == "sqrt") {
            OperationPrefixCode prefixCode = getUnaryOperationCodeByName(keyword);
            return new MathOperationInstruction(prefixCode);
        } else if (keyword == "jmp" || keyword == "je" || keyword == "jne" || keyword == "ja" ||
                    keyword == "jae" || keyword == "jb" || keyword == "jbe" || keyword == "call") {
            OperationPrefixCode prefixCode = getJumpOperationPrefixCodeByName(keyword);
            std::string argument;
            std::cin >> argument;
            Instruction *instruction = new JumpInstruction(prefixCode, argument, identifiersTable);
            return instruction;
        } else if (keyword == "ret" || keyword == "halt") {

        }
    }

};


#endif //STACK_PROCESSOR_ASSEMBLER_H
