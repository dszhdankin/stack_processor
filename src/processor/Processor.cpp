//
// Created by dszhdankin on 24.01.2021.
//

#include "Processor.h"
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdio>

void RAM::store(double val, int address) {
    assert(address >= 0 && address < MEM_SIZE);

    DoubleChars buf;
    buf.db_val = val;
    for (int i = 0; i < sizeof (double); i++) {
        mem[i + address] = buf.chr_val[i];
    }

}

bool RAM::isValidAddr(int addr) { return addr >= 0 && addr < MEM_SIZE; }

double RAM::load(int address) {
    assert(address >= 0 && address < MEM_SIZE);

    DoubleChars buf;
    for (int i = 0; i < sizeof (double); i++)
        buf.chr_val[i] = mem[i + address];
    return buf.db_val;
}

bool Processor::isNoArgsOperation(char prefixCode) {
    return prefixCode >= OperationPrefixCode::IN && prefixCode <= OperationPrefixCode::POP;
}

bool Processor::isHalt(char prefixCode) {
    return prefixCode == OperationPrefixCode::HALT;
}

bool Processor::isJump(char prefixCode) {
    return (prefixCode >= OperationPrefixCode::JMP_OFFSET_EXACT_VAL && prefixCode <= CALL_OFFSET_EXACT_VAL)
           || prefixCode == OperationPrefixCode::RET_ABS;
}

double Processor::getDouble(char *buf) {
    DoubleChars res;
    for (int i = 0; i < sizeof (double); i++)
        res.chr_val[i] = buf[i];
    return res.db_val;
}

int Processor::getInt(char *buf) {
    IntChars res;
    for (int i = 0; i < sizeof (int); i++)
        res.chr_val[i] = buf[i];
    return res.itn_val;
}

bool Processor::isCommand(int prefixCode) {
    return OperationPrefixCode::IN <= prefixCode && prefixCode <= OperationPrefixCode::CALL_OFFSET_EXACT_VAL;
}

int Processor::getCommandLength(char prefixCode) {
    assert(isCommand(prefixCode));

    if (isNoArgsOperation(prefixCode))
        return 1;
    else if (isJump(prefixCode))
        return 1 + sizeof (int);
    else {
        if (prefixCode == OperationPrefixCode::POP_REG_ADDR || prefixCode == OperationPrefixCode::PUSH_REG_VAL ||
            prefixCode == OperationPrefixCode::PUSH_REG_ADDR || prefixCode == OperationPrefixCode::POP_REG_VAL)
            return 2;
        else if (prefixCode == OperationPrefixCode::PUSH_EXACT_ADDR ||
                 prefixCode == OperationPrefixCode::POP_EXACT_ADDR)
            return 1 + sizeof (int);
        else
            return 1 + sizeof (double);
    }
}

ProcessorStatus Processor::executeJumpOperation(char prefixCode) {
    assert(isJump(prefixCode));
    assert(prefixCode != OperationPrefixCode::HALT);

    if (prefixCode == OperationPrefixCode::RET_ABS) {
        if (_call_stack.empty())
            return ProcessorStatus::CALL_STACK_UNDERFLOW;
        _ip = _call_stack.back();
        _call_stack.pop_back();
        return ProcessorStatus::SUCCESS;
    }

    if (_ip + sizeof (int) >= _start + _operations_size)
        return ProcessorStatus::COMMAND_ARG_ERROR;

    int offset = getInt(_ip + 1);

    if (prefixCode == OperationPrefixCode::CALL_OFFSET_EXACT_VAL) {
        _call_stack.push_back(_ip + getCommandLength(prefixCode));
        _ip += offset;
    } else if (prefixCode == OperationPrefixCode::JMP_OFFSET_EXACT_VAL) {
        _ip += offset;
    } else {
        if (_data_stack.size() < 2)
            return ProcessorStatus::DATA_STACK_UNDERFLOW;
        double left = _data_stack[_data_stack.size() - 2], right = _data_stack.back();
        bool condition = false;
        switch (prefixCode) {
            case OperationPrefixCode::JE_OFFSET_EXACT_VAL:
                if (std::fabs(right - left) < PROCESSOR_EPSILON)
                    condition = true;
                break;
            case OperationPrefixCode::JNE_OFFSET_EXACT_VAL:
                if (std::fabs(right - left) >= PROCESSOR_EPSILON)
                    condition = true;
                break;
            case OperationPrefixCode::JA_OFFSET_EXACT_VAL:
                if (left > right + PROCESSOR_EPSILON)
                    condition = true;
                break;
            case OperationPrefixCode::JAE_OFFSET_EXACT_VAL:
                if (left > right + PROCESSOR_EPSILON || fabs(right - left) < PROCESSOR_EPSILON)
                    condition = true;
                break;
            case OperationPrefixCode::JB_OFFSET_EXACT_VAL:
                if (left + PROCESSOR_EPSILON < right)
                    condition = true;
                break;
            case OperationPrefixCode::JBE_OFFSET_EXACT_VAL:
                if (left + PROCESSOR_EPSILON < right || fabs(right - left) < PROCESSOR_EPSILON)
                    condition = true;
                break;
        }
        if (condition)
            _ip += offset;
        else
            _ip += getCommandLength(prefixCode);
    }

    if (_ip < _start || _ip >= _start + _operations_size)
        return ProcessorStatus::INVALID_INSTRUCTION_POINTER;
    return ProcessorStatus::SUCCESS;
}

