/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#include <array>
#include <chrono>
#include <cstdbool>
#include <cstdio>
#include <cstring>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

/*
 * Sorry for the lack of documentation.
 * This is unfortunately a code much easier to write
 *  than to read.
 */

#define FENCI 0

using namespace std;
#define countof(x) (sizeof(x)/sizeof(x[0]))
typedef long long lli;
typedef unsigned long long llu;

typedef int reg_t;
typedef struct txt_y {
    const txt_y *prev1;
    const txt_y *prev2;
    const size_t tail; /* index in 'txt_parts' */
} txt_t;

typedef struct {
    lli shift;
    int cost;
} loadshift_t;

typedef struct {
    int cost;
    reg_t src; /* source reg */
    reg_t dst; /* dst reg */
    reg_t trd; /* third register (when applicable) */
    lli value; /* immediate value */
    txt_t txt; /* instruction text */
} instr_t;

typedef struct {
    bool valid;
    int cost;
    lli value; /* loaded full value */
    reg_t dst; /* dst reg */
    string txt;
} seq_t;

typedef array<seq_t, 256*256> sequences_t;

static reg_t parse_reg(const string &reg)
{
    int nb;
    if(reg == "gp") {
        return 3;
    }
    if(reg == "zero") {
        return 3;
    }
    switch(reg[0]) {
        case 'a':
            nb = stoi(reg.substr(1, 1));
            return 10+nb;
        case 's':
            if(reg == "sp") {
                return 2;
            }
            nb = stoi(reg.substr(1, 1));
            if(nb < 2) {
                return 8+nb;
            }
            return 18-2+nb;
        case 't':
            if(reg == "tp") {
                return 4;
            }
            nb = stoi(reg.substr(1, 1));
            if(nb < 3) {
                return 5+nb;
            }
            return 28-3+nb;
        default:
            printf("BAD reg: %s\n", reg.c_str());
            exit(1);
    }
}
#define REG_COUNT 33
#define NO_REG 32
#define PR parse_reg

static vector<string> txt_parts; /* Contains all text string chunk associated to instructions */
static vector<instr_t> lui;
static array<vector<instr_t>, REG_COUNT> sra;
static array<vector<instr_t>, REG_COUNT> addiw;
static array<vector<instr_t>, REG_COUNT> addiw2;
static array<vector<instr_t>, REG_COUNT> addiw3;
static array<vector<instr_t>, REG_COUNT> addiw4;
static array<vector<instr_t>, REG_COUNT> li_shift;
static array<vector<instr_t>, REG_COUNT> li2_shift;
static array<bool, 256*256> badupval;
static array<bool, REG_COUNT> store_reg;

static stringstream read_file(const string &filename) {
    ifstream in(filename.c_str());
    stringstream sstr;
    while(in >> sstr.rdbuf());
    return sstr;
}

static int val2shift(lli val)
{
    return (((llu)val) & 0b111111ULL);
}

static void init_addiwn(const auto &addiwl, auto &addiwh)
{
    for(reg_t reg=0; reg < REG_COUNT; reg++) {
        array<bool, 1024> addiw_val;
        for (auto &used : addiw_val) {
            used = false;
        }

        /* Add values we can already do */
        for (const auto &e : addiw[reg]) {
            addiw_val[512+e.value] = true;
        }
        for (const auto &e : addiw2[reg]) {
            addiw_val[512+e.value] = true;
        }
        for (const auto &e : addiw3[reg]) {
            addiw_val[512+e.value] = true;
        }
        for (const auto &e : addiw4[reg]) {
            addiw_val[512+e.value] = true;
        }
        addiw_val[512] = true; /* no need to have '+0' */

        for (const auto &e : addiwl[reg]) {
            for (const auto &f : addiw[reg]) {
                lli sum = e.value + f.value;
                if(!addiw_val[512+sum]) {
                    instr_t g = {.cost = e.cost + f.cost,
                                 .src = e.src,
                                 .dst = e.dst,
                                 .trd = e.trd,
                                 .value = sum,
                                 .txt = {.prev1 = &e.txt, .prev2 = &f.txt, .tail = 0}};
                    addiwh[reg].push_back(g);
                    addiw_val[512+sum] = true;
                }
            }
        }
    }
}

