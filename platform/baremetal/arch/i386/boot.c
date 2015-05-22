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
#include <bmk/multiboot.h>

#include <bmk-core/core.h>
#include <bmk-core/sched.h>
#ifdef RUMP_ACPI
#include <rumprun-acpi/acpi.h>
#endif

#include "kvm.h"
#include "tsc.h"

unsigned long   bmk_cpu_frequency = 0;  /* for export via kernel.c */
unsigned long	bmk_cpu_count = 0;

void
bmk_cpu_boot(struct multiboot_info *mbi)
{
	bmk_cpu_init();
	bmk_sched_init();
	bmk_multiboot(mbi);

#ifdef RUMP_ACPI
	bmk_acpi_init();
#else
	bmk_cpu_count = 1;
#endif

	if (bmk_is_kvm_guest()) {
		bmk_kvm_init();
		bmk_cpu_frequency = bmk_kvm_get_tsc_frequency();
	} else {
		bmk_tsc_init();
		bmk_cpu_frequency = bmk_tsc_get_tsc_frequency();
	}

	spl0();

	bmk_run(bmk_multiboot_cmdline);

	bmk_sched_yield();
}
