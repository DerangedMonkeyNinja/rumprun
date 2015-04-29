#include <bmk-core/platform.h>

#include <bmk-rumpuser/core_types.h>
#include <bmk-rumpuser/rumpuser.h>

uint64_t
rumpuser_cpu_counter(void)
{
    return bmk_platform_cpu_counter();
}

uint64_t
rumpuser_cpu_frequency(void)
{
    return bmk_platform_cpu_frequency();
}
