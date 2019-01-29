#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "../CommonDef.h"
#include "CRT350.h"


typedef struct CRT350ReceiveFormat
{
	char stx;
	char len[2];
	char cmd;
	char param;
	char data[512];
}rFormat;

#define CRT350_RETRY   3
#define DATASTRUCTSIZE sizeof(CRT350ReceiveFormat)

void CRT350::SendCommand(int  len,char* data)
{
	char sendData[128];	
	int  i,sLen;
	unsigned char lenHByte;
	unsigned char lenLByte;
	unsigned char BCC = 0x00;
	comport.GetCTS();
	sleep(1);
	comport.SetRTS(false);
	usleep(5000);
	lenHByte = (unsigned char) ((len & 0xFF00)>>8);
	lenLByte = (unsigned char) (len& 0x00FF);
	
	memset(sendData,'\0',sizeof(sendData));
	
	sprintf(sendData,"\x02%c%c",lenHByte,lenLByte);
	while(1)
	{
		if(comport.GetCTS())
		{
			break;
			
		}
		sleep(1);
	}
	for(i=0;i<len;i++)
	{
		sendData[i+3] = data[i];
	}
	sendData[i+3] = 0x03;
	sLen = i+3+1;
	for(i=0;i<sLen;i++)
	{
		comport.out((unsigned char)sendData[i]);
		BCC ^= ((unsigned char)sendData[i]);
	}
	//comport.PortWrite(sendData,sLen);
	comport.out(BCC);
	
	usleep(5000);
	comport.SetRTS(true);
}

bool CRT350::GetResponse(char* rData)
{
//	int  len=0;
	/*
	memset(recvData,'\0',sizeof(recvData));
	usleep(300000L);	
	if(comport.PortRead(&len,recvData,100) == true)
	{
		DataStruct = (rFormat*)(&recvData);
		len |= (DataStruct->len[1]<<8 );
		len |= (DataStruct->len[0]);		
	}
*/	
	return false;
}

bool CRT350::Reset(char* versionString, char ResetValue)
{
	char sendData[64];
	char recvData[DATASTRUCTSIZE];
	rFormat* DataStruct=NULL;
	int  i,len,sLen=0;
	
	memset(sendData,'\0',sizeof(sendData));
	memset(recvData,'\0',sizeof(recvData));
	sprintf(sendData,"%c%c",CRT350_CMD_RESET,ResetValue);
	len =0;
	
	for(i=0;i< CRT350_RETRY;i++)
	{
		SendCommand(2,sendData);
		len = sizeof(recvData); // 20120221 Tony add
		
		if(comport.PortRead2(&len,recvData,250) == true)
		{
			DataStruct = (rFormat*)(&recvData);
			
			if(DataStruct->cmd != CRT350_CMD_RESET || DataStruct->param != ResetValue)
			{
				continue;
			}
			
			sLen |= (DataStruct->len[1]<<8 );
			sLen |= (DataStruct->len[0]);
			strncpy(versionString,DataStruct->data,sLen-2);
			return true;
		}
	}
	return false;
}

bool CRT350::GetStatus(enum CRT350_WhichStatus whichStatus,char* status)
{
	char sendData[64];
	char recvData[DATASTRUCTSIZE];
	rFormat* DataStruct=NULL;
	int  i,len,sLen=0;
	
	memset(sendData,'\0',sizeof(sendData));
	memset(recvData,'\0',sizeof(recvData));
	sprintf(sendData,"%c%c",CRT350_CMD_STATUS,whichStatus);
	len =0;
	for(i=0;i< CRT350_RETRY;i++)
	{
		SendCommand(2,sendData);
		usleep(250000L);	
		if(comport.PortRead(&len,recvData,100) == true)
		{		
			DataStruct = (rFormat*)(&recvData);
			if(DataStruct->cmd != CRT350_CMD_STATUS || DataStruct->param != whichStatus)
			{
				continue;
			}
			sLen |= (DataStruct->len[1]<<8 );
			sLen |= (DataStruct->len[0]);
			strncpy(status,DataStruct->data,sLen-2);
			return true;
		}
		printf("Retry...\n");
	}
	return false;
}

bool CRT350::MoveTicket(enum CRT350_MovePlace place)
{
	char sendData[64];
	char recvData[DATASTRUCTSIZE];
	rFormat* DataStruct=NULL;
	int  i,len,sLen;
	
	memset(sendData,'\0',sizeof(sendData));
	memset(recvData,'\0',sizeof(recvData));
	sprintf(sendData,"%c%c",CRT350_CMD_MOVE,place);
	len =0;
	for(i=0;i< CRT350_RETRY;i++)
	{
		SendCommand(2,sendData);
		usleep(250000L);	
		if(comport.PortRead(&len,recvData,100) == true)
		{		
			DataStruct = (rFormat*)(&recvData);
			if(DataStruct->cmd != CRT350_CMD_MOVE || DataStruct->param != place)
			{
				continue;
			}
			sLen |= (DataStruct->len[1]<<8 );
			sLen |= (DataStruct->len[0]);
			//strncpy(status,DataStruct->data,sLen-2);
			return true;
		}
	}
	return false;
}

bool CRT350:: SetCardInControl(enum CRT350_MovePlace place)
{
	char sendData[64];
	char recvData[DATASTRUCTSIZE];
	rFormat* DataStruct=NULL;
	int  i,len,sLen;
	
	memset(sendData,'\0',sizeof(sendData));
	memset(recvData,'\0',sizeof(recvData));
	sprintf(sendData,"%c%c",CRT350_CMD_MOVE,place);
	len =0;
	for(i=0;i< CRT350_RETRY;i++)
	{
		SendCommand(2,sendData);
		usleep(250000L);
		if(comport.PortRead(&len,recvData,100) == true)
		{		
			DataStruct = (rFormat*)(&recvData);
			if(DataStruct->cmd != CRT350_CMD_MOVE || DataStruct->param != place)
			{
				continue;
			}
			sLen |= (DataStruct->len[1]<<8 );
			sLen |= (DataStruct->len[0]);
			//strncpy(status,DataStruct->data,sLen-2);
			return true;
		}
	}
	return false;
}

void CRT350::init(enum COMPORT port)
{
	//comport.PortInit(port,9600,'N',8,1);
	comport.PortInit2(port,9600,'N',8,1);
	ReaderID=0;
}

void CRT350::ClearBuff(void)					//Frank add s 20111116
{
	comport.Clear232Bufer();
}					//Frank add e 20111116