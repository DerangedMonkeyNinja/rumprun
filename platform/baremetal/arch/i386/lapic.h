#ifndef _RUMP_LAPIC_H_
#define _RUMP_LAPIC_H_

extern uint32_t *local_apic;

void lapic_enable(void);

void lapic_set_lvt(void);

void lapic_boot_init(uintptr_t lapic_base);

void lapic_dump_registers(void);

#endif /* _RUMP_LAPIC_H_ */
