// Std Includes
#include <iostream>

// Local Includes
#include <MngrModule.h>
#include <common.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations


// Private Function Prototypes


// Functions

MngrModule::MngrModule(class Mngr *pManager)
{
	TRACE("Constructor\n");

	manager = pManager;
	mainLoopSkipTicks = 1;

	outgoingEvent = NULL;
	// initialize event mutex and cond. variable
	eventMut = thread_mutex_init();
	eventCond = thread_cond_init();
}

MngrModule::~MngrModule(void)
{
	TRACE("Destructor\n");

	thread_cond_signal(eventCond);
}

int MngrModule::init(void)
{
	TRACE("Init\n");

	return 0;
}

int MngrModule::unInit(void)
{
	TRACE("Uninit\n");

	return 0;
}

int MngrModule::mainLoop(void)
{
	//TRACE("Mainloop\n");

	return 0;
}

int MngrModule::signalHnd(int signal)
{
	TRACE("Signal Handler. signal=%d\n", signal);

	return 0;
}

int MngrModule::subscribeToEvent(EventType type)
{
	return manager->subscribeToEvent(type, this);
}

int MngrModule::unSubscribeFromEvent(EventType type)
{
	return manager->unSubscribeFromEvent(type, this);
}

int MngrModule::eventHnd(class MngrEvent *event)
{
	LOG(LEVEL_WARNING, "Subscribe olunmus fakat handle edilmemis event! event=%d\n", event->getType());

	return 0;
}

int MngrModule::sendEvent(class MngrEvent *event)
{
	if (!manager) {
		LOG(LEVEL_WARNING, "%s: manager is not registered! ignoring event=%d\n", CLASSNAME(event->getSender()), event->getType());
		return 0;
	}

	if (manager->getState() != RUNNING) {
		DEBUG("Ignoring event while in state=%d\n", manager->getState());
		return 0;
	}

	if (getpid() != gettid()) {
		DEBUG("%s: sending event=%d IN THREAD!!\n", CLASSNAME(event->getSender()), event->getType());
		while (1) {
			if (manager->getState() != RUNNING) {
				DEBUG("Ignoring event while in state=%d\n", manager->getState());
				return 0;
			}
			MUTEX_LOCK(eventMut);
			if (outgoingEvent == NULL) {
				break;
			}
			MUTEX_UNLOCK(eventMut);
			usleep(100000);
		}
		TRACE("Lock acquired! event=%d\n", event->getType());
		outgoingEvent = event;
		while (outgoingEvent != NULL) {
			thread_cond_wait(eventCond, eventMut);
		}
		MUTEX_UNLOCK(eventMut);
		TRACE("Lock released! event=%d\n", event->getType());
		return 0;
	}

	if ((event->getPayloadSize() > 0) && (event->getPayload() == NULL)) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Event payload does not exists! sender=%s event=%d\n", CLASSNAME(event->getSender()), event->getType());
	}

	DEBUG("%s: sending event=%d\n", CLASSNAME(event->getSender()), event->getType());

	return manager->triggerEvent(event);
}

int MngrModule::processThreadEvent(void)
{
	// consume any queued outgoing event
	MUTEX_LOCK(eventMut);
	if (outgoingEvent) {
		DEBUG("%s: processing threaded event=%d\n", CLASSNAME(outgoingEvent->getSender()), outgoingEvent->getType());
		sendEvent(outgoingEvent);
		thread_cond_signal(eventCond);
		outgoingEvent = NULL;
	}
	MUTEX_UNLOCK(eventMut);

	return 0;
}

void MngrModule::setMainloopDelayMsecs(unsigned long msec)
{
	mainLoopSkipTicks = msec / manager->mainLoopDelayMsecs;

	if (mainLoopSkipTicks == 0) {
		mainLoopSkipTicks = 1;
	}

	DEBUG("Setting mainloop delay to: %u msecs (%u ticks)\n", msec, mainLoopSkipTicks);
}

bool MngrModule::mainLoopShouldSkip(unsigned long ticks)
{
	return (ticks % mainLoopSkipTicks);
}

int MngrModule::getMngrState(void)
{
	if (!manager) {
		FATAL("manager is not registered!\n");
		return HALTED;
	}

	return manager->getState();
}
