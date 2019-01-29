/* MP90 Serial Reader Class */
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "../traceLog.h"
#include "../CommonDef.h"
#include "MF700.h"
#include "HF320.h"

#define RETRY 3 // nick edit 20151230 Ver:000-000-GIO_V2-13B251-0007-13C241 from 5 to 3 //

const unsigned char AuthenticateDefaultCodeA[16][13] = 
{
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF"
};

const unsigned char AuthenticateDefaultCodeB[16][13] = 
{
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF"
};

const unsigned char AuthenticateCodeA[16][13]=
{
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"3211C12E4293",
	"3211C12E4293",
	"05C38469ECDD",
	"05C38469ECDD",
	"05C38469ECDD",
	"05C38469ECDD",
	"05C38469ECDD"
};

const unsigned char AuthenticateCodeB[16][13]=
{
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"FFFFFFFFFFFF",
	"4CF3205D1687",
	"4CF3205D1687",
	"72E711153AA1",
	"72E711153AA1",
	"72E711153AA1",
	"72E711153AA1",
	"72E711153AA1"
};

const unsigned char VISUAL_CARD_DEFAULT_KEY_CODE[] = { 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 }; // nick add 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
const unsigned char VISUAL_CARD_AUTHENTICATE_KEY_CODE[] = { 0x32, 0x11, 0xC1, 0x2E, 0x42, 0x93, 0x55, 0x68 }; // nick add 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //

const unsigned char HF320_SOF = 0xAE; // Packet Header

// COMMON Command
const unsigned char HF320_SET_PROTOCAL[] = { 0x00, 0x00 };
const unsigned char HF320_GETFIRMWARE_VER[] = { 0x00, 0x01 };
const unsigned char HF320_SET_AUTOPOLLING[] = { 0x00, 0x02 };
// End COMMON Command
// TYPE:14443A
const unsigned char MF320_14443A_GETID[] = { 0x01, 0x00 };
// ADPU for 14443A
const unsigned char MF320_14443A_RATS[] = { 0x01, 0x01 };
const unsigned char MF320_14443A_APDU[] = { 0x01, 0x02 };
const unsigned char MF320_14443A_DESELECT[] = { 0x01, 0x03 };
// End ADPU for 14443A
const unsigned char MF320_14443A_READBLOCK[] = { 0x01, 0x04 };
const unsigned char MF320_14443A_WRITEBLOCK[] = { 0x01, 0x05 };
// End TYPE:14443A

HF320::HF320(void)
{
	BufferSize = 128;
	m_bTagMode = false;
	m_bAlreadySelCard = true;
	m_bHalted = false;
	m_bUltraLight = false;
	m_bUseAPDU = false; // nick add 20160624 Ver:000-000-GIO_V2-13C241-0001-166241 //
	
	memset(CardTag,'\0',sizeof(CardTag));
}

HF320::~HF320(void)
{
	Close();
}

bool HF320::init(enum COMPORT port)
{
	my_port = port;
	//comport.PortInitForHF320(port, 19200, 'N', 8, 1);
	comport.PortInit2(port, 19200, 'N', 8, 1);
	
	memset(SectorData, 0x00, sizeof(SectorData));

	unsigned char recvData[BufferSize];
	unsigned char data[BufferSize];
	unsigned char cmd[sizeof(HF320_GETFIRMWARE_VER)];
	int iRet = 0;
	int i = 0;
	
	memset(recvData, 0, sizeof(recvData));
	// Get Firmware //
	printf("HF320 -> GetFirmware s\n");
	memcpy(cmd, HF320_GETFIRMWARE_VER, sizeof(HF320_GETFIRMWARE_VER));
	
	for (i = 0; i < RETRY; i++)
	{
		SendCommand(cmd, 0, (unsigned char *)"");
		iRet = GetResponse(recvData);
		
		if (iRet < 1)
			continue;
			// nick mark 20151230 Ver:000-000-GIO_V2-13B251-0007-13C241 //return (false); // nick add 20150423 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		
		if (recvData[0] == HF320_ERR_TRANSFER)
			continue;
		
		if (recvData[0] != HF320_OK)
			return (false);
		else
			break;
	}
	
	printf("HF320 -> GetFirmware e\n");
	
	if (i >= RETRY)
		return (false);
	//
	// Set AutoPolling Disable //
	memset(data, 0, BufferSize);
	data[0] = 0x01; // Disable
	printf("HF320 -> AutoPolling Disable s\n");
	memcpy(cmd, HF320_SET_AUTOPOLLING, sizeof(HF320_SET_AUTOPOLLING));
	
	for (i = 0; i < RETRY; i++)
	{
		SendCommand(cmd, 1, data);
		iRet = GetResponse(recvData);
		
		if (iRet < 1)
			continue;
		
		if (recvData[0] == HF320_ERR_TRANSFER)
			continue;
		
		if (recvData[0] != HF320_OK)
			return(false);
		else
			break;
	}
	
	printf("HF320 -> AutoPolling Disable e\n");
	
	if (i >= RETRY)
		return(false);
	//
	// Set Protocol //
	memset(data, 0, BufferSize);
	printf("HF320 -> Set Protocol s\n");
	data[0] = 0x00; // ISO14443A
	memcpy(cmd, HF320_SET_PROTOCAL, sizeof(HF320_SET_PROTOCAL));
	
	for (i = 0; i < RETRY; i++)
	{
		SendCommand(cmd, 1, data);
		iRet = GetResponse(recvData);
		//
		
		if (iRet < 1)
			continue;
		
		if (recvData[0] == HF320_ERR_TRANSFER)
			continue;
		
		if (recvData[0] == HF320_OK)
			break;
		else
			return(false);
	}
	
	printf("HF320 -> Set Protocol e\n");
	
	if (i >= RETRY)
		return (false);
	else
		return (true);
}

