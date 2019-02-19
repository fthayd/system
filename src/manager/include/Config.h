#ifndef _CONFIG_H_
#define _CONFIG_H_

// Std Includes
#include <map>
#include <string>
#include <set>

// Local Includes
#include <common.h>

// Global Constant Definitions
#define ETH_IFNAME		"eth0"

// Global Macro Definitions


// Global Data Types
typedef enum {
	FLAG_NO_ERROR 			= 0,
	FLAG_MANYETIK_BILET_SIKISMASI 	= 1,
	FLAG_SD_KART_HATASI 		= 2,
	FLAG_FRAM_HATASI 		= 3,
	FLAG_ARACID_CHIP_HATASI 	= 4,
	FLAG_RF_READER_HATASI 		= 5,
	FLAG_SAM_HATASI 		= 6,
	FLAG_RTC_HATASI 		= 7,
	FLAG_BATARYA_TAKILI_DEGIL 	= 8,
	FLAG_MANYETIK_READER_HATASI 	= 9,
	FLAG_GPS_MODUL_HATASI 		= 10,
	FLAG_SD_BACKUP_KART_HATASI 	= 11,
	FLAG_CIHAZ_KULLANIM_DISI_HATASI = 12
} ErrorFlag;

typedef struct { int x, y; } Point;
typedef map<string, string> SSmap;
typedef set<ErrorFlag> ErrorList;

#ifdef YUZDE_INDIRIM_UYGULA
typedef map<string, string> IndMap;
#endif

class Config
{
	private:
		SSmap config_map;
		SSmap aracid_map;
		SSmap versiyonlar_map;
		SSmap GPSInfoMap;
#ifdef YUZDE_INDIRIM_UYGULA
		IndMap indTerife_map;
		int loadIndTarife(void);
#endif
		int topUpClear;
		bool serverConnection;
		ErrorList errList;
		int loadConfig(void);
		int loadConfigINI(const char *filename, SSmap *pMap);
		int saveConfigINI(const char *filename, SSmap *pMap);
		void updateVersionFile(void);
		void updateServerIP(void);
	public:
		DEFINE_CLASSNAME("Config");
		Config(void);
		virtual ~Config(void);
		int usage;
		static void create(void);
		static void destroy(void);
		int saveConfig(void);
		int reloadConfig(void);
		int freeConfig(void);
		string configGet(string key, SSmap **pMap = NULL);
		int configGetInt(string key);
		int configSet(string key, string value);
		int configSet(string key, int intValue);
		int save(void);
		string getMacAddress(void);
		string getMacAddress2(void);
		void getMacAddress(char *bin);
		string getSerialNo(void);
		string getGPSInfo(string key);
		void setTopUpClear(int val);
		int getTopUpClear(void);
		void setServerConnected(bool state);
		bool isServerConnected(void);
		void addErrorFlag(ErrorFlag flag);
		void removeErrorFlag(ErrorFlag flag);
		string getErrorStatus(void);
		bool errorFlagIsSet(ErrorFlag flag);
};


// Global Variable Externs
extern Config *Cfg;

// Global Function Prototypes


#endif	//_CONFIG_H_
