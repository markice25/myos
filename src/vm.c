#include <stdint.h>
#include "printf.h"
#include "msr.h"
#include "vm.h"

//static uint8_t pool[40 * 1024];

static struct l1table *kernel_l1table = 0;
//static struct l2table *kernel_l2table;

static void alloc_page_table()
{
/*    unsigned long addr = (void *)pool;*/
/*    if (addr & 0x3FFFul) {*/
/*        addr += 0x4000ul;*/
/*        addr &= ~0x3FFFul;*/
/*    }*/
/*    */
/*    kernel_l1table = (void *)addr;*/
/*    kernel_l2table = (void *)(addr + 0x4000ul);*/
    
    kernel_l1table = alloc_page(2);
}

/*            uint32_t     present         : 1;*/
/*            uint32_t     reserved1       : 2;*/
/*            uint32_t     non_secure      : 1;*/
/*            uint32_t     imp             : 1;*/
/*            uint32_t     domain          : 4;*/
/*            uint32_t     reserved3       : 1;*/
/*            uint32_t     pfn_ext         : 2;*/
/*            uint32_t     pfn             : 20;*/

/*            l1entry->block.user_access = 0;   // Kernel page*/
/*            l1entry->block.user_write = 1;    // Kernel page write is determined by read_only*/
/*            l1entry->block.cacheable = cache;*/
/*            l1entry->block.cache_inner = l1entry->block.cache_outer = 0x1;*/

void map_page(struct l1table *l1, unsigned long vaddr, unsigned long paddr)
{
    int vpn1 = vaddr >> 20;
    int vpn2 = (vaddr >> 12) & 0xFFul;
    
    struct l1page_table_entry *entry= &l1->entries[vpn1];
    struct l2table *l2 = 0;
    if (!entry->pte.present)
    {
        l2 = alloc_page(0);
        entry->value = 0;
        entry->pte.present = 1;
        entry->pte.pfn = (unsigned long)l2 >> 12;
    }
    else
    {
        l2 = entry->pte.pfn << 12; 
    }
    
    struct l2page_table_entry *l2entry = &l2->entries[vpn2];
    l2entry->value = 0;
    l2entry->present = 1;
    l2entry->pfn = paddr >> 12;
    l2entry->no_exec = 1;
    l2entry->read_only = 0;
    l2entry->user_access = 0;
    l2entry->user_write = 1;
    l2entry->cacheable = 0;
    l2entry->cache_inner = l2entry->cache_outer = 0x1;
} 

static void construct_page_table()
{
    for (int i = 0; i < 512; i++) {
        struct l1page_table_entry *entry = &kernel_l1table->entries[i];
        entry->value = 0;
        entry->block.user_access = 0;
        entry->block.present = 1;
        entry->block.user_write = 1;
        entry->block.cache_inner = entry->block.cache_outer = 0x1;
        entry->block.bfn = i;
    }
    
    for (int i = 512; i < 4096; i++) {
        kernel_l1table->entries[i].value = 0;
    }
    
/*    kernel_l2table = alloc_page(0);*/
/*    */
/*    struct l1page_table_entry *l1entry = &kernel_l1table->entries[512];*/
/*    l1entry->value = 0;*/
/*    l1entry->pte.present = 1;*/
/*    l1entry->pte.pfn = (unsigned long)kernel_l2table >> 12;*/
/*    */
/*    struct l2page_table_entry *l2entry = &kernel_l2table->entries[0];*/
/*    l2entry->value = 0;*/
/*    l2entry->present = 1;*/
/*    l2entry->pfn = 0x3F201;*/
/*    l2entry->no_exec = 1;*/
/*    l2entry->read_only = 0;*/
/*    l2entry->user_access = 0;*/
/*    l2entry->user_write = 1;*/
/*    l2entry->cacheable = 0;*/
/*    l2entry->cache_inner = l2entry->cache_outer = 0x1;*/
    
    map_page(kernel_l1table, 0x20000000ul, 0x3F201000ul);
}



static void start_mmu()
{
    // Copy the page table base
    write_trans_tab_base0((unsigned long)kernel_l1table);
    write_trans_tab_base1((unsigned long)kernel_l1table);
    
    // Enable permission check for domain0
    struct domain_access_ctrl_reg domain;
    read_domain_access_ctrl(domain.value);
    domain.domain0 = 0x1;
    write_domain_access_ctrl(domain.value);
    
    // Set page table ctrl reg
    struct trans_tab_base_ctrl_reg ttbcr;
    ttbcr.value = 0;
    write_trans_tab_base_ctrl(ttbcr.value);
    
    // Enable the MMU
    struct sys_ctrl_reg sys_ctrl;
    read_sys_ctrl(sys_ctrl.value);
    sys_ctrl.mmu_enabled = 1;
    write_sys_ctrl(sys_ctrl.value);
}

extern unsigned long pl011_base;

void start_paging()
{
    printk("l1 @ %h\n", &kernel_l1table);
    //printk("l2 @ %h\n", &kernel_l2table);
    alloc_page_table();
    construct_page_table();
    start_mmu();
    pl011_base = 0x20000000ul;
    printk("OK!\n");
}