void HF320::Close()
{
	//char buffMsg[BufferSize];
	
	comport.PortClose();
	//sprintf(buffMsg, "MF320 Port Close Port:[ COM%d ].", my_port);
	//ShowMessage(buffMsg);
}

// ========================================================= //
// nick add s 20160624 Ver:000-000-GIO_V2-13C241-0001-166241 //
void HF320::SetAPDUMode(bool bSet)
{
	m_bUseAPDU = bSet;
}

bool HF320::GetAPDUMode()
{
	return (m_bUseAPDU);
}
// nick add e 20160624 Ver:000-000-GIO_V2-13C241-0001-166241 //
// ========================================================= //

void HF320::SetTagMode(bool bSet)
{
	m_bTagMode = true;
}

void HF320::StringToASCII(char* result, char data[], int iDataLen)
{
	int i;
	unsigned char value;

	for(i = 0; i < iDataLen; i++)
	{
		sscanf(data + i * 2, "%2X", (unsigned int *)&value);
		result[i] = value;
	}
}

void HF320::ASCIIToString(char* result, char data[], int iDataLen)
{
	int i;
	char ascString[iDataLen * 2];

	memset(ascString, '\0', sizeof(ascString));
	
	for(i = 0; i < iDataLen; i++)
	{
		sprintf(ascString + i * 2, "%02X", (unsigned char)data[i]);
	}
	
	sprintf(result, "%s", ascString);
}

bool HF320::mfRead(int Block, char* data)
{
	unsigned char recvData[BufferSize];
	unsigned char sendData[BufferSize];
	unsigned char cmd[sizeof(MF320_14443A_READBLOCK)];
	char msg[128];
	int iRet = 0;
	int iLoc = 0;
	
	memset(recvData, 0, BufferSize);
	memset(sendData, 0, BufferSize);
	memset(msg, 0, sizeof(msg));
	
	printf("HF320 -> mfRead() s\n");
	
	memcpy(cmd, MF320_14443A_READBLOCK, sizeof(MF320_14443A_READBLOCK));
	sendData[iLoc] = Block & 0xFF;
	iLoc++;
	sendData[iLoc] = HF_MIFARE_KEYA;
	iLoc++;
	
	iRet = (Block - (Block % 4)) / 4 - 1;
	
	StringToASCII((char *)sendData + iLoc, (char *)AuthenticateCodeA[iRet], strlen((char *)AuthenticateCodeA[iRet]) / 2);
	iLoc += strlen((char *)AuthenticateCodeA[iRet]) / 2;
	
	SendCommand(cmd, iLoc, sendData);
	iRet = GetResponse(recvData);
	
	if (iRet < 1)
		return (false);
	
	ErrorCodeToMessage(msg, (char)recvData[0]);
	printf("Read Return:[%s]\n", msg);
	printf("HF320 -> mfRead() e\n");
	
	if (recvData[0] == HF320_OK)
	{
		memcpy(data, (char *)recvData + 1, iRet - 1);
		return(true);
	}
	else
		return(false);
}

