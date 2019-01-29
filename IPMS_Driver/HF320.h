/* HF320 Serial Reader Class Header file include HF9A , HF320 , MP100 , MP2 */
/* Mifare First Sector is 0 , but sector 0 can't write */
#ifndef ____HF320____
#define ____HF320____

#include "rs232.h"
#include "MF700.h"

// KEY Number
enum HF320_KEY_TYPE
{
	HF_MIFARE_KEYA = 0x00,
	HF_MIFARE_KEYB = 0x01
};

// HF320 ErrorCode
enum HF320_ERROR
{
	HF320_OK = 0x01,
	HF320_ERR_LRC = 0x10,
	HF320_ERR_CMD = 0xFF,
	HF320_ERR_NOCARD = 0xE0,
	HF320_ERR_TRANSFER = 0xE1,
	HF320_ERR_AUTHENTICATE = 0xA1
};

class HF320
{
public :
	HF320(void);
	~HF320(void);
	
	bool init(enum COMPORT port);
	void Close(void);
	
	bool mfRead(int Block, char* data); // read 16 byte data
	bool mfWrite(int Block, char data[], bool bChangeKey = false); //write 16 byte data
	void StringToASCII(char* result, char data[], int iDataLen = 16);
	void ASCIIToString(char* result, char data[], int iDataLen = 16);
	void ErrorCodeToMessage(char* msg, char data);
	char CacheCard(char *CardID);
	int ReadSector(char *Tag,int sector, sectorData* sdata,bool ReSelect = false);
	int WriteSector(char *Tag,int sector, sectorData sdata,bool ReSelect = false);
	int ChangeSectorKey(int sector);
	int Deselect(void); // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
	bool ClearSector(int sector);
	void SetTagMode(bool bSet);
	void ClearBuff(void);
	void SetAPDUMode(bool bSet);
	bool GetAPDUMode(void);
	bool ShowDisplay(char* DisplayData, int Datalen); // nick add 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
	
private:
	
	RS232 comport;
	int BufferSize;
	int SendCommand(unsigned char* Cmd, int len, unsigned char* data);
	int GetResponse(unsigned char* rData);
	// ========================================================= //
	// nick add s 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
	int APDU_RATS(void);
	int APDU_Unlock(unsigned char* Key);
	int APDU_ChangeKey(unsigned char* NewKey, unsigned char* OldKey);
	bool APDU_mfRead(int Block, char* data);
	bool APDU_mfWrite(int Block, char data[], bool bChangeKey = false);
	bool APDU_mfShowDisplay(char data[]);
	// nick add e 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //
	void MakeDBTagID(char* STagID,char* DTagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	char SectorData[3][17];
	bool m_bAlreadySelCard;
	bool m_bHalted ;
	char CardTag[24];
	int m_Sector;
	bool m_bUltraLight;
	bool m_bTagMode;
	bool m_bUseAPDU; // nick add 20160624 Ver:000-000-GIO_V2-13C241-0001-166241
	enum COMPORT my_port;
};

#endif
