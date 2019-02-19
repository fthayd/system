#ifndef _MNGR_EVENT_LIST_H_
#define _MNGR_EVENT_LIST_H_

// Std Includes
#include <list>

// Local Includes
#include <common.h>
#include <MngrEvent.h>

// Global Constant Definitions


// Global Macro Definitions


// Global Data Types

class MngrEventList
{
	private:
		list<class MngrModule*> subscribers;
	public:
		DEFINE_CLASSNAME("MngrEventList");
		MngrEventList(EventType type);
		virtual ~MngrEventList(void);
		EventType eventType;
		int subscribe(class MngrModule *module);
		int unsubscribe(class MngrModule *module);
		int trigger(class MngrEvent *event);
};


// Global Variable Externs


// Global Function Prototypes


#endif	//_MNGR_EVENT_LIST_H_
