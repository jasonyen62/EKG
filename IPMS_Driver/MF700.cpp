/* MF700 Reader Class */
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "../traceLog.h"
#include "../CommonDef.h"
#include "MF700.h"

#define RETRY 3

// ========================================================= //
// nick add s 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
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
// nick add e 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
// ========================================================= //

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

MF700::MF700(void)
{
	BufferSize = 128;   // 20120221 Tony add
	m_bTagMode = false;  //MF700 Auto Mode
	ReaderID=0x00;
	m_bAlreadySelCard=true;
	m_bHalted = false;
	m_bUltraLight = false;
	
	memset(CardID,'\0',sizeof(CardID));
}

MF700::~MF700(void)
{
	Close();
}

void MF700::init(enum COMPORT port)
//bool MF700::init(enum COMPORT port) // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
{
	//bool bRtn = false; // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	my_port = port;
	comport.PortInit2(port,19200,'N',8,1);
	//comport.PortInitForHF320(port,19200,'N',8,1);
	ReaderID = 0;
	
	memset(SectorData, 0x00, sizeof(SectorData));

	//nick add s 20110623
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
	int len = 0;
	int iRet = 0;
	
	memset(recvData,'\0',sizeof(recvData));
	
	len = SendCommand(mf_REQUEST,0,(char *)"");
	iRet = GetResponse(recvData);

	// 神邏輯 勿動 s //
	if (iRet == 0)
	{
		Close();
		usleep(100000L);
		comport.PortInit2(port,115200,'N',8,1);
		//comport.PortInitForHF320(port,115200,'N',8,1);
		ReaderID = 0;
		memset(SectorData, 0x00, sizeof(SectorData));
		//bRtn = true;
	}
	//else // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	//	bRtn = true; // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	// 神邏輯 勿動 e //

	//printf("Initial MF700 Result:[%d]\n", bRtn); // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	//return(bRtn); // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	//nick add e 20110623
}

void MF700::Close()
{
	char buffMsg[BufferSize];
	
	comport.PortClose();
	sprintf(buffMsg, "MF700 Port Close Port:[ COM%d ].", my_port);
	ShowMessage(buffMsg);
}

void MF700::SetTagMode(bool bSet)
{
	m_bTagMode = true;
//    mfHalt();
}

