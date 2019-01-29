/* MF700 Reader Class Header file */
/*Mifare First Sector is 0 , but sector 0 can't write */
#ifndef ____MF700____
#define ____MF700____

#include "rs232.h"

#define PARKING_DATA_SECTOR   10
#define PARKING_DATA_BAK_SECTOR   11
#define PARKING_DISCOUNT_SECTOR   12
#define PARKING_DISCOUNT_BAK_SECTOR   14

// Hourly data initial bcc = 79
#define INIT_DATA_BCC  0x4F
// discount initial bcc = 105
#define INIT_DISCOUNT_BCC  0x69

enum MF700_KEY_TYPE
{
	MF_KEYA = 0x60,
	MF_KEYB = 0x61
};

enum MF700_OPT
{
	MF_DEC = 0xC0,
	MF_INC = 0xC1
};

enum MF700_COMMAND
{
	mf_REQUEST=0x20,
	mf_ANTICOLL,
	mf_SELECTCARD,
	mf_AUTHENTICATE,
	mf_READBLOCK,
	mf_WRITEBLOCK,
	mf_SETVALUE,
	mf_READVALUE,
	mf_CREATVALUE,
	mf_ACCCONDITION,
	mf_HALT,
	mf_SAVEKEY,
	mf_GETSECOND,
	mf_GETCONDITION,
	mf_AUTHENWITHKEY,
	mf_REQALL
};

enum MF700_ERROR
{
	mf_ERR_EMPTY=0x03,
	mf_ERR_AUTHENTICATE=0x04,
	mf_ERR_KEY=0x09,
	mf_ERR_NOT_AUTHENT=0x0a,
	mf_ERR_TRANSFER=0x0e,
	mf_ERR_WRITE=0x0f,
	mf_ERR_INC=0x10,
	mf_ERR_DEC=0x11,
	mf_ERR_READ=0x12,
	mf_ERR_TIMEOUT=0x1c,
	mf_ERR_NOTAG=0x1F,
	mf_ERR_WRONG_PARAM=0x27,
	mf_ERR_HOST_AUTHENT=0x2D,
	mf_ERR_DESKEY=0x2f,
	mf_ERR_DESKEY_LOAD = 0x33,
	mf_ERR_COMMAND_DENY = 0xE0,
	mf_ERR_COMMAND_ILLEGAL =0xE1,
	mf_ERR_COMMAND_OVERRUN = 0xE2,
	mf_ERR_COMMAND_CRC = 0xE3,
	mf_ERR_COMMAND_MEMORY = 0xE4,
	mf_ERR_COMMAND_FRAME = 0xE5,
	mf_ERR_COMMAND_UNKNOW = 0xE6
};
#pragma pack(1) /// force alignment to 1 byte
typedef struct strBlock1
{
	unsigned char format;
	unsigned char parkingId;
	unsigned char areaId;
	unsigned char langId;
	unsigned long ticketID;
	unsigned char plate[8];
}Block1;

typedef struct strBlock2
{
	unsigned char status;
	unsigned char in_year[2];
	unsigned char in_month;
	unsigned char in_day;
	unsigned char in_hour;
	unsigned char in_min;
	unsigned long Value;
	unsigned long seasonVersion;
	unsigned char empty;
}Block2;

typedef struct strBlock3
{
	unsigned char out_year[2];	//纗程纗丁 
	unsigned char out_month;	//纗程纗丁 る	
	unsigned char out_day;		//纗程纗丁 ら
	unsigned char out_hour;		//纗程纗丁 
	unsigned char out_min;		//纗程纗丁 だ	
	unsigned char out_sec;		//纗程纗丁 
	unsigned char optime[3];
	unsigned char empty[4];
	unsigned char randon;
	unsigned char bcc;
}Block3;

// 20110506 Tony add s
typedef struct strBlock1V2
{
	unsigned char TicketVer;
	unsigned char NextSector;
	unsigned char parkingId;
	unsigned char areaId;
	unsigned long ticketID;
	unsigned char plate[8];
}Block1V2;

typedef struct strBlock2V2
{
	unsigned char status;
	unsigned char in_year[2];
	unsigned char in_month;
	unsigned char in_day;
	unsigned char in_hour;
	unsigned char in_min;
	unsigned char in_sec;
	unsigned long Value;
	unsigned long seasonVersion;
}Block2V2;

typedef struct strBlock3V2
{
	unsigned char out_year[2];	//纗程纗丁 
	unsigned char out_month;	//纗程纗丁 る
	unsigned char out_day;		//纗程纗丁 ら
	unsigned char out_hour;		//纗程纗丁 
	unsigned char out_min;		//纗程纗丁 だ
	unsigned char out_sec;		//纗程纗丁 
	unsigned char optime[3];		
//	unsigned char staytime[4];
	unsigned long staytime;
	unsigned char DisctSector;
	unsigned char bcc;
}Block3V2;
// 20110506 Tony add e

