#ifndef __ARCH_ARM_COMMON_INCLUDE_MSR_H__
#define __ARCH_ARM_COMMON_INCLUDE_MSR_H__



/*
 * Generic write to Coprocessor
 */
#define __mcr(value, coproc, op1, op2, n, m)                                \
    __asm__ __volatile__ (                                                  \
        "mcr " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"        \
        "isb;"                                                              \
        :                                                                   \
        : [r] "r" (value)                                                   \
        : "memory"                                                          \
    )

#define __mcr2(value, coproc, op1, op2, n, m)                               \
    __asm__ __volatile__ (                                                  \
        "mcr2 " #coproc ", " #op1 ", %[r], " n ", " m ", " #op2 ";"         \
        "isb;"                                                              \
        :                                                                   \
        : [r] "r" (value)                                                   \
        : "memory"                                                          \
    )

#define __mcrr(value1, value2, coproc, op1, m)                              \
    __asm__ __volatile__ (                                                  \
        "mcrr " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                 \
        "isb;"                                                              \
        :                                                                   \
        : [r1] "r" (value1), [r2] "r" (value2)                              \
        : "memory"                                                          \
    )

#define __mcrr2(value1, value2, coproc, op1, m)                             \
    __asm__ __volatile__ (                                                  \
        "mcrr2 " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                \
        "isb;"                                                              \
        :                                                                   \
        : [r1] "r" (value1), [r2] "r" (value2)                              \
        : "memory"                                                          \
    )



/*
 * Generic read from Coprocessor
 */
#define __mrc(value, coproc, op1, op2, n, m)                                \
    __asm__ __volatile__ (                                                  \
        "mrc " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"        \
        : [r] "=r" (value)                                                  \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrc2(value, coproc, op1, op2, n, m)                               \
    __asm__ __volatile__ (                                                  \
        "mrc2 " #coproc ", " #op1 ", %[r], " #n ", " #m ", " #op2 ";"       \
        : [r] "=r" (value)                                                  \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrrc(value1, value2, coproc, op1, m)                              \
    __asm__ __volatile__ (                                                  \
        "mrrc " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                 \
        : [r1] "=r" (value1), [r2] "=r" (value2)                            \
        :                                                                   \
        : "memory", "cc"                                                    \
    )

#define __mrrc2(value1, value2, coproc, op1, m)                             \
    __asm__ __volatile__ (                                                  \
        "mrrc2 " #coproc ", " #op1 ", %[r1], %[r2], " #m ";"                \
        : [r1] "=r" (value1), [r2] "=r" (value2)                            \
        :                                                                   \
        : "memory", "cc"                                                    \
    )


/*
 * Generic PSR read/write
 */
#define __mrs(value, which)             \
    __asm__ __volatile__ (              \
        "mrs %[r], " #which ";"         \
        : [r] "=r" (value)              \
        :                               \
        : "memory"                      \
    )

#define __msr(value, which)             \
    __asm__ __volatile__ (              \
        "msr " #which ", %[r];"         \
        "isb;"                          \
        :                               \
        : [r] "r" (value)               \
        : "memory", "cc"                \
    )


/*
 * Address translation
 */
struct trans_tab_base_reg {
    union {
        uint32_t value;

        struct {
            uint32_t walk_inner_cacheable    : 1;
            uint32_t walk_inner_shared       : 1;
            uint32_t reserved1               : 1;
            uint32_t walk_outer_attributes   : 2;
            uint32_t reserved2               : 27;
        };

        uint32_t base_addr;
    };
} __attribute__((packed, aligned(4)));

struct trans_tab_base_ctrl_reg {
    union {
        uint32_t value;

        struct {
            uint32_t ttbr0_width     : 3;
            uint32_t reserved        : 28;
            uint32_t ext_addr        : 1;
        };
    };
} __attribute__((packed, aligned(4)));

struct domain_access_ctrl_reg {
    union {
        uint32_t value;

        struct {
            uint32_t domain0     : 2;    // 00: no access
            uint32_t domain1     : 2;    // 01: client, accesses are checked
            uint32_t domain2     : 2;    // 10: reserved, any access generates a domain fault
            uint32_t domain3     : 2;    // 11: manager, accesses are not checked
            uint32_t domain4     : 2;
            uint32_t domain5     : 2;
            uint32_t domain6     : 2;
            uint32_t domain7     : 2;
            uint32_t domain8     : 2;
            uint32_t domain9     : 2;
            uint32_t domain10    : 2;
            uint32_t domain11    : 2;
            uint32_t domain12    : 2;
            uint32_t domain13    : 2;
            uint32_t domain14    : 2;
            uint32_t domain15    : 2;
        };
    };
} __attribute__((packed, aligned(4)));

#define read_trans_tab_base0(value)     __mrc(value, p15, 0, 0, c2, c0)
#define write_trans_tab_base0(value)    __mcr(value, p15, 0, 0, c2, c0)

#define read_trans_tab_base1(value)     __mrc(value, p15, 0, 1, c2, c0)
#define write_trans_tab_base1(value)    __mcr(value, p15, 0, 1, c2, c0)

#define read_trans_tab_base_ctrl(value) __mrc(value, p15, 0, 2, c2, c0)
#define write_trans_tab_base_ctrl(value) __mcr(value, p15, 0, 2, c2, c0)

#define read_domain_access_ctrl(value)  __mrc(value, p15, 0, 0, c3, c0)
#define write_domain_access_ctrl(value) __mcr(value, p15, 0, 0, c3, c0)


/*
 * System control
 */
struct sys_ctrl_reg {
    union {
        uint32_t value;

        struct {
            uint32_t mmu_enabled     : 1;
            uint32_t strict_align    : 1;
            uint32_t dcache_enabled  : 1;
            uint32_t reserved1       : 7;
            uint32_t swap_enabled    : 1;
            uint32_t bpred_enabled   : 1;
            uint32_t icache_enabled  : 1;
            uint32_t high_except_vec : 1;
            uint32_t rr_replacement  : 1;
            uint32_t reserved2       : 10;
            uint32_t reserved3       : 7;
        };
    };
} __attribute__((packed, aligned(4)));

#define read_sys_ctrl(value)    __mrc(value, p15, 0, 0, c1, c0)
#define write_sys_ctrl(value)   __mcr(value, p15, 0, 0, c1, c0)


/*
 * Interrupt Status Register
 */
struct int_status_reg {
    union {
        uint32_t value;

        struct {
            uint32_t reserved1   : 6;
            uint32_t fiq         : 1;
            uint32_t irq         : 1;
            uint32_t abort       : 1;
            uint32_t reserved2   : 23;
        };
    };
} __attribute__((packed, aligned(4)));

#define read_int_status(value)  __mrc(value, p15, 0, 0, c12, c1)
#define write_int_status(value) __mcr(value, p15, 0, 0, c12, c1)


/*
 * Software thread ID
 */
#define read_software_thread_id(value)  __mrc(value, p15, 0, 3, c13, c0)
#define write_software_thread_id(value) __mcr(value, p15, 0, 3, c13, c0)


/*
 * TLB
 */
#define inv_tlb_all()       __mcr(0, p15, 0, 0, c8, c7)
#define inv_tlb_addr(addr)  __mcr(0, p15, 0, 1, c8, c7)
#define inv_tlb_asid(asid)  __mcr(0, p15, 0, 2, c8, c7)


/*
 * Processor status
 */
struct proc_status_reg {
    union {
        uint32_t value;

        struct {
            uint32_t mode            : 5;
            uint32_t thumb           : 1;
            uint32_t fiq_mask        : 1;
            uint32_t irq_mask        : 1;
            uint32_t async_mask      : 1;
            uint32_t big_endian      : 1;
            uint32_t if_then_high    : 6;
            uint32_t greater_equal   : 4;
            uint32_t reserved        : 4;
            uint32_t jazelle         : 1;
            uint32_t if_then_low     : 2;
            uint32_t saturation      : 1;
            uint32_t overflow        : 1;
            uint32_t carry           : 1;
            uint32_t zero            : 1;
            uint32_t negative        : 1;
        };
    };
} __attribute__((packed, aligned(4)));

#define read_current_proc_status(value)     __mrs(value, CPSR)
#define write_current_proc_status(value)    __msr(value, CPSR)

#define read_saved_proc_status(value)       __mrs(value, SPSR)
#define write_saved_proc_status(value)      __msr(value, SPSR)


/*
 * Generic timer
 */
#define read_generic_timer_freq(value)      __mrc(value, p15, 0, 0, c14, c0)

#define read_generic_timer_phys_ctrl(value)     __mrc(value, p15, 0, 1, c14, c2)
#define write_generic_timer_phys_ctrl(value)    __mcr(value, p15, 0, 1, c14, c2)

#define read_generic_timer_phys_interval(value)  __mrc(value, p15, 0, 0, c14, c2)
#define write_generic_timer_phys_interval(value) __mcr(value, p15, 0, 0, c14, c2)

#define read_generic_timer_phys_interval(value)  __mrc(value, p15, 0, 0, c14, c2)
#define write_generic_timer_phys_interval(value) __mcr(value, p15, 0, 0, c14, c2)

#define read_generic_timer_phys_compare(hi, lo)  __mrrc(lo, hi, p15, 2, c14)
#define write_generic_timer_phys_compare(hi, lo) __mcrr(lo, hi, p15, 2, c14)


/*
 * MP CPU ID
 */
struct mp_affinity_reg {
    union {
        uint32_t value;

        struct {
            uint32_t cpu_id      : 2;
            uint32_t reserved1   : 6;
            uint32_t cluster_id  : 4;
            uint32_t reserved2   : 20;
        };

        struct {
            uint32_t aff0        : 8;
            uint32_t aff1        : 8;
            uint32_t aff2        : 8;
            uint32_t smt         : 1;
            uint32_t reserved3   : 5;
            uint32_t uniproc     : 1;
            uint32_t mp_ext      : 1;
        };

        struct {
            uint32_t lo24        : 24;
            uint32_t hi8         : 8;
        };

        struct {
            uint32_t lo12        : 12;
            uint32_t hi20        : 20;
        };
    };
} __attribute__((packed, aligned(4)));

#define read_cpu_id(value)     __mrc(value, p15, 0, 5, c0, c0)


#endif

