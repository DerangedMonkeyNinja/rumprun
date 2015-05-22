#ifndef _RUMP_IDT_H_
#define _RUMP_IDT_H_

/* interrupt-not-service-routine */
void bmk_cpu_insr(void);

/* actual interrupt service routines */
void bmk_cpu_isr_clock(void);
void bmk_cpu_isr_9(void);
void bmk_cpu_isr_10(void);
void bmk_cpu_isr_11(void);
void bmk_cpu_isr_14(void);
void bmk_cpu_isr_15(void);

struct gate_descriptor {
	unsigned gd_looffset:16;
	unsigned gd_selector:16;
	unsigned gd_stkcpy:5;
	unsigned gd_xx:3;
	unsigned gd_type:5;
	unsigned gd_dpl:2;
	unsigned gd_p:1;
	unsigned gd_hioffset:16;
} __attribute__((__packed__));

extern struct gate_descriptor idt[];

void
fillgate(struct gate_descriptor *gd, void *fun);

#endif /* _RUMP_IDT_H_ */