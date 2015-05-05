#include <bmk/types.h>

static inline uint64_t
rdmsr(u_int msr)
{
	uint32_t low, high;

	__asm volatile("rdmsr"
	    : "=a" (low), "=d" (high)
	    : "c" (msr));
	return (low | ((uint64_t)high << 32));
}

static inline void
wrmsr(uint32_t msr, uint64_t value)
{
	__asm volatile ("wrmsr"
	    :
	    : "c" (msr), "A" (value));
}

static inline void
cpuid(u_int ax, u_int *p)
{
	__asm volatile("cpuid"
	    : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
	    : "0" (ax));
}
