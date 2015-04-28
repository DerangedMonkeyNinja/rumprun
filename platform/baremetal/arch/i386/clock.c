/*-
 * Copyright (c) 1990 The Regents of the University of California.
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
 * 3. Neither the name of the University nor the names of its contributors
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
 *	@(#)clock.c	7.2 (Berkeley) 5/12/91
 */
/*-
 * Copyright (c) 1993, 1994 Charles M. Hannum.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)clock.c	7.2 (Berkeley) 5/12/91
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 * Primitive clock interrupt routines.
 */

#include <bmk/types.h>
#include <bmk/kernel.h>
#include <bmk-core/printf.h>

#include "isareg.h"
#include "i8253reg.h"

#include "clock.h"

void bmk_cpu_enable_intr(void);
void bmk_cpu_disable_intr(void);

#define UINT_MAX 0xffffffffU

static const u_long hz   = 100;  /* 10ms timer */

static u_long rtclock_tval;  /* i8254 reload value for countdown */
static int rtclock_init = 0;

static volatile uint32_t i8254_lastcount;
static volatile uint32_t i8254_offset;
static volatile int i8254_ticked;

static void
initrtclock(u_long freq)
{
    u_long tval;

    /*
     * Compute timer_count, the count-down count the timer will be
     * set to.  Also, correctly round
     * this by carrying an extra bit through the division.
     */
    tval = (freq * 2) / (u_long) hz;
    tval = (tval / 2) + (tval & 0x01);

    /* initialize 8254 clock */
    outb(IO_TIMER1+TIMER_MODE, TIMER_SEL0|TIMER_RATEGEN|TIMER_16BIT);

    /* Correct rounding will buy us a better precision in timekeeping */
    outb(IO_TIMER1+TIMER_CNTR0, tval % 256);
    outb(IO_TIMER1+TIMER_CNTR0, tval / 256);

    rtclock_tval = tval ? tval : 0xFFFF;
    rtclock_init = 1;
}

void
bmk_clock_startrtclock(void)
{
    if (!rtclock_init)
        initrtclock(TIMER_FREQ);
}

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

/*
 * Wait approximately `n' microseconds.
 * Relies on timer 1 counting down from (TIMER_FREQ / hz) at
 * TIMER_FREQ Hz.
 * Note: timer had better have been programmed before this is first
 * used!
 * (Note that we use `rate generator' mode, which counts at 1:1;
 * `square
 * wave' mode counts at 2:1).
 * Don't rely on this being particularly accurate.
 */
void
bmk_clock_delay(unsigned int n)
{
    unsigned int cur_tick, initial_tick;
    int remaining;

    /* allow DELAY() to be used before startrtclock() */
    if (!rtclock_init)
        initrtclock(TIMER_FREQ);

    /*
     * Read the counter first, so that the rest of the setup overhead
     * is counted.
     */
    initial_tick = gettick();

    if (n <= UINT_MAX / TIMER_FREQ) {
        /*
         * For unsigned arithmetic, division can be replaced with
         * multiplication with the inverse and a shift.
         */
        remaining = n * TIMER_FREQ / 1000000;
    } else {
        /* This is a very long delay.
         * Being slow here doesn't matter.
         */
        remaining = (unsigned long long) n * TIMER_FREQ / 1000000;
    }

    while (remaining > 1) {
        cur_tick = gettick();
        if (cur_tick > initial_tick)
            remaining -= rtclock_tval - (cur_tick - initial_tick);
        else
            remaining -= initial_tick - cur_tick;
        initial_tick = cur_tick;
    }
}
