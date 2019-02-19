// Std Includes
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mount.h>
//#include <openssl/md5.h>
#include <pthread.h>

// Local Includes
#include <Utility.h>

// Private Constant Definitions
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
				'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
				'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
				'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
				'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
				'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
				'w', 'x', 'y', 'z', '0', '1', '2', '3',
				'4', '5', '6', '7', '8', '9', '+', '/'};
static int mod_table[] = {0, 2, 1};

#define SD_INFO_FILE		"/tmp/sdcard.info"
#define SD_BILGI_KOMUTU		"df -h | grep mmcblk0p1 > "SD_INFO_FILE

// Private Macro Definitions
#define SD_CARD_LOCK()		pthread_mutex_lock(&sdMutex)
#define SD_CARD_UNLOCK()	pthread_mutex_unlock(&sdMutex)

// Private Data Types


// Global Variable Declarations
Utility *Utils = NULL;
pthread_mutex_t sdMutex = PTHREAD_MUTEX_INITIALIZER;

// Private Function Prototypes


// Functions

Utility::Utility(void)
{
	usage = 0;
}

Utility::~Utility(void)
{
	TRACE("Destructor\n");

	ASSERT((usage == 0), ("Usage not zero!!! %d\n", usage));
}

void Utility::create(void)
{
	if (!Utils) {
		Utils = new Utility();
	}

	Utils->usage++;

	TRACE("usage=%d\n", Utils->usage);
}

void Utility::destroy(void)
{
	if (Utils) {
		Utils->usage--;
		TRACE("usage=%d\n", Utils->usage);
		if (Utils->usage == 0) {
			delete Utils;
			Utils = NULL;
		}
	} else {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Utility fazladan destroy!\n");
	}
}

string Utility::trim(const string &str, string trimChars)
{
	string::size_type first = str.find_first_not_of(trimChars);
	string::size_type last = str.find_last_not_of(trimChars);

	if (first == string::npos) return string();
	if (last == string::npos) return string();

	return str.substr(first, last - first + 1);
}

bool Utility::fileExists(const char *filename)
{
	FILE *file = NULL;

	if ((file = fopen(filename, "r")) != NULL) {
		fclose(file);
		return TRUE;
	}

	return FALSE;
}

bool Utility::ifaceExists(const char *ifname)
{
	int sd;
	struct ifreq ifr;
	bool ret = TRUE;

	if (!ifname) {
		LOG(LEVEL_WARNING, "ifname NULL\n");
		return FALSE;
	}

	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "cannot open socket for %s()!\n", __func__);
		return FALSE;
	}

	memset(&ifr, 0x0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	if (ioctl(sd, SIOCGIFFLAGS, &ifr) < 0) {
		ret = FALSE;
	}

	close(sd);

	return ret;
}

int Utility::hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int Utility::hex2byte(const char *hex)
{
	int a, b;

	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int Utility::hexstr2bin(const char *hex, unsigned char *buf, unsigned int len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	unsigned char *opos = buf;
	unsigned int hexlen = strlen(hex);

	if (hexlen < (len * 2)) {
		len = hexlen / 2;
	}

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = (unsigned char)a;
		ipos += 2;
	}
	return 0;
}

int Utility::bin2hexstr(const unsigned char *buf, char *hex, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		sprintf(hex + (2 * i), "%02X", buf[i]);
	}

	return 0;
}

string Utility::hexstr2bin(const string hex)
{
	unsigned int len = hex.length() / 2;
	unsigned char *buf = (unsigned char *)calloc(1, len + 1);
	string ret;

	hexstr2bin(hex.c_str(), buf, len);

	ret = string((char *)buf, len);

	free(buf);

	return ret;
}

string Utility::bin2hexstr(const string str)
{
	unsigned int len = str.length();
	char *buf = (char *)calloc(1, len * 2 + 1);
	string ret;

	bin2hexstr((const unsigned char *)str.c_str(), buf, len);

	ret = string(buf);

	free(buf);

	return ret;
}

