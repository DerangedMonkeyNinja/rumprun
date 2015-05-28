#ifndef _RUMP_IDT_H_
#define _RUMP_IDT_H_

/* interrupt-not-service-routine */
void bmk_cpu_insr(void);

/* actual interrupt service routines */
void bmk_cpu_isr_clock(void);
void bmk_cpu_isr_0(void);
void bmk_cpu_isr_1(void);
void bmk_cpu_isr_2(void);
void bmk_cpu_isr_3(void);
void bmk_cpu_isr_4(void);
void bmk_cpu_isr_5(void);
void bmk_cpu_isr_6(void);
void bmk_cpu_isr_7(void);
void bmk_cpu_isr_8(void);
void bmk_cpu_isr_9(void);
void bmk_cpu_isr_10(void);
void bmk_cpu_isr_11(void);
void bmk_cpu_isr_12(void);
void bmk_cpu_isr_13(void);
void bmk_cpu_isr_14(void);
void bmk_cpu_isr_15(void);
void bmk_cpu_isr_16(void);
void bmk_cpu_isr_17(void);
void bmk_cpu_isr_18(void);
void bmk_cpu_isr_19(void);
void bmk_cpu_isr_20(void);
void bmk_cpu_isr_21(void);
void bmk_cpu_isr_22(void);
void bmk_cpu_isr_23(void);

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
