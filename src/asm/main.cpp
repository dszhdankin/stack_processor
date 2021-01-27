#include <fstream>
#include <iostream>
#include "Assembler.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Paths to input and output files should be specified!" << std::endl;
        return 0;
    }

    std::ifstream in(argv[1]);
    std::ofstream out(argv[2], std::ios_base::binary | std::ios_base::out);

    Assembler assembler(in, out, std::clog);

    assembler.assembleAll();

    return 0;
}
