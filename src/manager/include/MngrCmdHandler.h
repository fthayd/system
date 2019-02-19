#ifndef _MNGR_CMD_HANDLER_H_
#define _MNGR_CMD_HANDLER_H_

// Std Includes


// Local Includes
#include <MngrModule.h>
#include <common.h>

// Global Constant Definitions
#define MNGR_PROCESS_COMMAND_TICKS	10

// Global Macro Definitions


// Global Data Types
class MngrCmdHandler : public MngrModule
{
	private:
		int commandFile;
		int createCommandInterface(void);
		int removeCommandInterface(void);
	public:
		DEFINE_CLASSNAME("MngrCmdHandler");
		MngrCmdHandler(class Mngr *pManager);
		virtual ~MngrCmdHandler(void);
		class Mngr *manager;
		int usage;
		static void create(class Mngr *pManager);
		static void destroy(void);
		int processCommand(void);
};


// Global Variable Externs
extern MngrCmdHandler *CmdHandler;

// Global Function Prototypes


#endif	//_MNGR_CMD_HANDLER_H_
