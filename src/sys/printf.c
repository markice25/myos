#include "stdarg.h"
#include "math.h"


#define BCM2835_BASE            (0x3f000000ul)
#define PL011_BASE              (BCM2835_BASE + 0x201000ul)
#define PL011_DR                (pl011_base)
#define PL011_FR                (pl011_base + 0x18ul)


unsigned long pl011_base = PL011_BASE;

static int raspi2_putchar(int ch)
{
    unsigned char ready = 0;
    while (!ready) {
        volatile unsigned char *addr = (void *)PL011_FR;
        ready = !(*addr & 0x20);
    }
    
    volatile unsigned int *addr = (void *)PL011_DR;
    *addr = ch;
    return 1;
}

static void print_binary(unsigned int num)
{
    raspi2_putchar('0');
    raspi2_putchar('b');
    ;
    for (unsigned int i = clz32(num); i < 32; i++)
    {
        unsigned int temp = num << i >> 31;
        raspi2_putchar(temp+48);
    }
    
}



static void print_dec(unsigned int num)
{
    unsigned int divisor[] = { 1000000000u, 100000000u, 10000000u,
        1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };
    unsigned int q = 0;
    unsigned int r = 0;
    int flag = 0;
    for (int i = 0; i < 10; i++)
    {
        q = 0;
        r = 0;
        div32(num, divisor[i], &q, &r);
        num = r;
        flag = (q !=0 || flag != 0) ? 1 : 1;
        if (flag == 0)
        {
            continue;
        }
        else
        {
            raspi2_putchar(q+48);
        }
    }
}

static void print_hex32(unsigned int num)
{
    raspi2_putchar('0');
    raspi2_putchar('x');
    for (int i = 0; i < 8; i++){
        char temp =(char) (num << (i << 2) >>28);
        if (temp < 10){
            raspi2_putchar(temp+48);
        }
        else{
            raspi2_putchar(temp+55);
        }
/*        print_binary(num << (i << 2) >>28);*/
/*        raspi2_putchar(10);*/
    }
}



int vprintk(const char *fmt, va_list args)
{
    unsigned int num = 0;
    char *str;
    const char *s = fmt;
    while (*s) {
        char c = *s;
        switch (c) {
        case '%':
            s++;
            switch (*s) {
            case 's':
                str = va_arg(args,char*);
                while (*str){
                    raspi2_putchar(*str);
                    str++;
                }
                break;
            case 'b':
                num = va_arg(args, unsigned int);
                print_binary(num);
                break;
            case 'd':
                num = va_arg(args, unsigned int);
                print_dec(num);
                break;
            case 'c':
                num = va_arg(args, int);
                raspi2_putchar(num);
                break;
            case 'h':
                print_hex32(va_arg(args,unsigned int));
                break;
            }
            break;
        default:
            raspi2_putchar(c);
            break;
        }
        s++;
    }
    return 1;
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
