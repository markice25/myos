#include "printf.h"
void div32(unsigned int divident, unsigned int divisor, unsigned int *quotient, unsigned int *remainder)
{
/*    printk("start\n");*/
    int n = 0;
    while (divident > divisor && (divisor >> 31) == 0)
    {
        divisor = divisor << 1;
        n++;
    }

    while(divident > 0 && n >= 0)
    {
/*        printk("n:%h\n",n);*/
/*        printk("divident:%h\n",divident);*/
/*        printk("divisor:%h\n",divisor);*/
        if(divident >= divisor)
        {
            *quotient += (1u << n);
            divident -= divisor;
        }
        divisor = divisor >> 1;
        n--;
    }
    *remainder = divident;
/*    printk("done\n");*/
}

unsigned int mul32(unsigned int a, unsigned int b)
{
    unsigned int result = 0;
    for (int i = 0; i < 32; i++)
    {
        if (b << (32 - i -1) >> 31 == 1)
        {
            result += (a << i);
        } 
    }
    return result;
}


unsigned int rand(unsigned int *seed,unsigned int start, unsigned int end)
{
    unsigned int q = 0;
    unsigned int r = 0;
    div32(mul32(1103515245, *seed) + 12345, 1 << 31, &q, &r);
    *seed = r;
    q = 0;
    r = 0;
    div32(mul32((end-start),*seed >> 16), 1u << 15, &q, &r);
    return start + q;
}

unsigned int clz32(unsigned int num)
{
    unsigned int c = 0;
    for (int i = 0; i < 32; i++)
    {
        if (num << i >> 31 == 0)
        {
            c++;
        }
        else
        {
            break;
        }
    }
    return c;
}
