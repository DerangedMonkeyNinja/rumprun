#include <bmk-core/core.h>
#include <bmk-core/null.h>
#include <bmk-core/printf.h>

#include "pic.h"

static struct pic *bmk_pic = NULL;

void bmk_pic_register(struct pic *p)
{
	bmk_pic = p;
}

int bmk_pic_intr_init(int intr)
{
	bmk_assert(bmk_pic);
	return bmk_pic->pic_intr_init(intr);
}

int bmk_pic_intr_deinit(int intr)
{
	bmk_assert(bmk_pic);
	if (bmk_pic->pic_intr_deinit)
		return bmk_pic->pic_intr_deinit(intr);
	else
		return 0;
}

void bmk_pic_intr_ack(void)
{
	bmk_assert(bmk_pic);
	bmk_pic->pic_intr_ack();
}

void bmk_pic_intr_mask(int pin)
{
	bmk_assert(bmk_pic);
	bmk_pic->pic_intr_mask(pin);
}

void bmk_pic_intr_unmask(int pin)
{
	bmk_assert(bmk_pic);
	bmk_pic->pic_intr_unmask(pin);
}