bool HF320::mfWrite(int Block, char data[], bool bChangeKey)
{
	unsigned char recvData[BufferSize];
	unsigned char sendData[BufferSize];
	unsigned char cmd[sizeof(MF320_14443A_WRITEBLOCK)];
	char msg[128];
	int iRet = 0;
	int iLoc = 0;
	
	memset(recvData, 0, BufferSize);
	memset(sendData, 0, BufferSize);
	memset(msg, 0, sizeof(msg));
	
	printf("HF320 -> mfWrite() s\n");
	
	memcpy(cmd, MF320_14443A_WRITEBLOCK, sizeof(MF320_14443A_WRITEBLOCK)); // command
	// Block Number
	sendData[iLoc] = Block & 0xFF;
	iLoc++;
	//
	// Key A
	sendData[iLoc] = HF_MIFARE_KEYA;
	iLoc++;
	//
	// Key
	iRet = (Block - (Block % 4)) / 4 - 1;
	
	if (bChangeKey == false)
		StringToASCII((char *)sendData + iLoc, (char *)AuthenticateCodeA[iRet], strlen((char *)AuthenticateCodeA[iRet]) / 2); // Key
	else
		StringToASCII((char *)sendData + iLoc, (char *)AuthenticateDefaultCodeA[iRet], strlen((char *)AuthenticateDefaultCodeA[iRet]) / 2); // Key
	
	iLoc += strlen((char *)AuthenticateCodeA[iRet]) / 2;
	//
	// Data
	StringToASCII((char *)sendData + iLoc, data, strlen(data) / 2);
	iLoc += strlen(data) / 2;
	//
	SendCommand(cmd, iLoc, sendData);
	iRet = GetResponse(recvData);
	
	if (iRet < 1)
		return (false);
	
	ErrorCodeToMessage(msg, (char)recvData[0]);
	printf("Write Return:[%s]\n", msg);
	printf("HF320 -> mfWrite() e\n");
	
	if (recvData[0] == HF320_OK)
		return(true);
	else
		return(false);
}

int HF320::SendCommand(unsigned char* Cmd, int iDataLen, unsigned char* data)
{
	int sLen = iDataLen + 2; // data length + command length(2 byte)
	int iLoc = 0;
	unsigned char sendData[sLen + 5];
	char buf[256];
	char bufMsg[128];

	memset(sendData,'\0',sizeof(sendData));
	memset(buf, '\0', sizeof(buf));
	memset(bufMsg, '\0', sizeof(bufMsg));
	
	iLoc = 0;
	sendData[iLoc] = HF320_SOF;
	iLoc++;
	// Packet Length
	sendData[iLoc] = (sLen >> 8) & 0xFF;
	iLoc++;
	sendData[iLoc] = (sLen & 0xFF);
	iLoc++;
	//
	memcpy(sendData + iLoc, Cmd, 2);
	iLoc += 2;
	memcpy(sendData + iLoc, data, iDataLen);
	iLoc += iDataLen;
	// Culc Checksum
	sendData[iLoc] = 0;
	
	for (int i = 1; i < iLoc; i++)
		sendData[iLoc] ^= sendData[i];
	//
	iLoc++;
	
	ClearBuff();
	//sprintf(bufMsg, "HF320 Send Data. Port:[ COM%d ]", my_port);
	//ShowMessage(bufMsg, 5);
	comport.PortWriteForHF320(sendData, iLoc);
	return (iLoc);
}

int HF320::GetResponse(unsigned char* rData)
{   // return 0:no response 大於0:Received -1:received error
	int len = 0;
	int iDataLen = 0;
	char bufMsg[BufferSize];
	unsigned char recvData[BufferSize];
	unsigned char LRC = 0;

	memset(recvData,'\0',sizeof(recvData));
	memset(bufMsg, 0, BufferSize);
	len = BufferSize;
	
	if(comport.PortReadForHF320(recvData, len, &iDataLen, 2500))
	{
		for (int i = 1; i < iDataLen - 1; i++)
			LRC ^= recvData[i];
		
		if (LRC == recvData[iDataLen - 1])
		{
			memcpy(rData, recvData + 5, iDataLen - 6);
			return (iDataLen - 6);
		}
	}
	else
	{
		sprintf(bufMsg, "HF320 no received. Port:[ COM%d ]", my_port);
		ShowMessage(bufMsg, 2);
		return 0;
	}
	
	return -1;
}

