#ifndef __BUDDY_H__
#define __BUDDY_H__

extern void init_buddy();
extern int cal_order(unsigned int size);
extern void free_page(unsigned long address);
extern unsigned long alloc_page(int order);
extern void print_buddies();
unsigned long page_addr_index(struct page_info *page);
#endif
