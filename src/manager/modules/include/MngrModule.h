#ifndef _MNGR_MODULE_H_
#define _MNGR_MODULE_H_

// Std Includes


// Local Includes
#include <common.h>
#include <MngrEvent.h>
#include <thread.h>
#include <Mngr.h>

// Global Constant Definitions


// Global Macro Definitions
#define SEND_EVENT(_sender, _event, _payload, _payloadsize)	\
	do {	\
		MngrEvent *event = new MngrEvent(_sender, _event, _payload, _payloadsize);	\
		_sender->sendEvent(event);	\
		delete event;	\
	} while (0)

#define SEND_SIMPLE_EVENT(_event)	SEND_EVENT(this, _event, NULL, 0)

// Global Data Types
class MngrModule
{
	private:
		class Mngr *manager;
		unsigned int mainLoopSkipTicks;
		thread_mutex_t *eventMut;
		thread_cond_t *eventCond;
		MngrEvent *outgoingEvent;
	public:
		DEFINE_CLASSNAME("MngrModule");
		MngrModule(class Mngr *pManager);
		virtual ~MngrModule(void);
		virtual int init(void);
		virtual int unInit(void);
		virtual int mainLoop(void);
		virtual int signalHnd(int signal);
		virtual int eventHnd(class MngrEvent *event);
		int subscribeToEvent(EventType type);
		int unSubscribeFromEvent(EventType type);
		int sendEvent(class MngrEvent *event);
		int processThreadEvent(void);
		void setMainloopDelayMsecs(unsigned long msec);
		bool mainLoopShouldSkip(unsigned long ticks);
		int getMngrState(void);
};

// Global Variable Externs
extern pthread_mutex_t	eventMutex;

// Global Function Prototypes


#endif	//_MNGR_MODULE_H_
