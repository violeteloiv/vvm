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
#define VVM_DEFERRED_OPERANDS_CAPACITY 1024

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

typedef uint64_t inst_addr_t;

typedef union {
    int64_t as_i64;
    uint64_t as_u64;
    double as_f64;
    void* as_ptr;
} word_t;

static_assert(sizeof(word_t) == 8, "The Virtual Machine's Word Is Expected To Be 64 Bits");

word_t number_literal_as_word(string_view_t p_sv);

typedef enum {
    INST_NOP = 0,

    INST_PUSH,
    INST_DUP_REL,
    INST_SWAP,

    INST_ADDI,
    INST_SUBI,
    INST_MULI,
    INST_DIVI,
    INST_ADDF,
    INST_SUBF,
    INST_MULF,
    INST_DIVF,
    
    INST_JMP,
    INST_JMP_NZ,
    INST_EQ,
    INST_NOT,
    INST_GEQ,

    INST_HALT,
    INST_PRINT_DEBUG,
    NUMBER_OF_INSTS,
} inst_type;

const char *inst_name(inst_type p_type);
int inst_has_operand(inst_type p_type);
const char* inst_type_as_cstr(inst_type p_type);

typedef struct {
    inst_type type;
    word_t operand;
} inst_t;

typedef struct {
    string_view_t name;
    inst_addr_t addr;
} label_t;

typedef struct {
    inst_addr_t addr;            // Address of an instruction the operand of which
    string_view_t label;    // refers to a label. It's not the label address.
} deferred_operand_t;

typedef struct {
    label_t labels[VVM_LABEL_CAPACITY];
    size_t labels_size;
    deferred_operand_t deferred_operands[VVM_DEFERRED_OPERANDS_CAPACITY];
    size_t deferred_operands_size;
} vasm_t;

inst_addr_t vasm_find_label_addr(vasm_t* p_vasm, string_view_t p_name);
void vasm_push_label(vasm_t* p_vasm, string_view_t p_name, inst_addr_t p_addr);
void vasm_push_deferred_operand(vasm_t* p_vasm, inst_addr_t p_addr, string_view_t p_name);

typedef struct {
    word_t stack[VVM_STACK_CAPACITY];
    uint64_t stack_size;

    inst_t program[VVM_PROGRAM_CAPACITY];
    uint64_t program_size;
    inst_addr_t inst_pointer;

    int halt;
} vvm_t;

error vm_execute_inst(vvm_t* p_vm);
error vm_execute_program(vvm_t* p_vm, int p_limit);
void vm_dump_stack(FILE* p_stream, const vvm_t* p_vm);
void vm_push_inst(vvm_t* p_vm, inst_t p_inst);
void vm_load_program_from_memory(vvm_t* p_vm, inst_t* p_program, size_t p_program_size);
void vm_load_program_from_file(vvm_t* p_vm, const char* p_file_path);
void vm_save_program_to_file(const vvm_t* p_vm, const char* p_file_path);
void vm_translate_source(string_view_t p_source, vvm_t* p_vm, vasm_t* p_vasm);

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

word_t number_literal_as_word(string_view_t p_sv)
{
    assert(p_sv.count < 1024);
    char cstr[p_sv.count + 1];
    char* endptr = 0;

    memcpy(cstr, p_sv.data, p_sv.count);
    cstr[p_sv.count] = '\0';

    word_t result = {0};

    result.as_u64 = strtoull(cstr, &endptr, 10);
    if ((size_t)(endptr - cstr) != p_sv.count)
    {
        result.as_f64 = strtod(cstr, &endptr);
        if ((size_t)(endptr - cstr) != p_sv.count)
        {
            fprintf(stderr, "[ERROR]: `%s` is not a number literal\n", cstr);
            exit(1);
        }
    }

    return result;
}

