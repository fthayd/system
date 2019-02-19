#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

//#define MUTEX_DEBUG


/**
 * @brief Exported thread object of thread module.
 */
typedef struct thread_s thread_t;

/**
 * @brief Exported condition object of thread module.
 */
typedef struct thread_cond_s thread_cond_t;

/**
 * @brief Exported mutex object of thread module.
 */
typedef struct thread_mutex_s thread_mutex_t;

typedef struct thread_hnd_s thread_hnd_t;

/**
 * @brief Thread structure of the thread module.
 */
struct thread_s {
	/** Name of the thread. */
	char *name;
	/** Stores the ID of the created POSIX thread. */
	pthread_t thread;
};

/**
 * @brief Condition structure of the thread module.
 */
struct thread_cond_s {
	/** POSIX condition variable. */
	pthread_cond_t cond;
};

/**
 * @brief Mutex structure of the thread module.
 */
struct thread_mutex_s {
	/** POSIX mutex variable. */
	pthread_mutex_t mutex;
#ifdef MUTEX_DEBUG
	const char *func_locked;
	int line_locked;
#endif
};

struct thread_hnd_s {
	int started;
	int running;
	int stopped;
	thread_t *thread;
	thread_mutex_t *mut;
	thread_cond_t *cond;
	void *data;
};

/**
 * This function creates a condition object and if successful, then returns a
 * pointer to this object.
 *
 * @return
 * - Pointer to the created object
 * - NULL otherwise
 */
thread_cond_t *thread_cond_init(void);

/**
 * This function frees resources allocated and destroys condition variable.
 *
 * @param cond Pointer to the condition object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_cond_uninit(thread_cond_t *cond);

/**
 * This function signal the condition variable, thus shall unblock threads
 * blocked on this condition variable.
 *
 * @param cond Pointer to the condition object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_cond_signal(thread_cond_t *cond);

/**
 * This function block on the condition variable. It shall be called with
 * mutex locked by the calling thread.
 *
 * @param cond Pointer to the condition object.
 * @param mutex Pointer to the mutex object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_cond_wait(thread_cond_t *cond, thread_mutex_t *mutex);

/**
 * This function block on the condition variable for a period of time
 * specified. It shall be called with mutex locked by the calling thread.
 *
 * @param cond Pointer to the condition object.
 * @param mut Pointer to the mutex object.
 * @param msec Timeout value in miliseconds.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_cond_timedwait(thread_cond_t *cond, thread_mutex_t *mut, int msec);

/**
 * This function creates a mutex object and if successful, then returns a
 * pointer to this object.
 *
 * @return
 * - Pointer to the created object
 * - NULL otherwise
 */
thread_mutex_t *thread_mutex_init(void);

/**
 * This function frees resources allocated and destroys mutex variable.
 *
 * @param mutex Pointer to the mutex object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_mutex_uninit(thread_mutex_t *mutex);

/**
 * This function locks the mutex to create a safe zone to access and edit
 * shared critical data.
 *
 * @param mutex Pointer to the mutex object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_mutex_lock(thread_mutex_t *mutex);

/**
 * This function trys to lock the mutex to create a safe zone to access and edit
 * shared critical data.
 *
 * @param mutex Pointer to the mutex object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_mutex_trylock(thread_mutex_t *mutex);

/**
 * This function unlocks the mutex locked before.
 *
 * @param mutex Pointer to the mutex object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_mutex_unlock(thread_mutex_t *mutex);

int stop_thread(thread_hnd_t *thread);

int kill_thread(thread_hnd_t *thread);

void create_thread(thread_hnd_t *thread, const char *name, void *(*function)(void *), void *farg);

/**
 * This function creates a thread object with the function and its argument
 * and if successful, then returns a pointer to this object.
 *
 * @param function Pointer to the function desired to run as thread.
 * @param farg Pointer to the argument of the function desired to run as
 * thread.
 *
 * @return
 * - Pointer to the created object
 * - NULL otherwise
 */
thread_t *thread_create(thread_hnd_t *thread, const char *name, void *(*function)(void *), void *farg);

/**
 * This function waits for the thread to terminate.
 *
 * @param thread Pointer to the thread object.
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_join(thread_t *thread);

/**
 * This function
 *
 * @return
 * - 0 on success
 * - Error code otherwise
 */
int thread_sched_yield(void);

static inline void mutex_lock(thread_mutex_t *mut, const char *func, int line)
{
	UNUSED(func);
	UNUSED(line);
#ifdef MUTEX_DEBUG
	mut->func_locked = func;
	mut->line_locked = line;
#endif
	thread_mutex_lock(mut);
}

static inline int mutex_trylock(thread_mutex_t *mut, const char *func, int line)
{
	int ret;

	UNUSED(func);
	UNUSED(line);

	if ((ret = thread_mutex_trylock(mut)) == 0) {
#ifdef MUTEX_DEBUG
	mut->func_locked = func;
	mut->line_locked = line;
#endif
	}

	return ret;
}

static inline void mutex_unlock(thread_mutex_t *mut, const char *func, int line)
{
	UNUSED(func);
	UNUSED(line);

	thread_mutex_unlock(mut);
}

#ifdef MUTEX_DEBUG
#define MUTEX_LOCK(_mutex)	\
		do {		\
			printf("%s:%d acquiring lock %p last:[%s:%d]\n", __func__, __LINE__, _mutex, _mutex->func_locked, _mutex->line_locked);	\
			mutex_lock(_mutex, __func__, __LINE__); 		\
			printf("%s:%d acquired lock %p\n", __func__, __LINE__, _mutex);	\
		} while (0)

#define MUTEX_TRYLOCK(_mutex)	mutex_trylock(_mutex, __func__, __LINE__)

#define MUTEX_UNLOCK(_mutex)	\
		do {		\
			printf("%s:%d releasing lock %p\n", __func__, __LINE__, _mutex);	\
			mutex_unlock(_mutex, __func__, __LINE__); \
		} while (0)
#else
#define MUTEX_LOCK(_mutex)	\
		do {		\
			mutex_lock(_mutex, __func__, __LINE__); 		\
		} while (0)

#define MUTEX_TRYLOCK(_mutex)	mutex_trylock(_mutex, __func__, __LINE__)

#define MUTEX_UNLOCK(_mutex)	\
		do {		\
			mutex_unlock(_mutex, __func__, __LINE__); \
		} while (0)
#endif

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
