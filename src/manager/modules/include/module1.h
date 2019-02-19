#ifndef _MODULE1_H_
#define _MODULE1_H_

// Std Includes


// Local Includes
#include <common.h>
#include <MngrModule.h>

// Global Constant Definitions


// Global Macro Definitions


// Global Data Types
class Module1 : public MngrModule
{
	public:
		DEFINE_CLASSNAME("Module1");
		Module1(class Mngr *pManager);
		~Module1(void);
		int mainLoop(void);
};

// Global Variable Externs


// Global Function Prototypes


#endif	//_MODULE1_H_
