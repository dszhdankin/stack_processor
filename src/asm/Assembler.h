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
        _status = InstructionStatus::OK;
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

    LabelInstruction(const std::string &identifier): _identifier(identifier) {
        _status = InstructionStatus::OK;
    }

    std::string getIdentifier() {

        return _identifier;
    }

    virtual ~LabelInstruction() {}

};

class NoArgsInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;
public:
    NoArgsInstruction(OperationPrefixCode prefixCode) {
        _prefixCode = prefixCode;
        _status = InstructionStatus::OK;
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

    virtual ~NoArgsInstruction() {}
};

class UnaryInstruction : public Instruction {
private:
    OperationPrefixCode _prefixCode;
    char _operationArgument[8];
    int _argumentSize;

public:
    UnaryInstruction(OperationPrefixCode prefixCode, char *operationArgument, int argumentSize) {
        assert(argumentSize > 0 && argumentSize <= 8);
        _prefixCode = prefixCode;
        std::memcpy(_operationArgument, operationArgument, argumentSize);
        _argumentSize = argumentSize;
        _status = InstructionStatus::OK;
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

    virtual ~UnaryInstruction() {}

};

//TODO implement instruction factory for Unary NoArgs Jump and label instructions
class InstructionParser {
private:

    static int getRegCodeByName(const std::string &name) {
        if (name == "ax")
            return RegisterCode::AX;
        else if (name == "bx")
            return RegisterCode::BX;
        else if (name == "cx")
            return RegisterCode::CX;
        else if (name == "dx")
            return RegisterCode::DX;
        else
            return -1;
    }

    static int getNoArgsOperationCodeByName(const std::string &name) {
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
            return OperationPrefixCode::RET_ABS;
        else if (name == "popd")
            return OperationPrefixCode::POP;
        else if (name == "halt")
            return OperationPrefixCode::HALT;
        else
            return -1;
    }

    static int getJumpOperationPrefixCodeByName(const std::string &name) {
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
        else if (name == "call")
            return OperationPrefixCode::CALL_OFFSET_EXACT_VAL;
        else
            return -1;
    }

    static Instruction *getNoArgsInstructionByName(const std::string &name) {
        assert(getNoArgsOperationCodeByName(name) != -1);

        OperationPrefixCode prefixCode = static_cast<OperationPrefixCode>(getNoArgsOperationCodeByName(name));
        NoArgsInstruction *instruction = new NoArgsInstruction(prefixCode);

        return instruction;
    }

    static Instruction *getJumpInstruction(std::istream &in, std::ostream &logsStream, std::string keyword,
                                           std::unordered_map<std::string, int> &identifiersTable) {
        assert(getJumpOperationPrefixCodeByName(keyword) != -1);

        std::string identifier;
        in >> identifier;

        if (identifier.empty()) {
            logsStream << "Label cannot be empty!" << std::endl;
            Instruction *instruction = new Instruction;
            return instruction;
        }

        JumpInstruction *instruction = new JumpInstruction(
                static_cast<OperationPrefixCode>(getJumpOperationPrefixCodeByName(keyword)),
                identifier, identifiersTable);
        return instruction;
    }

    static Instruction *getAddrInstruction(const std::string &keyword, std::string argument,
                                           std::ostream &logsStream) {
        assert(keyword == "push" || keyword == "pop");
        assert(argument.size() > 1);
        assert(argument[0] == '[' && argument.back() == ']');

        argument = argument.substr(1, argument.size() - 2);
        int address = -1;
        try {
            address = std::stoi(argument);
            if (address < 0) {
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
                    new UnaryInstruction(prefixCode, reinterpret_cast<char *>(&address), sizeof (int));
            return instruction;
        }

        int registerCode = getRegCodeByName(argument);
        if (registerCode < 0) {
            logsStream << "Invalid register in " << keyword << " command!" << std::endl;
            return new Instruction;
        }

        OperationPrefixCode prefixCode = OperationPrefixCode::POP_REG_ADDR;
        if (keyword == "push")
            prefixCode = OperationPrefixCode::PUSH_REG_ADDR;

        Instruction *instruction =
                new UnaryInstruction(prefixCode, reinterpret_cast<char *>(&registerCode), 1);
        return instruction;
    }

    static Instruction *getValInstruction(const std::string &keyword, const std::string &argument,
                                          std::ostream &logsStream) {
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
                        new UnaryInstruction(OperationPrefixCode::PUSH_EXACT_VAL,
                                             reinterpret_cast<char*>(&val), sizeof (double ));
                return instruction;
            }
        }

