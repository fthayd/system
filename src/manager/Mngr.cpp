// Std Includes
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

// Local Includes
#include <Mngr.h>
#include <MngrCmdHandler.h>
#include <common.h>
#include <Config.h>
#include <timer.h>

// Private Constant Definitions
#define MNGR_MAINLOOP_SLEEP		(1 * 1000)	// in usecs
#define MNGR_MAINLOOP_PROCESS_TIME	(49 * 1000)	// in usecs
#define MNGR_LOCK()			pthread_mutex_lock(&mngrMutex)
#define MNGR_UNLOCK()			pthread_mutex_unlock(&mngrMutex)

// Private Macro Definitions


// Private Data Types


// Global Variable Declarations
pthread_mutex_t	mngrMutex = PTHREAD_MUTEX_INITIALIZER;

// Private Function Prototypes


// Functions

Mngr::Mngr(void)
{
	TRACE("Constructor\n");

	state = INIT;

	initEventList();

	mainLoopDelayMsecs = (MNGR_MAINLOOP_SLEEP + MNGR_MAINLOOP_PROCESS_TIME) / 1000;

	TRACE("DONE\n");
}

Mngr::~Mngr(void)
{
	TRACE("Destructor\n");

	destroyEventList();

	TRACE("DONE\n");
}

void Mngr::initEventList(void)
{
	for (int i = 0; i < EVENT_MAX; i++) {
		eventList[i] = new MngrEventList((EventType)i);
	}
}

void Mngr::destroyEventList(void)
{
	for (int i = 0; i < EVENT_MAX; i++) {
		delete eventList[i];
	}
}


void Mngr::signalHnd(int signal)
{
	if (getState() == HALTED) {
		printf("Waiting for termination!\n");
		return;
	}

	if (getState() == INIT) {
		setState(HALTED);
		printf("Wait for manager to initialize!\n");
		return;
	}

	FATAL_LOG_SILENT(KAPANIS_ACILIS_LOGLARI, "Signal handled! sig=%d\n", signal);

	// first, call the signal handlers of modules
	FOREACH(list<class MngrModule*>, moduleList, module) {
		DEBUG("Calling signal handler of module: %s. sig=%d\n", CLASSNAME(*module), signal);
		(*module)->signalHnd(signal);
	}

	if ((signal == SIGINT) || (signal == SIGTERM)) {
		DEBUG("Entering state: HALTED\n");
		// setting state to HALTED will terminate mainloop gracefully
		setState(HALTED);
	}
}

int Mngr::deleteModules(void)
{
	TRACE("Entered\n");

	FOREACH(list<class MngrModule*>, moduleList, module) {
		EVENT_LOCK();
		(*module)->unInit();
		EVENT_UNLOCK();
	}

	FOREACH(list<class MngrModule*>, moduleList, module) {
		delete *module;
	}

	moduleList.clear();

	TRACE("Exited\n");

	return 0;
}

template <class ModuleType>
int Mngr::registerModule(void)
{
	int ret = 0;

	if (getState() != INIT) {
		return ret;
	}

	ModuleType *module = new ModuleType(this);

	DEBUG("Registering module: %s\n", CLASSNAME(module));
	// add to moduleList
	moduleList.push_back(module);

	EVENT_LOCK();
	ret = module->init();	// call init method of registered module
	EVENT_UNLOCK();

	return ret;
}

int Mngr::mainLoop(void)
{
	unsigned long ticks = 0;

	TRACE("Entered\n");

	while (getState() == RUNNING) {
		FOREACH(list<class MngrModule*>, moduleList, module) {
			EVENT_LOCK();
			if (!(*module)) {
				FATAL_LOG(GENEL_YAZILIM_HATASI, "module NULL!!\n");
			}
			(*module)->processThreadEvent();
			if (!(*module)->mainLoopShouldSkip(ticks)) {
				(*module)->mainLoop();	// call mainloop of module
			}
			EVENT_UNLOCK();
		}

		if ((ticks % MNGR_PROCESS_COMMAND_TICKS) == 0) {
			CmdHandler->processCommand();
		}

		usleep(MNGR_MAINLOOP_SLEEP);
		ticks++;
	}

	// son defa threadlerde event processing yap!
	FOREACH(list<class MngrModule*>, moduleList, module) {
		EVENT_LOCK();
		(*module)->processThreadEvent();
		EVENT_UNLOCK();
	}

	TimerAbortThread();
	TimerUninit();

	// ikinci cycle tekrar uzerinden gec
	FOREACH(list<class MngrModule*>, moduleList, module) {
		EVENT_LOCK();
		(*module)->processThreadEvent();
		EVENT_UNLOCK();
	}

	TRACE("Exited\n");

	return 0;
}

int Mngr::init(void)
{
	int ret = 0;

	TRACE("Entered\n");

	// REGISTER MODULES HERE, IN ORDER OF PRIORITY IN EVENT HANDLING!

	if (getState() == INIT) {

	}

	TRACE("Exited\n");

	return ret;
}

int Mngr::run(void)
{
	int ret = 0;

	TRACE("Entered\n");

	if (getState() != INIT) {
		goto out;
	}

	setState(RUNNING);

	ret = mainLoop();	// mainloop returns only when app is going to be terminated

out:
	deleteModules();

	TRACE("Exited\n");

	return ret;
}

int Mngr::subscribeToEvent(EventType type, class MngrModule *module)
{
	//TRACE("type = %d -> %p\n", type, eventList[type]);
	return eventList[type]->subscribe(module);
}

int Mngr::unSubscribeFromEvent(EventType type, class MngrModule *module)
{
	//TRACE("type = %d -> %p\n", type, eventList[type]);
	return eventList[type]->unsubscribe(module);
}

int Mngr::triggerEvent(class MngrEvent *event)
{
	EventType type = event->getType();

	//TRACE("type = %d -> %p\n", type, eventList[type]);

	if (getState() != RUNNING) {
		DEBUG("Ignoring event while in state=%d\n", getState());
		return 0;
	}

	return eventList[type]->trigger(event);
}

MngrState Mngr::getState(void)
{
	MngrState retState;

	MNGR_LOCK();
	retState = state;
	MNGR_UNLOCK();

	return retState;
}

void Mngr::setState(MngrState s)
{
	MNGR_LOCK();
	state = s;
	MNGR_UNLOCK();
}
