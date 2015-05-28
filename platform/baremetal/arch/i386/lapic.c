/*-
 * Copyright (c) 2000, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by RedBack Networks Inc.
 *
 * Author: Bill Sommerfeld
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk-core/platform.h>
#include <bmk-core/printf.h>

#include "cpufunc.h"
#include "i82489reg.h"
#include "idt.h"
#include "pic.h"
#include "lapic.h"

#define LAPIC_SPURIOUS_VECTOR			0xef
#define LAPIC_IPI_VECTOR			0xe0
#define LAPIC_TLB_VECTOR			0xe1

static int  lapic_intr_init(int);
static void lapic_intr_ack(void);
static void lapic_intr_mask(int);
static void lapic_intr_unmask(int);

uint32_t *local_apic;  /* local apic base address */

struct pic local_pic = {
	.pic_name = "lapic",
	.pic_intr_init = lapic_intr_init,
	.pic_intr_ack = lapic_intr_ack,
	.pic_intr_mask = lapic_intr_mask,
	.pic_intr_unmask = lapic_intr_unmask,
};

static uint32_t
i82489_readreg(int reg)
{
	return *((volatile uint32_t *)(((volatile uint8_t *)local_apic) + reg));
}

static void
i82489_writereg(int reg, uint32_t val)
{
	*((volatile uint32_t *)(((volatile uint8_t *)local_apic) + reg)) = val;
}

#define lapic_cpu_number() 	(i82489_readreg(LAPIC_ID) >> LAPIC_ID_SHIFT)
#define lapic_cpu_is_bsp()	(rdmsr(LAPIC_MSR) & LAPIC_MSR_BSP)

void
lapic_enable(void)
{
	i82489_writereg(LAPIC_SVR, LAPIC_SVR_ENABLE | LAPIC_SPURIOUS_VECTOR);
}

void
lapic_set_lvt(void)
{
	uint32_t lint0, lint1;

	/* XXX: only works for bsp */
	lint0 = LAPIC_DLMODE_EXTINT;
	if (!lapic_cpu_is_bsp())
		lint0 |= LAPIC_LVT_MASKED;
	i82489_writereg(LAPIC_LVINT0, lint0);

	/*
	 * Deliver NMIs to the primary CPU
	 */
	lint1 = LAPIC_DLMODE_NMI;
	if (!lapic_cpu_is_bsp())
		lint1 |= LAPIC_LVT_MASKED;
	i82489_writereg(LAPIC_LVINT1, lint1);

	i82489_writereg(LAPIC_TPRI, 0);  /* enable all interrupts */
}

void
lapic_boot_init(uintptr_t lapic_base)
{
	local_apic = (uint32_t *)lapic_base;
	fillgate(&idt[LAPIC_SPURIOUS_VECTOR], bmk_cpu_insr);
}


static int
lapic_intr_init(int intr)
{
	/* XXX: not yet */
	return 0;
}

static void
lapic_intr_ack(void)
{
	i82489_writereg(LAPIC_EOI, 0);
}

static void
lapic_intr_mask(int pin)
{
	int reg;
	uint32_t val;

	reg = LAPIC_LVTT + (pin << 4);
	val = i82489_readreg(reg);
	val |= LAPIC_LVT_MASKED;
	i82489_writereg(reg, val);
}

static void
lapic_intr_unmask(int pin)
{
	int reg;
	uint32_t val;

	reg = LAPIC_LVTT + (pin << 4);
	val = i82489_readreg(reg);
	val &= ~LAPIC_LVT_MASKED;
	i82489_writereg(reg, val);
}

void
lapic_dump_registers(void)
{
	bmk_printf(" ID: 0x%08x\n", i82489_readreg(LAPIC_ID));
	bmk_printf("VER: 0x%08x\n", i82489_readreg(LAPIC_VERS));
	bmk_printf("ISR: 0x%08x\n", i82489_readreg(LAPIC_ISR));
}
