/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint64_t shellcode(void);

__attribute__((weak))
uint64_t func2(const char *in, void **array, int offset) {
    char name[300];

    strcpy(name, in); /* Blatant SO incoming */
    name[sizeof(name)-1] = '\0';
    array[offset] = name;

    return strlen(name);
}

__attribute__((weak))
uint64_t func1(const char *bad, int size) {
    uint8_t arr[1024];
    if(size < 1024) {
        memset(arr, size, sizeof(arr));
        return func1(bad, size+1);
    }
    return func2(bad, (void **)arr, -1);
}

int main(void) {
    puts("Hello world from the buggy program on HifiveU board.");
    puts("Waiting for input...");

    char *bad = NULL; size_t n;
    ssize_t status = __getline(&bad, &n, stdin);
    if(status == -1) {
        fprintf(stderr, "getline failed\n");
        exit(1);
    }
    //printf("line:%s\n", bad);

    puts("Processing data...");
    uint64_t ret = func1(bad, 0);

    free(bad);
    printf("Exiting from shellcode (%#lx).\n", ret);
    return 0;
}
