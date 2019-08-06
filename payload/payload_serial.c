/*
 * (c) 2018-2019 Hadrien Barral
 * SPDX-License-Identifier: Apache-2.0
 */

#define SERIAL_BASE 0x10013000U
#define SERIAL_FLAG_OFFSET 0x18
#define SERIAL_BUFFER_FULL (1 << 5)

static void x_putc(char c)
{
#if 0
    while (*(volatile unsigned long*)(SERIAL_BASE + SERIAL_FLAG_OFFSET)
                                     & (SERIAL_BUFFER_FULL));
#endif
    *(volatile unsigned*)SERIAL_BASE = c;
#if 0
    if (c == '\n') x_putc('\r');
#endif
}

static void x_puts(const char *s)
{
    do {
        x_putc(*s);
    } while(*++s);
}

int main(void)
{
    x_puts("Hello, world!\n");
    return 0;
}
