# Violet's Virtual Machine (VVM)

## Usage

To compile the entire project, you can run ``make all``. This will compile the assembler, emulator, and disassembler. You can also use ``make examples`` to build the current set of examples. These binaries are put into the examples folder. We also have the following tools:

#### Violet Assembler (VASM)

To use the assembler, you must supply and input file (.vasm) and an output file (.vm). The output file does not necessarily have to be created. To use the assembler you run:
``./vasm <input.vasm> <output.vm>``

#### Violet Emulator (VEM)

To use the emulator, you must specify the binary code file (.vm) as well as an execution limit. An execution limit of -1 is infinite, as well as an unspecified limit. To use the emulator you run:
``./vme -i <input.vasm> [-l <limit>] [-h]``
You can use the help flag `-h` for usage information.

#### Violet Disassembler (DEVASM)

To use the disassembler, you must specify the binary code file (.vm). To use the disassembler you run:
``./devasm <input.vm>``

## Useful Information

#### Instruction Set

As of now, the instruction set of VASM consists of the following instructions:
- [x] ``nop`` does nothing and increments the program counter.
- [x] ``push <x>`` pushes the integer value of `x` onto the top of the stack. If the stack size is greater than the stack capacity, we invoke ``ERR_STACK_OVERFLOW``.
- [x] ``rdup <x>`` duplicates the integer which is `x` below the top of the stack. `0` in this case refers to the top of the stack. If the stack size is greater than the stack capacity, we invoke ``ERR_STACK_OVERFLOW``. If the desired relative address `x` is greater than the current stack size, then we invoke ``ERR_STACK_UNDERFLOW``. If the desired relative address `x` is less than `0`, we invoke ``ERR_ILLEGAL_OPERAND``.
- [x] ``swap`` swaps the top two elements on the stack with eachother. If the input `x` is 0, then it just does that. However, if the input is anything else, it swaps the relative position of the stack elements. If the stack has less than two elements, then we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``addi`` adds the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``subi`` subtracts the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``muli`` multiplies the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``divi`` divides the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``addf`` adds the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``subf`` subtracts the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``mulf`` multiplies the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``divf`` divides the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``jmp <x>`` jumps the instruction pointer to the address given by `x`. In this case, `0` refers to the first executable instruction in the program.
- [x] ``jnz <x>`` jumps the instruction pointer to the address given by `x` if the top of the stack is not `0`. If the stack size is less than `1`, or the stack is empty, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``eq`` sets the top of the stack to be `0` if the top two elements of the stack are not equal, and to `1` if the elements are equal. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``not`` sets the top element of the stack to be the binary complement. For example, 1 becomes 0, and 0 becomes 1. If the stack size is less than `1`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``geq`` sets the top of the stack to be `0` if the top element is greater than or equal to the second element, and to `1` otherwise. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``halt`` halts the program from running, setting the `halt` flag in the virtual machine to true.
- [x] ``print_debug`` prints the top of the stack and eats it. If the stack size is less than `1`, we invoke `ERR_STACK_UNDERFLOW``.