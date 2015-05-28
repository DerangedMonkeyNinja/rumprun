#ifndef _RUMP_IOAPIC_H_
#define _RUMP_IOAPIC_H_

void ioapic_enable(uintptr_t);

int ioapic_init_override(uint8_t pin, uint8_t gsi, uint32_t flags);

void ioapic_dump_registers(void);

#endif /* _RUMP_IOAPIC_H_ */
