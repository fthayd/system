// Std Includes
#include <iostream>

// Local Includes
#include <module1.h>
#include <common.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations


// Private Function Prototypes


// Functions

Module1::Module1(class Mngr *pManager) : MngrModule(pManager)
{
	TRACE("Constructor\n");
}

Module1::~Module1(void)
{
	TRACE("Destructor\n");
}

int Module1::mainLoop(void)
{
	//TRACE("Mainloop\n");

	SEND_SIMPLE_EVENT(EVENT_DUMMY);

	return 0;
}
