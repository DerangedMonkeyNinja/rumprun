/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * Copyright (c) 2010 Alexander Motin <mav@FreeBSD.org>
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz and Don Ahn.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)clock.c	7.2 (Berkeley) 5/12/91
 */

/*
 * Routines for driving the i8254 Programmable Interrupt Timer (PIT)
 */

#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk/timer.h>

#include <bmk-core/sched.h>
#include <bmk-core/printf.h>

#include "isareg.h"
#include "i8253reg.h"

void bmk_cpu_enable_intr(void);
void bmk_cpu_disable_intr(void);

#define UINT_MAX 0xffffffffU

#define MODE_STOP 0
#define MODE_PERIODIC 1
#define MODE_ONESHOT 2

static int timer0_period = -2;
static int timer0_mode = 0xffff;
static int timer0_last = 0xffff;

static unsigned int gettick(void)
{
    uint16_t rdval;

    bmk_cpu_disable_intr();

    /* Select counter 0 and latch it */
    outb(IO_TIMER1+TIMER_MODE, TIMER_SEL0 | TIMER_LATCH);
    rdval = inb(IO_TIMER1+TIMER_CNTR0);
    rdval |= (inb(IO_TIMER1+TIMER_CNTR0) << 8);

    bmk_cpu_enable_intr();

    return rdval;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

static void
set_i8254_freq(int mode, uint64_t period)
{
	int new_count = 0,
	    new_mode = 0;

	/*
	 * Avoid a slow division by 1000000000 here.
	 * 80073 = 1193182 * 2^26 / 10^9
	 */
	new_count = min(
	    (period * 80073 + (1 << 26) - 1) >> 26,
		0x10000);
	timer0_period = (mode == MODE_PERIODIC) ? new_count : -1;

	switch (mode) {
	case MODE_STOP:
		new_mode = TIMER_SEL0 | TIMER_INTTC | TIMER_16BIT;
		outb(IO_TIMER1+TIMER_MODE, new_mode);
		outb(IO_TIMER1+TIMER_CNTR0, 0);
		outb(IO_TIMER1+TIMER_CNTR0, 0);
		break;
	case MODE_PERIODIC:
		new_mode = TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT;
		outb(IO_TIMER1+TIMER_MODE, new_mode);
		outb(IO_TIMER1+TIMER_CNTR0, new_count & 0xff);
		outb(IO_TIMER1+TIMER_CNTR0, new_count >> 8);
		break;
	case MODE_ONESHOT:
		new_mode = TIMER_SEL0 | TIMER_INTTC | TIMER_16BIT;
		if (new_mode != timer0_mode)
			outb(IO_TIMER1+TIMER_MODE, new_mode);
		outb(IO_TIMER1+TIMER_CNTR0, new_count & 0xff);
		outb(IO_TIMER1+TIMER_CNTR0, new_count >> 8);
		break;
	default:
		bmk_platform_halt("set_i8254_freq: unknown operational mode");
	}

	timer0_mode = new_mode;
	timer0_last = new_count;
}

/*
 * Wait approximately `n' microseconds.
 * Don't rely on this being particularly accurate.
 */
void
bmk_timer_delay(unsigned int n)
{
    unsigned int cur_tick, initial_tick;
    int remaining;

    /*
     * Start the clock ticking.  The actual period doesn't particularly matter.
     */
    set_i8254_freq(MODE_PERIODIC, 1000000); /* 1000000 ns = 1 ms */

    /*
     * Read the counter first, so that the rest of the setup overhead
     * is counted.
     */
    initial_tick = gettick();

    if (n <= UINT_MAX / TIMER_FREQ) {
	    /*
	     * Avoid a slow division by 1000000 here.
	     * 39099 = 1193182 * 2^15 / 10^6
	     */
	    remaining = (n * 39099 + (1 << 15) - 1) >> 15;
    } else {
	    /* This is a very long delay.
	     * Being slow here doesn't matter.
	     */
	    remaining = ((unsigned long long) n * TIMER_FREQ + 999999) / 1000000;
    }

    while (remaining > 1) {
	    cur_tick = gettick();
	    if (cur_tick > initial_tick)
		    remaining -= timer0_period - (cur_tick - initial_tick);
	    else
		    remaining -= initial_tick - cur_tick;
	    initial_tick = cur_tick;
    }

    /*
     * No longer needed, so stop it
     */
    set_i8254_freq(MODE_STOP, 0);
}

int
bmk_timer_alarm(bmk_time_t wakeup)
{
	/* wakeup is specified in ns */
	bmk_time_t now = bmk_platform_clock_monotonic();

	/*
	 * It takes some time to set the timer, so make sure
	 * we actually have some time to set things up before
	 * the alarm goes off.
	 */
	if (now + 20000 >= wakeup)
		return (-1);  /* Eek! */

	uint32_t period = (wakeup - now);

	set_i8254_freq(MODE_ONESHOT, period);

	return (0);
}