/* get random unsigned interger value between [min, max] (inclusive) */
unsigned int Utility::getRandomUint(unsigned int min, unsigned int max)
{
	unsigned int random_int;
	int urandom_fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);

	if (urandom_fd < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "cannot open urandom file! FATAL!\n");
		return 0;
	}

	if (read(urandom_fd, &random_int, sizeof(random_int)) != sizeof(random_int)) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "cannot read urandom file! FATAL!\n");
		close(urandom_fd);
		return 0;
	}
	close(urandom_fd);

	if (min >= max) {
		return min;
	}

	return min + (random_int % (max + 1 - min));
}

unsigned int Utility::crc32(unsigned char *buf, size_t size)
{
	static unsigned int crc32_tab[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};
	unsigned int crc = 0;

	for (unsigned int i = 0; i < size; i++) {
		crc = crc32_tab[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
	}

	return crc;
}

string Utility::getCurrentTimestamp(void)
{
	struct tm tmTime;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tmTime);
	char temp[12 + 1] = {0};
	unsigned char buf[7] = {0};

	sprintf(temp, "%02X%02X%02X%02X%02X%02X", tmTime.tm_year % 100, tmTime.tm_mon + 1, tmTime.tm_mday,
		tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);

	hexstr2bin(temp, buf, 6);

	return string((char *)buf, 6);
}

string Utility::getCurrentTimeStr(const char *format)
{
	time_t curtime;
	struct tm curtm;
	char buf[256] = {0};

	/* Get the current time in seconds */
	time(&curtime);
	/* Break it down & store it in the structure tm */
	localtime_r(&curtime, &curtm);

	strftime(buf, sizeof(buf)-1, format, &curtm);

	return string(buf);
}

struct tm Utility::getCurrentTM(void)
{
	time_t curtime;
	struct tm timeinfo;

	/* Get the current time in seconds */
	time(&curtime);
	localtime_r(&curtime, &timeinfo);

	return timeinfo;
}

string Utility::buf2date(const unsigned char date[6], bool sadeceTarih)
{
	int yil = (date[0] & 0xFF) + 2000;
	int ay = (date[1] & 0xFF);
	int gun = (date[2] & 0xFF);
	int saat = (date[3] & 0xFF);
	int dak = (date[4] & 0xFF);
	int san = (date[5] & 0xFF);

	ostringstream stm;
	stm << setfill('0');

	if (sadeceTarih) {
		stm << setw(2) << gun << "." << setw(2) << ay << "." << setw(4) << yil;
	} else {
		stm << setw(2) << gun << "." << setw(2) << ay << "." << setw(4) << yil << " "
			<< setw(2) << saat << ":" << setw(2) << dak << ":" << setw(2) << san;
	}

	return stm.str();
}

unsigned short Utility::swapEndian(unsigned short val)
{
	return (val >> 8) | (val << 8);
}

unsigned int Utility::swapEndian(unsigned int val)
{
	return ((val >> 24) & 0xff) | // move byte 3 to byte 0
		((val << 8) & 0xff0000) | // move byte 1 to byte 2
		((val >> 8) & 0xff00) | // move byte 2 to byte 1
		((val << 24) & 0xff000000); // move byte 0 to byte 3
}

unsigned long long Utility::swapEndian(unsigned long long val)
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | (val >> 32);
}

int Utility::getIfaceMac(const char *ifname, unsigned char *mac)
{
	int err;
	struct ifreq ifr;
	int skfd = -1;

	memset(mac, 0 , 6);	// 6 byte ethernet MAC

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (skfd < 0) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ - 1, "%s", ifname);

	err = ioctl(skfd, SIOCGIFHWADDR, &ifr);

	close(skfd);

	if (!err) {
		memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);	// 6 byte ethernet MAC
	}

	return err;
}

string Utility::getIfaceMacStr(const char *ifname)
{
	int err;
	struct ifreq ifr;
	int skfd = -1;
	char buf[18] = {0};

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (skfd < 0) {
		return "00:00:00:00:00:00";
	}

	memset(&ifr, 0, sizeof(ifr));
	sprintf(ifr.ifr_name, "%s", ifname);

	err = ioctl(skfd, SIOCGIFHWADDR, &ifr);
	if (err) {
		return "00:00:00:00:00:00";
	}

	close(skfd);

	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
		(unsigned char)ifr.ifr_hwaddr.sa_data[0],
		(unsigned char)ifr.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr.ifr_hwaddr.sa_data[2],
		(unsigned char)ifr.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	return string(buf);
}

