/*-
 * Copyright (c) 2014 Antti Kantee.  All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <bmk/types.h>
#include <bmk/kernel.h>

#include <bmk-core/core.h>
#include <bmk-core/sched.h>

#include "idt.h"
#include "i8259.h"
#include "pic.h"

/* enter the kernel with interrupts disabled */
int bmk_spldepth = 1;

/*
 * i386 MD descriptors, assimilated from NetBSD sys/arch/i386/include/segments.h
 */

struct region_descriptor {
	unsigned short rd_limit:16;
	unsigned int rd_base:32;
} __attribute__((__packed__));

struct gate_descriptor idt[256];

/* clock routines */
void bmk_init_tsc_tc(void);

void bmk_cpu_identify(void);

void
fillgate(struct gate_descriptor *gd, void *fun)
{

	gd->gd_hioffset = (unsigned long)fun >> 16;
	gd->gd_looffset = (unsigned long)fun;

	/* i was born lucky */
	gd->gd_selector = 0x8;
	gd->gd_stkcpy = 0;
	gd->gd_xx = 0;
	gd->gd_type = 14;
	gd->gd_dpl = 0;
	gd->gd_p = 1;
}

struct segment_descriptor {
        unsigned sd_lolimit:16;
        unsigned sd_lobase:24;
        unsigned sd_type:5;
        unsigned sd_dpl:2;
        unsigned sd_p:1;
        unsigned sd_hilimit:4;
        unsigned sd_xx:2;
        unsigned sd_def32:1;
        unsigned sd_gran:1;
        unsigned sd_hibase:8;
} __attribute__((__packed__));

#define SDT_MEMRWA	19	/* memory read write accessed */
#define SDT_MEMERA	27	/* memory execute read accessed */

#define SEGMENT_CODE	1
#define SEGMENT_DATA	2
#define SEGMENT_GS	3

#define SEG_BYTEGRAN	0
#define SEG_PAGEGRAN	1

static struct segment_descriptor gdt[4];

static void
fillsegment(struct segment_descriptor *sd, int type)
{

	sd->sd_lobase = 0;
	sd->sd_hibase = 0;

	sd->sd_lolimit = 0xffff;
	sd->sd_hilimit = 0xf;

	sd->sd_type = type;

	/* i was born luckier */
	sd->sd_dpl = 0;
	sd->sd_p = 1;
	sd->sd_xx = 0;
	sd->sd_def32 = 1;
	sd->sd_gran = SEG_PAGEGRAN;
}

static void
adjustgs(uintptr_t p)
{
	struct segment_descriptor *sd = &gdt[SEGMENT_GS];

	sd->sd_lobase = p & 0xffffff;
	sd->sd_hibase = (p >> 24) & 0xff;

	__asm__ __volatile__("mov %0, %%gs" :: "r"(8*SEGMENT_GS));
}


/*
 * This routine fills out the interrupt descriptors so that
 * we can handle interrupts without involving a jump to hyperspace.
 */
void
bmk_cpu_init(void)
{
	struct region_descriptor region;
	int i;

	fillsegment(&gdt[SEGMENT_CODE], SDT_MEMERA);
	fillsegment(&gdt[SEGMENT_DATA], SDT_MEMRWA);
	fillsegment(&gdt[SEGMENT_GS], SDT_MEMRWA);

	region.rd_limit = sizeof(gdt)-1;
	region.rd_base = (unsigned int)(uintptr_t)(void *)gdt;
	bmk_cpu_lgdt(&region);

	region.rd_limit = sizeof(idt)-1;
	region.rd_base = (unsigned int)(uintptr_t)(void *)idt;
	bmk_cpu_lidt(&region);

	/*
	 * Apparently things work fine if we don't handle anything,
	 * so let's not.  (p.s. don't ship this code)
	 */
	for (i = 0; i < 32; i++) {
		fillgate(&idt[i], bmk_cpu_insr);
	}

	i8259_init();

	/*
	 * map clock interrupt.
	 * note, it's still disabled in the PIC, we only enable it
	 * during nanohlt
	 */
	fillgate(&idt[32], bmk_cpu_isr_clock);

	/*
	 * Figure out some information about our platform so we can pick
	 * a reasonable clock to use...
	 */
	bmk_cpu_identify();
}

int
bmk_cpu_intr_init(int intr)
{
	return bmk_pic_intr_init(intr);
}

void
bmk_cpu_intr_ack(void)
{
	bmk_pic_intr_ack();
}

uint64_t
bmk_cpu_counter(void)
{
    uint64_t val;
    unsigned long eax, edx;

    __asm__ __volatile__(
        "lfence\n"
        "rdtsc" : "=a"(eax), "=d"(edx));
    val = ((uint64_t)edx<<32)|(eax);

    return val;
}

void
bmk_cpu_nanohlt(void)
{

	/*
	 * Enable clock interrupt and wait for the next whichever interrupt
	 */
	bmk_pic_intr_unmask(0);
	hlt();
	bmk_pic_intr_mask(0);
}

void
bmk_platform_cpu_sched_settls(struct bmk_tcb *next)
{

	adjustgs(next->btcb_tp);
}
