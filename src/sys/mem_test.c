#include "math.h"
#include "printf.h"
#include "buddy.h"

void mem_test()
{
    //init_buddy();
    static unsigned long addresses[512 * 1024 * 1024 / 4096];
    unsigned int seed = 1234;
    unsigned long address;
    int index = 0;
    int print_idx = 1;
    print_buddies();
    do {
        unsigned int order = rand(&seed,0,13);
        //printk("order:%h\n",order);
        
        //printk("address:%h",address);
        
/*        if (index == print_idx) { print_buddies(); printk("\n"); }*/
        address = alloc_page(order);
        addresses[index] = address;
        printk("//////////////////////////////");
        printk("order: %h \n", order);
        print_buddies();
        
/*        if (index == print_idx) { print_buddies(); printk("order: %h, addr: %h\n\n", order, address); }*/
/*        printk("has done %h\n",index);*/
        
        index++;
    //} while(address || index == 1);
    } while(index < 100);

    for (int i = 0; i < index; i++)
    //while (index > 0)
    {
        //--index;
/*        if (index == print_idx) { print_buddies(); printk("\n"); }*/
        free_page(addresses[i]);
/*        if (index == print_idx) { print_buddies(); printk("\n"); }*/
        printk("//////////////////////////////");
        printk("has free %h\n ",i);
        print_buddies();
    } 
    print_buddies();
/*    */
    printk("done\n");
}