int HF320::APDU_RATS() // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
{
	unsigned char cmd[sizeof(MF320_14443A_RATS)];
	unsigned char recvData[BufferSize];
	int i = 0;
	int iRet = 0;
	
	printf("HF320->APDU_RATS() s\n");
	
	memset(recvData, 0, BufferSize);
	memcpy(cmd, MF320_14443A_RATS, sizeof(MF320_14443A_RATS));
	
	SendCommand(cmd, 0, (unsigned char *)"");
	i = GetResponse(recvData);
	
	if (i == 0)
	{
		iRet = -1;
		goto FUNCEXIT;
	}
	else if (i == -1)
	{
		iRet = 0;
		goto FUNCEXIT;
	}
	else
	{
		if (recvData[0] == HF320_OK)
		{
			iRet = 1;
			goto FUNCEXIT;
		}
		else if (recvData[0] == HF320_ERR_NOCARD)
		{
			memset(CardTag, 0, sizeof(CardTag));
			iRet = 0;
			goto FUNCEXIT;
		}
		else
		{
			iRet = 0;
			goto FUNCEXIT;
		}
	}
	
	FUNCEXIT:
	
	printf("HF320->APDU_RATS() e\n");
	
	return (iRet);
}

int HF320::APDU_Unlock(unsigned char* Key) // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
{
	unsigned char cmd[sizeof(MF320_14443A_APDU)];
	unsigned char datacmd[] = {0x00, 0x08, 0x00, 0x00, 0x08};
	unsigned char data[sizeof(datacmd) + 8];
	unsigned char recvData[BufferSize];
	char APDU_Resp[10];
	int i = 0;
	int iRet = 0;
	
	printf("HF320->APDU_Unlock() s\n");
	
	memset(recvData, 0, BufferSize);
	memset(data, 0, sizeof(data));
	memset(APDU_Resp, 0, sizeof(APDU_Resp));
	
	memcpy(cmd, MF320_14443A_APDU, sizeof(MF320_14443A_APDU));
	memcpy(data, datacmd, sizeof(datacmd));
	memcpy(data + sizeof(datacmd), Key, sizeof(data) - sizeof(datacmd));
	
	SendCommand(cmd, sizeof(data), data);
	i = GetResponse(recvData);
	
	if (i == 0)
	{
		iRet = -1;
		goto FUNCEXIT;
	}
	else if (i == -1)
	{
		iRet = 0;
		goto FUNCEXIT;
	}
	else
	{
		if (recvData[0] == HF320_OK)
		{
			ASCIIToString(APDU_Resp, (char *)recvData + i - 2, 2);
			printf("Rst:[%s]\n", APDU_Resp);
			
			if (strcmp(APDU_Resp, "9000") == 0)
				iRet = 1;
			else if (strcmp(APDU_Resp, "9840") == 0)
			{	// Key error
				iRet = -2;
			}
			
			goto FUNCEXIT;
		}
		else if (recvData[0] == HF320_ERR_NOCARD)
		{
			memset(CardTag, 0, sizeof(CardTag));
			iRet = 0;
			goto FUNCEXIT;
		}
		else
		{
			iRet = 0;
			goto FUNCEXIT;
		}
	}
	
	FUNCEXIT:
	
	printf("HF320->APDU_Unlock() e\n");
	
	return (iRet);
}

int HF320::APDU_ChangeKey(unsigned char* NewKey, unsigned char* OldKey) // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
{ // 待開發
	unsigned char cmd[sizeof(MF320_14443A_APDU)];
	unsigned char recvData[BufferSize];
	unsigned char datacmd[] = {0x00, 0x15, 0x00, 0x00, 0x08};
	unsigned char data[sizeof(datacmd) + 8];
	char APDU_Resp[10];
	int i = 0;
	int iRet = 0;
	
	printf("HF320->APDU_ChangeKey() s\n");
	
	iRet = APDU_Unlock(OldKey);
	
	if (iRet != 1)
		goto FUNCEXIT;
	
	memset(recvData, 0, BufferSize);
	memset(data, 0, sizeof(data));
	memset(APDU_Resp, 0, sizeof(APDU_Resp));
	
	memcpy(cmd, MF320_14443A_APDU, sizeof(MF320_14443A_APDU));
	memcpy(data, datacmd, sizeof(datacmd));
	memcpy(data + sizeof(datacmd), NewKey, sizeof(data) - sizeof(datacmd));
	
	SendCommand(cmd, sizeof(data), data);
	i = GetResponse(recvData);
	
	if (i == 0)
	{
		iRet = -1;
		goto FUNCEXIT;
	}
	else if (i == -1)
	{
		iRet = 0;
		goto FUNCEXIT;
	}
	else
	{
		if (recvData[0] == HF320_OK)
		{
			ASCIIToString(APDU_Resp, (char *)recvData + i - 2, 2);
			printf("Rst:[%s]\n", APDU_Resp);
			
			if (strcmp(APDU_Resp, "9000") == 0)
				iRet = 1;
			
			goto FUNCEXIT;
		}
		else if (recvData[0] == HF320_ERR_NOCARD)
		{
			memset(CardTag, 0, sizeof(CardTag));
			iRet = 0;
			goto FUNCEXIT;
		}
		else
		{
			iRet = 0;
			goto FUNCEXIT;
		}
	}
	
	FUNCEXIT:
	
	printf("HF320->APDU_ChangeKey() e\n");
	
	return (iRet);
}

