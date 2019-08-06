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
#include <fcntl.h>
#include <set>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

/* See paper for algorithm explanation */

static_assert(sizeof(double) == sizeof(uint64_t),
              "invalid 'double' size");
union udouble {
    double d;
    uint64_t u;
};

#define GBN 63
static uint8_t GB[GBN] = {
    '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '\'',
};

static bool isgb(uint8_t gb)
{
    return (gb > '0' && gb <= '9')
        || (gb > 'a' && gb <= 'z')
        || (gb > 'A' && gb <= 'Z')
        //|| (gb == '\'')
        ;
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

struct cache_hash {
public:
    size_t operator()(const __uint128_t n) const {
        uint64_t target_A = ((uint64_t) n);
        uint64_t target_B = ((uint64_t) (n >> 64));
        static_assert(sizeof(size_t) == sizeof(uint64_t), "bad cast to come");
        return target_A + target_B;
    }
};

std::unordered_set<__uint128_t, cache_hash> cache_bad;
std::unordered_set<__uint128_t, cache_hash> cache_good;
#define CACHE_PATH "build/x/%012lx-%012lx.b"

static void cache_add_local(bool good, __uint128_t target_AB)
{
    if(good) {
        cache_good.insert(target_AB);
    } else {
        cache_bad.insert(target_AB);
    }
}

static void cache_add(bool good, uint64_t target_A, uint64_t target_B)
{
    const __uint128_t target_AB = (((__uint128_t)target_A) << 64) + ((__uint128_t)target_B);

    char s[40];
    sprintf(s, CACHE_PATH, target_A, target_B);
    if(good) {
        s[34] = 'g';
    }

    /* Add to local cache */
    cache_add_local(good, target_AB);

    /* Add to global cache */
    int fd = open(s, O_RDWR|O_CREAT, 0777);
    if (fd == -1) {
        printf("OPEN ERROR\n");
        exit(1);
    }
    close(fd);
}

static int cached_status(uint64_t target_A, uint64_t target_B)
{
    const __uint128_t target_AB = (((__uint128_t)target_A) << 64) * ((__uint128_t)target_B);

    #if 1
    auto search = cache_bad.find(target_AB);
    if (search != cache_bad.end()) {
        return -1;
    }

    search = cache_good.find(target_AB);
    if (search != cache_good.end()) {
        return 1;
    }
    #endif

    /* Now try global cache */
    char s[40];
    sprintf(s, CACHE_PATH, target_A, target_B);
    if(access(s, F_OK) != -1) {
        //printf("Cached bad\n");
        cache_add_local(false, target_AB);
        return -1;
    }
    s[34] = 'g';
    if(access(s, F_OK) != -1) {
        //printf("Cached good\n");
        cache_add_local(true, target_AB);
        return 1;
    }

    return 0;
}

#define NO_FORCE false
#define FORCE true

#define FOUNDN 48
#define FOUND_MASK ((1UL << FOUNDN) - 1UL)
#define MANT_MASK 0x3FFFFFFFFFFFFUL

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

typedef struct {
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

static bool t3_mul(t3_t &t, int idx)
{
    long int max_tries = 2*1000*1000;
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

bool test3(uint64_t target_A, uint64_t target_B, bool force, int filei)
{
    if((target_A & FOUND_MASK) != target_A) {
        printf("Bad target A.\n");
        exit(1);
    }
    if((target_B & FOUND_MASK) != target_B) {
        printf("Bad target B.\n");
        exit(1);
    }

    int status = cached_status(target_A, target_B);
    if(status == -1) {
        //printf("Cached bad\n");
        return false;
    } else if((status == 1) && (!force)) {
        //printf("Cached good\n");
        return true;
    }

    printf("XX[%d]: %012lx-%012lx not found!!\n", filei, target_A, target_B);

    t3_t t = {};
    t.fix = u2d(0x4131555555555555UL);
    t.mulu = 0x4131555555555555UL;
    t.addu_base = 0x4260000000000000UL;

    t.midx = 5;

    t.target_A = target_A;
    t.target_B = target_B;

    bool ret = t3_mul(t, t.midx);

    if(!((status == 1) && force)) {
        cache_add(ret, target_A, target_B);
    }

    return ret;
}

static uint64_t getT(const uint16_t *buf, size_t idx)
{
    idx = 3*idx;
    uint64_t r = buf[idx];
    r += ((uint64_t) buf[++idx]) << 16;
    r += ((uint64_t) buf[++idx]) << 32;
    return r;
}

static bool try_config(uint16_t *buf, bool force, int filei)
{
    bool b;

    uint64_t target_A = getT(buf, 0);
    uint64_t target_B = getT(buf, 1);
    b = test3(target_A, target_B, force, filei);
    if(!b) {
        return 0;
    }

    uint64_t target_C = getT(buf, 2);
    uint64_t target_D = getT(buf, 3);
    b = test3(target_C, target_D, force, filei);
    if(!b) {
        return 0;
    }

    uint64_t target_E = getT(buf, 4);
    uint64_t target_F = getT(buf, 5);
    target_F &= 0xFFFFFFFFUL;
    target_F |= target_E & 0xFFFF00000000UL;
    b = test3(target_E, target_F, force, filei);
    if(!b) {
        return 0;
    }

    return 1;
}

#define BCOUNT 20

static bool try_file(const uint16_t *orig_buf, bool force, int filei)
{
    uint16_t buf[BCOUNT];
    memcpy(buf, orig_buf, sizeof(buf));

    for(int a=0; a<6; a++) {
        switch(a) {
            case 0:
                break;
            case 1:
                buf[4] = orig_buf[4];
                buf[5] = orig_buf[5];
                buf[6] = orig_buf[7];
                buf[7] = orig_buf[6];
                break;
            case 2:
                buf[4] = orig_buf[6];
                buf[5] = orig_buf[4];
                buf[6] = orig_buf[5];
                buf[7] = orig_buf[7];
                break;
            case 3:
                buf[4] = orig_buf[6];
                buf[5] = orig_buf[7];
                buf[6] = orig_buf[4];
                buf[7] = orig_buf[5];
                break;
            case 4:
                buf[4] = orig_buf[7];
                buf[5] = orig_buf[4];
                buf[6] = orig_buf[5];
                buf[7] = orig_buf[6];
                break;
            case 5:
                buf[4] = orig_buf[7];
                buf[5] = orig_buf[6];
                buf[6] = orig_buf[4];
                buf[7] = orig_buf[5];
                break;

        }

        for(int b=0; b<7; b++) {
            for(int i=0; i<7; i++) {
                if(i < b) {
                    buf[8+i] = orig_buf[9+i];
                } else if(b == i) {
                    buf[8+i] = orig_buf[8];
                } else { /* i > b */
                    buf[8+i] = orig_buf[9+i-1];
                }
            }
            bool ret = try_config(buf, NO_FORCE, filei);
            if(ret) {
                if(force) {
                    printf("CONFIG [%d]:", b);
                    uint8_t *bbuf = (uint8_t *) buf;
                    for(size_t i=0; i<BCOUNT*2; i++) {
                        if(i%6 == 0) {
                            if(i%12 == 0) {
                                printf(" ");
                            } else {
                                printf("-");
                            }
                        }
                        printf("%02x", bbuf[i]);
                    } printf("\n");
                    try_config(buf, force, filei);
                }
                return true;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    std::fesetround(FE_UPWARD);

    if(argc != 3) {
        fprintf(stderr, "Usage: %s <start> <end>\n", argv[0]);
        fprintf(stderr, "\tWill run %s on [start;end[ range\n", argv[0]);
        exit(1);
    }

    int start = atoi(argv[1]);
    int end = atoi(argv[2]);
    printf("Try: %d->%d\n", start, end-1);

    char infile[] = "build/xxxxxxxxxxxxxxx";
    for(int i=start; i<end; i++) {
        uint16_t orig_buf[BCOUNT];
        sprintf(infile+6, "%d.x", i);
        FILE *fp = fopen(infile, "r");
        if(fp == NULL) {
            printf("fopen failed.\n");
            exit(1);
        }
        size_t fret = fread(orig_buf, sizeof(orig_buf), 1, fp);
        if(fret != 1) {
            printf("fread failed.\n");
            exit(1);
        }
        fclose(fp);

        bool ret = try_file(orig_buf, NO_FORCE, i);
        if(ret) {
            printf("Found [%d]!\n", i);
            try_file(orig_buf, FORCE, i);
            exit(42);
        }
    }
}