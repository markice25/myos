#include "vm.h"
#include <stdint.h>
#include "buddy.h"
#include "printf.h"
#include "msr.h"

extern void map_page(struct l1table *l1, unsigned long vaddr, unsigned long paddr, int read_only, int user_access, int no_exec);
extern void construct_page_table(struct l1table *table);

struct context {
    union {
        unsigned long gpr[16];
        
        struct {
            unsigned long r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
            union { unsigned long r13; unsigned long sp; };
            union { unsigned long r14; unsigned long lr; };
            union { unsigned long r15; unsigned long pc; };
        };
    };
    
    unsigned long state_reg;
};

struct process {
    int allocated;
    int pid;
    
    struct l1table *page_table;
    struct context regs;
    struct {
        unsigned long entry;
        unsigned long heap_start, heap_end;
        unsigned long stack_top, stack_limit;
    } mem;
};

#define EI_NIDENT 16
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Word;

typedef struct {
        unsigned char   e_ident[EI_NIDENT];
        Elf32_Half      e_type;
        Elf32_Half      e_machine;
        Elf32_Word      e_version;
        Elf32_Addr      e_entry;
        Elf32_Off       e_phoff;
        Elf32_Off       e_shoff;
        Elf32_Word      e_flags;
        Elf32_Half      e_ehsize;
        Elf32_Half      e_phentsize;
        Elf32_Half      e_phnum;
        Elf32_Half      e_shentsize;
        Elf32_Half      e_shnum;
        Elf32_Half      e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr;


#define MAX_NUM_PROCESSES 1024

static struct process proc_table[MAX_NUM_PROCESSES];

static struct process *alloc_process()
{
    //find a empty table in proc_table
    struct process *proc = (void *)0;
    for (int pid = 0; pid < MAX_NUM_PROCESSES; pid++)
    {
        if (!proc_table[pid].allocated) {
            proc = &proc_table[pid];
            break;
        }
    }
    
    if (!proc) {
        return proc;
    }
    
    //initialize the parameter in proc entry
    proc->page_table = (struct l1table *)alloc_page(2);
    construct_page_table(proc->page_table);
    proc->allocated = 1;
    proc->mem.entry = 0;
    //(proc->regs).regs = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    for (int i = 0; i < 16; i++){
        (proc->regs).gpr[i] = 0;
    }
    proc->regs.state_reg = 0;
    proc->mem.heap_start = proc->mem.heap_end = 0x80000000;
    proc->mem.stack_top = proc->mem.stack_limit = 0xc0000000;
    
    // Set CPSR
    proc->regs.state_reg = 0x10;
    
    return proc;
}

static void load_elf(struct process *proc, void *elf)
{
    Elf32_Ehdr *e_head = (Elf32_Ehdr *)elf;
    Elf32_Phdr *p_head;
    uint8_t *src;
    uint8_t *dst;
    for (int i = 0; i < e_head->e_phnum; i++){
        p_head = (Elf32_Phdr *)((void *)e_head + e_head->e_phoff + e_head->e_phentsize * i);
        int order = cal_order(p_head->p_memsz);
        unsigned long paddr = alloc_page(order);
        for (int p = 0; p < (1 << order); p++){
            map_page(proc->page_table, p_head->p_vaddr + p * 4096, paddr + p * 4096,1,1,0);
        }
        src = (uint8_t *)((void *)e_head + p_head->p_offset);
        dst = (uint8_t *) paddr;
        int left = p_head->p_filesz;
        
        while (left > 0){
            *dst = * src;
            dst++;
            src++;
            left--;
        } 
    }
    
    proc->regs.pc = e_head->e_entry;
    
    printk("elf @ %h\n", elf);
    printk("Entry @ %h\n", e_head->e_entry);
    //while (1);
}

struct process *create_process(void *elf)
{
    struct process *proc = alloc_process();
    load_elf(proc, elf);
    return proc;
}

void switch_to(struct process *proc)
{
    // switch address space
    write_trans_tab_base0((unsigned long)proc->page_table);
    write_trans_tab_base1((unsigned long)proc->page_table);
    
    // restore registers
    __asm__ __volatile__ (
        /* Switch to SVC mode */
        "cpsid aif, #0x13;" //switch to SVC and disable interrupts

        /* Load SPSR */
        "mov sp, %[ctxt];" //move general reg
        "ldr r0, [sp, #64];" //load mem to reg
        "msr SPSR, r0;" //move general reg to control reg

        /* Restore user/system SP (R13) and LR (R14) */
/*        "add sp, sp, #4;"*/
        "add r0, sp, #52;"
        "ldmfd r0, {sp, lr}^;" // load 2 words at the address of r0 to sp and lr

        /* Restore user/system R0-R12 */
        "ldmfd sp, {r0-r12}^;" 
/*        "nop;"*/

        /* Restore PC and trigger SPSR->CPSR */
        "add sp, sp, #60;"
/*        "b .;"*/
        "ldmfd sp, {pc}^;"

        /* Should never reach here */
        "b .;"
/*        "nop;"*/
        :
        : [ctxt] "r" (&proc->regs) // src register "r" dst
        :
    );
}

extern uint8_t program_data[];

void init_proc()
{
    struct process *proc = create_process(program_data);
    switch_to(proc);
}

