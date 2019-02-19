// Std Includes
#include <fstream>
#include <pthread.h>
#include <sstream>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/if_ether.h>
//#include <QtCore/QTextStream>

// Local Includes
#include <Config.h>
#include <Utility.h>

// Private Constant Definitions
#define CONFIG_INI_PATH		"config.ini"

// Private Macro Definitions
#define CONFIG_LOCK()		pthread_mutex_lock(&configMutex)
#define CONFIG_UNLOCK()		pthread_mutex_unlock(&configMutex)

// Private Data Types


// Global Variable Declarations
Config *Cfg = NULL;
pthread_mutex_t	configMutex = PTHREAD_MUTEX_INITIALIZER;

// Private Function Prototypes


// Functions

Config::Config(void)
{
	TRACE("Constructor\n");

	usage = 0;

	Utility::create();

	Cfg = this;	// WORKAROUND: Construct edilirken DB'den aracID'ye ulasmak icin ve AracID'den addErrorFlag cagirabilmek icin gerekiyor
}

Config::~Config(void)
{
	TRACE("Destructor\n");

	ASSERT((usage == 0), ("Usage not zero!!! %d\n", usage));

	Utility::destroy();
}

void Config::create(void)
{
	if (!Cfg) {
		Cfg = new Config();
	}

	Cfg->usage++;

	TRACE("usage=%d\n", Cfg->usage);
}

void Config::destroy(void)
{
	if (Cfg) {
		Cfg->usage--;
		TRACE("usage=%d\n", Cfg->usage);
		if (Cfg->usage == 0) {
			delete Cfg;
			Cfg = NULL;
		}
	} else {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Config fazladan destroy!\n");
	}
}

void Config::updateVersionFile(void)
{
	//Cfg->configSet("BILET", PROGRAM_VERSIYON_FULL);
	//Cfg->configSet("EKRAN", PROGRAM_VERSIYON_FULL);
	//Cfg->configSet("UPDATER", PROGRAM_VERSIYON_FULL);
	//Cfg->configSet("DB", db->cihazParams.DBVersion);
}

void Config::updateServerIP(void)
{
	//Cfg->configSet("ServerIP_UDP", Cfg->db->cihazParams.ServerIP);
}

int Config::loadConfigINI(const char *filename, SSmap *pMap)
{
	ifstream ifs(filename, ios_base::in);
	string line;

	DEBUG("Loading %s\n", filename);

	if (!ifs || !ifs.is_open()) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Cannot open file: %s\n", filename);
		return -1;
	}

	CONFIG_LOCK();
	// read an ini file line by line
	while (getline(ifs, line)) {
		//TRACE("got line %s\n", line.c_str());

		// split to key-value pair and add to map
		string::size_type index = line.find_first_of(":=");

		if (index == string::npos) {
			continue;
		}

		string keyStr = Utils->trim(line.substr(0, index), " \t\r\n");
		string valStr = Utils->trim(line.substr(index + 1), " \t\r\n");

		TRACE("Adding %s = %s\n", keyStr.c_str(), valStr.c_str());

		if (pMap->find(keyStr) != pMap->end()) {
			LOG(LEVEL_ERROR, "Duplicate config entry: %s\n", keyStr.c_str());
		}
		pMap->insert(make_pair(keyStr, valStr));
	}

	ifs.close();

	CONFIG_UNLOCK();

	return 0;
}

int Config::saveConfigINI(const char *filename, SSmap *pMap)
{
	ofstream ofs(filename, ios_base::out);
	SSmap::iterator pos;

	DEBUG("Saving %s\n", filename);

	if (!ofs || !ofs.is_open()) {
		FATAL_LOG(GENEL_YAZILIM_HATASI, "Cannot open file: %s\n", filename);
		return -1;
	}

	CONFIG_LOCK();

	for (pos = pMap->begin(); pos != pMap->end(); pos++) {
		ofs << pos->first << " = " << pos->second << endl;
	}
	ofs << endl;

	ofs.close();

	CONFIG_UNLOCK();

	return 0;
}

int Config::loadConfig(void)
{
	int ret = 0;

	ret += loadConfigINI(CONFIG_INI_PATH, &config_map);

	return ret;
}

int Config::saveConfig(void)
{
	int ret = 0;

	ret += saveConfigINI(CONFIG_INI_PATH, &config_map);

	sync();

	return ret;
}

int Config::freeConfig(void)
{
	CONFIG_LOCK();
	config_map.clear();
	aracid_map.clear();
	versiyonlar_map.clear();
	CONFIG_UNLOCK();

	return 0;
}

int Config::reloadConfig(void)
{
	freeConfig();

	return loadConfig();
}

string Config::configGet(string key, SSmap **pMap)
{
	SSmap::iterator it;
	string value = "";

	//TRACE("key=%s\n", key.c_str());

	CONFIG_LOCK();
	// get the value having key
	if ((it = config_map.find(key)) != config_map.end()) {
		if (pMap) {
			*pMap = &config_map;
		}
		goto found;
	}
	if ((it = aracid_map.find(key)) != aracid_map.end()) {
		if (pMap) {
			*pMap = &aracid_map;
		}
		goto found;
	}
	if ((it = versiyonlar_map.find(key)) != versiyonlar_map.end()) {
		if (pMap) {
			*pMap = &versiyonlar_map;
		}
		goto found;
	}

	LOG(LEVEL_WARNING, "Cannot find key: %s\n", key.c_str());
	goto out;

found:
	value = it->second;

out:
	CONFIG_UNLOCK();

	//DEBUG("%s = %s\n", key.c_str(), value.c_str());

	return value;
}

