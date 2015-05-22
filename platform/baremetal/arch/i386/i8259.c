#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk-core/errno.h>
#include <bmk-core/printf.h>

#include "idt.h"
#include "i8259.h"
#include "pic.h"

#define PIC1_CMD	0x20
#define PIC1_DATA	0x21
#define PIC2_CMD	0xa0
#define PIC2_DATA	0xa1
#define ICW1_IC4	0x01	/* we're going to do the fourth write */
#define ICW1_INIT	0x10
#define ICW4_8086	0x01	/* use 8086 mode */

static int pic1mask = 0xff & ~(1 << 2); /* unmask slave IRQ */
static int pic2mask = 0xff;

static int i8259_intr_init(int);
static void i8259_intr_ack(void);
static void i8259_intr_mask(int);
static void i8259_intr_unmask(int);

static struct pic i8259_pic = {
	.pic_name = "pic0",
	.pic_intr_init = i8259_intr_init,
	.pic_intr_ack = i8259_intr_ack,
	.pic_intr_mask = i8259_intr_mask,
	.pic_intr_unmask = i8259_intr_unmask,
};

void
i8259_init(void)
{
	/*
	 * init pic1: cycle is write to cmd followed by 3 writes to data
	 */
	outb(PIC1_CMD, ICW1_INIT | ICW1_IC4);
	outb(PIC1_DATA, 32);	/* interrupts start from 32 in IDT */
	outb(PIC1_DATA, 1<<2);	/* slave is at IRQ2 */
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC1_DATA, pic1mask);

	/* do the slave PIC */
	outb(PIC2_CMD, ICW1_INIT | ICW1_IC4);
	outb(PIC2_DATA, 32+8);	/* interrupts start from 40 in IDT */
	outb(PIC2_DATA, 2);	/* interrupt at irq 2 */
	outb(PIC2_DATA, ICW4_8086);
	outb(PIC2_DATA, pic2mask);

	/*
	 * map clock interrupt.
	 * note, it's still disabled in the PIC, we only enable it
	 * during nanohlt
	 */
	fillgate(&idt[32], bmk_cpu_isr_clock);

	/* Tell the system about this PIC */
	bmk_pic_register(&i8259_pic);
}

int
i8259_intr_init(int intr)
{

#define FILLGATE(n) case n: fillgate(&idt[32+n], bmk_cpu_isr_##n); break;
	switch (intr) {
		FILLGATE(1);
		FILLGATE(2);
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
	default:
		return BMK_EGENERIC;
	}
#undef FILLGATE

	/* unmask interrupt in PIC */
	i8259_intr_unmask(intr);

	return 0;
}

void
i8259_intr_ack(void)
{

	/*
	 * ACK interrupts on PIC
	 */
	__asm__ __volatile(
	    "movb $0x20, %%al\n"
	    "outb %%al, $0xa0\n"
	    "outb %%al, $0x20\n"
	    ::: "al");
}

void
i8259_intr_mask(int pin)
{
	if (pin >= 0 && pin < 8) {
		pic1mask |= (1 << pin);
		outb(PIC1_DATA, pic1mask);
	} else if (pin <= 8 && pin < 16) {
		pic2mask |= (1 << (pin - 8));
		outb(PIC2_DATA, pic2mask);
	} else {
		bmk_printf("%s: Unsupported pin %d\n", __func__, pin);
	}

	return;
}

void
i8259_intr_unmask(int pin)
{
	if (pin >= 0 && pin < 8) {
		pic1mask &= ~(1 << pin);
		outb(PIC1_DATA, pic1mask);
	} else if (pin >= 8 && pin < 16) {
		pic2mask &= ~(1 << (pin - 8));
		outb(PIC2_DATA, pic2mask);
	} else {
		bmk_printf("%s: Unsupported pin %d\n", __func__, pin);
	}

	return;
}
