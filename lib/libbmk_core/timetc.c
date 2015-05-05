/* $NetBSD: kern_tc.c,v 1.46 2013/09/14 20:52:43 martin Exp $ */

/*-
 * Copyright (c) 2008, 2009 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
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

/*-
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ---------------------------------------------------------------------------
 */

#include <bmk-core/core.h>
#include <bmk-core/null.h>
#include <bmk-core/string.h>
#include <bmk-core/memalloc.h>
#include <bmk-core/platform.h>
#include <bmk-core/printf.h>

#include <bmk-core/timetc.h>

/*
 * Implement a dummy timecounter which we can use until we get a real one
 * in the air.  This allows the console and other early stuff to use
 * time services.
 */

static uint64_t
dummy_get_timecount(struct timecounter *tc)
{
	static unsigned int now;
	(void)tc;
	return (++now);
}

static struct timecounter dummy_timecounter = {
	dummy_get_timecount, ~0u, 1000000, "dummy", -1000000, NULL, NULL,
};

struct timecounter *timecounter = &dummy_timecounter;
static struct timecounter *timecounters = &dummy_timecounter;

static unsigned int timecounter_mods;
static uint64_t timecounter_scale = 0;

#define NS_PER_SECOND 1000000000ULL
static uint64_t
get_scale_factor(struct timecounter *tc)
{
	return ((NS_PER_SECOND << 32) / tc->tc_frequency);
}

/*
 * Initialize a new timecounter and possibly use it.
 */
void
bmk_tc_init(struct timecounter *tc)
{
	tc->tc_next = timecounters;
	timecounters = tc;
	timecounter_mods++;

	if (tc->tc_quality > 0) {
		bmk_printf("bmk timecounter: Timecounter \"%s\" frequency %"PRI_BMK_TIME" Hz "
		    "quality %d\n", tc->tc_name, tc->tc_frequency,
		    tc->tc_quality);
	}

	/*
	 * Never automatically use a timecounter with negative quality.
	 * Even though we run on the dummy counter, switching here may be
	 * worse since this timecounter may not be monotonous.
	 */
	if (tc->tc_quality >= 0 && (tc->tc_quality > timecounter->tc_quality ||
	    (tc->tc_quality == timecounter->tc_quality &&
	    tc->tc_frequency > timecounter->tc_frequency))) {
		(void)tc->tc_get_timecount(tc);
		(void)tc->tc_get_timecount(tc);
		timecounter = tc;
		timecounter_scale = get_scale_factor(tc);
	}
}

bmk_time_t
bmk_tc_gettime(void)
{
	uint64_t ticks = (timecounter->tc_get_timecount(timecounter)
	    & timecounter->tc_counter_mask);

	uint64_t ns = (ticks / timecounter->tc_frequency) * NS_PER_SECOND;
	ns += ((ticks % timecounter->tc_frequency) * timecounter_scale) >> 32;

	return ((bmk_time_t)(ns & BMK_TIME_T_MAX));
}