static void init_glo(char *list_filename)
{
    std::stringstream in = read_file(list_filename);
    std::string l;
    int cost = 2;
    txt_parts.clear();
    txt_parts.push_back(""); /* Index 0 is empty string */
    lui.clear();
    for(size_t i=0; i<REG_COUNT; i++) {
        sra[i].clear();
        addiw[i].clear();
        addiw2[i].clear();
        addiw3[i].clear();
        addiw4[i].clear();
        li_shift[i].clear();
        li2_shift[i].clear();
    }

    while (std::getline(in, l)) {
        if((cost == 2) && (l.substr(0, 5) == "=====")) {
            cost = 4;
            continue;
        }
        std::stringstream ll(l);
        string iname;
        getline(ll, iname, '\t');
        if(iname == "lui") {
            string dst_s;
            string value_s;
            getline(ll, dst_s, ',');
            getline(ll, value_s, ',');
            reg_t dst = parse_reg(dst_s);
            lli value = stoll(value_s, 0, 16) << 12;
            if(value > INT32_MAX) {
                value = (lli)(int32_t)(uint32_t) value;
            }

            txt_parts.push_back(l);
            instr_t new_elem = {.cost = cost, .src = NO_REG, .dst = dst, .trd = NO_REG, .value = value,
                                .txt = {.prev1 = NULL, .prev2 = NULL, .tail = txt_parts.size()-1}};
            lui.push_back(new_elem);
        } else if(iname == "addiw") {
            string dst_s;
            string src_s;
            string value_s;
            getline(ll, dst_s, ',');
            getline(ll, src_s, ',');
            getline(ll, value_s, ',');
            reg_t src = parse_reg(src_s);
            reg_t dst = parse_reg(dst_s);
            lli value = stoll(value_s);

            if(src != dst) {
                printf("Unsupported addiw\n");
                exit(1);
            }

            txt_parts.push_back(l);
            instr_t new_elem = {.cost = cost, .src = src, .dst = dst, .trd = NO_REG, .value = value,
                                .txt = {.prev1 = NULL, .prev2 = NULL, .tail = txt_parts.size()-1}};
            addiw[src].push_back(new_elem);
        } else if(iname == "sra") {
            string dst_s;
            string src_s;
            string trd_s;
            getline(ll, dst_s, ',');
            getline(ll, src_s, ',');
            getline(ll, trd_s, ',');
            reg_t src = parse_reg(src_s);
            reg_t dst = parse_reg(dst_s);
            reg_t trd = parse_reg(trd_s);

            if(src == trd) {
                continue; /* useless ! */
            }

            txt_parts.push_back(l);
            instr_t new_elem = {.cost = cost, .src = src, .dst = dst, .trd = trd, .value = 0,
                                .txt = {.prev1 = NULL, .prev2 = NULL, .tail = txt_parts.size()-1}};
            sra[src].push_back(new_elem);
        } else if(iname == "li") {
            string dst_s;
            string value_s;
            getline(ll, dst_s, ',');
            getline(ll, value_s, ',');
            reg_t dst = parse_reg(dst_s);
            lli value = stoll(value_s, 0, 10);

            txt_parts.push_back(l);
            instr_t new_elem = {.cost = cost, .src = NO_REG, .dst = dst, .trd = NO_REG, .value = value,
                                .txt = {.prev1 = NULL, .prev2 = NULL, .tail = txt_parts.size()-1}};
            li_shift[dst].push_back(new_elem);
        }
    }

    init_addiwn(addiw, addiw2);  /* Add useful sum of two addiw */
    init_addiwn(addiw2, addiw3); /* Add useful sum of three addiw */
    init_addiwn(addiw3, addiw4); /* Add useful sum of four addiw */

    /* Add useful sum of li+addiw */
    for(reg_t reg=0; reg < REG_COUNT; reg++) {
        array<bool, 64> li_val;
        for (auto &used : li_val) {
            used = false;
        }

        /* Add values we can do with a single li_shift */
        for (const auto &e : li_shift[reg]) {
            int val = val2shift(e.value);
            li_val[val] = true;
        }
        li_val[0] = true; /* no need to have 'li 0' */

        for (const auto &e : li_shift[reg]) {
            for (const auto &f : addiw[reg]) {
                lli sum = e.value + f.value;
                int sum_shift = val2shift(sum);
                if(!li_val[sum_shift]) {
                    instr_t g = {.cost = e.cost + f.cost,
                                 .src = e.src,
                                 .dst = e.dst,
                                 .trd = e.trd,
                                 .value = sum,
                                 .txt = {.prev1 = &e.txt, .prev2 = &f.txt, .tail = 0}};
                    li2_shift[reg].push_back(g);
                    li_val[sum_shift] = true;
                }
            }
        }
    }

    vector<reg_t> store_reg_inv{PR("gp"), PR("s3"), PR("s4"), PR("s5"),
                                PR("s6"), PR("s7"), PR("t0"), PR("t1"),
                                PR("t2"), PR("tp")                    };
    std::fill(begin(store_reg), end(store_reg), 0);
    for (reg_t reg : store_reg_inv) {
        store_reg[reg] = 1;
    }

    std::fill(begin(badupval), end(badupval), 0);
    badupval[0x0000] = 1; /* "all zero bits is not legal" */
    badupval[0x0C00] = 1;

    printf("init_glo done\n");
}

