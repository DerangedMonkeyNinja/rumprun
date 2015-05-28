#include <bmk-core/errno.h>
#include <bmk-core/platform.h>
#include <bmk-core/printf.h>

#include <rumprun-acpi/acpica.h>

#include "i82093reg.h"
#include "i82489reg.h"
#include "i8259.h"
#include "idt.h"
#include "pic.h"
#include "lapic.h"
#include "ioapic.h"

uintptr_t ioapic_base;

#define ISA_IRQ_MAX 16
#define IOAPIC_IRQ_MAX 24
#define IOAPIC_IRQ_BASE 0x20

int isa_irq_map[ISA_IRQ_MAX];

static int ioapic_intr_init(int);
static void ioapic_intr_ack(void);
static void ioapic_intr_mask(int);
static void ioapic_intr_unmask(int);

static struct pic ioapic_pic = {
	.pic_name = "ioapic",
	.pic_intr_init = ioapic_intr_init,
	.pic_intr_ack = ioapic_intr_ack,
	.pic_intr_mask = ioapic_intr_mask,
	.pic_intr_unmask = ioapic_intr_unmask,
};

static __inline void
i82489_writereg(int reg, uint32_t val)
{
	*((volatile uint32_t *)(((volatile uint8_t *)local_apic) + reg)) = val;
}

static inline uint32_t
ioapic_read(uint32_t offset)
{
	*(uint32_t *)(ioapic_base + IOAPIC_REG) = offset;
	return *(uint32_t *)(ioapic_base + IOAPIC_DATA);
}

static inline void
ioapic_write(uint32_t offset, uint32_t val)
{
	*(uint32_t *)(ioapic_base + IOAPIC_REG) = offset;
	*(uint32_t *)(ioapic_base + IOAPIC_DATA) = val;
}

static inline uint32_t
ioapic_vector(int intr)
{
	return IOAPIC_IRQ_BASE + intr;
}

static inline int
ioapic_intr(int pin)
{
	if (pin < ISA_IRQ_MAX && isa_irq_map[pin] != -1)
		return isa_irq_map[pin];
	else
		return pin;
}

void
ioapic_enable(uintptr_t ioapic_addr)
{
	ioapic_base = ioapic_addr;

	/* Mask all interrupts by default */
	for (size_t i = 0; i < IOAPIC_IRQ_MAX; i++) {
		if (i < ISA_IRQ_MAX)
			isa_irq_map[i] = -1;

		ioapic_write(IOAPIC_REDHI(i), 0x0);
		ioapic_write(IOAPIC_REDLO(i), IOAPIC_REDLO_MASK | ioapic_vector(i));
		fillgate(&idt[ioapic_vector(i)], bmk_cpu_insr);
	}

	/* Clear any pending interrupts */
	ioapic_intr_ack();

	/* disable i8259 PIC */
	i8259_set_mask(0xffff);

	/* Switch to the ioapic */
	bmk_pic_register(&ioapic_pic);
}

int
ioapic_init_override(uint8_t pin, uint8_t gsi, uint32_t flags)
{
	uint32_t redir = 0;

	if (pin < ISA_IRQ_MAX)
		isa_irq_map[pin] = gsi;

	switch(flags & ACPI_MADT_POLARITY_MASK) {
	case ACPI_MADT_POLARITY_ACTIVE_HIGH:
		redir &= ~IOAPIC_REDLO_ACTLO;
		break;
	case ACPI_MADT_POLARITY_ACTIVE_LOW:
		redir |= IOAPIC_REDLO_ACTLO;
		break;
	case ACPI_MADT_POLARITY_CONFORMS:
		if (pin == AcpiGbl_FADT.SciInterrupt)
			redir |= IOAPIC_REDLO_ACTLO;
		else
			redir &= ~IOAPIC_REDLO_ACTLO;
		break;
	default:
		bmk_platform_halt("ioapic_init_overide: fix me polarity\n");
		break;
	}

	redir |= (IOAPIC_REDLO_DEL_FIXED << IOAPIC_REDLO_DEL_SHIFT);

	switch (flags & ACPI_MADT_TRIGGER_MASK) {
	case ACPI_MADT_TRIGGER_LEVEL:
		redir |= IOAPIC_REDLO_LEVEL;
		break;
	case ACPI_MADT_TRIGGER_EDGE:
		redir &= ~IOAPIC_REDLO_LEVEL;
		break;
	case ACPI_MADT_TRIGGER_CONFORMS:
		if (pin == AcpiGbl_FADT.SciInterrupt)
			redir |= IOAPIC_REDLO_LEVEL;
		else
			redir &= ~IOAPIC_REDLO_LEVEL;
		break;
	default:
		bmk_platform_halt("ioapic_init_override: fix me trigger\n");
	}

	/*
	 * Leave the ACPI interrupt unmasked.   Actual hardware will
	 * make a specific request to enable its interrupt.
	 */
	if (gsi != AcpiGbl_FADT.SciInterrupt)
		redir |= IOAPIC_REDLO_MASK;

	ioapic_intr_init(gsi);

	ioapic_write(IOAPIC_REDHI(gsi), 0x00000000);
	ioapic_write(IOAPIC_REDLO(gsi), redir | ioapic_vector(gsi));

	return (0);
}

static int
ioapic_intr_init(int intr)
{
	if (intr < ISA_IRQ_MAX && isa_irq_map[intr] == -1)
		isa_irq_map[intr] = intr;

#define FILLGATE(n) case n: fillgate(&idt[ioapic_vector(intr)], bmk_cpu_isr_##n); break;
	switch (intr) {
		FILLGATE(0);
		FILLGATE(1);
	case 2: fillgate(&idt[ioapic_vector(intr)], bmk_cpu_isr_clock); break;
		FILLGATE(3);
		FILLGATE(4);
		FILLGATE(5);
		FILLGATE(6);
		FILLGATE(7);
		FILLGATE(8);
		FILLGATE(9);
		FILLGATE(10);
		FILLGATE(11);
		FILLGATE(12);
		FILLGATE(13);
		FILLGATE(14);
		FILLGATE(15);
		FILLGATE(16);
		FILLGATE(17);
		FILLGATE(18);
		FILLGATE(19);
		FILLGATE(20);
		FILLGATE(21);
		FILLGATE(22);
		FILLGATE(23);
		break;
	default:
		return BMK_EGENERIC;
	}
#undef FILLGATE

	ioapic_intr_unmask(intr);

	return (0);
}

static void
ioapic_intr_ack(void)
{
	i82489_writereg(LAPIC_EOI, 0);
}

static void
ioapic_intr_mask(int pin)
{
	uint32_t redlo;
	int intr = ioapic_intr(pin);

	redlo = ioapic_read(IOAPIC_REDLO(intr));
	redlo |= IOAPIC_REDLO_MASK;
	ioapic_write(IOAPIC_REDLO(intr), redlo);
}

static void
ioapic_intr_unmask(int pin)
{
	uint32_t redlo;
	int intr = ioapic_intr(pin);

	redlo = ioapic_read(IOAPIC_REDLO(intr));
	redlo &= ~IOAPIC_REDLO_MASK;
	ioapic_write(IOAPIC_REDLO(intr), redlo);
}

void
ioapic_dump_registers(void)
{
	for (size_t i = 0; i < 24; i++) {
		uint32_t low = ioapic_read(IOAPIC_REDLO(i));
		uint32_t high = ioapic_read(IOAPIC_REDHI(i));

		bmk_printf("interrupt %d: 0x%08x%08x\n", i, high, low);
	}
}
