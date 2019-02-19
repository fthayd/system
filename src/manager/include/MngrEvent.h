#ifndef _MNGR_EVENT_H_
#define _MNGR_EVENT_H_

// Std Includes
#include <pthread.h>

// Local Includes
#include <common.h>

// Global Constant Definitions


// Global Macro Definitions
#define EVENT_LOCK()	pthread_mutex_lock(&eventMutex)
#define EVENT_TRYLOCK()	pthread_mutex_trylock(&eventMutex)
#define EVENT_UNLOCK()	pthread_mutex_unlock(&eventMutex)

// Global Data Types
typedef enum {
	EVENT_UNUSED		= 0,	// FIRST ELEMENT SHOULD STAY !!
	EVENT_DUMMY		= 1,
	EVENT_KONTAK_KAPANDI	= 2,	// Kontak acikken kontak enerjisi gittiginde Diagnostic modulu tarafindan gonderilir.
	EVENT_KONTAK_ACILDI	= 3,	// Kontak kapaliyken kontak enerjisi geri geldiginde Diagnostic modulu tarafindan gonderilir.
	EVENT_SISTEM_KAPANDI	= 4,	// Sistem enerjisi kesildiginde Diagnostic modulu tarafindan gonderilir.
	EVENT_SICAKLIK_YUKSEK	= 5,	// Sicaklik threashodu gecince Diagnostic modulu tarafindan gonderilir.
	EVENT_HAT_KODU_GIRIS	= 6,	// Sofor tarafindan ekrandan hat kodu girildiginde Ekran modulunden gonderilir. Payload olarak hat kodunu icerir.
	EVENT_SAAT_AYARLANDI	= 7,	// Saat ayari degistirildiginde Ekran ve Network modulu tarafindan gonderilir.
	EVENT_VARDIYA_BASI	= 8,	// Vardiya acildigindan Ekran modulu tarafindan gonderilir.
	EVENT_VARDIYA_SONU	= 9,	// Vardiya kapandiginda Ekran modulu tarafindan gonderilir.
	EVENT_TUR_BASI		= 10,	// Tur acildigindan Ekran modulu tarafindan gonderilir.
	EVENT_TUR_SONU		= 11,	// Tur kapandiginda Ekran modulu tarafindan gonderilir.
	EVENT_KONTROL_MODU	= 12,	// Cihaz kontrol moduna gecirildiginde Ekran modulunden gonderilir.
	EVENT_TAHSILAT_MODU	= 13,	// Tahsilat moduna gecildiginde Ekran veya Bilet tarafindan gonderilir.
	EVENT_NTPUPDATE_REQ	= 14,	// NTP update edilmek istendiginde gonderilir. Network Modulu tarafindan handle edilir.
	EVENT_KART_GECIS	= 15,	// Gecis eventi. Bilet modulu tarafindan gonderilir.
	EVENT_KART_ALANDA	= 16,	// Kart alanda tutuldugu surece Bilet modulu tarafindan gonderilir.
	EVENT_KART_ISLEM_TAMAMLANAMADI	= 17,
	EVENT_KART_KONTROL		= 18,	// Kontrol eventi. Kontrol modunda iken kart okutuldugunda Bilet modulu tarafindan gonderilir.
	EVENT_FARKLI_SOFOR_KARTI	= 19,	// Vardiyayi acandan farkli bir sofor karti okutuldugunda Ekran modulu tarafindan gonderilir.
	EVENT_AKTIVITE_DOSYALANIYOR	= 20,	// Aktivite dosyalanirken Aktivite modulu tarafindan gonderilir. Bilet ve Ekran programlari tarafindan handle edilir.
	EVENT_AKTIVITE_DOSYALANDI	= 21,	// Aktivite dosyalama tamamlandiginda Aktivite modulu tarafindan gonderilir. Bilet ve Ekran programlari tarafindan handle edilir.
	EVENT_AKTIVITE_DOSYALANAMADI	= 22,	// Aktivite dosyalama basarisiz tamamlandiginda Aktivite modulu tarafindan gonderilir. Bilet ve Ekran programlari tarafindan handle edilir.
	EVENT_ARACID_CHIP_HATASI	= 23,	// AracID chip okuma/yazma hata verdiginde gonderilir.
	EVENT_SISTEM_GERILIM_UYARI	= 24,	// Sistem gerilimi belli bir degerin altina duserse Diagnostic modulu tarafindan gonderilir.
	EVENT_BATARYA_HATASI		= 25,	// Batarya takili degilse acilista Diagnostic modulu tarafindan gonderilir.
	EVENT_AKTIVITE_DOSYALA		= 26,	// Aktivite dosyalama istegi
	EVENT_ATS_MESAJ			= 27,	// ATS Mesaj geldiginde Network modulu tarafindan gonderilir. Ekran modulu tarafindan handle edilir.
	EVENT_HIZ_SINIRI_ASILDI		= 28,	// Hiz siniri asildigindan Network modulu tarafindan gonderilir.
	EVENT_GPS_BAGLANTI_KOPTU	= 29,	// GPS baglantisi koptugunda Network modulu tarafindan gonderilir.
	EVENT_GPS_BAGLANTI_SAGLANDI	= 30,	// GPS baglantisi saglandiginda Network modulu tarafindan gonderilir.
	EVENT_SUNUCU_BAGLANTI_KOPTU	= 31,	// Sunucu baglantisi koptugunda Network modulu tarafindan gonderilir.
	EVENT_SUNUCU_BAGLANTI_SAGLANDI	= 32,	// Sunucu baglantisi saglandiginda Network modulu tarafindan gonderilir.
	EVENT_MANYETIK_ARIZA		= 33,	// Manyetik okuyucu ariza tesbit edildiginde Bilet modulu tarafindan gonderilir.
	EVENT_MANYETIK_BILET_YAZMA_HATASI	= 34,	// Manyetik bilete yazma yapilamayinca Bilet tarafindan gonderilir.
	EVENT_SAYACLARI_BAS		= 35,	// Ekran modulu tarafindan gonderilir.
	EVENT_BILET_CIKAR		= 36,	// Ekran tarafindan gonderilir.
	EVENT_GPS_MODULE_RESET		= 37,	// GPS Modulunde sorun tespit edilip resetlendiginde Network modulu tarafindan gonderilir.
	EVENT_UPDATE_ALINDI		= 38,	// Update paketi alindiginda Diagnostic tarafindan gonderilir.
	EVENT_UPDATE_BASLAT		= 39,	// Kosullar saglaniyorsa update icin cihazin reboot edecegini kullaniciya bildirmek icin Diagnostic tarafindan gonderilir.
	EVENT_FRAM_HATASI		= 40,	// FRAM hatasi tesbit edildiginde Aktivite modulu tarafindan gonderilir. Ekran modulu tarafindan hata gosterilir.
	EVENT_KART_ALANA_GIRDI		= 41,	// Alanda kart tesbit edildiginde Bilet modulu tarafindan gonderilir.
	EVENT_KART_ALANDAN_CIKTI	= 42,	// Kart alandan ciktiginda Bilet modulu tarafindan gonderilir.
	EVENT_SAM_HATASI		= 43,	// SAM kartta hata olusmasi durumunda Bilet modulu tarafindan gonderilir.
	EVENT_ATS_MESAJ_GONDER		= 45,	// ATS Mesaj gondermek icin Ekran modulu tarafindan gonderilir. Network modulu tarafindan handle edilir.
	EVENT_ENERJI_KESINTISI		= 46,	// 5V enerji gidip pil devreye girdiginde powerman uygulamasi tarafindan gonderilir. Bu event alindiginda sistem safe shutdown yapilir.
	EVENT_MANYETIK_TIMEOUT		= 47,	// Manyetik okuyucudan cevap gelemdiginde Bilet modulu tarafindan gonderilir.
	EVENT_HATA_KONTROLU		= 48,	// Ekranda "Hata Kontrolu Yapiliyor" uyarisi cikarmak istenildiginde gonderilir.
	EVENT_DURAKTAN_CIKIS		= 49,	// En son kart okutmadan 15 saniye sonra Bilet modulu tarafindan gonderilir.
	EVENT_QR_OKUYUCU_TAKILDI	= 50,	// Qr okuyucu takıldığında sistem yeniden baslatiliyor.
	EVENT_QR_BAGLANTI_KOPTU		= 51,	// Qr okuyucu pasif
	EVENT_QR_BAGLANTI_SAGLANDI	= 52,	// Qr okuyucu aktif
	EVENT_QR_ALANA_GIRDI		= 53,	// Qr Barkod okuma alanına girdi
	EVENT_CIHAZ_KULLANIM_DISI	= 54,	// Cihaz kullanım dışı
	EVENT_MAX				// LAST ELEMENT SHOULD STAY !!
} EventType;

class MngrEvent
{
	private:
		EventType eventType;
		const void *payload;
		unsigned int payloadSize;
		class MngrModule *sender;
	public:
		DEFINE_CLASSNAME("MngrEvent");
		MngrEvent(class MngrModule *pSender, EventType type, const void *pPayload = NULL, unsigned int size = 0);
		virtual ~MngrEvent(void);
		EventType getType(void);
		MngrModule *getSender(void);
		const void *getPayload(void);
		unsigned int getPayloadSize(void);
		void freePayload(void);
		bool operator==(EventType pEventType);
};


// Global Variable Externs


// Global Function Prototypes


#endif	//_MNGR_EVENT_H_
