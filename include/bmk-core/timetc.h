/* $NetBSD: timetc.h,v 1.6 2009/01/11 02:45:56 christos Exp $ */

/*-
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * $FreeBSD: src/sys/sys/timetc.h,v 1.58 2003/08/16 08:23:52 phk Exp $
 */

#ifndef _BMK_TIMETC_H_
#define	_BMK_TIMETC_H_

#include <bmk-core/types.h>

/*-
 * `struct timecounter' is the interface between the hardware which implements
 * a timecounter and the MI code which uses this to keep track of time.
 *
 * A timecounter is a binary counter which has two properties:
 *    * it runs at a fixed, known frequency.
 *    * it has sufficient bits to not roll over in less than approximately
 *      max(2 msec, 2/HZ seconds).  (The value 2 here is really 1 + delta,
 *      for some indeterminate value of delta.)
 */

struct timecounter;
typedef uint64_t timecounter_get_t(struct timecounter *);

struct timecounter {
	timecounter_get_t	*tc_get_timecount;
		/*
		 * This function reads the counter.  It is not required to
		 * mask any unimplemented bits out, as long as they are
		 * constant.
		 */
	uint64_t     		tc_counter_mask;
		/* This mask should mask off any unimplemented bits. */
	uint64_t		tc_frequency;
		/* Frequency of the counter in Hz. */
	const char		*tc_name;
		/* Name of the timecounter. */
	int			tc_quality;
		/*
		 * Used to determine if this timecounter is better than
		 * another timecounter higher means better.  Negative
		 * means "only use at explicit request".
		 */

	void			*tc_priv;
		/* Pointer to the timecounter's private parts. */
	struct timecounter	*tc_next;
		/* Pointer to the next timecounter. */
};

extern struct timecounter *timecounter;

void	   bmk_tc_init(struct timecounter *tc);
bmk_time_t bmk_tc_gettime(void);

#endif /* !_BMK_TIMETC_H_ */