#if FENCI
static bool badup(lli value)
{
    uint16_t up = (value>>16) & 0xFFFFU;
    return up != 0;
}
#else
static bool badup(lli value)
{
    return false;

    if(((value & 0xE003U) == 0xA001U) /* j */ ||
       (((value & 0xE003U) == 0x8002U) && ((value & 0x007CU) == 0x0000U) /* jalr */)) {
        return false; /* This is a jump. 'up' will not get executed. */
    }

    uint16_t up = (value>>16) & 0xFFFFU;
    if((up%4) == 0b11U) {
        return true;
    }
    return badupval[up];
}
#endif

static void build_instr_txt_sub(stringstream &store, const txt_t *txt)
{
    if(txt->prev1) {
        build_instr_txt_sub(store, txt->prev1);
    }
    if(txt->prev2) {
        build_instr_txt_sub(store, txt->prev2);
    }
    if(txt->tail != 0) {
        store << txt_parts[txt->tail] << endl;
    }
}

static string build_instr_txt(const txt_t &txt)
{
    stringstream store;
    build_instr_txt_sub(store, &txt);
    return store.str();
}

static void try_add_value(sequences_t &t, const instr_t &e)
{
    /* Check if the upper bits lead to an incorrect instruction */
    if(badup(e.value)) {
        return;
    }

    /* Check if we can actually store the value */
    if(!store_reg[(size_t)e.dst]) {
        return;
    }

    /* Check if we already have a better one for this value */
    lli lo = e.value & 0xFFFFULL;
    if(t[lo].valid && (t[lo].cost <= e.cost)) {
        return;
    }

    /* We are good to add it! */
    string txt = build_instr_txt(e.txt);
    seq_t seq = {.valid = 1, .cost = e.cost, .value = e.value, .dst = e.dst, .txt = txt};
    t[lo] = seq;
}

static void try_add_addiw_sub(sequences_t &t, const instr_t &e, const instr_t &f)
{
    instr_t g =  {.cost = e.cost + f.cost,
                  .src = e.src,
                  .dst = f.dst,
                  .trd = NO_REG,
                  .value = e.value + f.value,
                  .txt = {.prev1 = &e.txt, .prev2 = &f.txt, .tail = 0}};
    try_add_value(t, g);
}

static void try_add_addiw(sequences_t &t, const instr_t &e)
{
    #if 1
    /*
     * We know that addiw does not change the destination.
     * Let us do some check from 'try_add_value' here.
     */
    if(!store_reg[(size_t)e.dst]) {
        return;
    }
    #endif

    for (const instr_t &f : addiw[e.dst]) {
        try_add_addiw_sub(t, e, f);
    }
    for (const instr_t &f : addiw2[e.dst]) {
        try_add_addiw_sub(t, e, f);
    }
    for (const instr_t &f : addiw3[e.dst]) {
        try_add_addiw_sub(t, e, f);
    }
    for (const instr_t &f : addiw4[e.dst]) {
        try_add_addiw_sub(t, e, f);
    }
}