        int regCode = getRegCodeByName(argument);
        if (regCode < 0) {
            logsStream << "Invalid register in " << keyword << " command!" << std::endl;
            return new Instruction;
        }

        OperationPrefixCode prefixCode = OperationPrefixCode::PUSH_REG_VAL;
        if (keyword == "pop")
            prefixCode = OperationPrefixCode::POP_REG_VAL;
        return new UnaryInstruction(prefixCode, reinterpret_cast<char*>(&regCode), 1);
    }

    static Instruction *getUnaryInstruction(std::istream &in, std::ostream &logsStream, std::string keyword) {
        assert(keyword == "push" || keyword == "pop");

        std::string argument;
        in >> argument;

        if (argument.empty()) {
            logsStream << "Command argument cannot be empty!" << std::endl;
            Instruction *instruction = new Instruction;
            return instruction;
        }

        if (argument[0] == '[' && argument.back() == ']')
            return getAddrInstruction(keyword, argument, logsStream);

        return getValInstruction(keyword, argument, logsStream);
    }

    static bool isLabel(const std::string &identifier) {
        if (identifier.empty())
            return false;
        return identifier.back() == ':';
    }

    static Instruction *getLabelInstruction(std::ostream &logsStream, std::string identifier) {
        assert(isLabel(identifier));
        identifier.pop_back();

        if (identifier.empty()) {
            logsStream << "Command argument cannot be empty!" << std::endl;
            return new Instruction;
        }

        return new LabelInstruction(identifier);
    }

public:

    //Returns pointer. It is needed to perform delete on this pointer since you don't need it anymore
    static Instruction *getInstruction(std::istream &in, std::unordered_map<std::string, int> &identifiersTable,
                                       std::ostream &logsStream) {
        std::string keyword;
        in >> keyword;
        if (getNoArgsOperationCodeByName(keyword) != -1) {
            Instruction *instruction = getNoArgsInstructionByName(keyword);
            return instruction;
        } else if (getJumpOperationPrefixCodeByName(keyword) != -1) {
            Instruction *instruction = getJumpInstruction(in, logsStream, keyword, identifiersTable);
            return instruction;
        } else if (keyword == "push" || keyword == "pop") {
            Instruction *instruction = getUnaryInstruction(in, logsStream, keyword);
            return instruction;
        } else if (isLabel(keyword)) {
            Instruction *instruction = getLabelInstruction(logsStream, keyword);
            return instruction;
        } else {
            Instruction *instruction = new Instruction;
            return instruction;
        }
    }

};

class Assembler {
private:
    std::istream &_in;
    std::ostream &_out;
    std::ostream &_assemblerLogsStream;
    std::unordered_map<std::string, int> _identifiersTable;
    std::vector<Instruction *> _instructions;

    void freeInstructions() {
        for (Instruction* val : _instructions)
            delete val;
        _instructions.clear();
    }

    void prepareLabels() {
        int curAddr = 0;
        for (Instruction* val : _instructions) {
            if (LabelInstruction *v = dynamic_cast<LabelInstruction *>(val)) {
                _identifiersTable[v->getIdentifier()] = curAddr;
            }
            curAddr += val->getOperationSize();
        }
    }

public:
    Assembler(std::istream &in, std::ostream &out, std::ostream &logs): _in(in), _out(out), _assemblerLogsStream(logs){

    }

    bool assembleAll() {
        _identifiersTable.clear();
        freeInstructions();
        _instructions.reserve(1000);

        while (!_in.eof()) {
            Instruction *instruction = InstructionParser::getInstruction(_in, _identifiersTable, _assemblerLogsStream);

            if (instruction->getStatus() == InstructionStatus::FAILED) {
                delete instruction;
                break;
            }

            _instructions.push_back(instruction);
        }

        prepareLabels();

        int curAddr = 0;
        char buf[20];
        for (Instruction* curInst : _instructions) {
            if (LabelInstruction *v = dynamic_cast<LabelInstruction *>(curInst))
                continue;
            bool success = curInst->tryGetOperationCode(buf, 20, curAddr);
            if (!success) {
                _assemblerLogsStream << "Cannot generate code!" << std::endl;
                freeInstructions();
                return false;
            }
            _out.write(buf, curInst->getOperationSize());
            curAddr += curInst->getOperationSize();
        }

        return true;
    }



};


#endif //STACK_PROCESSOR_ASSEMBLER_H
