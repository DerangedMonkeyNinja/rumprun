#include <bmk/types.h>

#include <bmk-core/platform.h>
#include <bmk-core/printf.h>
#include <bmk-core/string.h>

#include "cpufunc.h"
#include "specialreg.h"

u_int cpu_high = 0;
u_int cpu_id = 0;
u_int cpu_exthigh = 0;
u_int cpu_feature = 0;
u_int cpu_feature2 = 0;
char  cpu_vendor[16];

u_int amd_pminfo = 0;

u_int hv_high = 0;
char  hv_vendor[16];

u_int vm_guest = 0;

void bmk_cpu_identify(void);

static void
hypervisor_identify(void)
{
	u_int regs[4];
	bmk_memset(&hv_vendor[0], 0, sizeof(hv_vendor));

	/* See if we're virtualized */
	if (cpu_feature2 & CPUID2_RAZ)
		vm_guest = 1;  /* definitely virtualized */

	/*
	 * Check hypervisor leaf for platform.
	 * KVM sets EAX to 0 on this call, so check for that as well.
	 */
	cpuid(0x40000000, regs);
	if ((regs[0] >= 0x40000000)
	    || ((regs[0] == 0) && regs[1] != 0)) {
		hv_high = regs[0];
		bmk_memcpy(&hv_vendor[0], &regs[1], sizeof(u_int));
		bmk_memcpy(&hv_vendor[4], &regs[2], sizeof(u_int));
		bmk_memcpy(&hv_vendor[8], &regs[3], sizeof(u_int));
		hv_vendor[12] = '\0';
		vm_guest = 1;
	} else {
		hv_vendor[0] = '\0';
	}

	return;
}

void bmk_cpu_identify(void)
{
	u_int regs[4];

	cpuid(0x0, regs);
	cpu_high = regs[0];
	bmk_memcpy(&cpu_vendor[0], &regs[1], sizeof(u_int));
	bmk_memcpy(&cpu_vendor[4], &regs[3], sizeof(u_int));
	bmk_memcpy(&cpu_vendor[8], &regs[2], sizeof(u_int));
	cpu_vendor[12] = '\0';

	cpuid(0x1, regs);
	cpu_id = regs[0];
	cpu_feature = regs[2];
	cpu_feature2 = regs[3];

	cpuid(0x80000000, regs);
	if (regs[0] >= 80000000)
		cpu_exthigh = regs[0];

	if (cpu_exthigh >= 0x80000007) {
		cpuid(0x80000007, regs);
		amd_pminfo = regs[3];
	}

	hypervisor_identify();
}
