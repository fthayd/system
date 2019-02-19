#ifndef _MNGR_H_
#define _MNGR_H_

// Std Includes
#include <list>

// Local Includes
#include <common.h>
#include <MngrModule.h>
#include <MngrEventList.h>

// Global Constant Definitions


// Global Macro Definitions


// Global Data Types
typedef enum {
	INIT = 0,
	RUNNING,
	HALTED,
} MngrState;

class Mngr
{
	private:
		list<MngrModule*> moduleList;
		MngrEventList *eventList[EVENT_MAX];
		MngrState state;
		void initEventList(void);
		void destroyEventList(void);
		int mainLoop(void);
		int deleteModules(void);
	public:
		DEFINE_CLASSNAME("Mngr");
		Mngr(void);
		virtual ~Mngr(void);
		unsigned long mainLoopDelayMsecs;
		int init(void);
		int run(void);
		template <class ModuleType> int registerModule(void);
		void signalHnd(int signal);
		int subscribeToEvent(EventType type, class MngrModule *module);
		int unSubscribeFromEvent(EventType type, class MngrModule *module);
		int triggerEvent(class MngrEvent *event);
		MngrState getState(void);
		void setState(MngrState s);
};


// Global Variable Externs


// Global Function Prototypes


#endif	//_MNGR_H_
