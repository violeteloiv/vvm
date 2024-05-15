#ifndef __VVM_H_INCLUDED__
#define __VVM_H_INCLUDED__

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

#define VVM_STACK_CAPACITY 1024
#define VVM_PROGRAM_CAPACITY 1024
#define VVM_LABEL_CAPACITY 1024
#define VVM_UNRESOLVED_JMPS_CAPACITY 1024

typedef struct {
    size_t count;
    const char* data;
} string_view_t;

string_view_t cstr_as_sv(const char* p_cstr);
string_view_t sv_trim_left(string_view_t p_sv);
string_view_t sv_trim_right(string_view_t p_sv);
string_view_t sv_trim(string_view_t p_sv);
string_view_t sv_chop_by_delim(string_view_t* p_sv, char p_delim);
int sv_equal(string_view_t p_a, string_view_t p_b);
int sv_to_int(string_view_t p_sv);
string_view_t sv_slurp_file(const char* p_file_path);

typedef enum {
    ERR_OK = 0,

    ERR_STACK_OVERFLOW,
    ERR_STACK_UNDERFLOW,

    ERR_ILLEGAL_INSTRUCTION,
    ERR_ILLEGAL_INSTRUCTION_ACCESS,
    ERR_ILLEGAL_OPERAND,

    ERR_DIV_BY_ZERO,
} error;

const char* error_as_cstr(error p_error);

typedef int64_t word_t;

typedef enum {
    INST_NOP = 0,

    INST_PUSH,
    INST_DUP_REL,

    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    
    INST_JMP,
    INST_JMP_NZ,
    INST_EQ,

    INST_HALT,
    INST_PRINT_DEBUG,
} inst_type;

const char* inst_type_as_cstr(inst_type p_type);

typedef struct {
    inst_type type;
    word_t operand;
} inst_t;

#define MAKE_INST_NOP           {.type = INST_NOP}

#define MAKE_INST_PUSH(value)   {.type = INST_PUSH, .operand=(value)}
#define MAKE_INST_DUP_REL(pos)  {.type = INST_DUP_REL, .operand=(pos)}

#define MAKE_INST_PLUS          {.type = INST_PLUS}
#define MAKE_INST_MINUS         {.type = INST_MINUS}
#define MAKE_INST_MULT          {.type = INST_MULT}
#define MAKE_INST_DIV           {.type = INST_DIV}

#define MAKE_INST_JMP(addr)     {.type = INST_JMP, .operand=(addr)}
#define MAKE_INST_JMP_NZ(addr)  {.type = INST_JMP_NZ, .operand=(addr)}
#define MAKE_INST_EQ            {.type = INST_EQ}

#define MAKE_INST_HALT          {.type = INST_HALT}
#define MAKE_INST_PRINT_DEBUG   {.type = INST_PRINT_DEBUG}

typedef struct {
    string_view_t name;
    word_t addr;
} label_t;

typedef struct {
    word_t addr;
    string_view_t label;
} unresolved_jump_t;

typedef struct {
    label_t labels[VVM_LABEL_CAPACITY];
    size_t labels_size;
    unresolved_jump_t unresolved_jumps[VVM_UNRESOLVED_JMPS_CAPACITY];
    size_t unresolved_jumps_size;
} label_table_t;

word_t label_table_find(label_table_t* p_lt, string_view_t p_name);
void label_table_push(label_table_t* p_lt, string_view_t p_name, word_t p_addr);
void label_table_push_unresolved_jmp(label_table_t* p_lt, word_t p_addr, string_view_t p_name);

typedef struct {
    word_t stack[VVM_STACK_CAPACITY];
    word_t stack_size;

    inst_t program[VVM_PROGRAM_CAPACITY];
    word_t program_size;
    word_t inst_pointer;

    int halt;
} vvm_t;

error vm_execute_inst(vvm_t* p_vm);
error vm_execute_program(vvm_t* p_vm, int p_limit);
void vm_dump_stack(FILE* p_stream, const vvm_t* p_vm);
void vm_push_inst(vvm_t* p_vm, inst_t p_inst);
void vm_load_program_from_memory(vvm_t* p_vm, inst_t* p_program, size_t p_program_size);
void vm_load_program_from_file(vvm_t* p_vm, const char* p_file_path);
void vm_save_program_to_file(const vvm_t* p_vm, const char* p_file_path);
void vm_translate_source(string_view_t p_source, vvm_t* p_vm, label_table_t* p_lt);

