#include "vm.h"
#include <stdint>
#include "buddy.h"
struct context {
    unsigned long regs[16];
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

extern uint8_t *program;

/*static uint8_t *program[] = alloc_page();*/

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
    proc->page_table = alloc_page(2);
    proc->allocated = 1;
    proc->mem.entry = 0;
    proc->regs.regs = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    proc->regs.state_reg = 0;
    proc->mem.heap_start = proc->mem.heap_end = 0x80000000;
    proc->mem.stack_start = proc->mem.stack_end = 0xc0000000;
    
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
        for (int p = 0; p < 1 << order; i++)
            map_page(proc->page_table, p_head->p_vaddr + p * 4096, paddr + p * 4096);
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
    
}

struct process *create_process(void *elf)
{
    struct process *proc = alloc_process();
    load_elf(proc, elf);
    return proc;
}

