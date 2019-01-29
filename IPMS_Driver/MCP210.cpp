/*MCP210 Reader Class*/
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "../traceLog.h"
#include "../CommonDef.h"
#include "../Reader.h"
#include "MCP210.h"

char* cls = NULL;

MCP210::MCP210(void)
{
	BufferSize = 128;
}

MCP210:: ~MCP210(void)
{
	Close();
}

void MCP210::Init(enum COMPORT port)
{
	//comport.PortInit(port , 9600 , 'N' , 8 , 1);
	comport.PortInit2(port , 9600 , 'N' , 8 , 1);
	usleep(100000L);
}

void MCP210::Close(void)
{
	comport.PortClose();
}

void MCP210::ClearBuff(void)
{
	comport.Clear232Bufer();
}

//Frank mark 20121102 void MCP210::Reset(void)
bool MCP210::Reset(void)					//Frank add 20121102
{
	char RecvData[BufferSize];
	char iRet;					//Frank add 20121102
	
	memset(RecvData , '\0' , BufferSize);
	
	Init(ReaderCFG.HourlyReaderComPort);
	
	SendCommand(MCP210_C_RESET, cls , 0);
	//Frank mark 20121102 Response(RecvData , 5000);
	
	// ========================================================== //
	// nick mark s 20140527 Ver:000-000-GIO_V2-135101-0004-13B251 //
	////Frank add s 20121102
	//if(Response(RecvData , 5000) == false)
	//	iRet = true;
	//else
	//	iRet = false;
	////Frank add e 20121102
	// nick mark e 20140527 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================== //
	
	iRet = Response(RecvData , 5000); // nick add 20140527 Ver:000-000-GIO_V2-135101-0004-13B251 //
	Close();
	
	return iRet;					//Frank add 20121102
}

void MCP210::Restart(void)
{
	Close();
	Init(ReaderCFG.HourlyReaderComPort);
}

