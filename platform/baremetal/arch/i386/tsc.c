/*-
 * Copyright (c) 1998-2003 Poul-Henning Kamp
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <bmk/kernel.h>
#include <bmk/types.h>

#include <bmk-core/printf.h>
#include <bmk-core/string.h>
#include <bmk-core/timetc.h>

#include "clock.h"
#include "pvclock.h"
#include "cpufunc.h"
#include "cpuvar.h"
#include "specialreg.h"
#include "tsc.h"

static uint64_t tsc_freq = 0;
static int      tsc_is_invariant = 0;

static uint64_t tsc_get_timecount(struct timecounter *tc);

static struct timecounter tsc_timecounter = {
	tsc_get_timecount,
	~0ull,
	0,
	"TSC",
	0,
};

static uint64_t
tsc_freq_vmware(void)
{
	u_int regs[4];
	uint64_t freq = 0;
	if (hv_high >= 0x40000010) {
		cpuid(0x40000010, regs);
		freq = regs[0] * 1000;
	} else {
		bmk_platform_halt("Fix me!  I need a vmware_hvcall function!");
	}

	return (freq);
}

static uint64_t
tsc_freq_intel(void)
{
	char brand[48];
	u_int regs[4];
	uint64_t freq;
	char *p;
	u_int i;

	/*
	 * Intel Processor Identification and the CPUID Instruction
	 * Application Note 485.
	 *  http://www.intel.com/assets/pdf/appnote/241618.pdf
	 */

	if (cpu_exthigh >= 0x80000004) {
		p = brand;
		for (i = 0x80000002; i < 0x80000005; i++) {
			cpuid(i, regs);
			bmk_memcpy(p, regs, sizeof(regs));
			p += sizeof(regs);
		}
		p = NULL;
		for (i = 0; i < sizeof(brand) - 1; i++)
			if (brand[i] == 'H' && brand[i + 1] == 'z')
				p = brand + i;
		if (p != NULL) {
			p -= 5;
			switch (p[4]) {
			case 'M':
				i = 1;
				break;
			case 'G':
				i = 1000;
				break;
			case 'T':
				i = 1000000;
				break;
			default:
				return (0);
			}
#define C2D(c)((c) - '0')
			if (p[1] == '.') {
				freq = C2D(p[0]) * 1000;
				freq += C2D(p[2]) * 100;
				freq += C2D(p[3]) * 10;
				freq *= i * 1000;
			} else {
				freq = C2D(p[0]) * 1000;
				freq += C2D(p[1]) * 100;
				freq += C2D(p[2]) * 10;
				freq += C2D(p[3]);
				freq *= i * 1000000;
			}
#undef C2D
		}
	}

	return (freq);
}

static void
init_tsc_tc(void)
{
	/* Update timecounter with the info we've collected */
	tsc_timecounter.tc_frequency = tsc_freq;
	tsc_timecounter.tc_quality = tsc_is_invariant ? 800 : 100;

	bmk_tc_init(&tsc_timecounter);
}

void
bmk_tsc_init(void)
{
	/*
	 * We should have some basic CPU/platform information at this
	 * point.  Let's use it to try to guestimate our TSC frequency
	 */

	if (hv_high > 0 && (bmk_strcmp(hv_vendor, "VmwareVmware") == 0)) {
		tsc_freq = tsc_freq_vmware();
		tsc_is_invariant = 1;
	} else {
		if (bmk_strcmp(cpu_vendor, "GenuineIntel") == 0) {
			if ((amd_pminfo & CPUID_APM_TSC) != 0
			    || ((CPUID_TO_FAMILY(cpu_id) == 0x6
				    && CPUID_TO_MODEL(cpu_id) >= 0xe)
				|| (CPUID_TO_FAMILY(cpu_id) == 0xf
				    && CPUID_TO_MODEL(cpu_id) >= 0x3))) {
				tsc_is_invariant = 1;
				tsc_freq_intel();
			}
		} else if (bmk_strcmp(cpu_vendor, "AuthenticAMD") == 0) {
			if ((amd_pminfo & CPUID_APM_TSC) != 0
			    || (CPUID_TO_FAMILY(cpu_id) >= 0x10)) {
				tsc_is_invariant = 1;
			}
		}
	}

	if (!tsc_freq) {
		/* Bummer. Try to measure it */
		//bmk_printf("Calibrating TSC value...");
		uint64_t last_tsc = bmk_cpu_counter();
		bmk_clock_delay(100000);
		tsc_freq = (bmk_cpu_counter() - last_tsc) * 10;
		//bmk_printf("TSC clock: %llu Hz\n", tsc_freq);
	}

	init_tsc_tc();
}

uint64_t
tsc_get_timecount(struct timecounter *tc)
{
	(void)tc;
	uint64_t val;
	unsigned long eax, edx;

	__asm__ __volatile__(
	    "lfence\n"
	    "rdtsc" : "=a"(eax), "=d"(edx));
	val = (eax | ((uint64_t)edx << 32));

	return val;
}

uint64_t
bmk_tsc_get_tsc_frequency(void)
{
	return (tsc_freq);
}
