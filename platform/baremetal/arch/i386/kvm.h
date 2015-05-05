#ifndef _BMK_KVM_H_
#define _BMK_KVM_H_

#define KVM_MSR_WALL_CLOCK_OLD  0x11
#define KVM_MSR_SYSTEM_TIME_OLD 0x12
#define KVM_MSR_WALL_CLOCK_NEW  0x4b564d00
#define KVM_MSR_SYSTEM_TIME_NEW 0x4b564d01
#define KVM_MSR_ASYNC_PF        0x4b564d02
#define KVM_MSR_STEAL_TIME      0x4b564d03
#define KVM_MSR_PV_EOI          0x4b564d04

#define KVM_FEATURE_CLOCKSOURCE             (1 <<  0)
#define KVM_FEATURE_NOP_IO_DELAY            (1 <<  1)
#define KVM_FEATURE_MMU_OP                  (1 <<  2)
#define KVM_FEATURE_CLOCKSOURCE2            (1 <<  3)
#define KVM_FEATURE_ASYNC_PF                (1 <<  4)
#define KVM_FEATURE_STEAL_TIME              (1 <<  5)
#define KVM_FEATURE_PV_EOI                  (1 <<  6)
#define KVM_FEATURE_PV_UNHALT               (1 <<  7)
#define KVM_FEATURE_CLOCKSOURCE_STABLE_BIT  (1 << 24)

int      bmk_is_kvm_guest(void);
void     bmk_kvm_init(void);
uint64_t bmk_kvm_get_tsc_frequency(void);

#endif
