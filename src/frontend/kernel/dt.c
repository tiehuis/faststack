#include "core.h"
#include "dt.h"

// see _dt.s
extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

static void init_gdt(void);
static void gdt_set_gate(int, uint32_t, uint32_t, uint8_t, uint8_t);
static void init_idt(void);
static void idt_set_gate(uint8_t, uint32_t, uint16_t, uint8_t);

struct gdt_entry gdt_entries[3];
struct gdt_ptr gdt_ptr;
struct idt_entry idt_entries[256];
struct idt_ptr idt_ptr;

void init_dt(void)
{
    init_gdt();
    init_idt();
}

static void init_gdt(void)
{
    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t) &gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                    // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);     // Data segment

    gdt_flush((uint32_t) &gdt_ptr);
}

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_mid = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (gran & 0xF0) | ((limit >> 16) & 0x0F);
    gdt_entries[num].access = access;
}

static void init_idt(void)
{
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    kmemset(&idt_entries, 0, sizeof(idt_entries));

#define X(n) idt_set_gate(n, (uint32_t) isr##n, 0x08, 0x8E)
    X( 0); X( 1); X( 2); X( 3); X( 4); X( 5); X( 6); X( 7);
    X( 8); X( 9); X(10); X(11); X(12); X(13); X(14); X(15);
    X(16); X(17); X(18); X(19); X(20); X(21); X(22); X(23);
    X(24); X(25); X(26); X(27); X(28); X(29); X(30); X(31);
#undef X

    idt_flush((uint32_t) &idt_ptr);
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t select, uint8_t flags)
{
    idt_entries[num].base_low = (base & 0xFFFF);
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].select = select;
    idt_entries[num].always0 = 0;

    // Provides interrupt gate privilage to 3 in user-mode (not needed)
    idt_entries[num].flags = flags /* | 0x60 */ ;
}
