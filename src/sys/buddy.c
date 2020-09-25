#include "printf.h"
#include <stdint.h>

struct page_info {
    uint32_t usable : 1;
    uint32_t inuse  : 1;
    uint32_t is_head: 1;
    uint32_t order  : 4;
    struct page_info *next;
    struct page_info *last;
} __attribute__ ((packed, aligned(4)));

static struct page_info *pages;

static struct page_info *buddies[13];

static void init_page_array()
{
    pages = (void *)0x1000000ul;
    unsigned long array_size = 512 * 1024 * 1024 / 4096 * sizeof(struct page_info);
    unsigned long preserved_pages = 32 * 1024 * 1024 / 4096;
    
    int i = 0;
    for (i = 0; i < preserved_pages; i++) {
        pages[i].usable = 0;
        pages[i].inuse = 1;
    }
    for (; i < 512 * 1024 * 1024 / 4096; i++) {
        pages[i].usable = 1;
        pages[i].inuse = 0;
    }
}

/*static void alloc_preserved()*/
/*{*/
/*    unsigned long array_size = 512 * 1024 * 1024 / 4096 * sizeof(struct page_info);*/
/*    unsigned long preserved_pages = array_size / 4096 + 1 + 16 * 1024 * 1024 / 4096;*/
/*    while(preserved_pages > (1ul << 13))*/
/*    {*/
/*        preserved_pages - (1ul << 13); */
/*    }*/

/*}*/

void set_not_inuse(int order, struct page_info *page)
{
    for (int i = 0; i < (1ul << order); i++)
    {
        page[i].inuse = 0;
    }
}

void set_inuse(int order, struct page_info *page)
{
    for (int i = 0; i < (1ul << order); i++)
    {
        page[i].inuse = 1;
    }
}

void set_order(int order, struct page_info *page)
{
    for (int i = 0; i < (1ul << order); i++)
    {
        page[i].order = order;
    }    
}

unsigned long page_addr_index(struct page_info *page)
{
    return ((unsigned long)page - 0x1000000ul) / sizeof(struct page_info);
}

unsigned long page_addr_to_real_addr(struct page_info *page)
{
    return page_addr_index(page) * 4096;
}

struct page_info* real_addr_to_page_addr(unsigned long real_addr)
{
    return (struct page_info*)(real_addr / 4096 * sizeof(struct page_info) + 0x1000000ul);
}

void insert_page(int order,struct page_info *page)
{
    set_order(order,page);
/*    page->is_head = 1;*/
    struct page_info *current_page;
    current_page = buddies[order];
/*    print_buddy(order);*/
    if (!current_page)
    {
        buddies[order] = page;
    }
    else if (page < current_page)
    {
        buddies[order] = page;
        page->next = current_page;
        current_page->last = page;
    }
    else
    {
        struct page_info *next_page;
        next_page = current_page->next; 
        
        while (next_page && page > next_page)
        {
            current_page = next_page;
            next_page = current_page->next;
        }
        current_page->next = page;
        page->last = current_page;
        page->next = next_page;
        next_page->last = page;
        
    }
/*    print_buddy(order);*/
}

struct page_info* pop_page(int order)
{
    struct page_info *pop_out = buddies[order];
    buddies[order] = pop_out->next;
    buddies[order]->last = 0;
    pop_out->next = 0;
    return pop_out; 
}

/*struct page_info* get_last_page(struct page_info *page)*/
/*{*/
/*    int order = page->order;*/
/*    return (struct page_info*)((unsigned long)page - (1ul<<order) * sizeof(struct page_info));*/
/*    */
/*}*/

/*struct page_info* get_next_page(struct page_info *page)*/
/*{*/
/*    int order = page->order;*/
/*    return (struct page_info*)((unsigned long)page + (1ul<<order) * sizeof(struct page_info));*/
/*    */
/*}*/


struct page_info* _alloc_page(int order)
{
    int i = 0;
    while(!buddies[order + i] && order + i < 13)
    {
        i++;
    }
    if (i+order == 13)
    {
        return 0;
    }
    else
    {
        while(i > 0)
        {
            struct page_info *pop_out = pop_page(order + i);
            insert_page(order + i -1, pop_out);
            insert_page(order + i -1, pop_out + (1ul << (order + i -1)));
            i--;
        }
        struct page_info *page;
        page = pop_page(order);
        set_inuse(order,page);
        page->is_head = 1;
        return page;
    }
}

