#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>

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

#endif

