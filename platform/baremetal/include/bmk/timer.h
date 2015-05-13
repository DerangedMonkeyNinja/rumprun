#ifndef _BMK_TIMER_H_
#define _BMK_TIMER_H_

#include <bmk/types.h>

void bmk_timer_delay(unsigned int);

int bmk_timer_alarm(bmk_time_t wakeup);

#endif /* _BMK_TIMER_H_ */
