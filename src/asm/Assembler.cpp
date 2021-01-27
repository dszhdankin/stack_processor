//
// Created by dszhdankin on 22.01.2021.
//

#include "Assembler.h"
#include <exception>
#include <cstring>
#include <iostream>
#include <cassert>

Instruction::Instruction() {
    _status = InstructionStatus::FAILED;
}

InstructionStatus Instruction::getStatus() { return _status; }

bool Instruction::tryGetOperationCode(char *buf, int bufSize, int instructionAddress) { return false; }

int Instruction::getOperationSize() { return 0; }

std::string Instruction::getIdentifier() { return ""; }

JumpInstruction::JumpInstruction(OperationPrefixCode prefixCode, std::string &argumentIdentifier,
                                 std::unordered_map<std::string, int> &identifiersTable):
        _identifiersTable(identifiersTable), _argumentIdentifier(argumentIdentifier){
    _prefixCode = prefixCode;
    _status = InstructionStatus::OK;
}

bool JumpInstruction::tryGetOperationCode(char *buf, int bufSize, int instructionAddress)  {
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

int JumpInstruction::getOperationSize() {
    return 1 + sizeof (int);
}

std::string JumpInstruction::getIdentifier() {
    return _argumentIdentifier;
}

LabelInstruction::LabelInstruction(const std::string &identifier): _identifier(identifier) {
    _status = InstructionStatus::OK;
}

std::string LabelInstruction::getIdentifier() {
    return _identifier;
}

NoArgsInstruction::NoArgsInstruction(OperationPrefixCode prefixCode) {
    _prefixCode = prefixCode;
    _status = InstructionStatus::OK;
}

bool NoArgsInstruction::tryGetOperationCode(char *buf, int bufSize, int instructionAddress) {
    if (bufSize < 1)
        return false;
    buf[0] = static_cast<char>(_prefixCode);
    return true;
}

int NoArgsInstruction::getOperationSize() {
    return 1;
}

UnaryInstruction::UnaryInstruction(OperationPrefixCode prefixCode, char *operationArgument, int argumentSize) {
    assert(argumentSize > 0 && argumentSize <= 8);
    _prefixCode = prefixCode;
    std::memcpy(_operationArgument, operationArgument, argumentSize);
    _argumentSize = argumentSize;
    _status = InstructionStatus::OK;
}

bool UnaryInstruction::tryGetOperationCode(char *buf, int bufSize, int instructionAddress) {
    if (bufSize < 1 + _argumentSize)
        return false;
    buf[0] = static_cast<char>(_prefixCode);
    std::memcpy(buf + 1, _operationArgument, _argumentSize);
    return true;
}

int UnaryInstruction::getOperationSize() {
    return 1 + _argumentSize;
}

int InstructionParser::getRegCodeByName(const std::string &name) {
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

int InstructionParser::getNoArgsOperationCodeByName(const std::string &name) {
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

int InstructionParser::getJumpOperationPrefixCodeByName(const std::string &name) {
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

Instruction * InstructionParser::getNoArgsInstructionByName(const std::string &name) {
    assert(getNoArgsOperationCodeByName(name) != -1);

    OperationPrefixCode prefixCode = static_cast<OperationPrefixCode>(getNoArgsOperationCodeByName(name));
    NoArgsInstruction *instruction = new NoArgsInstruction(prefixCode);

    return instruction;
}

Instruction * InstructionParser::getJumpInstruction(std::istream &in, std::ostream &logsStream, std::string keyword,
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

Instruction * InstructionParser::getAddrInstruction(const std::string &keyword, std::string argument,
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

Instruction * InstructionParser::getValInstruction(const std::string &keyword, const std::string &argument,
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

Instruction * InstructionParser::getUnaryInstruction(std::istream &in, std::ostream &logsStream, std::string keyword) {
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

bool InstructionParser::isLabel(const std::string &identifier) {
    if (identifier.empty())
        return false;
    return identifier.back() == ':';
}

Instruction * InstructionParser::getLabelInstruction(std::ostream &logsStream, std::string identifier) {
    assert(isLabel(identifier));
    identifier.pop_back();

    if (identifier.empty()) {
        logsStream << "Command argument cannot be empty!" << std::endl;
        return new Instruction;
    }

    return new LabelInstruction(identifier);
}

Instruction * InstructionParser::getInstruction(std::istream &in,
                                                std::unordered_map<std::string, int> &identifiersTable,
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

void Assembler::freeInstructions() {
    for (Instruction* val : _instructions)
        delete val;
    _instructions.clear();
}

void Assembler::prepareLabels() {
    int curAddr = 0;
    for (Instruction* val : _instructions) {
        if (LabelInstruction *v = dynamic_cast<LabelInstruction *>(val)) {
            _identifiersTable[v->getIdentifier()] = curAddr;
        }
        curAddr += val->getOperationSize();
    }
}

Assembler::Assembler(std::istream &in, std::ostream &out, std::ostream &logs): _in(in), _out(out),
    _assemblerLogsStream(logs) {
}

bool Assembler::assembleAll() {
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