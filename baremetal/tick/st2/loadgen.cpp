/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cfenv>
#include <unistd.h>

/* See paper for algorithm explanation */

static_assert(sizeof(double) == sizeof(uint64_t));
union udouble {
    double d;
    uint64_t u;
};

#define GBN 63
static uint8_t GB[GBN] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '`',
};

static bool isgb(uint8_t gb)
{
    return (gb >= '0' && gb <= '9')
        || (gb >= 'a' && gb <= 'z')
        || (gb >= 'A' && gb <= 'Z')
        || (gb == '`');
}

static inline uint64_t d2u(double d)
{
    udouble ud;
    ud.d = d;
    return ud.u;
}

static inline double u2d(uint64_t u)
{
    udouble ud;
    ud.u = u;
    return ud.d;
}

//__attribute__((noinline))
double fmadd(double a, double b, double c)
{
    return a*b + c;
}

#define FOUNDN 48
#define FOUND_MASK ((1UL << FOUNDN) - 1UL)
typedef struct {
    bool found[1UL << FOUNDN] = {0};
    size_t found_n = 0;
    double fix;

    int midx;
    uint64_t mulu;
    int aidx;
    uint64_t addu;
} t1_t;

static void t1_do(t1_t &t)
{
    double add = u2d(t.addu);
    double mul = u2d(t.mulu);
    double d = fmadd(mul, t.fix, add);

    uint64_t u = d2u(d);
    //printf("X: [0x%lx %e]*[FIX]+[0x%lx %e] == 0x%lx %e\n", t.mulu, mul, t.addu, add, u, d);
    if(t.found[u&FOUND_MASK] == 0) {
        t.found[u&FOUND_MASK] = 1;
        t.found_n++;
    }
}

static void t1_add(t1_t &t, int idx)
{
    if(idx < 0) {
        t1_do(t);
        return;
    }

    for(size_t i=0; i<GBN; i++) {
        t.addu &= ~(0xFFUL << (8*idx));
        t.addu |= ((uint64_t)GB[i]) << (8*idx);

        t1_add(t, idx-1);
    }
}

static void t1_mul(t1_t &t, int idx)
{
    if(idx < 0) {
        t1_add(t, t.aidx);
        return;
    }

    for(size_t i=0; i<GBN; i++) {
        t.mulu &= ~(0xFFUL << (8*idx));
        t.mulu |= ((uint64_t)GB[i]) << (8*idx);

        t1_mul(t, idx-1);
    }
}

void test1(void)
{
    t1_t t = {};
    t.fix = u2d(0x4131555555555555UL);
    t.mulu = 0x4131555555555555UL;
    t.addu = 0x4261555555555555UL;

    t.midx = 2;
    t.aidx = 1;

    t1_mul(t, t.midx);

    #if 1
    uint64_t percent = (100*t.found_n) / (1UL << FOUNDN);
    printf("FOUND[%zu/%lu]: %lu%%\n", t.found_n, 1UL << FOUNDN, percent);
    #else
    printf("FOUND[%zu]:\n    ", t.found_n);
    for(size_t i=0; i<16; i++) {
        for(size_t j=0; j<16; j++) {
            printf("%d", t.found[16*i+j]);
        } printf("\n    ");
    } printf("\n");
    #endif
}

#define MANT_MASK 0x3FFFFFFFFFFFFUL
typedef struct {
    uint64_t add;
    uint64_t mul;
} t2_r_t;

typedef struct {
    size_t found_n = 0;
    double fix;

    uint64_t target;
    uint64_t addu_base;

    int midx;
    uint64_t mulu;
} t2_t;

static bool isaddalnum(uint64_t addu)
{
    addu &= FOUND_MASK;
    static_assert(FOUNDN % 8 == 0);
    int shift = 0;
    while(shift < FOUNDN) {
        if(!isgb((addu >> shift) & 0xFFUL)) {
            return false;
        }
        shift += 8;
    }
    return true;
}

static bool t2_add(t2_t &t)
{
    double mul = u2d(t.mulu);
    double prod = mul * t.fix;
    uint64_t produ = d2u(prod);

    uint64_t produ_mant = produ & MANT_MASK;

    uint64_t add_mantb = ((t.target - produ_mant) << 1) & MANT_MASK;

    uint64_t addu = t.addu_base | add_mantb;
    double add = u2d(addu);
    double d = prod + add; //fmadd(mul, t.fix, add);
    uint64_t u = d2u(d);

    t.found_n++;
    //printf("*: [0x%lx %.16e]*[FIX]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target);

    if(!isaddalnum(addu)) {
        return false;
    }

    printf("Y: [0x%lx %.16e]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", produ, prod, addu, add, u, d, FOUNDN/4, t.target, t.found_n);
    //printf("X: [0x%lx %.16e]*[FIX]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target);

    if((u&FOUND_MASK) != t.target) {
        printf("error\n");
        exit(1);
    }

    return true;
}

