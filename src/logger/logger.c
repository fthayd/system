// Std Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

// Local Includes
#include <logger.h>

// Private Constant Definitions
#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL	LEVEL_WARNING
#endif

#define MAX_FILENAME		1024
#define DEFAULT_LOGFILE_SIZE	(8*1024*1024)	// 8MB

// Private Macro Definitions


// Private Data Types


// Global Variable Declarations
LogLevel	logLevel = DEFAULT_LOG_LEVEL;
char		logFileName[MAX_FILENAME] = "stdout";
FILE		*logFile = NULL;
unsigned int	maxLogFileSize = DEFAULT_LOGFILE_SIZE;
pthread_mutex_t	logMutex = PTHREAD_MUTEX_INITIALIZER;

const char *logLevelStr[] = {
	"FATAL",
	"ERROR",
	"WARNG",
	"DEBUG",
	"TRACE",
	"ALL",
};

// Private Function Prototypes


// Functions

static int __LoggerSetLevel(LogLevel pLogLevel)
{
	logLevel = pLogLevel;

	return 0;
}

static int __LoggerSetLogFile(const char *pLogFileName, unsigned int maxSize)
{
	// special case: stdout
	if (pLogFileName && strcasecmp(pLogFileName, "stdout") == 0) {
		pLogFileName = NULL;
	}

	if (!pLogFileName) {
		strcpy(logFileName, "stdout");
	} else {
		strncpy(logFileName, (char *)pLogFileName, MAX_FILENAME-1);
		logFileName[MAX_FILENAME-1] = '\0';
	}

	// close the old logfile first
	if (logFile && (logFile != stdout)) {
		fflush(logFile);
		fsync(fileno(logFile));
		fclose(logFile);
		logFile = stdout;
	}

	if (maxSize == 0) {
		maxSize = DEFAULT_LOGFILE_SIZE;
	} else if (maxSize < BUFSIZ) {
		maxSize = BUFSIZ;
	}

	if (pLogFileName && strlen(pLogFileName) > 0) {
		if ((logFile = fopen(pLogFileName, "w")) == NULL) {
			printf("Cannot open logfile: \"%s\"! Defaulting to stdout!!\n", pLogFileName);
			logFile = stdout;
			return -1;
		}
		fcntl(fileno(logFile), F_SETFD, FD_CLOEXEC);
		setvbuf(logFile, NULL, _IOFBF, BUFSIZ);
		maxLogFileSize = maxSize;
	}

	return 0;
}

int LoggerInit(LogLevel pLogLevel, const char *pLogFileName, unsigned int maxSize)
{
	int ret = 0;

#ifdef NO_DEBUG
	printf("Compiled in NO_DEBUG mode!\n");
#endif

	pthread_mutex_lock(&logMutex);

	ret = __LoggerSetLevel(pLogLevel);

	if (!ret) {
		ret = __LoggerSetLogFile(pLogFileName, maxSize);
	}

	pthread_mutex_unlock(&logMutex);

	return 0;
}

int LoggerGetLevel(void)
{
	int level;

	pthread_mutex_lock(&logMutex);
	level = logLevel;
	pthread_mutex_unlock(&logMutex);

	return level;
}

int LoggerSetLevel(LogLevel pLogLevel)
{
	int ret = 0;

	if (pLogLevel == logLevel) {
		return 0;
	}

	LOG(LEVEL_DEBUG, "Setting loglevel to: %d\n", pLogLevel);

	pthread_mutex_lock(&logMutex);
	ret = __LoggerSetLevel(pLogLevel);
	pthread_mutex_unlock(&logMutex);

	return ret;
}

const char *LoggerGetLogFile(void)
{
	char *file;

	pthread_mutex_lock(&logMutex);
	file = logFileName;
	pthread_mutex_unlock(&logMutex);

	return (strlen(file) > 0) ? file : "stdout";
}

int LoggerSetLogFile(const char *pLogFileName, unsigned int maxSize)
{
	int ret = 0;
	struct tm curtm;
	time_t curtime;
#ifndef NO_DEBUG
	char buf[128] = {0};
#endif

	LOG(LEVEL_DEBUG, "Setting logfile to: %s MaxLogFileSize=%u\n", pLogFileName, maxSize);

	pthread_mutex_lock(&logMutex);
	ret = __LoggerSetLogFile(pLogFileName, maxSize);
	pthread_mutex_unlock(&logMutex);

	/* Get the current time in seconds */
	time(&curtime);
	/* Break it down & store it in the structure tm */
	localtime_r(&curtime, &curtm);

	LOG(LEVEL_DEBUG, "New logfile: %s started at: %s\n", pLogFileName ? pLogFileName : "stdout", asctime_r(&curtm, buf));

	return ret;
}

// called under lock!
static void _logRotate(void)
{
	// rotate the logfile if maxLogFileSize reached
	if ((logFile != stdout) && (ftell(logFile) > maxLogFileSize)) {
		char buf[MAX_FILENAME] = {0};
		char *newFileName = strdup(logFileName);

		snprintf(buf, sizeof(buf)-1, "%s.1", logFileName);

		fprintf(logFile, "ROTATING: %s\n", buf);
		fflush(logFile);
		fsync(fileno(logFile));
		fclose(logFile);
		rename(logFileName, buf);
		logFile = stdout;

		__LoggerSetLogFile(newFileName, maxLogFileSize);

		free(newFileName);
	}
}

static pid_t gettid(void)
{
	return syscall(__NR_gettid);
}

int LoggerLog(LogLevel level, const char *file, const char *caller, const int line, const char *format, ...)
{
	struct tm curtm;
	struct timeval tv;
	struct timezone tz;
	char buf[17] = {0};
	int pid = getpid();
	int tid = gettid() - pid;
	va_list args;
	va_start(args, format);

	pthread_mutex_lock(&logMutex);

	// fast case: if level does not match, return immediately
	if (level > logLevel) {
		goto out;
	}

	/* Get the current time */
	gettimeofday(&tv, &tz);
	/* Break it down & store it in the structure tm */
	localtime_r(&tv.tv_sec, &curtm);

	snprintf(buf, sizeof(buf)-1, "%02d:%02d:%02d.%06ld", curtm.tm_hour, curtm.tm_min, curtm.tm_sec, tv.tv_usec);

	// initialize, if NULL
	if (!logFile) {
		logFile = stdout;
	}

	fprintf(logFile, "[%s][PID:%d:%02X][%s][%s:%s:%d] ", logLevelStr[level], pid, tid, buf, file, caller, line);
	vfprintf(logFile, format, args);
	fflush(logFile);

	// write the fatal logs to stdout additionally!
	if ((logFile != stdout) && (level == LEVEL_FATAL)) {
		fprintf(stdout, "[%s][PID:%d:%02X][%s][%s:%s:%d] ", logLevelStr[level], pid, tid, buf, file, caller, line);
		vfprintf(stdout, format, args);
		fflush(stdout);
	}

	_logRotate();

out:
	pthread_mutex_unlock(&logMutex);

	va_end(args);

	return 0;
}
