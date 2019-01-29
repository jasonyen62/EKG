/* 天騰 發卡,收卡機控制程式 */
#include "TTCE.h"

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "../CommonDef.h"
#include "TTCE.h"
#include "../traceLog.h"

TTCE::~TTCE(void)
{
	Close();
}	

TTCE::TTCE(void)
{	
}

void TTCE::init(enum COMPORT port)
{
	//comport.PortInit(port,9600,'N',8,1);
	comport.PortInit2(port,9600,'N',8,1);
	my_port = port;
}

void TTCE::Close()
{
	char buffMsg[200];
	
	comport.PortClose();
	sprintf(buffMsg, "TTCE Port Close Port:[ COM%d ].", my_port);
	ShowMessage(buffMsg);
}

void TTCE::Reset1000(short ID)
{
	char buff[16];
	
	memset(buff,'\0',sizeof(buff));
	
	SendCommand(ID,"RS",buff);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);	
	}	
}

bool TTCE::GetStatus(short ID,char *status)
{
	char buff[16];
	
	usleep(200000L);
	memset(buff,'\0',sizeof(buff));
	
	SendCommand(ID,"RF",buff);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		if(GetResponse(status)==true)
		{			
			return true;
		}		
	}
	return false;
}

bool TTCE::IsCardInMachine(short ID)
{
	char status[16];
	
	usleep(200000L);
	
	if(GetStatus( ID,status) == true)
	{
		if(status[7] == '0')
			return false;
		else
			printf("%c\n",status[7]);
	}
	return true;
}

bool TTCE::DispenseCard(short ID,enum TTCE_DISPENSE place)
{
	char buff[16];
	
	memset(buff,'\0',sizeof(buff));
	
	sprintf(buff,"%1d",place);
	SendCommand(ID,"FC",buff);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		return true;
	}
	return false;
}

short TTCE::SendCommand(short ID,char command[],char parm[])
{
	char sendData[16];
	short sLen=0;

	memset(sendData,'\0',sizeof(sendData));
	
	sprintf(sendData,"%c%02d%s%s%c",STX,ID,command,parm,ETX);
	
	sLen = strlen(sendData);
	comport.PortWriteWithBCC(sendData,sLen);

	return sLen;
}

bool TTCE::GetResponse(char* rData)
{
	short i,len=0;
	char* response = NULL;
	short rID,Func;
	
	if(comport.PortRead2(&len,rData,500) == true)
	{
	//	printf("R:");
	//	for(i=0;i<len;i++)
	//	{
	//		printf("%02x ",(unsigned char)recvData[i]);
	//	}
	}
	else
	{
		printf("No resp\n");
	}
	
	return false;
}

void TTCE::DoCommand(short ID)
{	//Send ENQ+ADDR
	char sendData[16];
	short sLen=0;

	usleep(200000L);
	memset(sendData,'\0',sizeof(sendData));
	
	sprintf(sendData,"%c%02d",ENQ,ID);  
	comport.PortWrite(sendData,3);
}

bool TTCE::GetACK(short ID)
{	
	char  recvData[16];
	short len,id,i;
	
	memset(recvData,'\0',sizeof(recvData));
	
	len = sizeof(recvData); // 20120221 Tony add
	
	if(comport.PortRead2(&len,recvData,500) == true)
	{
	//	//printf("R:");
	//	//for(i=0;i<len;i++)
	//	{
	//		printf("%02x ",recvData[i]);
	//	}
	//	printf(".\n");
		if(recvData[0] == ACK)
		{
			sscanf(recvData+1,"%2d",&id);
			
			if(ID == id) 
				return true;
			else
				printf("id=%d ID= %d",id,ID);
		}
	}
	else
	{
		printf("No Ack\n");
	}
	return false;
}

bool TTCE::DispenseCardOut(short ID)
{	// Dispense Card to TAKE Ticket
	char buff[16];
	
	memset(buff,'\0',sizeof(buff));
	
	SendCommand(ID,"DC",buff);
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		return true;
	}
	return false;
}

bool TTCE::RetrieveCard(short ID)
{
	char buff[16];
	
	memset(buff,'\0',sizeof(buff));
	usleep(200000L);
	SendCommand(ID,"CP",buff);
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		return true;
	}
	return false;
}

	