const char* inst_name(inst_type p_type)
{
    switch (p_type)
    {
        case INST_NOP:          return "nop";

        case INST_PUSH:         return "push";
        case INST_DUP_REL:      return "rdup";
        case INST_SWAP:         return "swap";

        case INST_ADDI:         return "addi";
        case INST_SUBI:         return "subi";
        case INST_MULI:         return "muli";
        case INST_DIVI:         return "divi";
        case INST_ADDF:         return "addf";
        case INST_SUBF:         return "subf";
        case INST_MULF:         return "mulf";
        case INST_DIVF:         return "divf";

        case INST_JMP:          return "jmp";
        case INST_JMP_NZ:       return "jnz";
        case INST_EQ:           return "eq";
        case INST_NOT:          return "not";
        case INST_GEQ:          return "geq";

        case INST_HALT:         return "halt";
        case INST_PRINT_DEBUG:  return "print_debug";
        case NUMBER_OF_INSTS:
        default: assert(0 && "inst_name: unreachable");
    }
}

int inst_has_operand(inst_type p_type)
{
    switch (p_type) {
        case INST_NOP:          return 0;

        case INST_PUSH:         return 1;
        case INST_DUP_REL:      return 1;
        case INST_SWAP:         return 1;

        case INST_ADDI:         return 0;
        case INST_SUBI:         return 0;
        case INST_MULI:         return 0;
        case INST_DIVI:         return 0;
        case INST_ADDF:         return 0;
        case INST_SUBF:         return 0;
        case INST_MULF:         return 0;
        case INST_DIVF:         return 0;

        case INST_JMP:          return 1;
        case INST_JMP_NZ:       return 1;
        case INST_EQ:           return 0;
        case INST_NOT:          return 0;
        case INST_GEQ:          return 0;

        case INST_HALT:         return 0;
        case INST_PRINT_DEBUG:  return 0;
        case NUMBER_OF_INSTS:
        default: assert(0 && "inst_name: unreachable");
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
        case INST_SWAP:
            return "INST_SWAP";
        case INST_ADDI:
            return "INST_ADDI";
        case INST_SUBI:
            return "INST_SUBI";
        case INST_MULI:
            return "INST_MULI";
        case INST_DIVI:
            return "INST_DIVI";
        case INST_ADDF:
            return "INST_ADDF";
        case INST_SUBF:
            return "INST_SUBF";
        case INST_MULF:
            return "INST_MULF";
        case INST_DIVF:
            return "INST_DIVF";
        case INST_JMP:
            return "INST_JMP";
        case INST_JMP_NZ:
            return "INST_JMP_NZ";
        case INST_EQ:
            return "INST_EQ";
        case INST_NOT:
            return "INST_NOT";
        case INST_GEQ:
            return "INST_GEQ";
        case INST_HALT:
            return "INST_HALT";
        case INST_PRINT_DEBUG:
            return "INST_PRINT_DEBUG";
        case NUMBER_OF_INSTS:
        default:
            assert(0 && "inst_type_as_cstr: Unreachable (How Did You Get Here)");
    }
}

inst_addr_t vasm_find_label_addr(vasm_t* p_vasm, string_view_t p_name)
{
    for (size_t i = 0; i < p_vasm->labels_size; ++i)
    {
        if (sv_equal(p_vasm->labels[i].name, p_name))
            return p_vasm->labels[i].addr;
    }

    fprintf(stderr, "[ERROR]: label `%.*s` does not exist\n", (int)p_name.count, p_name.data);
    exit(1);
}

void vasm_push_label(vasm_t* p_vasm, string_view_t p_name, inst_addr_t p_addr)
{
    assert(p_vasm->labels_size < VVM_LABEL_CAPACITY);
    p_vasm->labels[p_vasm->labels_size++] = (label_t){
        .name = p_name,
        .addr = p_addr
    };
}

void vasm_push_deferred_operand(vasm_t* p_vasm, inst_addr_t p_addr, string_view_t p_name)
{
    assert(p_vasm->deferred_operands_size < VVM_DEFERRED_OPERANDS_CAPACITY);
    p_vasm->deferred_operands[p_vasm->deferred_operands_size++] = (deferred_operand_t){
        .addr = p_addr,
        .label = p_name,
    };
}

