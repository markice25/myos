#ifndef __MATH_H__
#define __MATH_H__


extern void div32(unsigned int divident, unsigned int divisor, unsigned int *quotient, unsigned int *remainder);
extern unsigned int mul32(unsigned int a, unsigned int b);
extern unsigned int rand();
extern unsigned int clz32();

#endif