#if 0
static bool t2_mul(t2_t &t, int idx)
{
    if(idx < 0) {
        return t2_add(t);
    }

    for(size_t i=0; i<GBN; i++) {
        t.mulu &= ~(0xFFUL << (8*idx));
        t.mulu |= ((uint64_t)GB[i]) << (8*idx);

        if(t2_mul(t, idx-1)) {
            return true;
        }
    }

    return false;
}
#else
static bool t2_mul(t2_t &t, int idx)
{
    int retries = 1000*1000;
    srand(0U);

    while(retries > 0) {
        for(int i=0; i<=idx; i++) {
            t.mulu &= ~(0xFFUL << (8*i));
            t.mulu |= ((uint64_t)GB[rand()%GBN]) << (8*i);
        }

        if(t2_add(t)) {
            return true;
        }
    }
    return false;
}
#endif


void test2(uint64_t target)
{
    if((target & FOUND_MASK) != target) {
        printf("Bad target.\n");
        exit(1);
    }

    t2_t t = {};
    t.fix = u2d(0x4131555555555555UL);
    t.mulu = 0x4131555555555555UL;
    t.addu_base = 0x4260000000000000UL;

    t.midx = 5;

    #if 0
    for(uint64_t target = 0; target < (1UL << FOUNDN); target++) {
        t.target = target;
        t2_mul(t, t.midx);
    }
    uint64_t percent = (100*t.found_n) / (1UL << FOUNDN);
    printf("FOUND[%zu/%lu]: %lu%%\n", t.found_n, 1UL << FOUNDN, percent);
    #else
    t.target = target;
    t2_mul(t, t.midx);
    #endif
}

typedef struct {
    size_t found_n = 0;
    double fix;

    uint64_t target_A;
    uint64_t target_B;
    uint64_t addu_base;

    int midx;
    uint64_t mulu;
} t3_t;

static bool t3_add(t3_t &t, uint64_t &addu, double &prod, uint64_t target, double mul, bool sub)
{
    prod = mul * t.fix;
    uint64_t produ = d2u(prod);
    uint64_t produ_mant = produ & MANT_MASK;

    uint64_t add_mantb;
    if(sub == false) {
        add_mantb = ((target - produ_mant) << 1) & MANT_MASK;
    } else {
        add_mantb = (((produ_mant << 1) - target) << 0) & MANT_MASK;
    }

    addu = t.addu_base | add_mantb;

#if 1
    double d = prod + u2d(addu);
    if(sub) {
        d = prod - u2d(addu);
    }
    uint64_t u = d2u(d);
    if((u&FOUND_MASK) != target) {
        //printf("H: [0x%lx %.16e]*C+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", d2u(mul), mul, addu, u2d(addu), u, d, FOUNDN/4, t.target_A, t.found_n);
        printf("G[%c]: [0x%lx %.16e]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", sub?'s':' ', produ, prod, addu, u2d(addu), u, d, FOUNDN/4, t.target_A);
        printf("error target: have:0x%lx tgt:0x%lx [mul:%f]\n", u, target, mul);
        exit(1);
    }

#endif

    return isaddalnum(addu);
}

static bool t3_third(t3_t &t, uint64_t &addu, double &prod, bool &n, uint64_t target, double mul)
{ //Note: fmadd and fnmsub lead to the same result (mantisse wise)
    bool ret = false;
    ret = t3_add(t, addu, prod, target, mul, 0);
    if(ret) {
        n = false;
        return ret;
    }
    ret = t3_add(t, addu, prod, target, mul, 1);
    if(ret) {
        n = true;
        return ret;
    }
    return ret;
}

static bool t3_second(t3_t &t, long int retries)
{
    double mul = u2d(t.mulu);

    uint64_t addu; double prod; bool n;
    bool ret = t3_third(t, addu, prod, n, t.target_A, mul);
    if(!ret) {
        return false;
    }
    double add = u2d(addu);
    double d = prod + add; /* f(n)madd(mul, t.fix, add); */
    uint64_t u = d2u(d);

    uint64_t B_addu; double B_prod; bool B_n;
    ret = t3_third(t, B_addu, B_prod, B_n, t.target_B, mul);
    if(!ret) {
        return false;
    }
    double B_add = u2d(B_addu);
    double B_d = B_prod + B_add; //fmadd(mul, t.fix, add);
    if(B_n) {
        B_d = B_prod - B_add;
    }
    uint64_t B_u = d2u(B_d);

    //printf("FIX: [0x%lx %.16e]\n", d2u(t.fix), t.fix);
    printf("Y[%c]: [0x%lx %.16e]*C+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", n?'s':' ', t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target_A, retries);
    //printf("y[%c]: [0x%lx %.16e]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", n?'s':' ', d2u(prod), prod, addu, add, u, d, FOUNDN/4, t.target_A, retries);
    printf("Z[%c]: [0x%lx %.16e]*C+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", B_n?'s':' ', t.mulu, mul, B_addu, B_add, B_u, B_d, FOUNDN/4, t.target_B, retries);
    //printf("z[%c]: [0x%lx %.16e]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", B_n?'s':' ', d2u(B_prod), B_prod, B_addu, B_add, B_u, B_d, FOUNDN/4, t.target_B, retries);

    //printf("X: [0x%lx %.16e]*[FIX]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target);

    return true;
}

