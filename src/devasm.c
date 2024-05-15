#define VM_IMPLEMENTATION
#include "./vvm.h"

vvm_t vm = {0};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./devasm <input.vm>\n");
        fprintf(stderr, "[ERROR]: No Input Provided\n");
        exit(1);
    }

    const char* input_file_path = argv[1];
    vm_load_program_from_file(&vm, input_file_path);

    for (word_t i = 0; i < vm.program_size; ++i)
    {
        switch (vm.program[i].type)
        {
            case INST_NOP:
                fprintf(stdout, "nop\n");
                break;
            case INST_PUSH:
                fprintf(stdout, "push %ld\n", vm.program[i].operand);
                break;
            case INST_DUP_REL:
                fprintf(stdout, "rdup %ld\n", vm.program[i].operand);
                break;
            case INST_PLUS:
                fprintf(stdout, "add\n");
                break;
            case INST_MINUS:
                fprintf(stdout, "sub\n");
                break;
            case INST_MULT:
                fprintf(stdout, "mul\n");
                break;
            case INST_DIV:
                fprintf(stdout, "div\n");
                break;
            case INST_JMP:
                fprintf(stdout, "jmp %ld\n", vm.program[i].operand);
                break;
            case INST_JMP_NZ:
                fprintf(stdout, "jnz %ld\n", vm.program[i].operand);
                break;
            case INST_EQ:
                fprintf(stdout, "eq\n");
                break;
            case INST_HALT:
                fprintf(stdout, "halt\n");
                break;
            case INST_PRINT_DEBUG:
                fprintf(stdout, "print_debug\n");
                break;
        }
    }

    return 0;
}