int Utility::setIfaceMac(const char *ifname, const unsigned char *mac)
{
	int err;
	struct ifreq ifr;
	int skfd = -1;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (skfd < 0) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ - 1, "%s", ifname);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	memcpy(ifr.ifr_hwaddr.sa_data, mac, 6);

	err = ioctl(skfd, SIOCSIFHWADDR, &ifr);

	close(skfd);

	return err;
}

string Utility::getIfaceIP(const char *ifname)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *sin;
	struct in_addr *act;
	int err;

	memset(&ifr, 0, sizeof(ifr));

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0) {
		return string("0.0.0.0");
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

	// is iface up?
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);

	if (err) {
		close(fd);
		return string("0.0.0.0");
	}

	if (!(ifr.ifr_flags & IFF_UP)) {
		close(fd);
		return string("0.0.0.0");
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

	err = ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	if (err) {
		return string("0.0.0.0");
	}

	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	act = &(sin->sin_addr);

	return string(inet_ntoa(*act));
}

int Utility::strToInt(string str)
{
	int ret = 0;

	istringstream(str) >> ret;

	return ret;
}

string Utility::intToStr(int i)
{
	ostringstream stm;

	stm << i;

	return stm.str();
}

int Utility::timeElapsedSecs(unsigned char date[6])
{
	time_t curtime, time1;
	struct tm timeinfo;
	int yil = (date[0] & 0xFF) + 2000;
	int ay = (date[1] & 0xFF);
	int gun = (date[2] & 0xFF);
	int saat = (date[3] & 0xFF);
	int dak = (date[4] & 0xFF);
	int san = (date[5] & 0xFF);

	/* Get the current time in seconds */
	time(&curtime);

	time(&time1);
	localtime_r(&time1, &timeinfo);
	timeinfo.tm_year = yil - 1900;
	timeinfo.tm_mon = ay - 1;
	timeinfo.tm_mday = gun;
	timeinfo.tm_sec = san;
	timeinfo.tm_min = dak;
	timeinfo.tm_hour = saat;

	time1 = mktime(&timeinfo);

	//TRACE("curtime=%u time1=%u\n", curtime, time1);

	// 2038 yilindan sonraki tarihler time_t structure'ina sigmadigi icin sorun cikiyor
	if (time1 == -1) {
		return -1;
	}

	double diff = difftime(curtime, time1);

	return (int)diff;
}

int Utility::timeElapsedSecs(struct timeval *tv)
{
	struct timeval now;

	gettimeofday(&now, NULL);

	return (now.tv_sec - tv->tv_sec) + ((now.tv_usec - tv->tv_usec) / 1000000);
}

void Utility::kartTarihToSistemTarih(unsigned int kartTarih, unsigned char sistemTarih[6])
{
	sistemTarih[0] = (kartTarih >> 20) & 0x0000007F;
	sistemTarih[1] = (kartTarih >> 16) & 0x0000000F;
	sistemTarih[2] = (kartTarih >> 11) & 0x0000001F;
	sistemTarih[3] = (kartTarih >> 6) & 0x0000001F;
	sistemTarih[4] = (kartTarih) & 0x0000003F;
	sistemTarih[5] = 0;
}

unsigned int Utility::sistemTarihToKartTarih(const unsigned char sistemTarih[6])
{
	char buf[4];
	unsigned int ret;

	buf[3] = (sistemTarih[0] >> 4) & 0x07;
	buf[2] = ((sistemTarih[0] << 4) | (sistemTarih[1]));
	buf[1] = ((sistemTarih[2] << 3) & 0xF8) | ((sistemTarih[3] >> 2) &0x07);
	buf[0] = ((sistemTarih[3] & 0x03) << 6) | (sistemTarih[4] & 0x3F);

	memcpy(&ret, buf, sizeof(int));

	return ret;
}

