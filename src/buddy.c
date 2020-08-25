#include "printf.h"
#include <stdint.h>

struct page_info {
    uint32_t usable : 1;
    uint32_t inuse  : 1;
    uint32_t is_head: 1;
    uint32_t order  : 4;
    struct page_info *next;
} __attribute__ ((packed, aligned(4)));

static struct page_info *pages;

static struct page_info *buddies[13];

static void init_page_array()
{
    pages = (void *)0x1000000ul;
    unsigned long array_size = 512 * 1024 * 1024 / 4096 * sizeof(struct page_info);
    unsigned long preserved_pages = array_size / 4096 + 1 + 16 * 1024 * 1024 / 4096;
    
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

unsigned int page_addr_index(struct page_info *page)
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
    page->is_head = 1;
    struct page_info *current_page;
    current_page = buddies[order];
    if (!current_page)
    {
        buddies[order] = page;
    }
    else if (page < current_page)
    {
        buddies[order] = page;
        page->next = current_page;
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
        page->next = next_page;
    }
}

struct page_info* pop_page(int order)
{
    struct page_info *pop_out = buddies[order];
    buddies[order] = pop_out->next;
    pop_out->next = 0;
    return pop_out; 
}

struct page_info* get_last_page(struct page_info *page)
{
    int order = page->order;
    return (struct page_info*)((unsigned long)page - (1ul<<order) * sizeof(struct page_info));
    
}

void init_buddy()
{
    init_page_array();
    int i = 0;
    while (i < 512 * 1024 * 1024 / 4096)
    {
        insert_page(12,pages + i);
        i += 1ul << 12;
    }
}


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
            insert_page(order + i -1, (struct page_info*)((unsigned long)pop_out + (1ul << (order + i -1)) * sizeof(struct page_info)));
            i--;
        }
        struct page_info *page;
        page = pop_page(order);
        set_inuse(order,page);
        return page;
    }
}

void remove_page(struct page_info *page)
{
    int order = page->order;
    struct page_info *last_page;
    last_page = get_last_page(page);
    if ((unsigned long) last_page < 0x1000000ul || last_page->order != order)
    {
        buddies[order] = page->next;
        page->next = 0;
        
    }
    else
    {
        last_page->next = page->next;
        page->next = 0;
    }
}



void merge_page(struct page_info *page)
{
    int order;
    int index;
    struct page_info *last_page;
    
/*    cannot merge if it is already the highest order*/
    while(1)
    {
        order = page->order;
        index = page_addr_index(page);
        last_page = get_last_page(page);
        if (order == 12)
        {
            printk("case1");
            break;
            
        }
    /*    merge with the next page block*/
        printk("index cal: %h\n",(index - ((index << order) >> order)));
        printk("order: %h\n",order);
        if ((index - ((index << order) >> order)) == 0 && page->next)
        {
            printk("case2");
            remove_page(page->next);
            remove_page(page);
            printk("done1");
            insert_page(order + 1, page);
            printk("*****");
            printk("%h",buddies[order]);
            printk("%h",buddies[order+1]);
        }
    /*    merge with the last page block*/
        else if (last_page->order == order && last_page->inuse == 0 && (unsigned long)last_page >= 0x1000000ul)
        {
            printk("case3");
            remove_page(last_page);
            remove_page(page);
            insert_page(order + 1, last_page);
            page = last_page;
        }
        else
        {
            printk("case4");
            break;
        }
    }

}

void _free_page(struct page_info *page)
{
    if (page->is_head == 0)
    {
        return;
    }
    else
    {
        insert_page(page->order,page);
        merge_page(page);
    }
}



void test(){
    init_buddy();



    struct page_info *page;
    page = _alloc_page(0);
    for (int i = 0; i < 13; i++)
    {
        int j = 0;
        struct page_info *cur = buddies[i];
        while(cur)
        {
            cur = cur->next;
            j++;
        }
        printk("buddy %d has %d buddies\n",i,j);
        printk("order: %h\n",buddies[i]->order);
        
    }
    printk("allocated page: %h",page);
    
    _free_page(page);
    printk("*************");
    for (int i = 0; i < 13; i++)
    {
        int j = 0;
        struct page_info *cur = buddies[i];
        while(cur)
        {
            cur = cur->next;
            j++;
        }
        printk("buddy %d has %d buddies\n",i,j);
        printk("%h\n",buddies[i]);
        printk("%h\n",buddies[i]->next);
        
    }

}
