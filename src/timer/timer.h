#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Std Includes


// Local Includes


// Global Constant Definitions
#define TIMER_F_ONESHOT		(1 << 0)
#define TIMER_F_PERIODIC	(1 << 1)

// Global Macro Definitions


// Global Data Types
typedef void (*timer_handler)(void *param);

// Global Variable Externs


// Global Function Prototypes
#ifndef STR
#define STRx(x) #x
#define STR(x) STRx(x)
#endif

int TimerInit(void);
int TimerUninit(void);
#define TimerAddTimer(_func, _flags, _secs, _usecs, _param)	_TimerAddTimer(_func, STR(_func), _flags, _secs, _usecs, _param)
int _TimerAddTimer(timer_handler handler, const char *func, int flags, unsigned int secs, unsigned int usecs, void *param);
int TimerCancelTimer(timer_handler handler);
int TimerIsActive(timer_handler handler);
int TimerAbortThread(void);

#ifdef __cplusplus
}
#endif

#endif	//_TIMER_H_