int Config::configGetInt(string key)
{
	return Utils->strToInt(configGet(key));
}

int Config::configSet(string key, string value)
{
	string oldValue;
	SSmap *pMap = NULL;

	DEBUG("%s = %s\n", key.c_str(), value.c_str());

	oldValue = configGet(key, &pMap);

	if (oldValue.empty()) {
		return -1;
	}

	CONFIG_LOCK();
	(*pMap)[key] = value;
	CONFIG_UNLOCK();

	return 0;
}

int Config::configSet(string key, int intValue)
{
	string oldValue;
	string value = Utils->intToStr(intValue);
	SSmap *pMap = NULL;

	DEBUG("%s = %s\n", key.c_str(), value.c_str());

	oldValue = configGet(key, &pMap);

	if (oldValue.empty()) {
		return -1;
	}

	CONFIG_LOCK();
	(*pMap)[key] = value;
	CONFIG_UNLOCK();

	return 0;
}

// Taken from busybox ifconfig.c
static int in_ether(const char *bufp, struct sockaddr *sap)
{
	char *ptr;
	int i, j;
	unsigned char val;
	unsigned char c;

	sap->sa_family = ARPHRD_ETHER;
	ptr = sap->sa_data;

	i = 0;
	do {
		j = val = 0;

		/* We might get a semicolon here - not required. */
		if (i && (*bufp == ':')) {
			bufp++;
		}

		do {
			c = *bufp;
			if (((unsigned char)(c - '0')) <= 9) {
				c -= '0';
			} else if (((unsigned char)((c|0x20) - 'a')) <= 5) {
				c = (c|0x20) - ('a'-10);
			} else if (j && (c == ':' || c == 0)) {
				break;
			} else {
				return -1;
			}
			++bufp;
			val <<= 4;
			val += c;
		} while (++j < 2);
		*ptr++ = val;
	} while (++i < ETH_ALEN);

	return *bufp; /* Error if we don't end at end of string. */
}

string Config::getMacAddress(void)
{
	return Utils->getIfaceMacStr(ETH_IFNAME);
}

string Config::getMacAddress2(void)
{
	string mac = getMacAddress();

	Utils->replaceStringInPlace(mac, ":", "");

	return mac;
}

void Config::getMacAddress(char *bin)
{
	string mac = getMacAddress();
	const char *str = mac.c_str();
	struct ifreq ifr;

	in_ether(str, &(ifr.ifr_hwaddr));

//	memcpy(bin, ifr.ifr_hwaddr.sa_data, sizeof(ifr.ifr_hwaddr.sa_data));
}

string Config::getSerialNo(void)
{
	string mac = getMacAddress();
	const char *str;
	int serial;
	ostringstream convert;
	struct ifreq ifr;

	if (mac.empty() || mac.length() != 17) {
		return "0";
	}

	str = mac.c_str();
	in_ether(str, &(ifr.ifr_hwaddr));

	serial = ((unsigned char)ifr.ifr_hwaddr.sa_data[4] << 8) + (unsigned char)ifr.ifr_hwaddr.sa_data[5];

	convert << serial;

	return convert.str();
}

string Config::getGPSInfo(string key)
{
	SSmap::iterator it;
	string value = "";

	//TRACE("key=%s\n", key.c_str());

	CONFIG_LOCK();
	// get the value having key
	if ((it = GPSInfoMap.find(key)) != GPSInfoMap.end()) {
		goto found;
	}

	//LOG(LEVEL_WARNING, "Cannot find key: %s\n", key.c_str());
	goto out;

found:
	value = it->second;

out:
	CONFIG_UNLOCK();

	//TRACE("%s = %s\n", key.c_str(), value.c_str());

	return value;
}

void Config::setTopUpClear(int val)
{
	topUpClear = val;
}

int Config::getTopUpClear(void)
{
	return topUpClear;
}

void Config::setServerConnected(bool state)
{
	serverConnection = state;
}

bool Config::isServerConnected(void)
{
	return serverConnection;
}

void Config::addErrorFlag(ErrorFlag flag)
{
	CONFIG_LOCK();

	errList.insert(flag);

	CONFIG_UNLOCK();
}

void Config::removeErrorFlag(ErrorFlag flag)
{
	CONFIG_LOCK();

	errList.erase(flag);

	CONFIG_UNLOCK();
}

string Config::getErrorStatus(void)
{
	string ret = "";

	CONFIG_LOCK();

	for (ErrorList::iterator it = errList.begin(); it != errList.end(); it++) {
		ret.append(Utils->intToStr(*it) + ";");
	}

	if (ret.length() == 0) {
		ret = "0";
	}

	CONFIG_UNLOCK();

	TRACE("ret=%s\n", ret.c_str());

	return ret;
}


bool Config::errorFlagIsSet(ErrorFlag flag)
{
	ErrorList::iterator it;

	CONFIG_LOCK();

	it = errList.find(flag);

	CONFIG_UNLOCK();

	return (it != errList.end());
}
