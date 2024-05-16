#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// We represent a number with a certain amount of 1s by saying
// (1LL - xLL) - 1LL.
#define EXPONENTIAL_MASK (((1LL << 11LL) - 1LL) << 52LL)
#define FRACTION_MASK ((1LL << 52LL) - 1LL)
#define SIGN_MASK (1LL << 63LL)
#define TYPE_MASK (((1LL << 4LL) - 1LL) << 48LL)
#define VALUE_MASK ((1LL << 48LL) - 1LL)

#define TYPE(n) ((1LL << 3LL) + n)

static double mk_inf(void)
{
    uint64_t y = EXPONENTIAL_MASK;
    return *(double*)&y;
}

static double set_type(double p_x, uint64_t p_type)
{
    uint64_t y = *(uint64_t*)&p_x;
    y = (y & (~TYPE_MASK)) | (((TYPE_MASK >> 48LL) & p_type) << 48LL);
    return *(double*)&y;
}

static uint64_t get_type(double p_x)
{
    uint64_t y = (*(uint64_t*)&p_x);
    return (y & TYPE_MASK) >> 48LL;
}

static double set_value(double p_x, uint64_t p_value)
{
    uint64_t y = *(uint64_t *)&p_x;
    y = (y & (~VALUE_MASK)) | (p_value & VALUE_MASK);
    return *(double*)&y;
}

static uint64_t get_value(double p_x)
{
    uint64_t y = (*(uint64_t*)&p_x);
    return (y & VALUE_MASK);
}

#define INTEGER_TYPE 1
#define POINTER_TYPE 2

static int is_double(double p_x)
{
    return !isnan(p_x);
}

static int is_integer(double p_x)
{
    return isnan(p_x) && get_type(p_x) == TYPE(INTEGER_TYPE);
}

static int is_pointer(double p_x)
{
    return isnan(p_x) && get_type(p_x) == TYPE(POINTER_TYPE);
}

static double as_double(double p_x)
{
    return p_x;
}

static uint64_t as_integer(double p_x)
{
    return get_value(p_x);
}

static void *as_pointer(double p_x)
{
    return (void*) get_value(p_x);
}

static double box_double(double p_x)
{
    return p_x;
}

static double box_integer(uint64_t p_x)
{
    return set_value(set_type(mk_inf(), TYPE(INTEGER_TYPE)), p_x);
}

static double box_pointer(void* p_x)
{
    return set_value(set_type(mk_inf(), TYPE(POINTER_TYPE)), (uint64_t) p_x);
}

int main(void)
{
    double pi = 3.14159265359;

    assert(pi == as_double(box_double(pi)));
    assert(12345678LL == as_integer(box_integer(12345678LL)));
    assert(&pi == as_pointer(box_pointer(&pi)));

    fprintf(stdout, "OK\n");

    return 0;
}

