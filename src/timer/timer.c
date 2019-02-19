// Std Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Local Includes
#include <timer.h>
#include <list.h>
#include <logger.h>
#include <thread.h>

// Private Constant Definitions
#define SEC_TO_USEC(_sec)	(_sec * 1000000)
#define HOUR_TO_SEC(_hr)	(_hr * 3600)
#define TIMER_SENSITIVITY	100000ul	// usec

// Private Macro Definitions

// Private Data Types

struct timer {
	struct list list;
	unsigned int secs;
	unsigned int usecs;
	unsigned long ticks_left;
	void *param;
	timer_handler handler;
	const char *func;
	int flags;
};

// Global Variable Declarations
static struct list timer_list;
thread_hnd_t timer_thread;

// Private Function Prototypes


// Functions

// called under lock!!
static void __add_timer(struct timer *timer)
{
	struct timer *tmp, *prev;
	/* Maintain timers in order of increasing time */
	list_for_each_safe(tmp, prev, &timer_list, struct timer, list) {
		if (timer->ticks_left < tmp->ticks_left) {
			list_add(tmp->list.prev, &timer->list);
			return;
		}
    }
	list_add_tail(&timer_list, &timer->list);
}

static int __timer_set_time(struct timer *timer, unsigned int secs, unsigned int usecs)
{
	unsigned long total_usecs = SEC_TO_USEC(secs) + usecs;

	// check overflow
	if (total_usecs < SEC_TO_USEC(secs)) {
		FATAL("OVERFLOW! %u secs %u usecs\n", secs, usecs);
		return -1;
	}

	timer->ticks_left = (total_usecs) / TIMER_SENSITIVITY;

	return 0;
}

int _TimerAddTimer(timer_handler handler, const char *func, int flags, unsigned int secs, unsigned int usecs, void *param)
{
	struct timer *timer;

	if (!timer_thread.mut) {
		DEBUG("Not inited!\n");
		return 0;
	}

	// check if lock is acquired
	if (MUTEX_TRYLOCK(timer_thread.mut) != 0) {
		FATAL("registering timer in another timer not supported! func=%s (%p) secs=%u usecs=%u\n", func, handler, secs, usecs);
		return -1;
	}
	MUTEX_UNLOCK(timer_thread.mut);	// unlock and continue

	timer = (struct timer *)calloc(1, sizeof(*timer));
	if (timer == NULL) {
		FATAL("WTF?? handler=%p %u secs %u usecs\n", handler, secs, usecs);
		return -1;
	}

	LOG(LEVEL_DEBUG, "registering timer: %s (%p) %u secs %u usecs\n", func, handler, secs, usecs);

	if (__timer_set_time(timer, secs, usecs) < 0) {
		free(timer);
		return -1;
	}

	LOG(LEVEL_TRACE, "timer: %s (%p) ticks left: %lu\n", func, handler, timer->ticks_left);

	timer->param = param;
	timer->handler = handler;
	timer->secs = secs;
	timer->usecs = usecs;
	timer->flags = flags;
	timer->func = func;

	if (TimerIsActive(handler)) {
		LOG(LEVEL_WARNING, "%s (%p) already registered! Removing old entry.\n", func, handler);
		TimerCancelTimer(handler);
	}

	MUTEX_LOCK(timer_thread.mut);
	__add_timer(timer);
	MUTEX_UNLOCK(timer_thread.mut);

	return 0;
}

// called under lock!
static void __remove_timer(struct timer *timer)
{
	list_del(&timer->list);
	memset(timer, 0, sizeof(*timer));
	free(timer);
}

int TimerCancelTimer(timer_handler handler)
{
	struct timer *timer, *prev;
	int removed = 0;

	LOG(LEVEL_TRACE, "handler=%p\n", handler);

	if (!timer_thread.mut) {
		DEBUG("Not inited!\n");
		return 0;
	}

	MUTEX_LOCK(timer_thread.mut);
	if (list_empty(&timer_list)) {
		MUTEX_UNLOCK(timer_thread.mut);
		return 0;
	}

	list_for_each_safe(timer, prev, &timer_list, struct timer, list) {
		if (timer->handler == handler) {
			LOG(LEVEL_DEBUG, "func=%s (%p)\n", timer->func, handler);
			__remove_timer(timer);
			removed++;
		}
	}
	MUTEX_UNLOCK(timer_thread.mut);

	LOG(LEVEL_TRACE, "DONE handler=%p\n", handler);

	return removed;
}

