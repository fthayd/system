// Std Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <execinfo.h>
#include <fcntl.h>

// Local Includes
#include <main.h>
#include <common.h>
#include <Mngr.h>
#include <timer.h>
#include <Config.h>
#include <Utility.h>

// Private Constant Definitions
#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL	LEVEL_TRACE
#endif

#define PID_FILE		"/tmp/"APP_NAME".pid"

#ifndef DEFAULT_LOG_FILE
#define DEFAULT_LOG_FILE	"/tmp/"APP_NAME".log"
#endif

// Private Macro Definitions


// Private Data Types


// Global Variable Declarations
Mngr *manager = NULL;
pthread_mutex_t	fatalMutex = PTHREAD_MUTEX_INITIALIZER;
static int stopped = 0;

// Private Function Prototypes


// Functions

void fatal_log(FatalErrorCode code, const char *format, ...)
{
	FILE *fptr;
	va_list args;
	struct tm curtm;
	struct timeval tv;

	pthread_mutex_lock(&fatalMutex);

	/* Get the current time */
	gettimeofday(&tv, NULL);
	/* Break it down & store it in the structure tm */
	localtime_r(&tv.tv_sec, &curtm);

	if ((fptr = fopen(FATAL_LOG_FILE, "a")) == NULL) {
		FATAL("Cannot open output file: %s\n", FATAL_LOG_FILE);
		goto out;
	}
	fcntl(fileno(fptr), F_SETFD, FD_CLOEXEC);

	fprintf(fptr, "%02d.%02d.%04d %02d:%02d:%02d|%d|", curtm.tm_mday, curtm.tm_mon + 1, curtm.tm_year + 1900, curtm.tm_hour, curtm.tm_min, curtm.tm_sec, code);

	va_start(args, format);
	vfprintf(fptr, format, args);
	va_end(args);

	fflush(fptr);
	fsync(fileno(fptr));
	fclose(fptr);

out:
	pthread_mutex_unlock(&fatalMutex);

	return;
}

static void signalHnd(int signal)
{
	DEBUG("signal=%d\n", signal);

	if (!manager) {
		printf("wait for manager constructor\n");
		stopped = 1;
		return;
	}

	if ((signal == SIGSEGV) || (signal == SIGABRT)) {
		void *array[6];
		unsigned int size;
		char **funcNames;
		size = backtrace(array, 6);
		funcNames = backtrace_symbols(array, size);
		for (unsigned int i = 0; i < size; i++) {
			FATAL("%s\n", funcNames[i]);
		}
		exit(128 + signal);
	}

	if (!stopped && manager) {
		manager->signalHnd(signal);
	}

	return;
}

static void registerSignals(void)
{
	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = signalHnd;

	/* handle ctrl + C signal */
	sigaction(SIGINT, &action, NULL);
	/* handle kill signal */
	sigaction(SIGTERM, &action, NULL);
	/* handle segmentation fault signal */
	sigaction(SIGSEGV, &action, NULL);
	/* handle abort signal */
	sigaction(SIGABRT, &action, NULL);
}

static void removePidFile(void)
{
	unlink(PID_FILE);
}

void forceSingleInstance(void)
{
	pid_t pid;
	char buf[1024] = {0};
	FILE *fp = NULL;

	if ((fp = fopen(PID_FILE, "r")) == NULL) {
		//printf("Cannot open PID file!\n");
		goto not_running;
	}
	fcntl(fileno(fp), F_SETFD, FD_CLOEXEC);

	if (fscanf(fp, "%d\n", &pid) == 1) {
		//printf("PID: %d\n", pid);
		fclose(fp);
	} else {
		printf("Failed to Read PID\n");
		fclose(fp);
		goto not_running;
	}

	sprintf(buf, "/proc/%d/comm", pid);

	if ((fp = fopen(buf, "r")) == NULL) {
		goto not_running;
	}
	fcntl(fileno(fp), F_SETFD, FD_CLOEXEC);

	if (fgets(buf, sizeof(buf), fp) == NULL) {
		printf("Failed to Read comm file\n");
		fclose(fp);
		goto not_running;
	}

	fclose(fp);

	if (strncmp(buf, APP_NAME, strlen(APP_NAME)) != 0) {
		printf("Obsolete PID file found! pid=%d buf=%s\n", pid, buf);
		goto not_running;
	}

	// in this case the same application is running!
	printf("%s is already running with PID: %d!\n", APP_NAME, pid);
	exit(1);

not_running:
	if ((fp = fopen(PID_FILE, "w")) == NULL) {
		printf("Cannot open PID file: %s\n", PID_FILE);
		exit(2);
	}

	if (fprintf(fp, "%d", getpid()) <= 0) {
		printf("Cannot write to PID file: %s\n", PID_FILE);
	}

	fclose(fp);

	atexit(removePidFile);
}

int setSchedPriority(void)
{
	struct sched_param sp;

	sp.sched_priority = sched_get_priority_max(SCHED_FIFO);

	if (sched_setscheduler(getpid(), SCHED_FIFO, &sp) < 0) {
		perror("sched");
		return -1;
	}
#if 1
	if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) < 0) {
		perror("pthread_sched");
		return -1;
	}
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	signed char c;
	int ret = 0;
	LogLevel loglevel = DEFAULT_LOG_LEVEL;
	const char *logfile = DEFAULT_LOG_FILE;
	int option_index = 0;
	static struct option long_options[] = {
		{"help",	no_argument,		0,	'h'},
		{"Version",	no_argument,		0,	'v'},
		{0, 0, 0, 0},
	};

	while ((c = getopt_long(argc, argv, "hl:L:FDAvRa:", long_options, &option_index)) != -1) {
		switch (c) {
			case 'h':
				printf("Help:\n");
				//usage();
				return 0;
			case 'v':
				printf(PROGRAM_VERSIYON_FULL);
				return 0;
			case '?':
			default:
				//usage();
				return -1;
		}
	}

	// ignore ctrl+c signal until initialization completed
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	// set our process priority to highest!
	setSchedPriority();

	LoggerInit(loglevel, logfile, 0);

	DEBUG("Application started. Loglevel=%d LogFile=%s\n", loglevel, logfile);

	TimerInit();

	forceSingleInstance();

	registerSignals();

	manager = new Mngr();

	if (stopped) {
		goto out2;
	}

	ret = manager->init();

	if (ret < 0) {
		FATAL_LOG(GENEL_DONANIM_HATASI, "error on manager init! ret=%d\n", ret);
		ret = 0;
		goto out2;
	}

	if (manager->getState() != INIT) {
		goto out2;
	}

	try {
		ret = manager->run();
	} catch (const char *s) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Unhandled exception: %s\n", s);
		ret = -100;
	} catch (exception const &e) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Unhandled exception: Type: %s What: %s\n", typeid(e).name(), e.what());
		ret = -101;
	}

	stopped = 1;
out2:
	delete manager;
	manager = NULL;

//out:
	TimerUninit();

	DEBUG("Application finished with return value: %d\n", ret);

	return ret;
}