void Utility::unixTimeToSistemTarih(time_t unixTime, unsigned char sistemTarih[6])
{
	struct tm *tmTime = gmtime(&unixTime);
	char temp[12 + 1] = {0};

	sprintf(temp, "%02X%02X%02X%02X%02X%02X", tmTime->tm_year % 100, tmTime->tm_mon + 1, tmTime->tm_mday,
		tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);

	hexstr2bin(temp, sistemTarih, 6);
}

unsigned int Utility::countSubstr(const string str, const string sub)
{
	unsigned int count = 0;

	if ((sub.length() == 0) || (str.length() == 0)) {
		return 0;
	}

	for (unsigned int offset = str.find(sub); offset != string::npos; offset = str.find(sub, offset + sub.length())) {
		count++;
	}

	return count;
}

unsigned int Utility::turkceKarakterSayisi(const string s)
{
	unsigned int count = 0;

	count += countSubstr(s, "ö");
	count += countSubstr(s, "Ö");
	count += countSubstr(s, "ç");
	count += countSubstr(s, "Ç");
	count += countSubstr(s, "ı");
	count += countSubstr(s, "İ");
	count += countSubstr(s, "ş");
	count += countSubstr(s, "Ş");
	count += countSubstr(s, "ğ");
	count += countSubstr(s, "Ğ");
	count += countSubstr(s, "ü");
	count += countSubstr(s, "Ü");

	return count;
}

void Utility::replaceStringInPlace(string &subject, const string search, const string replace)
{
	size_t pos = 0;

	while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

string Utility::replaceString(string subject, const string search, const string replace)
{
	size_t pos = 0;

	while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}

	return subject;
}

void Utility::turkceKarakterTemizle(string &s)
{
	replaceStringInPlace(s, "ö", "o");
	replaceStringInPlace(s, "Ö", "O");
	replaceStringInPlace(s, "ç", "c");
	replaceStringInPlace(s, "Ç", "C");
	replaceStringInPlace(s, "ı", "i");
	replaceStringInPlace(s, "İ", "I");
	replaceStringInPlace(s, "ş", "s");
	replaceStringInPlace(s, "Ş", "S");
	replaceStringInPlace(s, "ğ", "g");
	replaceStringInPlace(s, "Ğ", "G");
	replaceStringInPlace(s, "ü", "u");
	replaceStringInPlace(s, "Ü", "U");
}

bool Utility::checkIfacePing(const char *ifname, const char *addr)
{
	char cmd[128] = {0};

	snprintf(cmd, sizeof(cmd) - 1, "ping -c1 -W3 -s1000 -I %s %s 1>/dev/null 2>&1", ifname, addr);

	for (int i = 0; i < 3; i++) {
		if (system(cmd) == 0) {
			return TRUE;
		}
	}

	TRACE("ping failed!\n");

	return FALSE;
}

bool Utility::checkConnection(const char *addr)
{
	char cmd[128] = {0};
	int sock;

	snprintf(cmd, sizeof(cmd) - 1, "ping -c1 -W3 -s1000 %s 1>/dev/null 2>&1", addr);

	for (int i = 0; i < 3; i++) {
		if (system(cmd) == 0) {
			return TRUE;
		}
	}

	TRACE("ping failed! Checking TCP connection\n");

	if (tcpConnectSocket(&sock, addr, 80)) {
		TRACE("TCP connection success\n");
		close(sock);
		return TRUE;
	}

	return FALSE;
}