#endif // __VVM_H_INCLUDED__

#ifdef VM_IMPLEMENTATION

string_view_t cstr_as_sv(const char* p_cstr)
{
    return (string_view_t) {
        .count = strlen(p_cstr),
        .data = p_cstr
    };
}

string_view_t sv_trim_left(string_view_t p_sv)
{
    size_t i = 0;
    while (i < p_sv.count && isspace(p_sv.data[i]))
        i+=1;
    
    return (string_view_t){
        .count = p_sv.count - i,
        .data = p_sv.data + i,
    };
}

string_view_t sv_trim_right(string_view_t p_sv)
{
    size_t i = 0;
    while (i < p_sv.count && isspace(p_sv.data[p_sv.count - 1 - i]))
        i+=1;
    
    return (string_view_t){
        .count = p_sv.count - i,
        .data = p_sv.data,
    };
}

string_view_t sv_trim(string_view_t p_sv)
{
    return sv_trim_right(sv_trim_left(p_sv));
}

string_view_t sv_chop_by_delim(string_view_t* p_sv, char p_delim)
{
    size_t i = 0;
    while (i < p_sv->count && p_sv->data[i] != p_delim)
        i += 1;

    string_view_t result = {
        .count = i,
        .data = p_sv->data,
    };

    if (i < p_sv->count)
    {
        p_sv->count -= i + 1;
        p_sv->data += i + 1;
    }
    else
    {
        p_sv->count -= i;
        p_sv->data += i;
    }

    return result;
}

int sv_equal(string_view_t p_a, string_view_t p_b)
{
    if (p_a.count != p_b.count)
        return 0;
    else
        return memcmp(p_a.data, p_b.data, p_a.count) == 0;
}

int sv_to_int(string_view_t p_sv)
{
    int result = 0;

    for (size_t i = 0; i < p_sv.count && isdigit(p_sv.data[i]); ++i)
        result = result * 10 + *p_sv.data - '0';

    return result;
}