static void try_add_sra_sub(sequences_t &t, const instr_t &e, const instr_t &f, const instr_t &h)
{
    const txt_t txt_tmp = {.prev1 = &h.txt, .prev2 = &f.txt, .tail = 0};
    instr_t g = {.cost = e.cost + f.cost + h.cost,
                 .src = e.src,
                 .dst = f.dst,
                 .trd = NO_REG,
                 .value = e.value >> val2shift(h.value),
                 .txt = {.prev1 = &e.txt, .prev2 = &txt_tmp, .tail = 0}};
    try_add_value(t, g);
    try_add_addiw(t, g);
}

static void try_add_sra(sequences_t &t, const instr_t &e)
{
    for (const instr_t &f : sra[e.dst]) {
        for(const instr_t &h : li_shift[f.trd]) {
            try_add_sra_sub(t, e, f, h);
        }
        for(const instr_t &h : li2_shift[f.trd]) {
            try_add_sra_sub(t, e, f, h);
        }
    }
}

static void sequence_lui(sequences_t &t)
{
    size_t count = lui.size();
    size_t steps = 100;
    size_t forstep = (count + steps - 1)/steps;
    size_t foridx = 0;
    printf("sequence_lui:"); fflush(stdout);
    for (const instr_t &e : lui) {
        if((++foridx % forstep)== 0) {
            printf("%d%%", (int)(((100./steps)*foridx)/forstep)); fflush(stdout);
            return;
        }
        try_add_value(t, e);
        try_add_addiw(t, e);
        try_add_sra(t, e);
    }
    printf("\n");
}

vector<sequences_t> workers_t;

static void sequence_lui_p(unsigned int core_id, size_t start_idx, size_t end_idx, lli *elapsed)
{
    size_t count = end_idx - start_idx;
    size_t steps = 50;
    size_t forstep = (count + steps - 1)/steps;
    size_t foridx = 0;

    auto time_begin = std::chrono::high_resolution_clock::now();
    sequences_t &t = workers_t[core_id];
    #if 0
    printf("slui:%zu %zu\n", start_idx, end_idx);
    #endif

    for (size_t idx = start_idx; idx < end_idx; idx++) {
        const instr_t &e = lui[idx];
        if((++foridx % forstep)== 0) {
            if(core_id == 0) {
                printf("%d%%", (int)(((100./steps)*foridx)/forstep)); fflush(stdout);
            }
        }
        try_add_value(t, e);
        try_add_addiw(t, e);
        try_add_sra(t, e);
    }

    printf("[%u]", core_id); fflush(stdout);

    auto time_end = std::chrono::high_resolution_clock::now();
    lli _elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end-time_begin).count();
    *elapsed = _elapsed;
}

static void best_sequences(char *split_filename, sequences_t &t)
{
    for (seq_t &seq : t) {
        seq.valid = 0;
    }

    unsigned int cores = std::thread::hardware_concurrency();
    if(cores == 0) {
        printf("Unable to determine how many workers to spawn. Using 1.");
        cores = 1;
    }

    if(cores == 1) {
        /* Non-parallel version */
        sequence_lui(t);
        return;
    }

    size_t whole_count = lui.size();

    vector<lli> workers_time;
    for(unsigned int core=0; core<cores; core++) {
        workers_t.push_back(t);
        workers_time.push_back(0);
    }

    printf("[best_sequences]\n"); fflush(stdout);
    vector<std::thread> workers;

    /* Compute workload repartition */
    vector<size_t> idx_table;
    if(access(split_filename, F_OK) != -1) {
        idx_table.push_back(0);
        std::stringstream in_idx = read_file(split_filename);
        size_t idx;
        while(in_idx >> idx) {
            idx_table.push_back(idx);
        }
        if((idx_table.size() != (cores+1)) ||(idx_table[cores] != whole_count)) {
            printf("'%s' is corrupt (%zu/%u).\n", split_filename, idx_table.size(), cores+1);
            exit(1);
        }
    } else {
        size_t chunk = whole_count / cores;
        for(unsigned int core=0; core<=cores; core++) {
            size_t idx = core*chunk;
            if(core == cores) {
                idx = whole_count; /* because of rounding issues */
            }
            idx_table.push_back(idx);
        }
    }

    /* Start workers */
    for(unsigned int core=0; core<cores; core++) {
        workers.push_back(std::thread(sequence_lui_p, core, idx_table[core], idx_table[core+1], &workers_time[core]));
    }

    /* Wait for workers to end */
    for (auto& worker : workers) {
        worker.join();
    }
    printf("\n");

    /* Try to compute a better workload repartition */
    std::ofstream time_out(split_filename);
    for(unsigned int core=1; core<cores; core++) {
        lli mult = 10000;
        lli diff =    50;
        if(workers_time[core-1] > workers_time[core]) {
            idx_table[core] = ((mult-diff)*idx_table[core])/mult;
        } else {
            idx_table[core] = ((mult+diff)*idx_table[core])/mult;
        }
    }
    for(unsigned int core=1; core<=cores; core++) {
        time_out << idx_table[core] << endl;
    }
    time_out.close();

    /* Join values from our workers */
    for(const auto& worker_t : workers_t) {
        size_t size = worker_t.size();
        for(size_t lo=0; lo<size; lo++) {
            const seq_t &e = worker_t[lo];
            if(!e.valid) {
                continue;
            }
            if(t[lo].valid && (t[lo].cost <= e.cost)) {
                continue;
            }
            t[lo] = e;
        }
    }
}

