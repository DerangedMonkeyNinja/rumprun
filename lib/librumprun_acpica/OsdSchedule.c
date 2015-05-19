/*	$NetBSD: OsdSchedule.c,v 1.17 2013/12/27 18:53:25 christos Exp $	*/

/*
 * Copyright 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Jason R. Thorpe for Wasabi Systems, Inc.
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
 *	This product includes software developed for the NetBSD Project by
 *	Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * OS Services Layer
 *
 * 6.3: Scheduling services
 */
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <bmk-core/sched.h>
#include <acpica.h>

void *rumpuser_curlwp(void);

#define	_COMPONENT	ACPI_OS_SERVICES
ACPI_MODULE_NAME("SCHEDULE")

/*
 * AcpiOsGetThreadId:
 *
 *	Obtain the ID of the currently executing thread.
 */
ACPI_THREAD_ID
AcpiOsGetThreadId(void)
{
	return (ACPI_THREAD_ID)(uintptr_t)rumpuser_curlwp();
}

/*
 * AcpiOsQueueForExecution:
 *
 *	Schedule a procedure for deferred execution.
 */
ACPI_STATUS
AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function,
    void *Context)
{
	int pri;

	switch (Type) {
	case OSL_GPE_HANDLER:
		pri = 10;
		break;
	case OSL_GLOBAL_LOCK_HANDLER:
	case OSL_EC_POLL_HANDLER:
	case OSL_EC_BURST_HANDLER:
		pri = 5;
		break;
	case OSL_NOTIFY_HANDLER:
		pri = 3;
		break;
	case OSL_DEBUGGER_THREAD:
		pri = 0;
		break;
	default:
		return AE_BAD_PARAMETER;
	}

#if 0
	switch (sysmon_task_queue_sched(pri, Function, Context)) {
	case 0:
		return AE_OK;

	case ENOMEM:
		return AE_NO_MEMORY;

	default:
		return AE_BAD_PARAMETER;
	}
#endif
	(void)pri;
	return AE_BAD_PARAMETER;
}

/*
 * AcpiOsSleep:
 *
 *	Suspend the running task (coarse granularity).
 */
void
AcpiOsSleep(ACPI_INTEGER Milliseconds)
{

	bmk_sched_nanosleep(
	    (Milliseconds < 1000 ? Milliseconds : 1000) * 1000 * 1000);
}

/*
 * AcpiOsStall:
 *
 *	Suspend the running task (fine granularity).
 */
void
AcpiOsStall(UINT32 Microseconds)
{

	bmk_sched_nanosleep(Microseconds * 1000);
}

/*
 * AcpiOsGetTimer:
 *
 *	Get the current system time in 100 nanosecond units
 */
UINT64
AcpiOsGetTimer(void)
{
	bmk_time_t now;
	UINT64 t;

	now = bmk_platform_clock_monotonic();
	t = now / 100;

	return t;
}

/*
 *
 * AcpiOsWaitEventsComplete:
 *
 * 	Wait for all asynchronous events to complete. This implementation
 *	does nothing.
 */
void
AcpiOsWaitEventsComplete(void)
{
	return;
}
