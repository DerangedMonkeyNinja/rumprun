#ifndef _RUMP_PIC_H_
#define _RUMP_PIC_H_

/*
 * Description for generic Programmable Interrupt Controller
 */
struct pic {
	char *pic_name;
	int  (*pic_intr_init)(int);
	int  (*pic_intr_deinit)(int);
	void (*pic_intr_ack)(void);
	void (*pic_intr_mask)(int);
	void (*pic_intr_unmask)(int);
};

void bmk_pic_register(struct pic *);
int  bmk_pic_intr_init(int);
int  bmk_pic_intr_deinit(int);
void bmk_pic_intr_ack(void);
void bmk_pic_intr_mask(int);
void bmk_pic_intr_unmask(int);

#endif