static void print_stats(const sequences_t &t, lli elapsed)
{
    size_t size = t.size();
    size_t count = 0;
    for(size_t i=0; i<t.size(); i++) {
        if(t[i].valid) {
            count++;
        }
    }
    printf("Time: %lli.%03llis\n", elapsed/1000000000, (elapsed%1000000000)/1000000);
    printf("ALL: %zu/%zu (%zu%%)\n", count, size, (count*100)/size);

    #if 0
    size_t found = 0;
    for(size_t i=0; i<size; i++) {
        if(t[i].valid) {
            printf("VALID[0x%zx]{%d}: '%s'\n", i, t[i].cost, t[i].txt.c_str());
            if(++found >= 1) {
                break;
            }
        }
    }
    #endif
}

#if FENCI
static void print_fenci(const sequences_t &t)
{
    for(size_t i=0xFC0; i<0x1040; i++) {
        if(t[i].valid) {
            printf("VALID[0x%zx]: '%s'\n", i, t[i].txt.c_str());
        }
    }
}
#endif

static void save_json(char *filename, sequences_t &t)
{
    ofstream json;
    json.open(filename);
    json << "{";

    bool first = true;

    size_t size = t.size();
    for(size_t i=0; i<size; i++) {
        if(t[i].valid) {
            if(!first) {
                json << ",\n";
            } else {
                first = 0;
            }
            json << "\"" << i << "\":{\"cost\":" << t[i].cost <<
                    ",\"dst\":\"" << t[i].dst << "\",\"txt\":\"";
            for(const char &c: t[i].txt) {
                if(isalnum(c) || (c == ' ') || (c == ',') || (c == '.') ||
                   (c == '(') || (c == ')') || (c == '-')) {
                    json << c;
                } else if(c == '\t') {
                    json << "\\t";
                } else if(c == '\n') {
                    json << "\\n";
                } else {
                    printf("unknown char: '%c' (0x%x)\n", c, c);
                    exit(1);
                }
            }
            json << "\",\"value\":" << t[i].value << "}";
        }
    }

    json << "}";
    json.close();
}

int main(int argc, char *argv[])
{
    printf("Loadgen starting. [Compiled at %s %s]\n", __DATE__, __TIME__);

    if(argc != 4) {
        fprintf(stderr, "Usage: %s <out_file.json> <in_list.txt> <internal_state.file>", argv[0]);
        exit(1);
    }

    init_glo(argv[2]);

    auto time_begin = std::chrono::high_resolution_clock::now();
    sequences_t t;
    best_sequences(argv[3], t);
    auto time_end = std::chrono::high_resolution_clock::now();

    lli elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end-time_begin).count();
    print_stats(t, elapsed);

#if FENCI
    (void)save_json;
    print_fenci(t);
#else
    save_json(argv[1], t);
#endif

    return 0;
}