unsigned long alloc_page(int order)
{
    struct page_info *page_addr = _alloc_page(order);
    if (page_addr == 0)
    {
        return 0;
    }
    else
    {
    return page_addr_to_real_addr(page_addr);
    }
}

void remove_page(struct page_info *page)
{
    int order = page->order;
    if (!page->last)
    {
        //printk("free1: %h, last_page: %h, last order: %h\n", page, last_page, last_page->order);
        buddies[order] = page->next;
        if (page->next) page->next->last = 0;
        page->next = 0;
/*        printk("free1: %h, page_order: %h, page next: %h\n", page, page->order, page->next);*/
/*        printk("after remove:%h\n",buddies[page->order]);*/
        
    }
    else
    {
        //printk("free2: %h, last_page: %h, last order: %h\n", page, last_page, last_page->order);
        page->last->next = page->next;
        if (page->next) page->next->last = page->last;
        
/*        printk("free2: %h, page_order: %h, page next: %h\n", page, page->order, page->next);*/
/*        printk("after remove:%h\n",buddies[page->order]);*/
    }
    page->next = 0;
    page->last = 0;
    page->order = -1;
}



void merge_page(struct page_info *page)
{
    int order;
    int index;

    
/*    cannot merge if it is already the highest order*/
    while(1)
    {
        order = page->order;
        index = page_addr_index(page);
        if (order == 12)
        {
            break;
            
        }
        
/*        if ((index - ((index >> (order + 1)) << (order + 1))))*/
/*            printk("order: %h\n", order);*/
    /*    merge with the next page block*/
        //if ((index - ((index >> order) << order)) == 0 && page->next)
        if ((index - ((index >> (order + 1)) << (order + 1))) == 0 && page->next && page + (1u<<order) == page->next)
        {
/*            printk("merge next\n");*/
/*            printk("next page: %d, inuse: %h\n",page_addr_index(page->next),page->next->inuse);*/
            page->next->is_head = 0;
            remove_page(page->next);
            remove_page(page);
            insert_page(order + 1, page);
        }
    /*    merge with the last page block*/
        else if ((index - ((index >> (order + 1)) << (order + 1))) != 0 && page->last && page->last + (1u<<order) == page)
        {
/*            printk("merge last\n");*/
            page->is_head = 0;
            remove_page(page->last);
            remove_page(page);
            page->last->next = 0;
            page->last = 0;
            insert_page(order + 1, page - (1u<<order));
/*            printk("after merge buddy %d: %d",order + 1, page_addr_index(buddies[order+1]));*/
            page = page - (1u<<order);
            
        }
        else
        {
            break;
        }
    }

}

void _free_page(struct page_info *page)
{
    if (page->is_head == 0)
    {
/*        printk("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");*/
        return;
    }
    else
    {
/*        printk("free order: %h, free page: %d\n", page->order, page_addr_index(page));*/
        set_not_inuse(page->order,page);
        insert_page(page->order,page);
        merge_page(page);
    }
}

void free_page(unsigned long address)
{
    _free_page(real_addr_to_page_addr(address));
}

int cal_order(unsigned int size)
{
    int order;
    for (order = 0; order < 13; order++){
        if (size <= (1 << (12 + order))) return order;
    }
    return order;
}

void init_buddy()
{
    init_page_array();
    int i = 32 * 1024 * 1024 / 4096;
    while (i < 512 * 1024 * 1024 / 4096)
    {
        insert_page(12,pages + i);
        i += 1ul << 12;
    }
}

void print_buddies()
{
    for(int i = 0; i < 13 ; i ++){
        printk("buddy%h\n",i);
        if (!buddies[i]) printk("Empty");
        else
        {
            struct page_info *cur = buddies[i];
            do
            {
                printk("%d   ",page_addr_index(cur));
                cur = cur->next;
            
            }
            while(cur);
        }
        printk("\n*******************\n");
    }
}

void print_buddy(int order)
{
    
    printk("buddy%h\n",order);
    if (!buddies[order]) printk("Empty");
    else
    {
        struct page_info *cur = buddies[order];
        do
        {
            printk("%d   ",page_addr_index(cur));
            cur = cur->next;
        
        }
        while(cur);
    }
    printk("\n");

   
}

/*void test(){*/
/*    init_buddy();*/
/*    unsigned long page;*/
/*    page = _alloc_page(0);*/
/*    printk("allocated page: %h",page);*/
/*    */
/*}*/
