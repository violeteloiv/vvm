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

    for (inst_addr_t i = 0; i < vm.program_size; ++i)
    {
        fprintf(stdout, "%s", inst_name(vm.program[i].type));
        if (inst_has_operand(vm.program[i].type))
            fprintf(stdout, " %ld", vm.program[i].operand.as_i64);
        fprintf(stdout, "\n");
    }

    return 0;
}