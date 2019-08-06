/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

#include "payload_linux_shared.c"

/* We use this define to overcome linker limitations */
#define WIN_STRING "Hello from shellcode, I will now spawn a shell...\n"

int main(void)
{
    sys_write(1 /* stdout */, WIN_STRING, sizeof(WIN_STRING)-1);
    sys_execve("/bin/sh", NULL, NULL);

    #define ERR_STRING "Failed to spawn shell\n"
    sys_write(1 /* stdout */, ERR_STRING, sizeof(ERR_STRING)-1);
    return 0;
}