bool HF320::APDU_mfRead(int Block, char* data) // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
{ // 待開發
	unsigned char recvData[BufferSize];
	unsigned char cmd[sizeof(MF320_14443A_APDU)];
	unsigned char datacmd[] = {0x00, 0x21, 0x28, 0x00, 0x01, 0x10};
	char msg[128];
	char APDU_Resp[10];
	int iRet = 0;
	
	printf("HF320->APDU_mfRead() s\n");
	
	memset(recvData, 0, BufferSize);
	memset(msg, 0, sizeof(msg));
	memset(APDU_Resp, 0, sizeof(APDU_Resp));
	
	memcpy(cmd, MF320_14443A_APDU, sizeof(MF320_14443A_APDU));
	datacmd[2] = Block & 0xFF;
	
	SendCommand(cmd, sizeof(datacmd), datacmd);
	iRet = GetResponse(recvData);
	
	if (iRet < 1)
		return (false);
	
	ErrorCodeToMessage(msg, (char)recvData[0]);
	printf("APDU CMD Read Return:[%s]\n", msg);
	printf("HF320->APDU_mfRead() e\n");
	
	if (recvData[0] == HF320_OK)
	{
		ASCIIToString(APDU_Resp, (char *)recvData + iRet - 2, 2);
		printf("Rst:[%s]\n", APDU_Resp);
		
		if (strcmp(APDU_Resp, "9000") == 0)
		{
			memcpy(data, (char *)recvData + 1, iRet - 1);
			return(true);
		}
		else
			return(false);
	}
	else
		return(false);
}

bool HF320::APDU_mfWrite(int Block, char data[], bool bChangeKey) // nick add 20160627 Ver:000-000-GIO_V2-13C241-0001-166241 //
{
	unsigned char recvData[BufferSize];
	unsigned char sendData[BufferSize];
	unsigned char cmd[sizeof(MF320_14443A_APDU)];
	unsigned char datacmd[] = {0x00, 0x22, 0x28, 0x00, 0x10};
	char msg[128];
	char APDU_Resp[10];
	int iRet = 0;
	int iLoc = 0;
	
	printf("HF320->APDU_mfWrite() s\n");
	
	memset(recvData, 0, BufferSize);
	memset(sendData, 0, BufferSize);
	memset(msg, 0, sizeof(msg));
	memset(APDU_Resp, 0, sizeof(APDU_Resp));
	
	memcpy(cmd, MF320_14443A_APDU, sizeof(MF320_14443A_APDU)); // command
	// Block Number
	datacmd[2] = Block & 0xFF;
	memcpy(sendData, datacmd, sizeof(datacmd));
	iLoc += sizeof(datacmd);
	//
	// Data
	StringToASCII((char *)sendData + iLoc, data, strlen(data) / 2);
	iLoc += strlen(data) / 2;
	//
	SendCommand(cmd, iLoc, sendData);
	iRet = GetResponse(recvData);
	
	if (iRet < 1)
		return (false);
	
	ErrorCodeToMessage(msg, (char)recvData[0]);
	printf("APDU CMD Write Return:[%s]\n", msg);
	printf("HF320->APDU_mfWrite() e\n");
	
	if (recvData[0] == HF320_OK)
	{
		ASCIIToString(APDU_Resp, (char *)recvData + iRet - 2, 2);
		printf("Rst:[%s]\n", APDU_Resp);
		
		if (strcmp(APDU_Resp, "9000") == 0)
		{
			memcpy(data, (char *)recvData + 1, iRet - 1);
			return(true);
		}
		else
			return(false);
	}
	else
		return(false);
}

