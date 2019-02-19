#ifndef _MODULE2_H_
#define _MODULE2_H_

// Std Includes


// Local Includes
#include <common.h>
#include <MngrModule.h>

// Global Constant Definitions


// Global Macro Definitions


// Global Data Types
class Module2 : public MngrModule
{
	public:
		DEFINE_CLASSNAME("Module2");
		Module2(class Mngr *pManager);
		~Module2(void);
		int mainLoop(void);
};

// Global Variable Externs


// Global Function Prototypes


#endif	//_MODULE2_H_
