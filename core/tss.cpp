/**
 * @file        tss.cpp
 * @brief       Task State Segment (TSS) Implementation
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <core/tss.h>

TSS g_tss;

extern "C" uint32_t get_eip();
// asm tss functions
extern "C" void load_tss();

static void set_tss_entry(int index, uint16_t ss0, uint32_t esp0) {
    memset(&g_tss, 0, sizeof(TSS));  // clear first

    g_tss.ss0 = ss0;
    g_tss.esp0 = esp0;

    g_tss.cs = 0x0b;                                              // ring 3 code selector
    g_tss.ss = g_tss.ds = g_tss.es = g_tss.fs = g_tss.gs = 0x13;  // ring 3 data selector
    g_tss.iomap_base = sizeof(TSS);

    uint32_t base = (uint32_t)&g_tss;
    uint32_t limit = sizeof(TSS) - 1;

    gdt_set_entry(index, base, limit, 0xE9, 0x00);  // access: 0xE9, flags: 0x00
}

void tss_init() {
    set_tss_entry(5, 0x10, 0x0100000);
    load_tss();
}

void tss_set_stack(uint32_t esp0) {
    g_tss.esp0 = esp0;
}

void tss_print() {
    printf("previous: 0x%x\n", g_tss.previous);
    printf("esp0: 0x%x, ss0: 0x%x\n", g_tss.esp0, g_tss.ss0);
    printf("esp1: 0x%x, ss1: 0x%x\n", g_tss.esp1, g_tss.ss1);
    printf("esp2: 0x%x, ss2: 0x%x\n", g_tss.esp2, g_tss.ss2);
    printf("cr3: 0x%x, eip: 0x%x, eflags: 0x%x\n", g_tss.cr3, g_tss.eip, g_tss.eflags);
    printf("eax: 0x%x, ecx: 0x%x, edx: 0x%x, ebx: 0x%x\n", g_tss.eax, g_tss.ecx, g_tss.edx,
           g_tss.ebx);
    printf("esp: 0x%x, ebp: 0x%x, esi: 0x%x, edi: 0x%x\n", g_tss.esp, g_tss.ebp, g_tss.esi,
           g_tss.edi);
    printf("es:0x%x, cs:0x%x, ss:0x%x, ds:0x%x, fs:0x%x, gs:0x%x\n", g_tss.es, g_tss.cs, g_tss.ss,
           g_tss.ds, g_tss.fs, g_tss.gs);
    printf("ldt: 0x%x, trap: 0x%x, iomap_base: 0x%x\n", g_tss.iomap_base, g_tss.trap,
           g_tss.iomap_base);
}
