#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <thread.h>


/**
 * @brief Thread creation argument object.
 * @note For internal use in thread module only.
 */
typedef struct thread_arg_s thread_arg_t;

/**
 * @brief Thread creation argument structure.
 * @note For internal use in thread module only.
 */
struct thread_arg_s {
    /** Name of thread */
	char *name;
	/** Flag indicating successful creation of the thread. */
	int flag;
	/** Pointer to the argument of the function desired to run as thread. */
	void *arg;
	/** Pointer to the encapsulation function to make sure the actual
	 * function desired to run as thread started successfuly. */
	void *(*r)(void *);
	/** Pointer to the actual function desired to run as thread. */
	void *(*f)(void *);
	/** Pointer to the condition object to guard shared critical data. */
	thread_cond_t *cond;
	/** Pointer to the mutex object to guard shared critical data. */
	thread_mutex_t *mut;
	thread_hnd_t *thread;
};

/*@}*/

thread_cond_t *thread_cond_init(void)
{
	thread_cond_t *c;

	c = (thread_cond_t *)calloc(1, sizeof(thread_cond_t));
	if (c == NULL) {
		return NULL;
	}

	if (pthread_cond_init(&c->cond, NULL) != 0) {
		free(c);
		return NULL;
	}
	return c;
}

int thread_cond_uninit(thread_cond_t *cond)
{
	pthread_cond_destroy(&cond->cond);
	free(cond);

	return 0;
}

int thread_cond_signal(thread_cond_t *cond)
{
	return pthread_cond_signal(&cond->cond);
}

int thread_cond_wait(thread_cond_t *cond, thread_mutex_t *mutex)
{
	return pthread_cond_wait(&cond->cond, &mutex->mutex);
}

int thread_cond_timedwait(thread_cond_t *cond, thread_mutex_t *mut, int msec)
{
	int ret;
	struct timeval tval;
	struct timespec tspec;

	if (msec < 0) {
		return thread_cond_wait(cond, mut);
	}

	gettimeofday(&tval, NULL);
	tspec.tv_sec = tval.tv_sec + (msec / 1000);
	tspec.tv_nsec = (tval.tv_usec + ((msec % 1000) * 1000)) * 1000;

	if (tspec.tv_nsec >= 1000000000) {
		tspec.tv_sec += 1;
		tspec.tv_nsec -= 1000000000;
	}

again:
	ret = pthread_cond_timedwait(&cond->cond, &mut->mutex, &tspec);
	switch (ret) {
		case EINTR:
			goto again;
			break;
		case ETIMEDOUT:
			ret = 1;
			break;
		case 0:
			break;
		default:
			assert(0);
			ret = -1;
			break;
	}
	return ret;
}

thread_mutex_t *thread_mutex_init(void)
{
	thread_mutex_t *m;

	m = (thread_mutex_t *)calloc(1, sizeof(thread_mutex_t));
	if (m == NULL) {
		return NULL;
	}

	if (pthread_mutex_init(&m->mutex, NULL) != 0) {
		free(m);
		return NULL;
	}
#ifdef MUTEX_DEBUG
	m->func_locked = NULL;
	m->line_locked = 0;
#endif
	return m;
}

int thread_mutex_uninit(thread_mutex_t *mutex)
{
	pthread_mutex_destroy(&mutex->mutex);
	free(mutex);

	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex)
{
	return pthread_mutex_lock(&mutex->mutex);
}

int thread_mutex_trylock(thread_mutex_t *mutex)
{
	return pthread_mutex_trylock(&mutex->mutex);
}

int thread_mutex_unlock(thread_mutex_t *mutex)
{
	return pthread_mutex_unlock(&mutex->mutex);
}

