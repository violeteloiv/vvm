#define VM_IMPLEMENTATION
#include "./vvm.h"

vvm_t vm = {0};

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./vasm <input.vasm> <output.vm>\n");
        fprintf(stderr, "[ERROR]: Expected Input & Output\n");
        exit(1);
    }

    const char* input_file_path = argv[1];
    const char* output_file_path = argv[2];

    string_view_t source = sv_slurp_file(input_file_path);
    vm.program_size = vm_translate_source(source, vm.program, VVM_PROGRAM_CAPACITY);
    vm_save_program_to_file(vm.program, vm.program_size, output_file_path);
    
    return 0;
}