char MCP210::SendCommand(enum MCP210_Command cmd, char* sData, int DataLen)
{	//1:傳送成功  0:傳送失敗  -1:命令錯誤  -2:NAK
	char SendData[BufferSize] , RecvData[BufferSize] , iRet = 0;
	unsigned char BCC  = 0x00;
	int len , i , iRetry = 0;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	static long lngTick = GetTickCount();
	static unsigned long lngTick = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	memset(SendData , '\0' , BufferSize);
	memset(RecvData , '\0' , BufferSize);
	
	BCC ^= cmd;
	BCC ^= ETX;
	
	if (DataLen > 0)
	{
		for (i = 0 ; i < DataLen ; i++)
			BCC ^= *(sData + i);
			
		sprintf(SendData , "%c%c%s%c%c" , STX , cmd , sData , ETX , BCC);
	}
	else
	{
		sprintf(SendData , "%c%c%c%c" , STX , cmd , ETX , BCC);
	}
	
again:
	
	while(true)
	{
		if (CheckTimeout(&lngTick, (unsigned long)200))
		{
			comport.PortWrite(SendData , 4 + DataLen);
			lngTick = GetTickCount();
			break;
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	memset(RecvData , '\0' , BufferSize);
	
	len = comport.PortRead3(RecvData , 1000 , 1);
	
	if (len > 0)
	{
		if(RecvData[0] == DLE)
		{
			iRet = -1;
		}
		else if (RecvData[0] == NAK)
		{		
			iRetry++;
			
			if (iRetry <= 3)
			{
				goto again;
			}
			else
			{
				iRet = -2;
			}
		}
		else
		{
			iRet = 1;
		}
	}
	else
	{
		printf("MCP210 SendCMD No Response !\n");
		iRet = 0;
	}
	
	return iRet;
}

char MCP210::GetStatus(void)
{
	char iRet = 0 , RecvData[BufferSize];
	
	memset(RecvData , '\0' , BufferSize);
		
	Init(ReaderCFG.HourlyReaderComPort);
	SendCommand(MCP210_C_STATUS , cls , 0);
	
	if((Response(RecvData , 500)) == false)
	{
	//	printf("MCP210 no response\n");
		iRet =-2 ;
	}
	else if(RecvData[3] == '2')
	{
	//	printf("Either invalid card or card waiting to be pulled out exists \n");
		iRet = 2;
	}
	else if(RecvData[4] == '1')
	{
	//	printf("Inside\n");
		iRet = 3;
	}
	else if((RecvData[3] == '0') && (RecvData[4] == '0'))
	{
	//	printf("NoCard\n");
		iRet = 0;
	}
	else if((RecvData[3] == '1') && (RecvData[4] == '0'))
	{
	//	printf("WaitIn\n");
		iRet = 1;
	}
	else
	{
	//	printf("Error\n");
		iRet = -1;
	}
	
	Close();
	return iRet;
}

char MCP210::EjectCardOut(char* strPrint)
{
	char iRet = 0 , RecvData[BufferSize] , Font[] = "0" ;
	
	memset(RecvData , '\0' , BufferSize);
	memset(buf , '\0' , sizeof(buf));
	
	Init(ReaderCFG.HourlyReaderComPort);
	
	if(strlen(strPrint) == 0)
	{
		SendCommand(MCP210_C_EJECT , cls , 0);
	}
	else
	{
		if(SendCommand(MCP210_C_SETFONT , Font , 1) != 1)
		{
			Restart();
//			printf("MCP210_C_SETFONT Restart!\n");
			SendCommand(MCP210_C_SETFONT , Font , 1);
		}
		
		Response(RecvData , 5000);		
		SendCommand(MCP210_C_PRINT , strPrint , strlen(strPrint));
	}
	
	if((Response(RecvData , 5000)) == true)
	{
		iRet = 1;
	}
	else
	{
		sprintf(buf , "MCP210 EjectCardOut SendCMD fail!");
		ShowMessage(buf);
		iRet = -1;
	}
	
	Close();
	return iRet;
}

char MCP210::RetrieveCard(void)
{
	char iRet = 0 , RecvData[BufferSize];
	
	memset(RecvData , '\0' , BufferSize);
	memset(buf , '\0' , sizeof(buf));
	
	Init(ReaderCFG.HourlyReaderComPort);
		
	if((SendCommand(MCP210_C_COLLECT , cls , 0)) == 1)
	{
		if(Response(RecvData , 5000) == true)
		{
			iRet = 1;
		}
		else
		{
			sprintf(buf , "MCP210 RetrieveCard fail!");
			ShowMessage(buf);
			iRet = -1;
		}
	}
	else
	{
		sprintf(buf , "MCP210 RetrieveCard SendCMD fail!");
		ShowMessage(buf);
		iRet = -1;
	}
	
	Close();
	return iRet;
}

char MCP210::InsertTicket(void)
{
	//Frank mark s 20120824
	/*char iRet = 1 , RecvData[BufferSize];
	
	memset(RecvData , '\0' , BufferSize);*/
	//Frank mark e 20120824
	
	Init(ReaderCFG.HourlyReaderComPort);
	SendCommand(MCP210_C_READ , cls , 0);
	//Frank mark 20120821 comport.PortRead3(RecvData , 10000 , BufferSize);
	
	Close();
	//Frank mark 20120824 return iRet;
	return 1;					//Frank add 20120824
}

bool MCP210::Response(char* RecvBuff, int mTimeout)
{
	char RecvData[BufferSize], ack = ACK;
	int len , i , locSTX = -1, locETX = -1;
	unsigned char BCC = 0x00;
	bool iRet = false;
	
	memset(RecvData, '\0', BufferSize);
	
	comport.PortRead3(RecvData , mTimeout , BufferSize);
	
	len = strlen(RecvData);
	
	for(i = 0 ; i < len ; i++)
	{
		//printf("%02X ",RecvData[i]);
		
		if ((i > 0) && (i < (len - 1)))
		{
			BCC ^= RecvData[i];
			printf("\nBCC:%02X\n",BCC);
		}
		
		if (RecvData[i] == STX && locSTX == -1)
		{
			locSTX = i;
		}
		
		if (RecvData[i] == ETX && locETX == -1)
		{
			locETX = i;
		}
	}
	
	//printf("\nResponse len : %d BCC : %02X  locSTX : %02X  locETX : %02X\n" , len , BCC , locSTX , locETX);
	
	//Frank mark s 20120824
	/*if (RecvData[locSTX + 1] == MCP210_C_READ)
	{
		iRet = true;
	}	
	else if ((len == (locETX - locSTX + 2)) && (BCC == RecvData[locETX + 1]))*/
	//Frank mark e 20120824
	if ((len == (locETX - locSTX + 2)) && (BCC == RecvData[locETX + 1]))					//Frank add 20120824
	{
		memcpy(RecvBuff , RecvData , BufferSize);
		usleep(100000L);
		comport.PortWrite(&ack , 1);
		iRet = true;
	}
	else
	{
		iRet = false;
	}
	
	return iRet;
}