static void *thread_run(void *farg)
{
	thread_arg_t *arg = (thread_arg_t *)farg;
	thread_hnd_t *thread = arg->thread;
	void *p = arg->arg;
	void *(*f)(void *) = arg->f;
	sigset_t signal_mask;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	thread_mutex_lock(arg->mut);
	arg->flag = 1;
	/* infof("running thread %s (%d)\n", arg->name, getpid()); */
	thread_cond_signal(arg->cond);
	thread_mutex_unlock(arg->mut);

	MUTEX_LOCK(thread->mut);
	thread->started = 1;
	thread->running = 1;
	thread->stopped = 0;
	thread_cond_signal(thread->cond);
	MUTEX_UNLOCK(thread->mut);

	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGINT);
	/* sigaddset(&signal_mask, SIGTERM); */

	if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) != 0) {
		printf("%s:%d fatal errorr!\n", __func__, __LINE__);
		return NULL;
	}

	f(p);

	MUTEX_LOCK(thread->mut);
	thread->started = 0;
	thread->stopped = 1;
	thread->running = 0;
	thread_cond_signal(thread->cond);
	MUTEX_UNLOCK(thread->mut);

	thread_cond_uninit(arg->cond);
	thread_mutex_uninit(arg->mut);
	free(arg);

	return NULL;
}

int stop_thread(thread_hnd_t *thread)
{
	MUTEX_LOCK(thread->mut);
	if ((!thread->thread) || (thread->running == 0)) {
		MUTEX_UNLOCK(thread->mut);
		return -1;
	}

	thread->running = 0;

	thread_cond_signal(thread->cond);
	while (thread->stopped == 0) {
		thread_cond_wait(thread->cond, thread->mut);
	}

	thread_join(thread->thread);
	MUTEX_UNLOCK(thread->mut);

	return 0;
}

int kill_thread(thread_hnd_t *thread)
{
	if (!thread || !thread->thread) {
		return 0;
	}

	return pthread_cancel(thread->thread->thread);
}

void create_thread(thread_hnd_t *thread, const char *name, void *(*function)(void *), void *farg)
{
	MUTEX_LOCK(thread->mut);

	thread->thread = thread_create(thread, name, function, farg);

	while (thread->started == 0) {
		thread_cond_wait(thread->cond, thread->mut);
	}
	MUTEX_UNLOCK(thread->mut);

	/* wait for thread to run */
	while (1) {
		MUTEX_LOCK(thread->mut);
		if (thread->running) {
			MUTEX_UNLOCK(thread->mut);
			break;
		}
		MUTEX_UNLOCK(thread->mut);
		usleep(100);
	}
}

thread_t *thread_create(thread_hnd_t *thread, const char *name, void *(*function)(void *), void *farg)
{
	int ret;
	thread_t *tid;
	thread_arg_t *arg;

	tid = (thread_t *)calloc(1, sizeof(thread_t));
	arg = (thread_arg_t *)calloc(1, sizeof(thread_arg_t));
	tid->name = strdup(name);

	arg->r = &thread_run;
	arg->f = function;
	arg->arg = farg;
	arg->name = tid->name;
	arg->cond = thread_cond_init();
	arg->mut = thread_mutex_init();
	arg->flag = 0;
	arg->thread = thread;

	thread_mutex_lock(arg->mut);
#ifdef __UCLIBC__
	ret = pthread_create(&(tid->thread), NULL, arg->r, arg);
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	ret = pthread_create(&(tid->thread), &attr, arg->r, arg);
	pthread_attr_destroy(&attr);
#endif
	if (ret != 0) {
		printf("%s:%d fatal errorr!\n", __func__, __LINE__);
		goto err;
	}

	while (arg->flag != 1) {
		thread_cond_wait(arg->cond, arg->mut);
	}

	thread_mutex_unlock(arg->mut);
	return tid;
err:
	thread_mutex_unlock(arg->mut);

	thread_cond_uninit(arg->cond);
	thread_mutex_uninit(arg->mut);
	free(arg);
	arg = NULL;
	free(tid->name);
	free(tid);
	tid = NULL;

	return tid;
}

int thread_join(thread_t *thread)
{
	free(thread->name);
	pthread_join(thread->thread, NULL);
	free(thread);

	return 0;
}

int thread_sched_yield(void)
{
	return 0;
}