int TimerIsActive(timer_handler handler)
{
	struct timer *tmp, *prev;

	//LOG(LEVEL_TRACE, "handler=%p\n", handler);

	if (!timer_thread.mut) {
		DEBUG("Not inited!\n");
		return 0;
	}

	MUTEX_LOCK(timer_thread.mut);
	if (list_empty(&timer_list)) {
		MUTEX_UNLOCK(timer_thread.mut);
		return 0;
	}

	list_for_each_safe(tmp, prev, &timer_list, struct timer, list) {
		if (tmp->handler == handler) {
			MUTEX_UNLOCK(timer_thread.mut);
			return 1;
		}
	}
	MUTEX_UNLOCK(timer_thread.mut);

	return 0;
}

// called under lock!
static int timer_tick(void)
{
	struct timer tmp, *timer, *prev;

	//LOG(LEVEL_TRACE, "\n");

	if (list_empty(&timer_list)) {
		return 0;
	}

	//LOG(LEVEL_TRACE, "\n");

	list_for_each_safe(timer, prev, &timer_list, struct timer, list) {
		if (timer->ticks_left == 0) {
			tmp = *timer;	// copy timer to tmp
			__remove_timer(timer);
			if (tmp.handler) {
				MUTEX_UNLOCK(timer_thread.mut);
				//TRACE("Firing timer: %s (%p)\n", tmp.func, tmp.handler);
				tmp.handler(tmp.param);
				MUTEX_LOCK(timer_thread.mut);
			}
			if (tmp.flags & TIMER_F_PERIODIC) {
				timer = (struct timer *)calloc(1, sizeof(*timer));
				if (timer == NULL) {
					FATAL("WTF?? func=%s (%p) %u secs %u usecs\n", timer->func, timer->handler, tmp.secs, tmp.usecs);
					return -1;
				}
				*timer = tmp;
				if (__timer_set_time(timer, tmp.secs, tmp.usecs) < 0) {
					free(timer);
					return -1;
				}
				__add_timer(timer);
			}
		} else {
			timer->ticks_left--;
		}
	}

	return 0;
}

static void *timerThreadMain(void *arg)
{
	thread_hnd_t *thread = (thread_hnd_t *)arg;
	struct timer *timer, *prev;

	DEBUG("Thread started\n");

	/* thread main loop */
	while (1) {
		MUTEX_LOCK(thread->mut);

		if (thread->running == 0) {	/* some event want us to stop */
			MUTEX_UNLOCK(thread->mut);
			break;
		}

		/* place the code that needs to be locked here */
		timer_tick();

		MUTEX_UNLOCK(thread->mut);

		usleep(TIMER_SENSITIVITY);
	}

	MUTEX_LOCK(thread->mut);

	if (list_empty(&timer_list)) {
		goto skip;
	}

	list_for_each_safe(timer, prev, &timer_list, struct timer, list) {
		__remove_timer(timer);
	}

skip:
	MUTEX_UNLOCK(thread->mut);

	DEBUG("Thread completed\n");

	return 0;
}

int TimerInit(void)
{
	TRACE("Init\n");

	timer_thread.mut = thread_mutex_init();
	timer_thread.cond = thread_cond_init();

	list_init(&timer_list);

	create_thread(&timer_thread, "Timer_thread", timerThreadMain, &timer_thread);

	return 0;
}

int TimerUninit(void)
{
	struct timer *timer, *prev;

	TRACE("Uninit\n");

	if (!timer_thread.mut) {
		DEBUG("Not inited!\n");
		return 0;
	}

	TRACE("Stopping thread...\n");
	kill_thread(&timer_thread);
	TRACE("Thread stopped\n");

	MUTEX_LOCK(timer_thread.mut);
	list_for_each_safe(timer, prev, &timer_list, struct timer, list) {
		__remove_timer(timer);
	}
	MUTEX_UNLOCK(timer_thread.mut);

	return 0;
}

int TimerAbortThread(void)
{
	if (MUTEX_TRYLOCK(timer_thread.mut) != 0) {
		LOG(LEVEL_WARNING, "force unlocking!\n");
	}

	MUTEX_UNLOCK(timer_thread.mut);

	return 0;
}
