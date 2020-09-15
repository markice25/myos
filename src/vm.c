#include <stdint.h>
#include "printf.h"
#include "msr.h"

#define PAGE_LEVELS                 2

#define L1PAGE_TABLE_SIZE           16384
#define L1PAGE_TABLE_ENTRY_COUNT    4096
#define L1PAGE_TABLE_ENTRY_BITS     12
#define L1PAGE_TABLE_SIZE_IN_PAGES  4

#define L2PAGE_TABLE_SIZE           4096
#define L2PAGE_TABLE_ENTRY_COUNT    256
#define L2PAGE_TABLE_ENTRY_BITS     8
#define L2PAGE_TABLE_SIZE_IN_PAGES  1

#define L1BLOCK_SIZE                0x100000ul
#define L1BLOCK_PAGE_COUNT          256

#define GET_L1PTE_INDEX(addr)       ((addr) >> 20)
#define GET_L2PTE_INDEX(addr)       (((addr) >> 12) & 0xfful)
#define GET_PAGE_OFFSET(addr)       ((addr) & 0xffful)


struct l1page_table_entry {
    union {
        uint32_t         value;

        struct {
            uint32_t     present         : 1;
            uint32_t     reserved1       : 2;
            uint32_t     non_secure      : 1;
            uint32_t     imp             : 1;
            uint32_t     domain          : 4;
            uint32_t     reserved3       : 1;
            uint32_t     pfn_ext         : 2;
            uint32_t     pfn             : 20;
        } pte;

        struct {
            uint32_t     reserved1       : 1;
            uint32_t     present         : 1;
            uint32_t     cache_inner     : 2;    // c, b
            uint32_t     no_exec         : 1;
            uint32_t     domain          : 4;
            uint32_t     imp             : 1;
            uint32_t     user_write      : 1;    // AP[0]
            uint32_t     user_access     : 1;    // AP[1]
            uint32_t     cache_outer     : 2;    // TEX[1:0]
            uint32_t     cacheable       : 1;    // TEX[2]
            uint32_t     read_only       : 1;    // AP[2]
            uint32_t     shareable       : 1;
            uint32_t     non_global      : 1;
            uint32_t     reserved3       : 1;
            uint32_t     non_secure      : 1;
            uint32_t     bfn             : 12;
        } block;
    };
} __attribute__((packed, aligned(4)));

struct l2page_table_entry {
    union {
        uint32_t         value;

        struct {
            uint32_t     no_exec         : 1;
            uint32_t     present         : 1;
            uint32_t     cache_inner     : 2;    // c, b
            uint32_t     user_write      : 1;    // AP[0]
            uint32_t     user_access     : 1;    // AP[1]
            uint32_t     cache_outer     : 2;    // TEX[1:0]
            uint32_t     cacheable       : 1;    // TEX[2]
            uint32_t     read_only       : 1;    // AP[2]    for both user and kernel
            uint32_t     shareable       : 1;
            uint32_t     non_global      : 1;
            uint32_t     pfn             : 20;
        };
    };
} __attribute__((packed, aligned(4)));

struct page_frame {
    union {
        uint8_t value_uint8_t[4096];
        uint32_t value_uint32_t[1024];
    };
} __attribute__((packed, aligned(4)));

struct l1table {
    union {
        uint8_t value_uint8_t[16384];
        uint32_t value_uint32_t[4096];
        struct page_frame pages[4];

        struct l1page_table_entry entries[L1PAGE_TABLE_ENTRY_COUNT];
    };
} __attribute__((packed, aligned(4)));

struct l2table {
    union {
        uint8_t value_uint8_t[4096];
        uint32_t value_uint32_t[1024];
        struct page_frame page;

        struct {
            struct l2page_table_entry entries[256];
            struct l2page_table_entry entries_dup[3][256];
        };
    };
} __attribute__((packed, aligned(4)));


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

