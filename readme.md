# Stack machine

## Project description

This program is developed as a part of ISP RAS course.

This program contains a register stack machine with RAM. It can assemble and run programs.

NOTE: This program works only on POSIX-compliant OS.

### Dependencies
* Linux
* CMake 3.16 or later (in debian or ubuntu it can be installed in package cmake)
* make, g++, ld, etc. (in debian or ubuntu it can be installed in package build-essential)
```shell script
sudo apt install build-essential
sudo apt install cmake
```

### Structure

* src/ : Main project
    * asm/
        * Assembler.cpp : Assembler implementation
        * Assembler.h : Assembler definition
        * main.cpp : Assembler entry point
        * CMakeLists.txt
    * processor/
        * Processor.cpp : Processor implementation
        * Processor.h : Processor definition
        * main.cpp : Processor entry point
        * CMakeLists.txt
    * utils.h : Definitions used in both Processor and Assembler
    * CMakeLists.txt
* tests/ 
    * bytes_to_file.cpp : tool that was used to test processor when assembler was not ready
    * CMakeLists.txt
    * *.asm : assembler text files
    * files without extension : executable files
    * *.symb : were used to test processor when assembler was not ready  
* CMakeLists.txt

### Build and run

The best way to build is go to the project root folder and create a separate directory for cmake files.
Once you create the directory, you can go there and run cmake command giving parent as argument. 
Then rum make command.
```shell script
mkdir cmake_dir
cd cmake_dir
cmake ..
make
```
Now assembler and processor are src/asm/asm and src/processor/processor. Good idea to copy them to test
directory.
```shell script
cp src/asm/asm ../tests
cp src/processor/processor ../tests
cd ../tests
```
To run assembler and processor:
```shell script
./asm fibonacci.asm fibonacci
./processor fibonacci
```

NOTE: Certainly the mentor who will check this task is familiar with build tools. 
And probably he knows them much better than me)
So my apologies if it looks like a tutorial for dummies)

### Available operations

Assembly file can contain next operations:
```
in          # Read double value from console and put it on stack
out         # Pop value from stack and write it in console
popd        # Pop value from stack
pop ax      # Pop value from stack and put it into register
pop [1]     # Pop value from stack and put it into given RAM address
pop [ax]    # Pop value from stack and put it into RAM address located in register
push 1.5    # Put the given value on top of the stack
push ax     # Put the value from register on top of the stack
push [1]    # Put the value from RAM (at the given address) on top of the stack
push [ax]   # Put the value from RAM (at the address located in register) on top of the stack
add         # Pop two values from stack and put (under_top + top) on top of the stack
sub         # Pop two values from stack and put (under_top - top) on top of the stack
mul         # Pop two values from stack and put (under_top * top) on top of the stack
div         # Pop two values from stack and put (under_top / top) on top of the stack
sqrt        # Pop one value from stack and put the square root on top of the stack
sin         # Pop one value from stack and put the sine on top of the stack
cos         # Pop one value from stack and put the cosine on top of the stack
jmp label   # Unconditional jump to the given label
je label    # Jump to the given label if (under_top == top) (compared using 1e-9 epsilon)
jne label   # Jump to the given label if (under_top != top) (compared using 1e-9 epsilon)
ja          # Jump to the given label if (under_top > top) (compared using 1e-9 epsilon)
jae         # Jump to the given label if (under_top >= top) (compared using 1e-9 epsilon)
jb          # Jump to the given label if (under_top < top) (compared using 1e-9 epsilon)
jbe         # Jump to the given label if (under_top <= top) (compared using 1e-9 epsilon)
call label  # Put return address (PC of the command after this operation) on call stack and jump to the given label
ret         # Pop return address from call stack and move PC to that address
halt        # Stop the program
```

Program should end with `halt` command, otherwise it's behaviour is undefined.

Each command should be on separate line.

Possible registers: `ax`, `bx`, `cx`, `dx`.

Labels have to be written on separate lines, can not contain spaces and must end with `:` symbol. Example: `label:` 
(see more examples below).

#### Examples

Recursive factorial:
```
in
call fact
out
halt

fact:
push 0
ja rec
popd
popd
push 1
ret
rec:
popd
pop ax
push ax
push ax
push 1
sub
call fact
mul
ret
```

Recursive fibonacci number:
```
in
call fib
out
halt

fib:
push 1
ja rec
popd
popd
push 1
ret

rec:

popd
pop ax
push ax
push 1
sub
push ax
push 2
sub

call fib
pop bx
pop ax
push bx
push ax
call fib
add
ret
```

All examples can be found in tests directory