#ifndef _COMMON_H_
#define _COMMON_H_

// Std Includes
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>

// Local Includes
#include <logger.h>
#include <typeinfo>
#include <cstdio>
#include <cstdlib>

using namespace std;

// Global Constant Definitions
#define APP_NAME		"appName"
#define PROGRAM_VERSIYON_MAJOR	"1"
#define PROGRAM_VERSIYON_MINOR	"0"
#define PROGRAM_VERSIYON_BUILD	"0"
#define PROGRAM_VERSIYON	PROGRAM_VERSIYON_MAJOR "." PROGRAM_VERSIYON_MINOR
#define PROGRAM_VERSIYON_FULL	PROGRAM_VERSIYON "." PROGRAM_VERSIYON_BUILD
#define BUILD_TIME		__DATE__ " " __TIME__

#ifndef TRUE
#define TRUE	true
#endif
#ifndef FALSE
#define FALSE	false
#endif

#ifndef STR
#define STRx(x) #x
#define STR(x) STRx(x)
#endif

// Global Macro Definitions
#define DEFINE_CLASSNAME(_x)	virtual const char *_NAME() { return _x; }
#define CLASSNAME(_x)		((_x)->_NAME())

#ifndef FOREACH
#define FOREACH(_T, _c, _i) for (_T::iterator _i = _c.begin(); _i != _c.end(); ++_i)
#endif

#ifndef UNUSED
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

#ifndef MAX
#define MAX(_x, _y)	((_x) > (_y) ? (_x) : (_y))
#endif

#ifndef MIN
#define MIN(_x, _y)	((_x) < (_y) ? (_x) : (_y))
#endif

#ifndef TEST_BIT
#define TEST_BIT(_x, _y)	((_x) & (1 << (_y)))
#endif

/* structure packing */
#if defined(__GNUC__)
#define PACKED  __attribute__((packed))
#else
#pragma pack(1)
#define PACKED
#endif

#ifndef	PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif

#define ASSERT(exp, msg...) do {	\
	if (!(exp)) {	\
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Assertion failed at: %s:%s:%d\n", __FILE__, __func__, __LINE__);	\
		FATAL msg;	\
		/*exit(0);*/	\
	}	\
} while (0)

#define DEFINE_TIMER(_loglevel, _name)	CodeTimer _name(_loglevel, STR(_name))
#define START_TIMER(_name)	do { \
					_name.start(); \
				} while (0)

#define CHECK_TIMER(_name)	do { \
					_name.checkPoint(__func__, __LINE__); \
				} while (0)

// Global Data Types
class CodeTimer
{
	typedef unsigned long long timestamp_t;

	private:
		LogLevel logLevel;
		const char *name;
		timestamp_t t0;
		inline timestamp_t getTimestamp(void) {
			struct timeval now;
			gettimeofday(&now, NULL);
			return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
		}
	public:
		CodeTimer(LogLevel pLoglevel, const char *pName) {
			logLevel = pLoglevel;
			name = pName;
		}
		void start(void) {
			t0 = getTimestamp();
		}
		void checkPoint(const char *func, const int line) {
			timestamp_t t1 = getTimestamp();
			UNUSED(t1);
			LoggerLog(logLevel, name, func, line, "%s: Time elapsed: %Lf msecs\n", name, (t1 - t0) / 1000.0L);
		}
};

typedef enum {
	GENEL_DONANIM_HATASI		= 0,
	GENEL_YAZILIM_HATASI		= 1,
	FRAM_OKUMA_YAZMA_HATASI		= 2,
	ARACID_OKUMA_YAZMA_HATASI	= 3,
	RF_READER_HATASI		= 4,
	SAM_HATASI			= 5,
	MANYETIK_READER_HATASI		= 6,
	RTC_HATASI			= 7,
	SD_CARD_HATASI			= 8,
	GPS_MODUL_HATASI		= 9,
	WIFI_MODUL_HATASI		= 10,
	SES_HATASI			= 11,
	FLASH_HATASI			= 12,
	EEPROM_HATASI			= 13,
	ETOPUP_DOSYA_HATASI		= 14,
	BARCODE_READER_HATASI		= 15,
	DIE_TEMPERATURE_HATASI		= 16,
	KAPANIS_ACILIS_LOGLARI		= 17,
	BATARYA_HATASI			= 18,
	KONTAK_HAREKETI			= 19,
	AKTIVITE_DOSYALAMA		= 20,
	SUNUCU_BAGLANTI			= 21
} FatalErrorCode;

#define FATAL_LOG(_code, _arg...)	do { FATAL(_arg); fatal_log(_code, _arg); } while (0)
#define FATAL_LOG_SILENT(_code, _arg...)	do { fatal_log(_code, _arg); } while (0)

#define FATAL_LOG_FILE "fatal.log"
extern pthread_mutex_t fatalMutex;
void fatal_log(FatalErrorCode code, const char *format, ...);

// Global Variable Externs


// Global Function Prototypes
static inline pid_t gettid(void)
{
	return syscall(__NR_gettid);
}

#endif	//_COMMON_H_