char MF700::mfRequest(void)
{	//return Class number
	int len = 0;
	int i,iRet,ClassType = 0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char message[48];
	//char buf[128];

	m_bUltraLight = false;
	
	for(i = 0; i < RETRY; i++)
	{
		memset(recvData,'\0',sizeof(recvData));
		
		len = SendCommand(mf_REQUEST,0,(char *)"");
		
		iRet = GetResponse(recvData);
		//printf("[%s]\n",recvData);
		
		if(iRet == 1)
		{
			m_bHalted = false;
			sscanf(recvData,"%4X",&ClassType);
			
			if(ClassType == 0x4400)
				m_bUltraLight = true;
			
			printf("MF700 -> mfRequest() :: ClassType = [%d]\n", ClassType);
			// nick mark 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //return ClassType;
			return (iRet); // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			return -1;
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			//comport.PortClose();
			Close();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	
	return 0;
}

char MF700::mfAnticollision(char* CardID)
{   //return Card S/N
	int i,iRet,len=0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char message[48];
	char buf[128];

	for(i=0; i<RETRY; i++)
	{
		memset(recvData,'\0',sizeof(recvData));
		
		len = SendCommand(mf_ANTICOLL,0,(char *)"");
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			sprintf(CardID,"%s",recvData);
			return 1;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Anticollision : %s",message);
			ShowMessage(buf,2);
			return -1;
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
			
	}
	
	return 0;
}

int MF700::mfSelectCard(char CardSN[])
{   //return Card Memory size
	int i,iRet,len=0;
	int MSize=0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char message[48];
	char buf[128];

	for(i=0; i<RETRY; i++)
	{
		memset(recvData,'\0',sizeof(recvData));
		
		len = SendCommand(mf_SELECTCARD,8,CardSN);
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			sscanf(recvData,"%2X",&MSize);
			m_bAlreadySelCard = true;
			return MSize;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 SlectcCard : %s",message);
			ShowMessage(buf,2);
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	
	return 0;
}

// nick mark 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //bool MF700::mfAuthenticate(int Sector,enum MF700_KEY_TYPE keyType)
bool MF700::mfAuthenticate(int Sector,enum MF700_KEY_TYPE keyType, bool bUseDefaultKey) // nick add 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
{
	int i,iRet,len=0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char data[24];
	char Key[13];
	char message[48];
	char buf[128];
    
	if(m_bUltraLight)
	{
		return true;
	}
	m_Sector = Sector;
	
	memset(Key,'\0',sizeof(Key));
	
	// ========================================================== //
	// nick mark s 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	//if(keyType == MF_KEYA)
	//{
	//	sprintf(Key,"%s",AuthenticateCodeA[Sector-1]);
	//}
	//else
	//{
	//	sprintf(Key,"%s",AuthenticateCodeB[Sector-1]);
	//}
	// nick mark e 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	// ========================================================== //
	
	// ========================================================= //
	// nick add s 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	if (bUseDefaultKey == false)
	{
		if(keyType == MF_KEYA)
			sprintf(Key, "%s", AuthenticateCodeA[Sector-1]);
		else
			sprintf(Key, "%s", AuthenticateCodeB[Sector-1]);
	}
	else
	{
		if(keyType == MF_KEYA)
			sprintf(Key, "%s", AuthenticateDefaultCodeA[Sector-1]);
		else
			sprintf(Key, "%s", AuthenticateDefaultCodeB[Sector-1]);
	}
	// nick add e 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	// ========================================================= //
	
	// ========================================================== //
	// nick mark s 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	////Frank add s 20120911
	//if(Sector != 10 && Sector !=12)
	//{
	//	memset(Key , '\0' , sizeof(Key));
	//	sprintf(Key , "%s" , AuthenticateCodeA[11]);
	//}
	////Frank add e 20120911
	// nick mark e 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
	// ========================================================== //
	
	for(i=0; i<RETRY; i++)
	{
		memset(data,'\0',sizeof(data));
		memset(recvData,'\0',sizeof(recvData));
		
		sprintf(data,"%02X%02X%12s",keyType,Sector,Key);
		
		len = SendCommand(mf_AUTHENWITHKEY,16,data);
		
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			return true;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Authenticate : %s",message);
			ShowMessage(buf,3);
		}
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	
	return false;
}

void MF700::StringToASCII(char* result,char data[])
{
	int i;
	unsigned char value;

	for(i=0;i<16;i++)
	{
		sscanf(data+i*2,"%2X",(unsigned int *)&value);
		result[i] = value;
	}
}

void MF700::ASCIIToString(char* result,char data[])
{
	int i;
	char ascString[40];

	memset(ascString,'\0',sizeof(ascString));
	
	for(i=0;i<16;i++)
	{
		sprintf(ascString+i*2,"%02X",(unsigned char)data[i]);
	}
	
	sprintf(result,"%s",ascString);
}

bool MF700::mfRead(int Block,char* data)
{
	int i,iRet,len=0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char sdata[40];
	char BlockData[20];
	char message[48];
	char buf[128];

	memset(BlockData,'\0',sizeof(BlockData));
	
	for(i=0; i<RETRY; i++)
	{
		memset(sdata,'\0',sizeof(sdata));
		memset(recvData,'\0',sizeof(recvData));
		
		sprintf(sdata,"%02X",Block);
		len = SendCommand(mf_READBLOCK,2,sdata);
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			StringToASCII(BlockData,recvData);
			memcpy(data,BlockData,16);
			return true;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Read : %s",message);
			ShowMessage(buf,2);
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	
	return false;
}

bool MF700::mfWrite(int Block, char data[])
{
	int		i,iRet,len=0;
	// 20120221 Tony mark char		recvData[128];
	char		recvData[BufferSize];  // 20120221 Tony add
	char		sdata[40];
	char		wdata[40];
	char		message[48];
	char		buf[128];

	memset(wdata,'\0',sizeof(wdata));
	memset(message,'\0',sizeof(message));
	
	for(i=0; i<RETRY; i++)
	{
		memset(sdata,'\0',sizeof(sdata));
		memset(recvData,'\0',sizeof(recvData));

		ASCIIToString(wdata,data);
		if(m_bUltraLight == false)
		{
			sprintf(sdata,"%02X%32s",Block,wdata);		
			len = SendCommand(mf_WRITEBLOCK,34,sdata);
		}
		else
		{
			//sprintf(sdata,"%8s",wdata);
			memcpy(sdata,wdata,8);
			len = SendCommand(mf_WRITEBLOCK,8,sdata);
			sprintf(buf,"MF700 write : [%s]\n",sdata);
			ShowMessage(buf,2);
		}
		
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			return true;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Write : %s",message);
			ShowMessage(buf,2);
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	return false;
}

bool MF700::mfWriteKey(char data[])
{
	int		i,iRet,len=0;
	int Block = 3;
	// 20120221 Tony mark char		recvData[128];
	char		recvData[BufferSize];  // 20120221 Tony add
	char		sdata[40];
	char		wdata[40];
	char		message[48];
	char		buf[128];

	memset(wdata,'\0',sizeof(wdata));
	memset(message,'\0',sizeof(message));
	
	for(i=0; i<RETRY; i++)
	{
		memset(sdata,'\0',sizeof(sdata));
		memset(recvData,'\0',sizeof(recvData));

		if(m_bUltraLight == false)
		{
			sprintf(sdata,"%02X%32s", Block, data);		
			len = SendCommand(mf_WRITEBLOCK,34,sdata);
		}
		else
		{
			//sprintf(sdata,"%8s",wdata);
			memcpy(sdata,data,8);
			len = SendCommand(mf_WRITEBLOCK,8,sdata);
			sprintf(buf,"MF700 write : [%s]\n",sdata);
			ShowMessage(buf,2);
		}
		
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			return true;
		}
		else if(iRet == -1)
		{
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Write : %s",message);
			ShowMessage(buf,2);
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	return false;
}

bool MF700::mfValueSet(int Block,enum MF700_OPT option,long value)
{
	return true;
}

char MF700::mfHalt(void)
{
	int i,iRet,len=0;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize]; // 20120221 Tony add
	char sdata[40];
	char message[48];
	char buf[128];
	unsigned int ecode;

	return 1;					//Frank add 20120723
	//if( m_bTagMode == false)
	//    return 1;
	if(m_bHalted == true)
		return 1;
	
	for(i=0; i<RETRY; i++)
	{
		memset(sdata,'\0',sizeof(sdata));
		memset(recvData,'\0',sizeof(recvData));

		len = SendCommand(mf_HALT,0,sdata);  
		iRet = GetResponse(recvData);
		
		if(iRet == 1)
		{
			m_bHalted = true;
			m_bAlreadySelCard = false;
			memset(CardID,'\0',sizeof(CardID));
			return 1;
		}
		else if(iRet == -1)
		{
			sscanf(recvData,"%2X",&ecode);
			
			if (ecode == 0x00)
			{
				m_bHalted = true;
				m_bAlreadySelCard = false;
				memset(CardID,'\0',sizeof(CardID));
				return 1;
			}
			
			ErrorCodeToMessage(message,recvData);
			sprintf(buf,"MF700 Halt : %s recv:%s",message,recvData);
			ShowMessage(buf,2);
			return -1;
		}
		// ========================================================= //
		// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if (iRet == 0)
		{
			comport.PortClose();
			usleep(100000L);
			init(my_port);
		}
		// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
	}
	
	/*
	m_bHalted = true;
	m_bAlreadySelCard = false;
	memset(CardID,'\0',sizeof(CardID));
	return 1;
	*/
	
	return 0;
}

bool MF700::mfSaveKey(enum MF700_KEY_TYPE keyType,int Sector,char* Key)
{
	return true;
}

bool MF700::mfAccessCondition(char* keyA, char* keyB,BYTE CB0,BYTE CB1,BYTE CB2,BYTE CB3, BYTE GPByte)
{
	return true;
}

bool MF700::mfGetAccessCondition(BYTE CB0,BYTE CB1,BYTE CB2,BYTE CB3, BYTE GPByte)
{
	return true;
}

int MF700::SendCommand(enum MF700_COMMAND cmd,int len,char* data)
{
	char sendData[128];
	char buf[128];
	int sLen;

	memset(sendData,'\0',sizeof(sendData));
	memset(buf, '\0', sizeof(buf));
	
	len >>= 1;
	sprintf(sendData,":%02X%02X%02X%s\x0d",ReaderID,cmd,len,data);
	sprintf(buf, " MF700 Send:%s", sendData);
	ShowMessage(buf,2);
	sLen = strlen(sendData);
	ClearBuff();    // 20120221 Tony add
	// nick mark 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //comport.PortWrite(sendData,sLen);
	comport.PortWriteForMF700(sendData,sLen); // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
	return sLen;
}

int MF700::GetResponse(char* rData)
{   // return 0:no response 1:Received -1:received error
	int len = 0;
	char recvData[128];
	char* response = NULL;
	int rID,Func;
	int DataLen = 0; // nick add 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //

	memset(recvData,'\0',sizeof(recvData));

	printf("Start Recive~~~\n");
	
	len = BufferSize; // 20120221 Tony add
	
	if(comport.PortRead2(&len,recvData,100) == true)
	{
		response = strchr(recvData+2,':');
		if(response == NULL)
			return -1;
		
		if (len-(response-recvData)<8)	// 20110725 Nick add
			return -1;					// 20110725 Nick add
		
		// nick mark 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //sscanf(response,":%2X%2X",&rID,&Func);
		sscanf(response,":%2X%2X%2X", &rID, &Func, &DataLen); // nick add 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //
		sprintf(rData,"%s",response+7);
		DataLen *= 2; // nick add 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //
		rData[DataLen] = 0; // nick add 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //
		//ShowMessage(response,2);
		
		if(Func == 0x06)
			return 1;
	}
	else
	{
		ShowMessage((char *)"MF700 no received.",2);
		return 0;
	}
	
	return -1;
}

void MF700::ErrorCodeToMessage(char* msg,char data[])
{
	unsigned int ecode;

	sscanf(data,"%2X",&ecode);
	switch(ecode)
	{
	case mf_ERR_EMPTY:
		sprintf(msg,"Empty.");
		break;
	case mf_ERR_AUTHENTICATE:
		sprintf(msg,"Authenticate error.");
		break;
	case mf_ERR_KEY:
		sprintf(msg,"Key error.");
		break;
	case mf_ERR_NOT_AUTHENT:
		sprintf(msg,"Not Authenticate Error.");
		break;
	case mf_ERR_TRANSFER:
		sprintf(msg,"Transfer Error.");
		break;
	case mf_ERR_WRITE:
		sprintf(msg,"Write Error.");
		break;
	case mf_ERR_INC:
		sprintf(msg,"INC Error.");
		break;
	case mf_ERR_DEC:
		sprintf(msg,"DEC Error.");
		break;
	case mf_ERR_READ:
		sprintf(msg,"Read Error.");
		break;
	case mf_ERR_TIMEOUT:
		sprintf(msg,"Access Timeout.");
		break;
	case mf_ERR_NOTAG:
		sprintf(msg,"No TAG .");
		break;
	case mf_ERR_WRONG_PARAM:
		sprintf(msg,"Wrong Parameter value.");
		break;
	case mf_ERR_HOST_AUTHENT:
		sprintf(msg,"Host Authenticate Error.");
		break;
	case mf_ERR_DESKEY:
		sprintf(msg,"Wrong DES Key.");
		break;
	case mf_ERR_DESKEY_LOAD:
		sprintf(msg,"DES Key Load Error.");
		break;
	case mf_ERR_COMMAND_DENY:
		sprintf(msg,"Gnet Command Deny.");
		break;
	case mf_ERR_COMMAND_ILLEGAL:
		sprintf(msg,"Gnet Command Illegal.");
		break;
	case mf_ERR_COMMAND_OVERRUN:
		sprintf(msg,"Gnet Command Over Run.");
		break;
	case mf_ERR_COMMAND_CRC:
		sprintf(msg,"Gnet Package CRC Error.");
		break;
	case mf_ERR_COMMAND_MEMORY:
		sprintf(msg,"Gnet Out of Memory.");
		break;
	case mf_ERR_COMMAND_FRAME:
		sprintf(msg,"Gnet Out of Frame.");
		break;
	case mf_ERR_COMMAND_UNKNOW :
		sprintf(msg,"Gnet Unknow Command.");
		break;
	default:
		sprintf(msg,"Unknow Code:%02X ",ecode);
		break;
	}
}

char MF700::CacheCard(char *CardID)
{	//auto mode .return: -1 讀卡機無回應; 0:沒有票卡 1:偵測有票卡
//	int len=0;
//	char recvData[1024];

	if(mfHalt() == -1)
		return -1;
    
	/*
	memset( recvData,'\0', sizeof(recvData));
	if(comport.PortRead2(&len, recvData, 5) == true)
	{
		if(recvData[0] == 0x02)
		{
			//sprintf(CardID,"%s",recvData);
			strncpy(CardID,recvData,8);
			return 1;
		}
	}
	*/
	
	//Request
	// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(mfRequest()>=0)
	if(mfRequest() > 0) // nick add 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	{
		//Anti-coolision
		if(mfAnticollision(CardID)!=1)
		{
			ShowMessage((char *)"Read Anticollision Error!",2);
			return -2;
		}
		
		CardID[8]=0;
		return 1;
	}
//	sprintf(Tag,"%s",CardID);

	return 0;
}

char MF700::CacheCard()
{
	int len = 0;
	// 20120221 Tony mark char recvData[1024];
	char recvData[BufferSize];    // 20120221 Tony add

	if(mfHalt()==-1)
		return -1;
	
	memset(recvData,'\0', sizeof(recvData));
	
	len = BufferSize; // 20120221 Tony add
	
	if(m_bTagMode == false)
	{
		if(mfRequest()<0) 
		{
			//ShowMessage("No Ticket ",2);
			return -1;
		}
		return 1;
	}	
	else if(comport.PortRead2(&len, recvData, 5) == true)
	{
		if(recvData[0] == 0x02)
		{
			return 1;
		}
	}
	return 0;
}

int MF700::ReadSector(char *Tag,int sector,sectorData* sdata,bool ReSelect)
{
	int i,mSize,block=0;
	char ReadData[20];
	
//	if(m_bAlreadySelCard == false || sector != m_Sector)
//	{
		//Request
		for(i=0;i<5;i++)
		{
			if(mfRequest()<0) 
			{
				usleep(100000L);
				continue;
			}
			break;
		}
		if(i==5)
			return -1;
		//Anti-coolision
		if(mfAnticollision(CardID)!=1)
		{
			ShowMessage((char *)"Read Anticollision Error!",2);
			return -2;
		}
		// nick mark 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //CardID[8]=0;
		sprintf(Tag,"%s",CardID);
		//printf("CardID:%s \n",CardID);
		//Select Card
		mSize = mfSelectCard(CardID);
		
		if(mSize <1)
		{
			ShowMessage((char *)"Read Select Card Error!",2);
			return -3;
		}
		//Authenticate
		if(mfAuthenticate(sector,MF_KEYA) == false)
		{
			ShowMessage((char*)"Read Authenticate error!",2);
			return -4;
		}
//	}
	
	//Read block1
	if(m_bUltraLight)
		block = sector;
		
	memset(ReadData , '\0' , sizeof(ReadData));
	
	if(mfRead(block + 0 , ReadData) == false)
	{
		ShowMessage((char *)"Read Block1 Error!",2);
		return 0;
	}
	
	memcpy (sdata->block1 , &ReadData , sizeof(strBlock1));
	
	if(m_bUltraLight == false)
	{
		//Read block2
		
		memset(ReadData , '\0' , sizeof(ReadData));
		
		if(mfRead(block+1,ReadData) == false)
		{
			ShowMessage((char *)"Read Block2 Error!",2);
			return 0;
		}
		
		memcpy (sdata->block2,&ReadData,sizeof(strBlock2));
		//Read block3
		
		memset(ReadData , '\0' , sizeof(ReadData));
		
		if(mfRead(block+2,ReadData) == false)
		{
			ShowMessage((char *)"Read Block3 Error!",2);
			return 0;
		}
		
		memcpy (sdata->block3,&ReadData,sizeof(strBlock3));
	}
	return 1;
}

int MF700::WriteSector(char *Tag, int sector, sectorData sdata, bool ReSelect)
{
	int i,mSize,block = 0,iRet = 1;
	char CardID[24];
	char writeData[20];
	char buf[128];
	
	memset(buf,'\0',sizeof(buf));
	memset(CardID,'\0',sizeof(CardID));

//	if(m_bAlreadySelCard == false || sector != m_Sector || ReSelect == true)
//	{
		for(i=0;i<3;i++)
		{
			//Request
			if(mfRequest()<0)
			{
				//ShowMessage("Request error",2);
				iRet = -1;
				continue;
			}
			
			//Anti-coolision
			if(mfAnticollision(CardID) == false)
			{				
				ShowMessage((char*)"MF700::Anticollision Error! ",2);
				iRet = -2;
				continue;
			}
			
// nick mark 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //			CardID[8]=0;
// nick mark 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //			sprintf(Tag,"%s",CardID);
			//Select Card
			mSize = mfSelectCard(CardID);
			
			if(mSize <1)
			{
				sprintf(buf,"MF700::Select Card:[%s] ",CardID);
				ShowMessage(buf,2);
				iRet = -3;
				continue;
			}
			
			//Authenticate
			if(mfAuthenticate(sector, MF_KEYA) == false)
			{
				ShowMessage((char *)"MF700::Write sector Authenticate error!",2);
				iRet = -4;
				continue;
			}
			
			break;
		}
		
		if(i==3)
			return iRet;
//	}

	//Write block1
	if ((G_MemBlocksWriteSuc[sector] & 0x01) == 0) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
	{
		memset(writeData,'\0',sizeof(writeData));
		memcpy (writeData,sdata.block1,sizeof(strBlock1));

		if(mfWrite(block+0,writeData) == false)
		{
			ShowMessage((char *)"MF700::Write Block1 Error!",2);
			return 0;
		}
	}

	G_MemBlocksWriteSuc[sector] |= 0x01; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //

	if(m_bUltraLight == false)
	{
		//Write block2
		if ((G_MemBlocksWriteSuc[sector] & 0x02) == 0) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		{
			memset(writeData,'\0',sizeof(writeData));
			memcpy (writeData,sdata.block2,sizeof(strBlock2));
			
			if(mfWrite(block+1,writeData) == false)
			{
				ShowMessage((char *)"MF700::Write Block2 Error!",2);
				return 0;
			}
		}

		G_MemBlocksWriteSuc[sector] |= 0x02; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //

		//Write block3
		if ((G_MemBlocksWriteSuc[sector] & 0x04) == 0) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		{
			memset(writeData,'\0',sizeof(writeData));
			memcpy(writeData,sdata.block3,sizeof(strBlock3));
			
			if( mfWrite(block+2,writeData) == false)
			{
				ShowMessage((char*)"MF700::Write Block3 Error!",2);
				return 0;
			}
		}

		G_MemBlocksWriteSuc[sector] |= 0x04; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
	}
	
	return iRet;
}

bool MF700::ClearSector(int sector)
{
	int i, mSize, block = 0;
	char  CardID[24];
	char  writeData[20];

	if(m_bAlreadySelCard == false || sector != m_Sector)
	{
		for(i=0;i<3;i++)
		{
			//Request
			if(mfRequest()<0)
			{
				//ShowMessage("Request error",2);
				continue;
			}
			
			//Anti-coolision
			if(mfAnticollision(CardID) == false)
			{
				ShowMessage((char*)"Anticollision Error!",2);
				continue;
			}
			
			//printf("CardID:%s \n",CardID);
			//Select Card
			mSize = mfSelectCard(CardID);
			
			if(mSize <1)
			{
				ShowMessage((char*)"Select Card Error!",2);
				continue;
			}
			
			//Authenticate
			if( mfAuthenticate(sector,MF_KEYA)==false)
			{
				ShowMessage((char *)"Write sector Authenticate error!",2);
				continue;
			}
			
			break;
		}
		
		if(i==3)
			return false;
	}
	
	memset(writeData, 0xFF, sizeof(writeData));
	
	writeData[19] = 0;
	
	//Write block1
	if(mfWrite( block + 0, writeData) == false)
	{
		ShowMessage((char *)"Write Block1 Error!",2);
		return false;
	}
	
	if(m_bUltraLight == false)
	{
		//Write block2
		if(mfWrite( block + 1, writeData) == false)
		{
			ShowMessage((char*)"Write Block2 Error!",2);
			return false;
		}
		
		//Write block3
		if(mfWrite( block + 2, writeData) == false)
		{
			ShowMessage((char*)"Write Block3 Error!",2);
			return false;
		}
	}
	
	return true;
}

void MF700::ClearBuff(void)					//Frank add s 20111116
{
	comport.Clear232Bufer();
}					//Frank add e 20111116

// ========================================================= //
// nick add s 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
int MF700::ChangeSectorKey(int sector)
{
	int i;
	int mSize;
	char writeData[33];
	
	// Request
	for(i = 0; i < 5; i++)
	{
		if(mfRequest() < 0) 
		{		
			usleep(100000L);
			continue;
		}
		
		break;
	}
	
	if (i == 5) return -1;
	//
	//Anti-coolision
	if (mfAnticollision(CardID) != 1)
	{
		return -2;
	}
	//
	//Select Card
	mSize = mfSelectCard(CardID);
	
	if(mSize < 1)
	{
		return -3;
	}
	//
	//Authenticate
	if(mfAuthenticate(sector, MF_KEYA, true) == false)
	{
		return -4;
	}
	//
	// Write Key
	memset(writeData,'\0', sizeof(writeData));
	
	sprintf(writeData, "%s""7F078800""%s", AuthenticateCodeA[sector - 1], AuthenticateCodeB[sector - 1]);
	
	if (mfWriteKey(writeData) == false)
	{
		ShowMessage((char *)"ChangeSectorKey() Write Key Error!");
		return 0;
	}
	//
	
	return 1;
}
// nick add e 20140411 Ver:000-000-GIO_V2-135101-0002-13B251 //
// ========================================================= //