#include "printf.h"
#include "math.h"
#include "buddy.h"

extern void start_paging();
extern void mem_test();
extern void init_proc();

int main()
{
/*    while (1) {*/
/*        printk("test: %d\n", 1100100);*/
/*    }*/
/*    for (int i = 33; i < 130; i ++){*/
/*    raspi2_putchar(i);*/ 
/*    }*/
/*    */
/*    print_binary(1000);*/
/*    printk("Print 1000 as binary:%b\n",1000);*/
/*    printk("Print 1000 as hex:%h\n",1000);*/
/*    printk("Print string: %s\n", "abcdefg");*/
/*    printk("Print a as ASCII: %c\n",'a');*/
/*    unsigned int q = 0;*/
/*    unsigned int r = 0;*/
/*    div32(48,3,&q,&r);*/
/*    printk("48/3:quotient:%b\n, remainder:%b\n",q,r);*/
/*    q = 0;*/
/*    r = 0;*/
/*    div32(100,100,&q,&r);*/
/*    printk("100/100:quotient:%b\n, remainder:%b\n",q,r);*/
/*    q = 0;*/
/*    r = 0;*/
/*    div32(100,1000,&q,&r);*/
/*    printk("100/1000:quotient:%b\n, remainder:%b\n",q,r);*/
/*    q = 0;*/
/*    r = 0;*/
/*    div32(923,9,&q,&r);*/
/*    printk("923/9:quotient:%b\n, remainder:%b\n",q,r);*/
    init_buddy();
    start_paging();
    init_proc();
    //mem_test();

    
    

}