bool Utility::tcpConnectSocket(int *sockHandle, const char *address, int port)
{
#define TCP_RETRY	3
#define TCP_TIMEOUT	2
	struct hostent *he;
	int flags, res, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tv;
	int i = 0;
	sockaddr_in addr;
	int sock;

	if ((he = gethostbyname(address)) == 0) {
		LOG(LEVEL_ERROR, "gethostbyname error! address=%s\n", address);
		return FALSE;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((in_addr *) he->h_addr);
	memset(&(addr.sin_zero), 0, 8);

	for (i = 0; i < TCP_RETRY; i++) {
		TRACE("retry=%d\n", i);
		error = 0;
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			LOG(LEVEL_ERROR, "cannot open stream socket!\n");
			return FALSE;
		}
		flags = fcntl(sock, F_GETFL, 0);
		fcntl(sock, F_SETFL, flags | O_NONBLOCK | O_CLOEXEC);
		if ((res = connect(sock, (struct sockaddr *) &addr, sizeof (addr))) < 0) {
			if (errno != EINPROGRESS && errno != EALREADY) {
				close(sock);
				usleep(50000);
				continue;
			}
		}

		// Bağlantı çok hızlı gerçekleşmiş, nadir olabilecek bir durum
		// Bu durumda selecte gelmeden buradan çıkabiliriz
		if (res == 0) {
			fcntl(sock, F_SETFL, flags);
			*sockHandle = sock;
			TRACE("Connected!\n");
			return TRUE;
		}

		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		wset = rset;
		tv.tv_sec = TCP_TIMEOUT;
		tv.tv_usec = 0;

		if ((res = select(sock + 1, &rset, &wset, NULL, &tv)) == 0) {
			// Select'ten 0 dönmüş == timeout
			// tekrar deneyelim, onceki baglantiyi kapatalim
			close(sock);
			usleep(50000);
			continue;
		}

		if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)) {
			// Bağlantı tamam, porta bilgi geldi
			// Ya da connect hatalı bir duruma düştü
			// O yüzden socket readable ve writeable olarak set edildi
			// sockopt ile error varsa alalım
			len = sizeof (error);
			if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
				close(sock);
				usleep(50000);
				continue;
			}

			// Connect() hataya mı düşmüş
			if (error) {
				//errno = error;
				//perror("connect");
				close(sock);
				usleep(50000);
				continue;
			}

			// Buraya geldiysek her şey ok demektir.
			fcntl(sock, F_SETFL, flags);
			*sockHandle = sock;
			TRACE("Connected!\n");
			return TRUE;
		}
	}

	LOG(LEVEL_ERROR, "cannot connect to tcp socket!\n");

	return FALSE;
}

int Utility::httpPost(const char *address, int port, const char *data, int len, char *respBuf, int respBufLen, int timeout)
{
	int sock;
	int result = -1;

	if (!tcpConnectSocket(&sock, address, port)) {
		LOG(LEVEL_ERROR, "Cannot open socket. address=%s port=%d\n\n", address, port);
		return -2;
	}

	memset(respBuf, 0, respBufLen);

	TRACE("Sending %d bytes to %s\n", len, address);

	//HEXDUMP_STR(LEVEL_TRACE, "DATA", data, len);

	if (send(sock, data, len, 0) != len) {
		LOG(LEVEL_ERROR, "send returned %d\n", len);
		close(sock);
		return -3;
	}

	int recvd, total = 0;
	struct timeval begin, now;
	int timediff;

	// make socket non blocking
	fcntl(sock, F_SETFL, O_NONBLOCK);

	//beginning time
	gettimeofday(&begin, NULL);

	while (respBufLen > total) {
		gettimeofday(&now, NULL);

		// time elapsed in msecs
		timediff = (now.tv_sec - begin.tv_sec) * 1000 + ((now.tv_usec - begin.tv_usec) / 1000);

		//TRACE("timediff=%d timeout=%d\n", timediff, timeout);

		// break after timeout
		if (timeout && (timediff > timeout)) {
			TRACE("Timeout occured! total=%d\n", total);
			result = -4;
			break;
		}

		if ((recvd =  recv(sock, &respBuf[total], respBufLen - total, 0)) < 0) {
			if (errno == EAGAIN) {
				// if nothing was received then we want to wait a little before trying again, 10 ms
				usleep(10000);
				continue;
			}
			break;
		} else {
			if (recvd == 0) {
				result = -5;
				break;
			}

			TRACE("recvd=%d total=%d\n", recvd, total);
			total += recvd;
		}

		usleep(5000);
	}

	if (total > 0) {
		result = 0;
	}

	close(sock);

	TRACE("Received %d bytes response\n", total);

	return result;
}

list<string> Utility::getFileListInDir(string path)
{
	list<string> files;
	DIR *dir = opendir(path.c_str());
	struct dirent *d;

	if (!dir) {
		return files;
	}

	while ((d = readdir(dir))) {
		if (!isFolder(d->d_name)) {
			files.push_back(d->d_name);
		}
	}

	closedir(dir);

	return files;
}

