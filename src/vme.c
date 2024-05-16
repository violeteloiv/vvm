#define VM_IMPLEMENTATION
#include "./vvm.h"

static const char* shift(int* argc, char*** argv)
{
    assert(*argc > 0);

    char* result = **argv;
    *argv += 1;
    *argc -= 1;

    return result;
}

static void usage(FILE* p_stream, const char* p_program)
{
    fprintf(p_stream, "Usage: %s -i <input.vm> [-l <limit>] [-h]\n", p_program);
}

vvm_t vm = {0};

int main(int argc, char** argv)
{
    const char* program = shift(&argc, &argv);
    const char* input_file_path = NULL;
    int limit = -1;

    while (argc > 0)
    {
        const char* flag = shift(&argc, &argv);

        if (strcmp(flag, "-i") == 0) {
            if (argc == 0)
            {
                usage(stderr, program);
                fprintf(stderr, "[ERROR]: No Argument Provided For Flag `%s`\n", flag);
                exit(1);
            }
            input_file_path = shift(&argc, &argv);
        } else if (strcmp(flag, "-l") == 0) {
            if (argc == 0)
            {
                usage(stderr, program);
                fprintf(stderr, "[ERROR]: No Argument Provided For Flag `%s`\n", flag);
                exit(1);
            }
            limit = atoi(shift(&argc, &argv));
        } else if (strcmp(flag, "-h") == 0) {
            usage(stdout, program);
            exit(0);
        }else {
            usage(stderr, program);
            fprintf(stderr, "[ERROR]: Unknown Flag `%s`\n", flag);
            exit(1);
        }
    }

    if (input_file_path == NULL)
    {
        usage(stderr, program);
        fprintf(stderr, "[ERROR]: Input Was Not Provided\n");
        exit(1);
    }

    vm_load_program_from_file(&vm, input_file_path);
    error err = vm_execute_program(&vm, limit);
    vm_dump_stack(stdout, &vm);

    if (err != ERR_OK)
    {
        fprintf(stderr, "[ERROR]: %s\n", error_as_cstr(err));
        exit(1);
    }

    return 0;
}