error vm_execute_inst(vvm_t* p_vm)
{
    if (p_vm->inst_pointer >= p_vm->program_size)
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
            if (p_vm->stack_size - inst.operand.as_u64 <= 0)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size].as_u64 = p_vm->stack[p_vm->stack_size - 1 - inst.operand.as_u64].as_u64;
            p_vm->stack_size++;
            p_vm->inst_pointer++;
            break;

        case INST_SWAP:
            if (inst.operand.as_u64 >= p_vm->stack_size)
                return ERR_STACK_UNDERFLOW;

            const uint64_t a = p_vm->stack_size - 1;
            const uint64_t b = p_vm->stack_size - 1 - inst.operand.as_u64;
  
            word_t t = p_vm->stack[a];
            p_vm->stack[a] = p_vm->stack[b];
            p_vm->stack[b] = t;
            p_vm->inst_pointer++;
            break;

        case INST_ADDI:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_u64 += p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break; 

        case INST_SUBI:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_u64 -= p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_MULI:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_u64 *= p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_DIVI:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            if (p_vm->stack[p_vm->stack_size - 1].as_u64 == 0)
                return ERR_DIV_BY_ZERO;
            p_vm->stack[p_vm->stack_size - 2].as_u64 /= p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;

        case INST_ADDF:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_f64 += p_vm->stack[p_vm->stack_size - 1].as_f64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;

        case INST_SUBF:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_f64 -= p_vm->stack[p_vm->stack_size - 1].as_f64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_MULF:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_f64 *= p_vm->stack[p_vm->stack_size - 1].as_f64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_DIVF:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            // if (p_vm->stack[p_vm->stack_size - 1].as_f64 == 0.0)
             //    return ERR_DIV_BY_ZERO;
            p_vm->stack[p_vm->stack_size - 2].as_f64 /= p_vm->stack[p_vm->stack_size - 1].as_f64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;

        case INST_JMP:
            p_vm->inst_pointer = inst.operand.as_u64;
            break;

        case INST_JMP_NZ:
            if (p_vm->stack_size < 1)
                return ERR_STACK_UNDERFLOW;

            if (p_vm->stack[p_vm->stack_size - 1].as_u64)
                p_vm->inst_pointer = inst.operand.as_u64;
            else
                p_vm->inst_pointer++;

            p_vm->stack_size--;
            break;

        case INST_EQ:
            if (p_vm->stack_size < 2)
                return ERR_STACK_UNDERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_u64 = p_vm->stack[p_vm->stack_size - 2].as_u64 == p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case INST_NOT:
            if (p_vm->stack_size < 1)
                return ERR_STACK_OVERFLOW;
            p_vm->stack[p_vm->stack_size - 1].as_u64 = !p_vm->stack[p_vm->stack_size - 1].as_u64;
            p_vm->inst_pointer++;
            break;

        case INST_GEQ:
            if (p_vm->stack_size < 2)
                return ERR_STACK_OVERFLOW;
            p_vm->stack[p_vm->stack_size - 2].as_u64 = p_vm->stack[p_vm->stack_size - 1].as_f64 >= p_vm->stack[p_vm->stack_size - 2].as_f64;
            p_vm->stack_size--;
            p_vm->inst_pointer++;
            break;

        case INST_HALT:
            p_vm->halt = 1;
            break;

        case INST_PRINT_DEBUG:
            //if (p_vm->stack_size < 1)
            //   return ERR_STACK_UNDERFLOW;
            // fprintf(stdout, "  u64: %lu, i64: %ld, f64: %lf, ptr: %p\n", 
            //     p_vm->stack[p_vm->stack_size - 1].as_u64,
            //     p_vm->stack[p_vm->stack_size - 1].as_i64,
            //     p_vm->stack[p_vm->stack_size - 1].as_f64,
            //     p_vm->stack[p_vm->stack_size - 1].as_ptr);
            vm_dump_stack(stdout, p_vm);
            //p_vm->stack_size -= 1;
            p_vm->inst_pointer++;
            break;
        
        case NUMBER_OF_INSTS:
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
        for (uint64_t i = 0; i < p_vm->stack_size; ++i)
            fprintf(p_stream, "  u64: %lu, i64: %ld, f64: %lf, ptr: %p\n", 
                p_vm->stack[i].as_u64,
                p_vm->stack[i].as_i64,
                p_vm->stack[i].as_f64,
                p_vm->stack[i].as_ptr);
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

