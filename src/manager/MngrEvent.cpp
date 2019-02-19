// Std Includes
#include <cstdlib>

// Local Includes
#include <MngrEvent.h>
#include <MngrModule.h>
#include <common.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations
pthread_mutex_t	eventMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

// Private Function Prototypes


// Functions

MngrEvent::MngrEvent(class MngrModule *pSender, EventType type, const void *pPayload, unsigned int size)
{
	TRACE("sender=%s event=%d payload=%p payloadSize=%u\n", CLASSNAME(pSender), type, pPayload, size);

	eventType = type;
	sender = pSender;
	payload = pPayload;
	payloadSize = size;
}

MngrEvent::~MngrEvent(void)
{
	TRACE("Destructor type=%d\n", eventType);
}

EventType MngrEvent::getType(void)
{
	return eventType;
}

MngrModule *MngrEvent::getSender(void)
{
	return sender;
}

const void *MngrEvent::getPayload(void)
{
	return payload;
}

unsigned int MngrEvent::getPayloadSize(void)
{
	return payloadSize;
}

bool operator==(class MngrEvent *pEvent, EventType pEventType)
{
	return (pEvent->getType() == pEventType);
}
