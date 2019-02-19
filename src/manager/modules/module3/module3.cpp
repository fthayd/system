// Std Includes
#include <iostream>

// Local Includes
#include <module3.h>
#include <common.h>

// Private Constant Definitions


// Private Macro Definitions


// Private Data Types


// Global Variable Declarations


// Private Function Prototypes


// Functions

Module3::Module3(class Mngr *pManager) : MngrModule(pManager)
{
	TRACE("Constructor\n");
}

Module3::~Module3(void)
{
	TRACE("Destructor\n");
}

int Module3::mainLoop(void)
{
	//TRACE("Mainloop\n");

	SEND_SIMPLE_EVENT(EVENT_DUMMY);

	return 0;
}
