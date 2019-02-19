// Std Includes
#include <cstdlib>

// Local Includes
#include <MngrEventList.h>
#include <common.h>
#include <MngrModule.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations


// Private Function Prototypes


// Functions

MngrEventList::MngrEventList(EventType type)
{
	//TRACE("Creating eventList for event: %d -> %p\n", type, this);

	eventType = type;
}

MngrEventList::~MngrEventList(void)
{
	//TRACE("Destroying eventList for event: %d -> %p\n", eventType, this);

	subscribers.clear();
}

int MngrEventList::subscribe(class MngrModule *module)
{
	DEBUG("%s subcsribed to event: %d\n", CLASSNAME(module), eventType);
	// add to subscribers list
	subscribers.push_back(module);

	return 0;
}

int MngrEventList::unsubscribe(class MngrModule *module)
{
	DEBUG("%s unsubcsribed from event: %d\n", CLASSNAME(module), eventType);
	// remove from subscribers list
	subscribers.remove(module);

	return 0;
}

int MngrEventList::trigger(class MngrEvent *event)
{
	TRACE("event=%d payload=%p payloadSize=%d\n", event->getType(), event->getPayload(), event->getPayloadSize());

	EVENT_LOCK();
	// call event handler of registered modules
	FOREACH(list<class MngrModule*>, subscribers, module) {
		if (event->getSender() == *module) {
			TRACE("Ignoring event handler of %s which is actually the sender of event!\n", CLASSNAME(*module));
			continue;
		}
		TRACE("Calling event handler of %s\n", CLASSNAME(*module));
		(*module)->eventHnd(event);
	}

	TRACE("Done handling event\n");

	EVENT_UNLOCK();

	return 0;
}
