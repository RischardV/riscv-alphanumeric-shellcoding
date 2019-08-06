/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "payload_linux_shared.c"

/* We use this define to overcome linker limitations */
#define WIN_STRING "Hello, world from shellcode!\n"

int main(void)
{
    sys_write(1 /* stdout */, WIN_STRING, sizeof(WIN_STRING)-1);
    return 0;
}