string_view_t sv_slurp_file(const char* p_file_path)
{
    FILE* f = fopen(p_file_path, "r");
    if (f == NULL)
    {
        fprintf(stderr, "[ERROR]: Could Not Open File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    char* buffer = malloc(m);
    if (buffer == NULL)
    {
        fprintf(stderr, "[ERROR]: Could Not Allocate Memory For File: %s\n", strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_SET) < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if (ferror(f))
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
    return (string_view_t){
        .count = n,
        .data = buffer
    };
}

const char* error_as_cstr(error p_error)
{
    switch (p_error)
    {
        case ERR_OK:
            return "ERR_OK";
        case ERR_STACK_OVERFLOW:
            return "ERR_STACK_OVERFLOW";
        case ERR_STACK_UNDERFLOW:
            return "ERR_STACK_UNDERFLOW";
        case ERR_ILLEGAL_INSTRUCTION:
            return "ERR_ILLEGAL_INSTRUCTION";
        case ERR_ILLEGAL_INSTRUCTION_ACCESS:
            return "ERR_ILLEGAL_INSTRUCTION_ACCESS";
        case ERR_ILLEGAL_OPERAND:
            return "ERR_ILLEGAL_OPERAND";
        case ERR_DIV_BY_ZERO:
            return "ERR_DIV_BY_ZERO";
        default:
            assert(0 && "error_as_cstr: Unreachable (How Did You Get Here)");
    }
}

const char* inst_type_as_cstr(inst_type p_type)
{
    switch (p_type)
    {
        case INST_NOP:
            return "INST_NOP";
        case INST_PUSH:
            return "INST_PUSH";
        case INST_DUP_REL:
            return "INST_DUP_REL";
        case INST_PLUS:
            return "INST_PLUS";
        case INST_MINUS:
            return "INST_MINUS";
        case INST_MULT:
            return "INST_MULT";
        case INST_DIV:
            return "INST_DIV";
        case INST_JMP:
            return "INST_JMP";
        case INST_JMP_NZ:
            return "INST_JMP_NZ";
        case INST_EQ:
            return "INST_EQ";
        case INST_HALT:
            return "INST_HALT";
        case INST_PRINT_DEBUG:
            return "INST_PRINT_DEBUG";
        default:
            assert(0 && "inst_type_as_cstr: Unreachable (How Did You Get Here)");
    }
}

word_t label_table_find(label_table_t* p_lt, string_view_t p_name)
{
    for (size_t i = 0; i < p_lt->labels_size; ++i)
    {
        if (sv_equal(p_lt->labels[i].name, p_name))
            return p_lt->labels[i].addr;
    }

    fprintf(stderr, "[ERROR]: label `%.*s` does not exist\n", (int)p_name.count, p_name.data);
    exit(1);
}

void label_table_push(label_table_t* p_lt, string_view_t p_name, word_t p_addr)
{
    assert(p_lt->labels_size < VVM_LABEL_CAPACITY);
    p_lt->labels[p_lt->labels_size++] = (label_t){
        .name = p_name,
        .addr = p_addr
    };
}

void label_table_push_unresolved_jmp(label_table_t* p_lt, word_t p_addr, string_view_t p_name)
{
    assert(p_lt->unresolved_jumps_size < VVM_UNRESOLVED_JMPS_CAPACITY);
    p_lt->unresolved_jumps[p_lt->unresolved_jumps_size++] = (unresolved_jump_t){
        .addr = p_addr,
        .label = p_name,
    };
}

error vm_execute_inst(vvm_t* p_vm)
{
    if (p_vm->inst_pointer < 0 || p_vm->inst_pointer >= p_vm->program_size)
        return ERR_ILLEGAL_INSTRUCTION_ACCESS;

    inst_t inst = p_vm->program[p_vm->inst_pointer];

    switch (inst.type) {
        case INST_NOP:
            p_vm->inst_pointer++;
            break;
        
        case INST_PUSH:
            if (p_vm->stack_size >= VVM_STACK_CAPACITY)
                return ERR_STACK_OVERFLOW;
            p_vm->stack[p_vm->stack_size++] = inst.operand;
            p_vm->inst_pointer++;
            break;

        case INST_DUP_REL:
            if (p_vm->stack_size >= VVM_STACK_CAPACITY)
                return ERR_STACK_OVERFLOW;
            if (p_vm->stack_size - inst.operand <= 0)
                return ERR_STACK_UNDERFLOW;
            if (inst.operand < 0)
                return ERR_ILLEGAL_OPERAND;
            p_vm->stack[p_vm->stack_size] = p_vm->stack[p_vm->stack_size - 1 - inst.operand];
            p_vm->stack_size++;
            p_vm->inst_pointer++;
            break;

        case INST_PLUS:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2] += p_vm->stack[p_vm->stack_size - 1];
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break; 

        case INST_MINUS:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2] -= p_vm->stack[p_vm->stack_size - 1];
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_MULT:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2] *= p_vm->stack[p_vm->stack_size - 1];
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_DIV:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            if (p_vm->stack[p_vm->stack_size - 1] == 0)
                return ERR_DIV_BY_ZERO;
            p_vm->stack[p_vm->stack_size - 2] /= p_vm->stack[p_vm->stack_size - 1];
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;

        case INST_JMP:
            p_vm->inst_pointer = inst.operand;
            break;

        case INST_JMP_NZ:
            if (p_vm->stack_size < 1)
                return ERR_STACK_UNDERFLOW;
            if (p_vm->stack[p_vm->stack_size - 1]) {
                p_vm->stack_size--;
                p_vm->inst_pointer = inst.operand;
            } else {
                p_vm->inst_pointer++;
            }
            break;

        case INST_EQ:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2] = p_vm->stack[p_vm->stack_size - 2] == p_vm->stack[p_vm->stack_size - 1];
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;

        case INST_HALT:
            p_vm->halt = 1;
            break;

        case INST_PRINT_DEBUG:
            if (p_vm->stack_size < 1)
                return ERR_STACK_UNDERFLOW;
            fprintf(stdout, "%ld\n", p_vm->stack[p_vm->stack_size - 1]);
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        default:
            return ERR_ILLEGAL_INSTRUCTION;
    }

    return ERR_OK;
}

error vm_execute_program(vvm_t* p_vm, int p_limit)
{
    // If p_limit is positive, it will execute a finite number of instructions,
    // While if p_limit is negative, it will execute instructions without limit.
    while (p_limit != 0 && !p_vm->halt)
    {
        error err = vm_execute_inst(p_vm);
        if (err != ERR_OK)
            return err;

        if (p_limit > 0)
            --p_limit;
    }

    return ERR_OK;
}

void vm_dump_stack(FILE* p_stream, const vvm_t* p_vm)
{
    fprintf(p_stream, "Stack:\n");
    if (p_vm->stack_size > 0)
        for (word_t i = 0; i < p_vm->stack_size; ++i)
            fprintf(p_stream, "  %ld\n", p_vm->stack[i]);
    else
        fprintf(p_stream, "  [Empty]\n");
}