bool HF320::APDU_mfShowDisplay(char data[]) // nick add 20160628 Ver:000-000-GIO_V2-13C241-0001-166241 //
{
	unsigned char recvData[BufferSize];
	unsigned char sendData[BufferSize];
	unsigned char cmd[sizeof(MF320_14443A_APDU)];
	unsigned char datacmd[] = {0x00, 0x24, 0x00, 0x13, 0x10};
	char msg[128];
	char APDU_Resp[10];
	int iRet = 0;
	int iLoc = 0;
	int i = 0;
	
	printf("HF320->APDU_mfShowDisplay() s\n");
	
	memset(recvData, 0, BufferSize);
	memset(sendData, 0, BufferSize);
	memset(msg, 0, sizeof(msg));
	memset(APDU_Resp, 0, sizeof(APDU_Resp));
	
	if (strlen(data) > 16)
		data[16] = 0;
	
	for (i = 0; i < 2; i++)
	{
		iRet = APDU_Unlock((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE);
		
		if (iRet == -2)
		{
			iRet = APDU_ChangeKey((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE, (unsigned char *)VISUAL_CARD_DEFAULT_KEY_CODE);
			
			if (iRet < 1)
				break;
		}
	}
	
	if (iRet < 1)
		return (false);
	
	memcpy(cmd, MF320_14443A_APDU, sizeof(MF320_14443A_APDU)); // command
	// Block Number
	datacmd[2] = 0;
	datacmd[4] = strlen(data);
	memcpy(sendData, datacmd, sizeof(datacmd));
	iLoc += sizeof(datacmd);
	//
	// Data
	memcpy(sendData + iLoc, data, strlen(data));
	iLoc += strlen(data);
	//
	SendCommand(cmd, iLoc, sendData);
	iRet = GetResponse(recvData);
	
	if (iRet < 1)
		return (false);
	
	ErrorCodeToMessage(msg, (char)recvData[0]);
	printf("APDU CMD Write Return:[%s]\n", msg);
	printf("HF320->APDU_mfShowDisplay() e\n");
	
	if (recvData[0] == HF320_OK)
	{
		ASCIIToString(APDU_Resp, (char *)recvData + iRet - 2, 2);
		printf("Rst:[%s]\n", APDU_Resp);
		
		if (strcmp(APDU_Resp, "9000") == 0)
		{
			//sleep(1);
			usleep(1500000L); // 等待1.5秒讓電子紙變化
			return(true);
		}
		else
			return(false);
	}
	else
		return(false);
}

void HF320::ErrorCodeToMessage(char* msg, char data)
{
	unsigned char eCode = (unsigned char)data;
	
	switch(eCode)
	{
		case HF320_OK:
			sprintf(msg,"Success.");
			break;
		case HF320_ERR_AUTHENTICATE:
			sprintf(msg,"Authenticate error.");
			break;
		case HF320_ERR_LRC:
			sprintf(msg,"Checksum error.");
			break;
		case HF320_ERR_CMD:
			sprintf(msg,"Command error.");
			break;
		case HF320_ERR_TRANSFER:
			sprintf(msg,"Transfer packet error for reader and card.");
			break;
		case HF320_ERR_NOCARD:
			sprintf(msg,"No card response.");
			break;
		default:
			sprintf(msg,"Unknow Code:%02X ", eCode);
			break;
	}
}

char HF320::CacheCard(char *CardID)
{	//auto mode .return: -1 讀卡機無回應; 0:沒有票卡 1:偵測有票卡
	unsigned char cmd[sizeof(MF320_14443A_GETID)];
	unsigned char recvData[BufferSize];
	int i = 0;
	int iRet = 0;
	
	// ========================================================= //
	// nick add s 20160624 Ver:000-000-GIO_V2-13C241-0001-166241 //
	if (m_bUseAPDU) // 使用APDU CMD前先Deselect
		i = Deselect();
	// nick add e 20160624 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //
	
	printf("HF320->CacheCard(CardID) s\n");
	
	memset(recvData, 0, BufferSize);
	memcpy(cmd, MF320_14443A_GETID, sizeof(MF320_14443A_GETID));
	
	SendCommand(cmd, 0, (unsigned char *)"");
	i = GetResponse(recvData);
	
	if (i == 0)
	{
		iRet = -1;
		goto FUNCEXIT;
	}
	else if (i == -1)
	{
		iRet = 0;
		goto FUNCEXIT;
	}
	else
	{
		if (recvData[0] == HF320_OK)
		{
			ASCIIToString(CardID, (char *)recvData + 1, i - 1);
			memcpy(CardTag, CardID, strlen(CardID));
			// nick mark 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //iRet = 1;
			
			if (m_bUseAPDU)
				iRet = APDU_RATS(); // nick add 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
			else
				iRet = 1;
			
			goto FUNCEXIT;
		}
		else if (recvData[0] == HF320_ERR_NOCARD)
		{
			memset(CardTag, 0, sizeof(CardTag));
			iRet = 0;
			goto FUNCEXIT;
		}
		else
		{
			iRet = 0;
			goto FUNCEXIT;
		}
	}
	
	FUNCEXIT:
	
	printf("HF320->CacheCard(CardID) e\n");
	
	return (iRet);
}

int HF320::Deselect()
{
	unsigned char cmd[sizeof(MF320_14443A_DESELECT)];
	unsigned char recvData[BufferSize];
	int i = 0;
	int iRet = 0;
	
	printf("HF320->Deselect() s\n");
	
	memset(recvData, 0, BufferSize);
	memcpy(cmd, MF320_14443A_DESELECT, sizeof(MF320_14443A_DESELECT));
	
	SendCommand(cmd, 0, (unsigned char *)"");
	i = GetResponse(recvData);
	
	if (i == 0)
	{
		iRet = -1;
		goto FUNCEXIT;
	}
	else if (i == -1)
	{
		iRet = 0;
		goto FUNCEXIT;
	}
	else
	{
		if (recvData[0] == HF320_OK)
		{
			iRet = 1;
			goto FUNCEXIT;
		}
		else if (recvData[0] == HF320_ERR_NOCARD)
		{
			memset(CardTag, 0, sizeof(CardTag));
			iRet = 0;
			goto FUNCEXIT;
		}
		else
		{
			iRet = 0;
			goto FUNCEXIT;
		}
	}
	
	FUNCEXIT:
	
	printf("HF320->Deselect() e\n");
	
	return (iRet);
}

int HF320::ReadSector(char *Tag, int sector, sectorData* sdata, bool ReSelect)
{
	int i = 0;
	int iRetryCnt = 0;
	int iRet = 1;
	int iBlock = sector * 4;
	char ReadData[20];
	bool bGetRtn = false;
	
	if (strlen(CardTag) <= 0)
	{ // CacheCard
		for (i = 0; i < RETRY; i++)
		{
			if (CacheCard(Tag) == 1)
				break;
		}
		
		if (i >= RETRY)
		{
			iRet = -1;
			goto FUNCEXIT;
		}
	}
	else
		memcpy(Tag, CardTag, sizeof(CardTag));

	MakeDBTagID(CardTag,Tag);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	
	// ========================================================= //
	// nick add s 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
	if (m_bUseAPDU)
	{
		for (i = 0; i < 2; i++)
		{
			iRet = APDU_Unlock((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE);
			
			if (iRet == -2)
			{
				iRet = APDU_ChangeKey((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE, (unsigned char *)VISUAL_CARD_DEFAULT_KEY_CODE);
				
				if (iRet < 1)
					break;
			}
		}
		
		if (iRet < 1)
			goto FUNCEXIT;
	}
	// nick add e 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //
	
	for (i = 0; i < 3; i++)
	{
		memset(ReadData, 0, sizeof(ReadData));

		for (iRetryCnt = 0; iRetryCnt < RETRY; iRetryCnt++)
		{
			// ========================================================== //
			// nick mark s 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
			//if (mfRead(iBlock + i, ReadData) == true)
			//	break;
			// nick mark e 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
			// ========================================================== //
			
			// ========================================================= //
			// nick add s 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
			if (m_bUseAPDU == false)
				bGetRtn = mfRead(iBlock + i, ReadData);
			else
				bGetRtn = APDU_mfRead(iBlock + i, ReadData);
			
			if (bGetRtn)
				break;
			// nick add e 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
			// ========================================================= //
		}
		
		if (iRetryCnt >= RETRY)
		{
			iRet = 0;
			goto FUNCEXIT;
		}
		
		if (i == 0)
			memcpy(sdata->block1, &ReadData, sizeof(strBlock1));
		else if (i == 1)
			memcpy(sdata->block2, &ReadData, sizeof(strBlock2));
		else if (i == 2)
			memcpy(sdata->block3, &ReadData, sizeof(strBlock3));
	}
	
	FUNCEXIT:
	
	return (iRet);
}

int HF320::WriteSector(char *Tag, int sector, sectorData sdata, bool ReSelect)
{
	int i = 0;
	int iRetryCnt = 0;
	int iRet = 1;
	int iBlock = sector * 4;
	char writeData[33];
	bool bGetRtn = false; // nick add 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
	
	memset(writeData, 0, sizeof(writeData));

	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	if (strlen(CardTag) <= 0)
	{ // CacheCard
		for (i = 0; i < RETRY; i++)
		{
			if (CacheCard(Tag) == 1)
				break;
		}
		
		if (i >= RETRY)
		{
			iRet = -1;
			goto FUNCEXIT;
		}
	}
	else
		memcpy(Tag, CardTag, sizeof(CardTag));
	
	MakeDBTagID(CardTag,Tag);	
	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

	
	// ========================================================= //
	// nick add s 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
	if (m_bUseAPDU)
	{
		for (i = 0; i < 2; i++)
		{
			iRet = APDU_Unlock((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE);
			
			if (iRet == -2)
			{
				iRet = APDU_ChangeKey((unsigned char *)VISUAL_CARD_AUTHENTICATE_KEY_CODE, (unsigned char *)VISUAL_CARD_DEFAULT_KEY_CODE);
				
				if (iRet < 1)
					break;
			}
		}
		
		if (iRet < 1)
			goto FUNCEXIT;
	}
	// nick add e 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //
	
	for (i = 0; i < 3; i++)
	{
		if ((G_MemBlocksWriteSuc[sector] & (0x01 << i)) == 0)
		{
			memset(writeData, 0, sizeof(writeData));
			
			if (i == 0)
				ASCIIToString(writeData, (char *)sdata.block1, sizeof(strBlock1));
			else if (i == 1)
				ASCIIToString(writeData, (char *)sdata.block2, sizeof(strBlock2));
			else if (i == 2)
				ASCIIToString(writeData, (char *)sdata.block3, sizeof(strBlock3));
			else
			{
				iRet = 0;
				goto FUNCEXIT;
			}

			for (iRetryCnt = 0; iRetryCnt < RETRY; iRetryCnt++)
			{
				// ========================================================== //
				// nick mark s 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
				//if (mfWrite(iBlock + i, writeData) == true)
				//	break;
				// nick mark e 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
				// ========================================================== //
				
				// ========================================================= //
				// nick add s 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
				if (m_bUseAPDU == false)
					bGetRtn = mfWrite(iBlock + i, writeData);
				else
					bGetRtn = APDU_mfWrite(iBlock + i, writeData);
				
				if (bGetRtn)
					break;
				// nick add e 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //
				// ========================================================= //
			}
			
			if (iRetryCnt >= RETRY)
			{
				iRet = 0;
				goto FUNCEXIT;
			}
			
			G_MemBlocksWriteSuc[sector] |= (0x01 << i);
		}
	}
	
	FUNCEXIT:
	
	return (iRet);
}

bool HF320::ClearSector(int sector)
{
	int i = 0;
	int j = 0;
	int iBlock = sector * 4;
	char writeData[33];
	char CardID[24];
	
	for(i = 0; i < RETRY; i++)
	{
		if (CacheCard(CardID) == 1)
			break;
	}
	
	memset(writeData, 'F', sizeof(writeData));
	writeData[32] = 0x00;
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < RETRY; j++)
		{
			if (mfWrite(iBlock + i, writeData))
				break;
		}
		
		if (j >= RETRY)
			break;
	}
	
	if (i < 3)
		return (false);
	else
		return (true);
}

void HF320::ClearBuff(void)
{
	comport.Clear232Bufer();
}

int HF320::ChangeSectorKey(int sector) // 還沒測試 //
{
	int i = 0;
	int iBlock = sector * 4;
	char writeData[33];
	char CardID[24];
	
	for(i = 0; i < RETRY; i++)
	{
		if (CacheCard(CardID) == 1)
			break;
	}
	
	if (i >= RETRY)
		goto FUNCEXIT;
	
	memset(writeData,'\0', sizeof(writeData));
	sprintf(writeData, "%s""7F078800""%s", AuthenticateCodeA[sector - 1], AuthenticateCodeB[sector - 1]);
	
	for (i = 0; i < RETRY; i++)
	{
		if (mfWrite(iBlock + 3, writeData, true))
			break;
	}

	FUNCEXIT:
	
	if (i < RETRY)
		return (1);
	else
		return (0);
}

bool HF320::ShowDisplay(char* DisplayData, int Datalen) // nick add 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
{
	bool bRtn = true;
	
	if (m_bUseAPDU)
	{
		bRtn = APDU_mfShowDisplay(DisplayData);
	}
	
	//FUNCEXIT:
	
	return(bRtn);
}

// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
void HF320::MakeDBTagID(char* STagID,char* DTagID)
{	
	for(int a = strlen(STagID)-2, i = 0; a>=0;a-=2, i++)
	{
		memcpy(DTagID + (2 * i), STagID + a , 2);
	}
}

