typedef __builtin_va_list       va_list;
#define va_start(ap, last)      __builtin_va_start(ap, last)
#define va_arg(ap, type)        __builtin_va_arg(ap, type)
#define va_end(ap)              __builtin_va_end(ap)

#define BCM2835_BASE            (0x3f000000ul)
#define PL011_BASE              (BCM2835_BASE + 0x201000ul)
#define PL011_DR                (PL011_BASE)
#define PL011_FR                (PL011_BASE + 0x18ul)

static int raspi2_putchar(int ch)
{
    unsigned char ready = 0;
    while (!ready) {
        volatile unsigned char *addr = (void *)PL011_FR;
        ready = !(*addr & 0x20);
    }
    
    volatile unsigned char *addr = (void *)PL011_DR;
    *addr = ch;
    return 1;
}

static void print_hex(unsigned int num)
{
}

int vprintk(const char *fmt, va_list args)
{
    unsigned int num = 0;
    const char *s = fmt;
    while (*s) {
        char c = *s;
        switch (c) {
        case '%':
            s++;
            switch (*s) {
            case 'd':
                num = va_arg(args, unsigned int);
                raspi2_putchar(num >> )
            }
            break;
        default:
            raspi2_putchar(c);
            break;
        }
        s++;
    }
}

int printk(const char *fmt, ...)
{
    int ret = 0;
    
    va_list args;
    va_start(args, fmt);
    ret = vprintk(fmt, args);
    va_end(args);
    
    return ret;
}

int main()
{
    while (1) {
        printk("test: %d\n", 100);
    }
}