typedef struct SectorData
{
	 unsigned char block1[16];
	 unsigned char block2[16];
	 unsigned char block3[16];
}sectorData;

typedef struct strDiscountBlock1
{
	unsigned char DiscountYear[2];
	unsigned char DiscountMon;
	unsigned char DiscountDay;
	unsigned char DiscountHour;
	unsigned char DiscountMin;
	unsigned char LimitDiscount;
	unsigned char LimitAID;
	unsigned char LimitSID;
	unsigned char LimitVID;
	unsigned char LimitYear[2];
	unsigned char LimitMon;
	unsigned char LimitDay;
	unsigned char LimitHour; // :00 ago
	unsigned char DiscountCount;
}DiscountBlock1;

typedef struct strDiscountData
{
	unsigned char AID;
	unsigned char SID;
	unsigned char VID;
	unsigned char DiscountHour; // half hour or 1hour
}DiscountData;

typedef struct strDiscountBlock23
{
	DiscountData discounts[7];
	unsigned char empty[3];
	unsigned char bcc;
}DiscountBlock23;

typedef struct SectorDiscountData
{
	DiscountBlock1 block1;
	DiscountBlock23 block23;
}DiscountSectorData;

//Frank add s 20120912
typedef struct strDiscountBlock1v2
{
	unsigned char DiscountYear[2];
	unsigned char DiscountMon;
	unsigned char DiscountDay;
	unsigned char DiscountHour;
	unsigned char DiscountMin;
	unsigned char DiscountSec;
	unsigned char LHDisctCnt;
	unsigned char LimitAID;
	unsigned char LimitSID;
	unsigned char LimitVID;
	unsigned char LimitYear[2];
	unsigned char LimitMon;
	unsigned char LimitDay;
	unsigned char LimitHour;
}DiscountBlock1v2;

typedef struct strDiscountDatav2
{
	unsigned char AID;
	unsigned char SID;
	unsigned char VID;
	unsigned char DiscountHour[4];
}DiscountDatav2;

typedef struct strDiscountBlock
{
	DiscountDatav2 discount[2];
	unsigned char NextSector;
	unsigned char bcc;
}DiscountBlock;

typedef struct SectorDiscountDatav2
{
	DiscountBlock1v2 block1;
	DiscountBlock block23[2];
}DiscountSectorDatav2;

typedef struct strDiscount
{
	DiscountBlock1v2 limit;
	DiscountBlock block[15];
}Discountstr;
//Frank add e 20120912

#pragma pack()  /// set alignment back to default

class MF700
{
public :
	MF700(void);
	~MF700(void);
	
	void  init(enum COMPORT port);
	//bool  init(enum COMPORT port); // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	void  Close(void);
	
	char  mfRequest(void);
	char  mfAnticollision(char* CardID);
	int   mfSelectCard(char CardSN[]);
	// nick mark 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //bool  mfAuthenticate(int Sector,enum MF700_KEY_TYPE keyType);
	bool  mfAuthenticate(int Sector,enum MF700_KEY_TYPE keyType, bool bUseDefaultKey = false); // nick add 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	bool  mfRead(int Block,char* data); // read 16 byte data
	bool mfWriteKey(char data[]); // nick add 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	bool  mfWrite(int Block,char data[]); //write 16 byte data
	bool  mfValueSet(int Block,enum MF700_OPT option,long value);
	char  mfHalt(void);
	bool  mfSaveKey(enum MF700_KEY_TYPE keyType,int Sector,char* Key);
	bool  mfAccessCondition(char* keyA, char* keyB,BYTE CB0,BYTE CB1,BYTE CB2,BYTE CB3, BYTE GPByte);
	bool  mfGetAccessCondition(BYTE CB0,BYTE CB1,BYTE CB2,BYTE CB3, BYTE GPByte);
	
	void  StringToASCII(char* result,char data[]);
	void  ASCIIToString(char* result,char data[]);
	void  ErrorCodeToMessage(char* msg,char data[]);
	
	char   CacheCard(char *CardID);
	char   CacheCard(void);
	int    ReadSector(char *Tag,int sector, sectorData* sdata,bool ReSelect=false);
	int    WriteSector(char *Tag,int sector, sectorData sdata,bool ReSelect=false);
	int	ChangeSectorKey(int sector); // nick add 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	bool   ClearSector(int sector);
	void   SetTagMode(bool bSet);
	void   ClearBuff(void);			//Frank add 20111116
	
private:
	
	RS232		comport;
	unsigned char	ReaderID;
	int			BufferSize;    // 20120221 Tony add
	int			SendCommand(enum MF700_COMMAND cmd,int len,char* data);
	int			GetResponse(char* rData);
	char			SectorData[3][17];
	bool			m_bAlreadySelCard;
	bool			m_bHalted ;
	char			CardID[24];
	int			m_Sector;
	bool			m_bUltraLight;
	bool			m_bTagMode;
	enum COMPORT my_port; // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
};

#endif
