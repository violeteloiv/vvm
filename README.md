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
- [x] ``addi`` adds the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``subi`` subtracts the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``muli`` multiplies the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``divi`` divides the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``addf`` adds the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``subf`` subtracts the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``mulf`` multiplies the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``divf`` divides the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``jmp <x>`` jumps the instruction pointer to the address given by `x`. In this case, `0` refers to the first executable instruction in the program.
- [ ] ``jnz <x>`` jumps the instruction pointer to the address given by `x` if the top of the stack is not `0`. If the stack size is less than `1`, or the stack is empty, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``eq`` sets the top of the stack to be `0` if the top two elements of the stack are not equal, and to `1` if the elements are equal. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``halt`` halts the program from running, setting the `halt` flag in the virtual machine to true.