bool Utility::isFolder(const char *file)
{
	struct stat fstat;

	if (stat(file, &fstat) != 0) return false;

	return (fstat.st_mode & S_IFDIR) ? true : false;
}

int Utility::mountSDCard(void)
{
	int ret = 0;

	SD_CARD_LOCK();

	for (int retry = 0; retry < 3; retry++) {
		if (isSDCardMount()) {
			SD_CARD_UNLOCK();
			return 0;
		}

		TRACE("MOUNT SDCARD retry=%d\n", retry);

		ret = mount("/dev/mmcblk0p1", "/sdcard", "vfat", MS_MGC_VAL | MS_SYNCHRONOUS | MS_NODEV, "");

		if (ret == 0) {
			SD_CARD_UNLOCK();
			return ret;
		}

		// hata durumu
		ret = -errno;

		usleep(200000);
	}

	SD_CARD_UNLOCK();

	return ret;
}

int Utility::umountSDCard(void)
{
	int ret = 0;

	TRACE("UMOUNT SDCARD\n");

	SD_CARD_LOCK();

	ret = umount2("/sdcard", MNT_DETACH);

	if (ret < 0) {
		switch (errno) {
			case EINVAL:
				ret = -1;
				break;
			case EBUSY:
				ret = -2;
				break;
			default:
				break;
		}
	}

	SD_CARD_UNLOCK();

	return ret;
}

bool Utility::isSDCardMount(void)
{
	FILE *f;
	char sdcardMountAdeti[3] = {0};
	int MountAdeti = 0;

	if ((f = popen("mount | grep /dev/mmcblk0p1 | grep /sdcard | wc -l", "r")) != NULL) {
		if (fgets(sdcardMountAdeti, sizeof(sdcardMountAdeti) - 1, f) != NULL) {
			MountAdeti = atoi(sdcardMountAdeti);
		}
		pclose(f);
	} else {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "popen error=%d\n", errno);
	}

	if (MountAdeti >= 1) {
		TRACE("SD CARD mounted\n");
		return TRUE;
	}

	TRACE("SD CARD not mounted\n");
	return FALSE;
}

int Utility::getFileSize(const char *filename)
{
	struct stat fstat;

	if (stat(filename, &fstat) < 0) {
		return -1;
	}

	return fstat.st_size;
}


bool Utility::createFolders(string path)
{
	char cmdbuf[512] = {0};

	snprintf(cmdbuf, sizeof(cmdbuf) - 1, "mkdir -p %s", path.c_str());

	return (system(cmdbuf) == 0);
}

bool Utility::copyFile(const char *srcFileName, const char *dstFileName)
{
	char buf[BUFSIZ];
	size_t size;
	int temp;
	bool ret = TRUE;

	int source = open(srcFileName, O_RDONLY | O_CLOEXEC, 0);
	if (source < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Cannot open src %s errno=%d\n", srcFileName, errno);
		return FALSE;
	}

	int dest = open(dstFileName, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_SYNC, 0644);
	if (dest < 0) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Cannot open dst %s errno=%d\n", dstFileName, errno);
		close(source);
		return FALSE;
	}

	while ((size = read(source, buf, BUFSIZ)) > 0) {
		temp = write(dest, buf, size);
		if (temp != (int)size) {
			ret = FALSE;
			FATAL_LOG(GENEL_YAZILIM_HATASI, "Dosya yazma hatasi! dest=%s size=%d temp=%d errno=%d\n", dstFileName, size, temp, errno);
			goto out;
		}
	}

out:
	close(source);
	close(dest);

	return ret;
}

bool Utility::strend(const char *str, const char *match)
{
	size_t slen = strlen(str);
	size_t tlen = strlen(match);

	if (slen < tlen) {
		return FALSE;
	}

	if (strcasecmp(str + (slen - tlen), match) == 0) {
		return TRUE;
	}

	return FALSE;
}

int Utility::getFileCountInDir(const char *path, const char *extension)
{
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	if ((dirp = opendir(path)) == NULL) {
		return -1;
	}

	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) { /* If the entry is a regular file */
			if (!extension || strend(entry->d_name, extension)) {
				file_count++;
			}
		}
	}

	closedir(dirp);

	return file_count;
}

