#define VM_IMPLEMENTATION
#include "./vvm.h"

vvm_t vm = {0};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./vme <input.vm>\n");
        fprintf(stderr, "[ERROR]: Expected Input\n");
        exit(1);
    }

    vm_load_program_from_file(&vm, argv[1]);
    for (int i = 0; i < VVM_EXECUTION_LIMIT && !vm.halt; ++i)
    {
        error error = vm_execute_inst(&vm);
        //vm_dump_stack(stdout, &vm);
        if (error != ERR_OK)
        {
            fprintf(stderr, "Error: %s\n", error_as_cstr(error));
            exit(1);
        }
    }
    vm_dump_stack(stdout, &vm);

    return 0;
}