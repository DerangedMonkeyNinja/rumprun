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
#include <bmk/sched.h>

#include <bmk-core/core.h>
#include <bmk-core/sched.h>
#include <bmk-core/string.h>
#include <bmk-core/memalloc.h>

#define LIBRUMPUSER
#include <rump/rumpuser.h>

#include "rumpuser_int.h"

#include <bmk-rumpuser/rumpuser.h>

int
rumprun_platform_rumpuser_init(void)
{

	return 0;
}

/* reserve 1MB for bmk */
#define BMK_MEMRESERVE (1024*1024)

static int
ultostr(unsigned long n, char *buf, size_t buflen)
{
    char tmp[21];
    char *res = buf;
    unsigned i, j;
    int rv = 0;

    for (i = 0; n > 0; i++) {
			bmk_assert(i < sizeof(tmp)-1);
			tmp[i] = (n % 10) + '0';
			n = n / 10;
		}
		if (i >= buflen) {
			rv = 1;
		} else {
			res[i] = '\0';
			for (j = i; i > 0; i--) {
				res[j-i] = tmp[i-1];
			}
		}

    return (rv);
}

int
rumpuser_getparam(const char *name, void *buf, size_t buflen)
{
	int rv = 0;

	if (buflen <= 1)
		return 1;

	if (bmk_strcmp(name, RUMPUSER_PARAM_NCPU) == 0
	    || bmk_strcmp(name, "RUMP_VERBOSE") == 0) {
		bmk_strncpy(buf, "1", buflen-1);

  } else if (bmk_strcmp(name, RUMPUSER_PARAM_CPU_FREQUENCY) == 0) {
    ultostr(bmk_cpu_frequency, buf, buflen);

	} else if (bmk_strcmp(name, RUMPUSER_PARAM_HOSTNAME) == 0) {
		bmk_strncpy(buf, "rump-baremetal", buflen-1);

	/* for memlimit, we have to implement int -> string ... */
	} else if (bmk_strcmp(name, "RUMP_MEMLIMIT") == 0) {
    bmk_assert(bmk_memsize > BMK_MEMRESERVE);
		unsigned long memsize = bmk_memsize - BMK_MEMRESERVE;

    rv = ultostr(memsize, buf, buflen);

	} else {
		rv = 1;
	}

	return rv;
}

/* ok, this isn't really random, but let's make-believe */
int
rumpuser_getrandom(void *buf, size_t buflen, int flags, size_t *retp)
{
	static unsigned seed = 12345;
	unsigned *v = buf;

	*retp = buflen;
	while (buflen >= 4) {
		buflen -= 4;
		*v++ = seed;
		seed = (seed * 1103515245 + 12345) % (0x80000000L);
	}
	return 0;
}