ProcessorStatus Processor::executeNoArgsNonJump(char prefixCode) {
    assert(isNoArgsOperation(prefixCode) && !isJump(prefixCode));
    assert(prefixCode != OperationPrefixCode::HALT);
    assert(prefixCode != OperationPrefixCode::RET_ABS);

    if (prefixCode == OperationPrefixCode::IN) {
        double val = 0.0;
        std::printf("in: ");
        std::scanf("%lg", &val);
        _data_stack.push_back(val);
    } else if (prefixCode == OperationPrefixCode::OUT) {
        if (_data_stack.empty())
            return ProcessorStatus::DATA_STACK_UNDERFLOW;
        double val = _data_stack.back();
        _data_stack.pop_back();
        std::printf("out: %lg\n", val);
    } else if (OperationPrefixCode::ADD <= prefixCode && prefixCode <= OperationPrefixCode::DIV) {
        if (_data_stack.size() < 2)
            return ProcessorStatus::DATA_STACK_UNDERFLOW;
        double left = _data_stack[_data_stack.size() - 2], right = _data_stack.back();
        _data_stack.pop_back();
        _data_stack.pop_back();
        switch (prefixCode) {
            case OperationPrefixCode::ADD: _data_stack.push_back(left + right); break;
            case OperationPrefixCode::SUB: _data_stack.push_back(left - right); break;
            case OperationPrefixCode::MUL: _data_stack.push_back(left * right); break;
            case OperationPrefixCode::DIV: _data_stack.push_back(left / right); break;
        }
    } else {
        if (_data_stack.empty())
            return ProcessorStatus::DATA_STACK_UNDERFLOW;
        double val = _data_stack.back();
        _data_stack.pop_back();
        switch (prefixCode) {
            case OperationPrefixCode::SIN: _data_stack.push_back(std::sin(val)); break;
            case OperationPrefixCode::COS: _data_stack.push_back(std::cos(val)); break;
            case OperationPrefixCode::SQRT: _data_stack.push_back(std::sqrt(val)); break;
        }
    }

    _ip += getCommandLength(prefixCode);
    return ProcessorStatus::SUCCESS;
}

ProcessorStatus Processor::executeRegArg(char prefixCode) {
    if (_ip + 2 > _start + _operations_size)
        return ProcessorStatus::COMMAND_ARG_ERROR;

    char regCode = _ip[1];
    double val;
    int addr;
    switch (prefixCode) {
        case OperationPrefixCode::POP_REG_VAL:
            if (_data_stack.empty())
                return ProcessorStatus::DATA_STACK_UNDERFLOW;
            val = _data_stack.back();
            _data_stack.pop_back();
            _reg[regCode].db_val = val;
            break;
        case OperationPrefixCode::PUSH_REG_VAL:
            val = _reg[regCode].db_val;
            _data_stack.push_back(val);
            break;
        case OperationPrefixCode::PUSH_REG_ADDR:
            addr = _reg[regCode].ull_val;
            if (!RAM::isValidAddr(addr))
                return ProcessorStatus::INVALID_RAM_ADDRESS;
            val = _ram->load(addr);
            _data_stack.push_back(val);
            break;
        case OperationPrefixCode::POP_REG_ADDR:
            if (_data_stack.empty())
                return ProcessorStatus::DATA_STACK_UNDERFLOW;
            addr = _reg[regCode].ull_val;
            if (!RAM::isValidAddr(addr))
                return ProcessorStatus::INVALID_RAM_ADDRESS;
            val = _data_stack.back();
            _data_stack.pop_back();
            _ram->store(val, addr);
    }
    _ip += 2;
    return ProcessorStatus::SUCCESS;
}

