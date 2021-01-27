#include "Processor.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "No file path provided!" << std::endl;
        return 0;
    }

    FILE *executableFile = fopen(argv[1], "rb");
    if (executableFile == nullptr) {
        std::cout << "Cannot open file" << std::endl;
        return 0;
    }

    struct stat fileStat;
    int res = fstat(fileno(executableFile), &fileStat);
    if (res < 0 || fileStat.st_size == 0) {
        fclose(executableFile);
        std::cout << "File is empty!" << std::endl;
        return 0;
    }

    void *codePtr = mmap(nullptr, fileStat.st_size, PROT_READ, MAP_PRIVATE, fileno(executableFile), 0);
    fclose(executableFile);
    if (codePtr == MAP_FAILED) {
        std::cout << "File memory mapping failed!" << std::endl;
        return 0;
    }

    Processor processor;

    ProcessorStatus status = processor.executeOperations(static_cast<char*>(codePtr), fileStat.st_size);

    std::cout << Processor::statusToStr(status) << std::endl;

    munmap(codePtr, fileStat.st_size);

    return 0;
}
