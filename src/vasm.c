#define VM_IMPLEMENTATION
#include "./vvm.h"

vvm_t vm = {0};
label_table_t lt = {0};

const char* shift(int* argc, char*** argv)
{
    assert(*argc > 0);

    char* result = **argv;
    *argv += 1;
    *argc -= 1;

    return result;
}

void usage(FILE* p_stream, const char* p_program)
{
    fprintf(p_stream, "Usage: %s <input.vasm> <output.vm>\n", p_program);
}

int main(int argc, char** argv)
{
    // Get the program name.
    const char* program = shift(&argc, &argv);
    if (argc == 0)
    {
        usage(stderr, program);
        fprintf(stderr, "[ERROR]: Expected Input\n");
        exit(1);
    }

    // Get the input path.
    const char* input_file_path = shift(&argc, &argv);
    if (argc == 0)
    {
        usage(stderr, program);
        fprintf(stderr, "[ERROR]: Expected Output\n");
        exit(1);
    }

    // Get the output file.
    const char* output_file_path = shift(&argc, &argv);

    string_view_t source = sv_slurp_file(input_file_path);
    vm_translate_source(source, &vm, &lt);
    vm_save_program_to_file(&vm, output_file_path);
    
    return 0;
}