ProcessorStatus Processor::executeExactAddr(char prefixCode) {
    if (_ip + 1 + sizeof (int) > _start + _operations_size)
        return ProcessorStatus::COMMAND_ARG_ERROR;

    int addr = getInt(_ip + 1);
    if (!RAM::isValidAddr(addr))
        return ProcessorStatus::INVALID_RAM_ADDRESS;

    if (prefixCode == OperationPrefixCode::PUSH_EXACT_ADDR) {
        double val = _ram->load(addr);
        _data_stack.push_back(val);
    } else if (prefixCode == OperationPrefixCode::POP_EXACT_ADDR) {
        if (_data_stack.empty())
            return ProcessorStatus::DATA_STACK_UNDERFLOW;
        double val = _data_stack.back();
        _data_stack.pop_back();
        _ram->store(val, addr);
    }

    _ip += 1 + sizeof (int);
    return ProcessorStatus::SUCCESS;
}

ProcessorStatus Processor::executeUnaryNonJump(char prefixCode) {
    assert(isCommand(prefixCode) && !isJump(prefixCode) && !isNoArgsOperation(prefixCode));

    if (prefixCode == OperationPrefixCode::POP_REG_VAL || prefixCode == OperationPrefixCode::PUSH_REG_VAL ||
        prefixCode == OperationPrefixCode::PUSH_REG_ADDR || prefixCode == OperationPrefixCode::POP_REG_ADDR) {
        return executeRegArg(prefixCode);
    } else if (prefixCode == OperationPrefixCode::POP_EXACT_ADDR ||
               prefixCode == OperationPrefixCode::PUSH_EXACT_ADDR) {
        return executeExactAddr(prefixCode);
    } else if (prefixCode == OperationPrefixCode::PUSH_EXACT_VAL) {
        if (_ip + 1 + sizeof (double) > _start + _operations_size)
            return ProcessorStatus::COMMAND_ARG_ERROR;

        double val = getDouble(_ip + 1);
        _data_stack.push_back(val);
        _ip += 1 + sizeof (double);
        return ProcessorStatus::SUCCESS;
    }

    return ProcessorStatus::UNRECOGNIZED_COMMAND;
}

Processor::Processor() {
    _call_stack.reserve(1000);
    _data_stack.reserve(1000);
    _ram.reset(new RAM);
}

ProcessorStatus Processor::executeOperations(char *start, int size) {
    std::memset(_reg, 0, sizeof(double) * 4);
    _start = _ip = start;
    _operations_size = size;

    while (_ip >= _start && _ip < _start + _operations_size) {
        char prefixCode = _ip[0];

        if (!isCommand(prefixCode))
            return ProcessorStatus::UNRECOGNIZED_COMMAND;

        if (isHalt(prefixCode))
            return ProcessorStatus::SUCCESS;

        if (isJump(prefixCode)) {
            ProcessorStatus status = executeJumpOperation(prefixCode);
            if (status != ProcessorStatus::SUCCESS)
                return status;
        } else if (isNoArgsOperation(prefixCode)) {
            ProcessorStatus status = executeNoArgsNonJump(prefixCode);
            if (status != ProcessorStatus::SUCCESS)
                return status;
        } else {
            ProcessorStatus status = executeUnaryNonJump(prefixCode);
            if (status != ProcessorStatus::SUCCESS)
                return status;
        }

    }

    return ProcessorStatus::SUCCESS;
}

std::string Processor::statusToStr(ProcessorStatus status) {
    switch (status) {
        case ProcessorStatus::SUCCESS:
            return "success";
        case ProcessorStatus::DATA_STACK_UNDERFLOW:
            return "data stack underflow";
        case ProcessorStatus::UNRECOGNIZED_COMMAND:
            return "unrecognized command";
        case ProcessorStatus::INVALID_RAM_ADDRESS:
            return "invalid RAM address";
        case ProcessorStatus::COMMAND_ARG_ERROR:
            return "command arg error";
        case ProcessorStatus::INVALID_INSTRUCTION_POINTER:
            return "invalid instruction pointer";
        case ProcessorStatus::CALL_STACK_UNDERFLOW:
            return "call stack underflow";
        default:
            return "";
    }
}