void vm_translate_source(string_view_t p_source, vvm_t* p_vm, vasm_t* p_vasm)
{
    while (p_source.count > 0)
    {
        assert(p_vm->program_size < VVM_PROGRAM_CAPACITY);

        string_view_t line = sv_trim(sv_chop_by_delim(&p_source, '\n'));
        if (line.count > 0 && *line.data != '#')
        {
            line = sv_trim_left(line);
            string_view_t token = sv_chop_by_delim(&line, ' ');

            if (token.count > 0 && token.data[token.count - 1] == ':') {
                string_view_t label = {
                    .count = token.count - 1,
                    .data = token.data
                };
                vasm_push_label(p_vasm, label, p_vm->program_size);

                // Try to repeat the instruction name.
                token = sv_trim(sv_chop_by_delim(&line, ' '));
            } 
            
            if (token.count > 0)
            {
                string_view_t operand = sv_trim(sv_chop_by_delim(&line, '#'));
                if (sv_equal(token, cstr_as_sv(inst_name(INST_NOP)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){0};
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_PUSH)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_PUSH, 
                        .operand = number_literal_as_word(operand),
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_DUP_REL)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_DUP_REL, 
                        .operand = { .as_i64 = sv_to_int(operand) }
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_SWAP)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_SWAP,
                        .operand = { .as_i64 = sv_to_int(operand) }
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_ADDI)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_ADDI
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_SUBI)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_SUBI
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_MULI)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_MULI
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_DIVI)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_DIVI
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_ADDF)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_ADDF
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_SUBF)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_SUBF
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_MULF)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_MULF
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_DIVF)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_DIVF
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_JMP)))) {
                    if (operand.count > 0 && isdigit(*operand.data)) {
                        p_vm->program[p_vm->program_size++] = (inst_t){
                            .type = INST_JMP,
                            .operand = { .as_i64 = sv_to_int(operand) }
                        };
                    } else {
                        vasm_push_deferred_operand(p_vasm, p_vm->program_size, operand);
                        p_vm->program[p_vm->program_size++] = (inst_t){
                            .type = INST_JMP
                        };
                    }
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_JMP_NZ)))) {
                    if (operand.count > 0 && isdigit(*operand.data)) {
                        p_vm->program[p_vm->program_size++] = (inst_t) {
                            .type = INST_JMP_NZ,
                            .operand = { .as_i64 = sv_to_int(operand)}
                        };
                    } else {
                        vasm_push_deferred_operand(p_vasm, p_vm->program_size, operand);
                        p_vm->program[p_vm->program_size++] = (inst_t) {
                            .type = INST_JMP_NZ
                        };
                    }
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_EQ)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_EQ
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_NOT)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_NOT
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_GEQ)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_GEQ
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_HALT)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_HALT
                    };
                } else if (sv_equal(token, cstr_as_sv(inst_name(INST_PRINT_DEBUG)))) {
                    p_vm->program[p_vm->program_size++] = (inst_t){
                        .type = INST_PRINT_DEBUG
                    };
                } else {
                    fprintf(stderr, "[ERROR]: Unknown Instruction `%.*s`.\n", (int)token.count, token.data);
                    exit(1);
                }
            }
        }
    }

    // Second pass to resolve labels.
    for (size_t i = 0; i < p_vasm->deferred_operands_size; ++i)
    {
        inst_addr_t addr = vasm_find_label_addr(p_vasm, p_vasm->deferred_operands[i].label);
        p_vm->program[p_vasm->deferred_operands[i].addr].operand.as_u64 = addr;
    }
}

#endif // VM_IMPLEMENTATION