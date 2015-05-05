#include <bmk-core/string.h>
#include <bmk-core/timetc.h>

#include "kvm.h"
#include "cpufunc.h"
#include "cpuvar.h"
#include "pvclock.h"

/* The hypervisor will write clock values into these structs */
static struct pvclock_vcpu_time_info hv_clock;
static struct pvclock_wall_clock wall_clock;

static uint64_t kvm_get_timecount(struct timecounter *tc);

static struct timecounter kvm_timecounter = {
	kvm_get_timecount,
	~0ull,
	1000000000,
	"kvm-pvclock",
	1000,
	NULL,
	NULL,
};

int bmk_is_kvm_guest(void)
{
	if (bmk_strcmp(hv_vendor, "KVMKVMKVM") == 0)
		return (1);
	else
		return (0);
}

static
int has_kvm_clock(void)
{
	/* Assume we're running under KVM */
	u_int regs[4];

	cpuid(0x40000001, regs);
	if (regs[0] & KVM_FEATURE_CLOCKSOURCE2)
		return (1);
	else
		return (0);
}

static
void init_kvm_pvclock_tc(void)
{
	/* XXX: see if wall clock works */
	wrmsr(KVM_MSR_WALL_CLOCK_NEW, ((uintptr_t)&wall_clock));

	wrmsr(KVM_MSR_SYSTEM_TIME_NEW, ((uintptr_t)&hv_clock | 1));
	bmk_tc_init(&kvm_timecounter);
}

void bmk_kvm_init(void)
{
	if (bmk_is_kvm_guest() && has_kvm_clock())
		init_kvm_pvclock_tc();
}


uint64_t
kvm_get_timecount(struct timecounter *tc)
{
	return bmk_pvclock_get_timecount(&hv_clock);
}

uint64_t bmk_kvm_get_tsc_frequency(void)
{
	return (bmk_pvclock_tsc_freq(&hv_clock));
}