void vm_push_inst(vvm_t* p_vm, inst_t p_inst)
{
    assert(p_vm->program_size < VVM_PROGRAM_CAPACITY);
    p_vm->program[p_vm->program_size++] = p_inst;
}

void vm_load_program_from_memory(vvm_t* p_vm, inst_t* p_program, size_t p_program_size)
{
    assert(p_program_size < VVM_PROGRAM_CAPACITY);
    memcpy(p_vm->program, p_program, sizeof(p_program[0]) * p_program_size);
    p_vm->program_size = p_program_size;
}

void vm_load_program_from_file(vvm_t* p_vm, const char* p_file_path)
{
    FILE *f = fopen(p_file_path, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "[ERROR]: Could Not Open File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    if (fseek(f, 0, SEEK_END) < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if (m < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    assert(m % sizeof(p_vm->program[0]) == 0);
    assert((size_t)m <= VVM_PROGRAM_CAPACITY * sizeof(p_vm->program[0]));

    if (fseek(f, 0, SEEK_SET) < 0)
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    p_vm->program_size = fread(p_vm->program, sizeof(p_vm->program[0]), m / sizeof(p_vm->program[0]), f);
    if (ferror(f))
    {
        fprintf(stderr, "[ERROR]: Could Not Read File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void vm_save_program_to_file(const vvm_t* p_vm, const char* p_file_path)
{
    FILE* f = fopen(p_file_path, "wb");
    if (f == NULL)
    {
        fprintf(stderr, "[ERROR]: Could Not Open File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    fwrite(p_vm->program, sizeof(p_vm->program[0]), p_vm->program_size, f);
    if (ferror(f))
    {
        fprintf(stderr, "[ERROR]: Could Not Write To File `%s`: %s\n", p_file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void vm_translate_source(string_view_t p_source, vvm_t* p_vm, label_table_t* p_lt)
{
    while (p_source.count > 0)
    {
        assert(p_vm->program_size < VVM_PROGRAM_CAPACITY);

        string_view_t line = sv_trim(sv_chop_by_delim(&p_source, '\n'));
        if (line.count > 0 && *line.data != '#')
        {
            line = sv_trim_left(line);
            string_view_t inst_name = sv_chop_by_delim(&line, ' ');
            string_view_t operand = sv_trim(sv_chop_by_delim(&line, '#'));

            if (inst_name.count > 0 && inst_name.data[inst_name.count - 1] == ':') {
                string_view_t label = {
                    .count = inst_name.count - 1,
                    .data = inst_name.data
                };
                label_table_push(p_lt, label, p_vm->program_size);
            } else if (sv_equal(inst_name, cstr_as_sv("nop"))) {
                p_vm->program[p_vm->program_size++] = (inst_t){0};
            } else if (sv_equal(inst_name, cstr_as_sv("push"))) {
                p_vm->program[p_vm->program_size++] = (inst_t){
                    .type = INST_PUSH, 
                    .operand = sv_to_int(operand)
                };
            } else if (sv_equal(inst_name, cstr_as_sv("rdup"))) {
                p_vm->program[p_vm->program_size++] = (inst_t){
                    .type = INST_DUP_REL, 
                    .operand = sv_to_int(operand)
                };
            } else if (sv_equal(inst_name, cstr_as_sv("add"))) {
                p_vm->program[p_vm->program_size++] = (inst_t){
                    .type = INST_PLUS
                };
            } else if (sv_equal(inst_name, cstr_as_sv("jmp"))) {
                if (operand.count > 0 && isdigit(*operand.data)) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_JMP,
                        .operand = sv_to_int(operand)
                    };
                } else {
                    label_table_push_unresolved_jmp(p_lt, p_vm->program_size, operand);
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_JMP
                    };
                }
            } else {
                fprintf(stderr, "[ERROR]: Unknown Instruction `%.*s`.\n", (int)inst_name.count, inst_name.data);
                exit(1);
            }
        }
    }

    // Second pass to resolve labels.
    for (size_t i = 0; i < p_lt->unresolved_jumps_size; ++i)
    {
        word_t addr = label_table_find(p_lt, p_lt->unresolved_jumps[i].label);
        p_vm->program[p_lt->unresolved_jumps[i].addr].operand = addr;
    }
}

#endif // VM_IMPLEMENTATION