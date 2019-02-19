#ifndef _UTILITY_H_
#define _UTILITY_H_

// Std Includes
#include <string>
#include <stdlib.h>
#include <sys/time.h>
#include <list>

// Local Includes
#include <common.h>

// Global Constant Definitions


// Global Macro Definitions
#define HEXDUMP(_level, _addr, _buf, _len)	\
	do {	\
		char *_hexdumpbuf = (char *)calloc(1, _len * 2 + 1);	\
		Utils->bin2hexstr(_buf, _hexdumpbuf, _len);	\
		LOG(_level, "addr=%p buffer[%d]=%s\n", _addr, _len, _hexdumpbuf);	\
		free(_hexdumpbuf);	\
	} while (0)

#define HEXDUMP_STR(_level, _str, _buf, _len)	\
	do {	\
		char *_hexdumpbuf = (char *)calloc(1, _len * 2 + 1);	\
		Utils->bin2hexstr(_buf, _hexdumpbuf, _len);	\
		LOG(_level, "%s: [%d]=%s\n", _str, _len, _hexdumpbuf);	\
		free(_hexdumpbuf);	\
	} while (0)

// Global Data Types

class Utility
{
	public:
		DEFINE_CLASSNAME("Utility");
		Utility(void);
		virtual ~Utility(void);
		int usage;
		static void create(void);
		static void destroy(void);
		string trim(const string &str, string trimChars);
		bool fileExists(const char *filename);
		bool ifaceExists(const char *ifname);
		int hex2num(char c);
		int hex2byte(const char *hex);
		int hexstr2bin(const char *hex, unsigned char *buf, unsigned int len);
		int bin2hexstr(const unsigned char *buf, char *hex, unsigned int len);
		string hexstr2bin(const string hex);
		string bin2hexstr(const string str);
		unsigned int getRandomUint(unsigned int min, unsigned int max);
		unsigned int crc32(unsigned char *buf, size_t size);
		string getCurrentTimestamp(void);
		string getCurrentTimeStr(const char *format);
		struct tm getCurrentTM(void);
		string buf2date(const unsigned char date[6], bool sadeceTarih = FALSE);
		unsigned short swapEndian(unsigned short val);
		unsigned int swapEndian(unsigned int val);
		unsigned long long swapEndian(unsigned long long val);
		int getIfaceMac(const char *ifname, unsigned char *mac);
		string getIfaceMacStr(const char *ifname);
		int setIfaceMac(const char *ifname, const unsigned char *mac);
		string getIfaceIP(const char *ifname);
		int strToInt(string str);
		string intToStr(int i);
		int timeElapsedSecs(unsigned char date[6]);
		int timeElapsedSecs(struct timeval *tv);
		void kartTarihToSistemTarih(unsigned int kartTarih, unsigned char sistemTarih[6]);
		void unixTimeToSistemTarih(time_t unixTime, unsigned char sistemTarih[6]);
		unsigned int sistemTarihToKartTarih(const unsigned char sistemTarih[6]);
		unsigned int countSubstr(const string str, const string sub);
		unsigned int turkceKarakterSayisi(string s);
		void replaceStringInPlace(string &subject, const string search, const string replace);
		string replaceString(string subject, const string search, const string replace);
		void turkceKarakterTemizle(string &s);
		bool checkIfacePing(const char *ifname, const char *addr);
		bool checkConnection(const char *addr);
		bool tcpConnectSocket(int *sockHandle, const char *address, int port);
		int httpPost(const char *address, int port, const char *data, int len, char *respBuf, int respBufLen, int timeout = 20000);
		list<string> getFileListInDir(string path);
		bool isFolder(const char *file);
		int mountSDCard(void);
		int umountSDCard(void);
		bool isSDCardMount(void);
		int getFileSize(const char *filename);
		string getFileMD5(const char *filename);
		bool createFolders(string path);
		bool copyFile(const char *srcFileName, const char *dstFileName);
		bool strend(const char *str, const char *match);
		int getFileCountInDir(const char *path, const char *extension = NULL);
		void copyString(char *dst, const char *src, unsigned int dstlen);
		string base64_encode_file(const char *filename);
		string base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
		string urlencode(const char *src);
		string getSDCapacity(void);
};


// Global Variable Externs
extern Utility *Utils;

// Global Function Prototypes


#endif	//_UTILITY_H_