void Utility::copyString(char *dst, const char *src, unsigned int dstlen)
{
	memset(dst, 0, dstlen);
	strncpy(dst, src, dstlen - 1);
}

string Utility::base64_encode_file(const char *filename)
{
	int fileSize = getFileSize(filename);
	unsigned char data[fileSize];
	size_t output_len = 0;
	FILE *inFile;
	string ret;

	if ((inFile = fopen(filename, "rb")) == NULL) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Error opening %s\n", filename);
		return "";
	}
	fcntl(fileno(inFile), F_SETFD, FD_CLOEXEC);

	int bytes = fread(data, 1, fileSize, inFile);

	if (bytes != fileSize) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Error reading %s. bytes=%d fileSize=%d\n", filename, bytes, fileSize);
		return "";
	}

	ret = base64_encode(data, fileSize, &output_len);

	DEBUG("%s output_len=%d\n", ret.c_str(), output_len);

	return ret;
}

string Utility::base64_encode(const unsigned char *data, size_t input_length, size_t *output_length)
{
	string ret;
	*output_length = 4 * ((input_length + 2) / 3);

	char *encoded_data = (char *)malloc(*output_length + 1);

	if (encoded_data == NULL) return NULL;

	for (unsigned int i = 0, j = 0; i < input_length;) {
		uint32_t octet_a = i < input_length ? data[i++] : 0;
		uint32_t octet_b = i < input_length ? data[i++] : 0;
		uint32_t octet_c = i < input_length ? data[i++] : 0;
		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++) {
		encoded_data[*output_length - 1 - i] = '=';
	}

	encoded_data[*output_length] = '\0';

	ret = string(encoded_data);

	free(encoded_data);

	return ret;
}

string Utility::urlencode(const char *src)
{
	char dest[1024] = {0};
	char *d = dest;

	for (int i = 0; src[i]; i++) {
		if (isalnum(src[i])) {
			*(d++) = src[i];
		} else {
			sprintf(d, "%%%02X", src[i]);
			d += 3;
		}
	}

	return string(dest);
}

string Utility::getSDCapacity(void)
{
	FILE *sdInfo = NULL;
	char buf[1024] = {0};
	char *pch, *ctx;
	bool mounted = FALSE;
	string ret = "KART OKUNAMADI!";

	if (mountSDCard() != 0) {
		DEBUG("mountSDCard hatasi!\n");
		goto out;
	}

	mounted = TRUE;

	SD_CARD_LOCK();

	if (system(SD_BILGI_KOMUTU) != 0) {
		LOG(LEVEL_ERROR, "SD_BILGI_KOMUTU hatasi! errno=%d\n", errno);
		SD_CARD_UNLOCK();
		goto out;
	}

	SD_CARD_UNLOCK();

	if ((sdInfo = fopen(SD_INFO_FILE, "r")) == NULL) {
		LOG(LEVEL_ERROR, "SD_INFO_FILE acilamadi!\n");
		goto out;
	}
	fcntl(fileno(sdInfo), F_SETFD, FD_CLOEXEC);

	if (fgets(buf, sizeof(buf), sdInfo) != NULL) {
		pch = strtok_r(buf, " ", &ctx);
		if (!pch) {
			DEBUG("strtok hatasi!\n");
			goto out;
		}

		pch = strtok_r(NULL, " ", &ctx);
		if (!pch) {
			DEBUG("strtok hatasi!\n");
			goto out;
		}

		ret = string("Kapasite: ") + string(pch);

		pch = strtok_r(NULL, " ", &ctx);
		if (!pch) {
			DEBUG("strtok hatasi!\n");
			goto out;
		}

		pch = strtok_r(NULL, " ", &ctx);
		if (!pch) {
			DEBUG("strtok hatasi!\n");
			goto out;
		}

		ret += string(" Boş Alan: ") + string(pch);
	}

out:
	if (sdInfo) {
		fclose(sdInfo);
	}

	if (mounted) {
		umountSDCard();
	}

	return ret;
}
