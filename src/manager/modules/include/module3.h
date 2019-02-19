#ifndef _MODULE3_H_
#define _MODULE3_H_

// Std Includes


// Local Includes
#include <common.h>
#include <MngrModule.h>

// Global Constant Definitions


// Global Macro Definitions


// Global Data Types
class Module3 : public MngrModule
{
	public:
		DEFINE_CLASSNAME("Module3");
		Module3(class Mngr *pManager);
		~Module3(void);
		int mainLoop(void);
};

// Global Variable Externs


// Global Function Prototypes


#endif	//_MODULE3_H_
