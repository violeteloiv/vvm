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
- [x] ``add`` adds the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``sub`` subtracts the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``mult`` multiplies the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``div`` divides the top two elements of the stack together, eliminating the operands from the stack and leaving the result. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``jmp <x>`` jumps the instruction pointer to the address given by `x`. In this case, `0` refers to the first executable instruction in the program.
- [ ] ``jnz <x>`` jumps the instruction pointer to the address given by `x` if the top of the stack is not `0`. If the stack size is less than `1`, or the stack is empty, we invoke ``ERR_STACK_UNDERFLOW``.
- [ ] ``eq`` sets the top of the stack to be `0` if the top two elements of the stack are not equal, and to `1` if the elements are equal. If the stack size is less than `2`, we invoke ``ERR_STACK_UNDERFLOW``.
- [x] ``halt`` halts the program from running, setting the `halt` flag in the virtual machine to true.

#### NaN Boxing

One way of storing information that this virtual machine uses is NaN boxing. Since data is stored in 64 bits, we split up these bits in the following way for floating point values:
``1 signed bit | 11 exponent bits | 52 fraction bits``
A value is called ``NaN`` if all eleven exponent bits are marked as `1`. When we store information on the VM, we say that if those eleven bits aren't all marked as `1`, we treat the value as a floating point. Otherwise, we use the first four significant bits of the fraction bits to determine the type of data we are storing. The remainder of the fraction bits are used to encode the value.

We do this in order to easily convert data on the stack to different types. If I were to use different memory arrays for different types, i'd either have to convert back and forth, or it would become a hassle to keep track of the different memory. This way, we can say the top of the stack represents an integer, and then just as easily say it represents a float in the next instruction by changing the exponent bits.