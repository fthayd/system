// Std Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

// Local Includes
#include <MngrCmdHandler.h>
#include <Mngr.h>
#include <Config.h>

// Private Constant Definitions
#define CMD_IFACE_FILE		"/tmp/"APP_NAME"-cmd"
#define CMD_NOTFOUND		-99

// Private Macro Definitions
#define CALL_CMD_FUNC(_cmd, _argc, _argv) (*_cmd->func)(_cmd, _argc, _argv)

#define TRACE_PRINT_ARGV(_argc, _argv)	\
	for (int i = 0; i < _argc; i++) {\
		TRACE("\t argv[%d]=%s\n", i, argv[i]);	\
	}

// Private Data Types
typedef struct cmd cmd_t;
typedef int (cmd_func_t)(cmd_t *cmd, int argc, char **argv);

struct cmd {
	const char *name;
	cmd_func_t *func;
	const char *help;
	const char *usage;
};

// Private Function Prototypes
static int cmdHelp(cmd_t *cmd, int argc, char **argv);
static int cmdQuit(cmd_t *cmd, int argc, char **argv);

// Global Variable Declarations
cmd_t commands[] = {
	{ "help", cmdHelp, "get help about commands", "help" },
	{ "quit", cmdQuit, "quit program", "quit" },
	{ NULL, NULL, NULL, NULL }	/* last one should be NULL! */
};

MngrCmdHandler *CmdHandler = NULL;

// Functions
static int cmdHelp(cmd_t *cmd, int argc, char **argv)
{
	cmd_t *cmds = NULL;

	UNUSED(cmd);
	TRACE("argc=%d\n", argc);
	TRACE_PRINT_ARGV(argc, argv);

	printf("Valid commands:\n");

	for (cmds = commands; cmds->name; cmds++) {
		printf(" %s", cmds->name);

		if (cmds->help) {
			printf(": \n\t %s. %s %s\n", cmds->help, cmds->usage ? "\n\t Usage: " : "", cmds->usage ? cmds->usage : "");
		} else {
			printf("\n");
		}
	}

	return 0;
}

static int cmdQuit(cmd_t *cmd, int argc, char **argv)
{
	UNUSED(cmd);
	TRACE("argc=%d\n", argc);
	TRACE_PRINT_ARGV(argc, argv);

	CmdHandler->manager->signalHnd(SIGINT);

	return 0;
}

static cmd_t *find_cmd(char *cmdname)
{
	cmd_t *cmd = NULL;

	/* search the commands for a matching name */
	for (cmd = commands; cmd->name && strncmp(cmd->name, cmdname, strlen(cmdname)); cmd++);

	if (cmd->name == NULL) {
		cmd = NULL;
	}

	return cmd;
}

static char *get_next_cmd(char **cmdline)
{
	char *cmdstart = *cmdline;

	TRACE("cmdline=\"%s\" (%p)\n", *cmdline, *cmdline);

	if ((cmdline == NULL) || (*cmdline == NULL)) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "what the heck?\n");
		return NULL;
	}

	if (*cmdstart == ' ') {
		(*cmdline)++;
		return NULL;
	}

	while (**cmdline) {
		if ((**cmdline == ' ') || (**cmdline == '\n')) {
			**cmdline = '\0';
			(*cmdline)++;
			return cmdstart;
		}
		(*cmdline)++;
	}

	return cmdstart;
}

static int parse_and_run_cmd(char *cmdline)
{
	int argc = 0;
	char **argv = NULL;
	char *token = NULL;
	int err = 0;

	TRACE("Parsing cmd=\"%s\" (%p)\n", cmdline, cmdline);

	if (cmdline == NULL) {
		return 0;
	}

	while (*cmdline) {
		token = get_next_cmd(&cmdline);

		TRACE("token=\"%s\" (%p)\n", token, token);

		if ((token == NULL) || (token[0] == '\0')) {
			continue;
		}

		argc++;
		argv = (char **)realloc(argv, argc * sizeof(char*));
		argv[argc - 1] = token;
	}

	/* have any commands? */
	if (argc > 0) {
		cmd_t *cmd = find_cmd(argv[0]);

		if (cmd == NULL) {
			err = CMD_NOTFOUND;
		} else {
			err = CALL_CMD_FUNC(cmd, --argc, &argv[1]);
		}
	}

	free(argv);

	return err;
}

MngrCmdHandler::MngrCmdHandler(class Mngr *pManager) : MngrModule(pManager)
{
	TRACE("Constructor\n");

	Config::create();

	manager = pManager;
	commandFile = -1;
	usage = 0;

	createCommandInterface();
}

MngrCmdHandler::~MngrCmdHandler(void)
{
	TRACE("Destructor\n");

	ASSERT((usage == 0), ("Usage not zero!!! %d\n", usage));

	Config::destroy();

	removeCommandInterface();
}

void MngrCmdHandler::create(class Mngr *pManager)
{
	if (!CmdHandler) {
		CmdHandler = new MngrCmdHandler(pManager);
	}

	CmdHandler->usage++;

	TRACE("usage=%d\n", CmdHandler->usage);
}

void MngrCmdHandler::destroy(void)
{
	if (CmdHandler) {
		CmdHandler->usage--;
		TRACE("usage=%d\n", CmdHandler->usage);
		if (CmdHandler->usage == 0) {
			delete CmdHandler;
			CmdHandler = NULL;
		}
	} else {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "MngrCmdHandler fazladan destroy!\n");
	}
}

int MngrCmdHandler::createCommandInterface(void)
{
	//remove first, if already created
	removeCommandInterface();

	if (mkfifo(CMD_IFACE_FILE, 0666) < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Unable to create fifo: %s\n", CMD_IFACE_FILE);
		return -1;
	}

	commandFile = open(CMD_IFACE_FILE, O_RDWR | O_NONBLOCK | O_CLOEXEC);

	if (commandFile < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "cannot open pipe: %s\n", CMD_IFACE_FILE);
		return -1;
	}

	return commandFile;
}

int MngrCmdHandler::processCommand(void)
{
	char buf[1024] = {0};

	if (commandFile < 0) {
		LOG(LEVEL_ERROR, "cannot open pipe: %s\n", CMD_IFACE_FILE);
		return -1;
	}

	if (read(commandFile, buf, sizeof(buf)) > 0) {
		char *rest; /* to point to the rest of the string after token extraction. */
		char *token; /* to point to the actual token returned. */
		char *ptr = buf;

		/* Split the commands by newline, and loop till strtok_r returns NULL. */
		while ((token = strtok_r(ptr, "\n", &rest)) != NULL) {
			DEBUG("Got command: \"%s\"\n", buf);
			if (parse_and_run_cmd(token) == CMD_NOTFOUND) {
				LOG(LEVEL_WARNING, "command: \"%s\" not found!\n", buf);
			}
			ptr = rest;
		}
	}

	return 0;
}

int MngrCmdHandler::removeCommandInterface(void)
{
	if (commandFile >= 0) {
		close(commandFile);
	}

	return remove(CMD_IFACE_FILE);
}
