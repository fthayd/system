// Std Includes
#include <iostream>

// Local Includes
#include <module2.h>
#include <common.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations


// Private Function Prototypes


// Functions

Module2::Module2(class Mngr *pManager) : MngrModule(pManager)
{
	TRACE("Constructor\n");
}

Module2::~Module2(void)
{
	TRACE("Destructor\n");
}

int Module2::mainLoop(void)
{
	//TRACE("Mainloop\n");

	SEND_SIMPLE_EVENT(EVENT_DUMMY);

	return 0;
}