#if 0
static bool t3_add(t3_t &t)
{
    double mul = u2d(t.mulu);
    double prod = mul * t.fix;
    uint64_t produ = d2u(prod);

    uint64_t produ_mant = produ & MANT_MASK;

    uint64_t add_mantb = ((t.target_A - produ_mant) << 1) & MANT_MASK;

    uint64_t addu = t.addu_base | add_mantb;
    double add = u2d(addu);
    double d = prod + add; //fmadd(mul, t.fix, add);
    uint64_t u = d2u(d);

    t.found_n++;
    //printf("*: [0x%lx %.16e]*[FIX]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target);

    if(!isaddalnum(addu)) {
        return false;
    }

    uint64_t B_add_mantb = ((t.target_B - produ_mant) << 1) & MANT_MASK;
    uint64_t B_addu = t.addu_base | B_add_mantb;
    double B_add = u2d(B_addu);
    double B_d = prod + B_add; //fmadd(mul, t.fix, add);
    uint64_t B_u = d2u(B_d);
    if(!isaddalnum(B_addu)) {
        return false;
    }

    printf("Y: [0x%lx %.16e]*C+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target_A, t.found_n);
    printf("Z: [0x%lx %.16e]*C+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx retries:%zu}\n", t.mulu, mul, B_addu, B_add, B_u, B_d, FOUNDN/4, t.target_B, t.found_n);
    //printf("X: [0x%lx %.16e]*[FIX]+[0x%lx %.16e] == 0x%lx %.16e {TRGT:0x%0*lx}\n", t.mulu, mul, addu, add, u, d, FOUNDN/4, t.target);

    if((u&FOUND_MASK) != t.target_A) {
        printf("error A\n");
        exit(1);
    }

    return true;
}
#endif

static bool t3_mul(t3_t &t, int idx)
{
    long int max_tries = 100*1000*1000;
    long int retries = 0;
    srand(0U);

    while(retries < max_tries) {
        for(int i=0; i<=idx; i++) {
            t.mulu &= ~(0xFFUL << (8*i));
            t.mulu |= ((uint64_t)GB[rand()%GBN]) << (8*i);
        }

        if(t3_second(t, retries)) {
            return true;
        }
        retries++;
    }
    return false;
}

void test3(uint64_t target_A, uint64_t target_B)
{
    if((target_A & FOUND_MASK) != target_A) {
        printf("Bad target A.\n");
        exit(1);
    }
    if((target_B & FOUND_MASK) != target_B) {
        printf("Bad target B.\n");
        exit(1);
    }

    t3_t t = {};
    t.fix = u2d(0x4131555555555555UL);
    t.mulu = 0x4131555555555555UL;
    t.addu_base = 0x4260000000000000UL;

    t.midx = 5;

    t.target_A = target_A;
    t.target_B = target_B;
    t3_mul(t, t.midx);
}

int main(int argc, char *argv[])
{
#if 0
    double fix = u2d(0x4131555555555555UL);
    printf("Fix: %.10f 0x%016lx\n", fix, d2u(fix));
    double mul = u2d(0x4131555555555555UL);
    printf("Mul: %.10f 0x%016lx\n", mul, d2u(mul));
    double add = u2d(0x4260000000000000UL);
    printf("Add: %f 0x%016lx\n", add, d2u(add));
    double d = fmadd(mul, fix, add);
    printf("  X: %f 0x%lx\n", d, d2u(d));
#endif

    std::fesetround(FE_UPWARD);

#if 0
    test1();
#endif

#if 0
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <target>\n", argv[0]);
        exit(1);
    }

    uint64_t target = strtoull(argv[1], NULL, 16);
    test2(target);
#endif

#if 1
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <target A> <target B>\n", argv[0]);
        exit(1);
    }

    uint64_t target_A = strtoull(argv[1], NULL, 16);
    uint64_t target_B = strtoull(argv[2], NULL, 16);
    test3(target_A, target_B);
#endif
}