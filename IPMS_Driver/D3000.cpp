#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "../CommonDef.h"
#include "D3000.h"
#include "../traceLog.h"

D3000::~D3000(void)
{
	Close();
}	

D3000::D3000(void)
{
	BufferSize = 64;    // 20120221 Tony add
	bMalfunction =false;
}

void D3000::init(enum COMPORT port)
{
	//comport.PortInit(port,9600,'N',8,1);
	comport.PortInit2(port,9600,'N',8,1);
	my_port = port;
}

void D3000::Close()
{
	//char buffMsg[BufferSize];
	
	comport.PortClose();
	//sprintf(buffMsg, "D3000 Port Close Port:[ COM%d ].", my_port);
	//ShowMessage(buffMsg);
}

void D3000::Reset(int ID)
{
	// 20120221 Tony mark char parm[16],data[32];
	char parm[16],data[BufferSize]; // 20120221 Tony add
	
	memset(data,'\0',BufferSize); // 20120221 Tony add
	memset(parm,'\0',sizeof(parm));
	
	SendCommand(ID,(char *)"RS0",parm);
	
	if(GetACK(ID) == true)
	{
		DoCommand(ID);
		
		if(GetResponse(data) == true)
		{
			// ========================================================= //
			// nick add s 20150511 Ver:000-000-GIO_V2-13B251-0003-13C241 //
			printf("D3000 Reset Reply:[%s]\n", data);
			
			if (G_ReadType == 0)
			{
				if (strncmp(data + 9, "ACT_F2", 6) == 0) //ACT_F2
					G_ReadType = 2;
				else //D3000
					G_ReadType = 1;
			}
			// nick add e 20150511 Ver:000-000-GIO_V2-13B251-0003-13C241 //
			// ========================================================= //
			
			bMalfunction = false;
		}
	}
}

void D3000::EnableWork(int ID,enum D3000_ENABLE bEnable)
{
	// 20120221 Tony mark char parm[16],data[32];
	char parm[16],data[BufferSize]; // 20120221 Tony add
	
	memset(parm,'\0',sizeof(parm));
	memset(data,'\0',sizeof(data));
	
	if(bMalfunction == true)
	{
		Reset(ID);
		usleep(500000L);
	}
	
	if (G_ReadType == 1 && bEnable == false)
		return;
	
	sprintf(parm,"%1d",bEnable);
	SendCommand(ID,(char *)"EN",parm);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		
		if(GetResponse(data)==true)
		{
			if(data[5]=='N')
				printf("Enable work Error Code: %02x\n",(unsigned char)data[8]);
		}
	}
}

bool D3000::GetStatus(int ID,char *status)
{
	// 20120221 Tony mark char parm[16],data[64];
	char parm[16]; // 20120221 Tony add
	
	memset(parm,'\0',sizeof(parm));
	
	usleep(200000L);
	
	SendCommand(ID,(char *)"AP0",parm);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		
		  // 20120221 Tony markif(GetResponse(data)==true)
		if(GetResponse(status)==true) // 20120221 Tony add
		{
			// 20120221 Tony mark if(data[5]=='P')
			 if(status[5]=='P')					//Frank add 20120223
			{
				memmove(status, status+9, 6);   // nick add 20120221
				status[6]=0;    // nick add 20120221
				// nick mark 20120221 memcpy(status,data+9,6);
				//printf("%02X %02X %02X %02X %02X %02X \n",status[0],status[1],status[2],status[3],status[4],status[5]);
				if(status[4] == '1')
					bMalfunction = true;
				
				return true;
			}
			else
			{
				// 20120221 Tony mark printf("Get status Error Code: %02x\n",(unsigned char)data[8]);
				printf("Get status Error Code: %02x\n",(unsigned char)status[8]);					//Frank add 20120223
			}
		}
		else
		{
			printf("Get status No response!\n");
		}
	}
	else
	{
		printf("D3000 status No ack!\n");					//Frank add 20111122
	}
	
	return false;
}

bool D3000::IsCardInMachine(int ID)
{	//卡片己到位(讀卡機位置)
	// 20120221 Tony mark char status[16];
	char status[BufferSize];    // 20120221 Tony add
	
	memset(status,'\0',sizeof(status));
	
	if(GetStatus( ID,status) == true)
	{
		if((status[0] & 0x04) > 0)
			return true;
		//else
			//printf("S:%6s\n",status);
	}
	
	return false;
}

bool D3000::IsCardIn(int ID)
{  	//有卡片
	// 20120221 Tony mark char status[16];
	char status[BufferSize];    // 20120221 Tony add
	
	memset(status,'\0',sizeof(status));
	
	if(GetStatus( ID,status) == true)
	{		
		if(status[0] == 0x00)
		{			
			return false;
		}
		//printf("D3000:%6s\n",status);
	}
	return true;
}

bool D3000::RejectCard(int ID)
{
	// 20120221 Tony mark char parm[16],data[32];
  	char parm[16],data[BufferSize]; // 20120221 Tony add

	memset(data,'\0',BufferSize); // 20120221 Tony add
	memset(parm,'\0',sizeof(parm));
	
	SendCommand(ID,(char *)"DC0",parm);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);

		if (G_ReadType == 1)
			sleep(2);
		
		if(GetResponse(data)==true)
		{
			if(data[5]=='P')
			{
				return true;
			}
			
			printf("Reject Error Code: %02x\n",(unsigned char)data[8]);
			Reset(ID);
		}
	}
	
	return false;
}

bool D3000::SetAddress(int ID,int newID)
{
	// 20120221 Tony mark char parm[16],data[32];
	char parm[16],data[BufferSize]; // 20120221 Tony add
	
	memset(data,'\0',BufferSize);   // 20120221 Tony add
	memset(parm,'\0',sizeof(parm));
	
	sprintf(parm,"%01d",newID);
	SendCommand(ID,(char *)"SA0",parm);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		
		if(GetResponse(data)==true)
		{
			if(data[5]=='P')
			{
				return true;
			}
			
			printf("Set address Error Code: %02x\n",(unsigned char)data[8]);
		}
	}
	
	return false;
}

int D3000::SendCommand(int ID,char command[],char parm[])
{
	char sendData[16];
	int sLen=0,dataLen;

	memset(sendData,'\0',sizeof(sendData));
	
	dataLen = strlen(command) + strlen(parm);
	sprintf(sendData,"%c%02d%02d%s%s%c",STX,ID,dataLen,command,parm,ETX);
	
	sLen = strlen(sendData);
	ClearBuff();    // 20120221 Tony add
	comport.PortWriteWithBCC(sendData,sLen);

	return sLen;
}

bool D3000::GetResponse(char* rData)
{
//	int i,len=0;
//	char* response = NULL;
//	int rID,Func;
	int len=0;
	len = BufferSize;   // 20120221 Tony add
	
	if(comport.PortRead2(&len,rData,500) == true)
	{
		printf("R:");
		
		for(int i=0;i<len;i++)
		{
			printf("%02x ",rData[i]);
		}
		
		printf("\n");
		return true;
	}
	else
	{
		printf("D3000 No resp\n");					//Frank add 20111122
	}
	
	return false;
}

void D3000::DoCommand(int ID)
{	//Send ENQ+ADDR
	char sendData[16];
//	int sLen=0;

	//printf("ENQ\n");
	//usleep(200000L);
	memset(sendData,'\0',sizeof(sendData));

	sprintf(sendData,"%c%02d",ENQ,ID);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(sendData,3);
}

bool D3000::GetACK(int ID)
{	
	// 20120221 Tony mark char  recvData[32];
	char  recvData[BufferSize]; // 20120221 Tony add
	int len,id;//,i;
	
	memset(recvData,'\0',sizeof(recvData));
	
	len = BufferSize;   // 20120221 Tony add
	
	if(comport.PortRead2(&len,recvData,600) == true)
	{	
		if(recvData[0] == ACK)
		{
			sscanf(recvData+1,"%2d",&id);
			
			if(ID == id) 
				return true;
			else
				printf("id=%d ID= %d",id,ID);
		}
		else
		{
			//printf("r:");
			//for(i=0;i<len;i++)
			//{
			//	printf("%02x ",recvData[i]);
			//}
			//printf("\n");
		}
	}
	else
	{
		printf("D3000 No Ack\n");					//Frank add 20111122
	}
	
	return false;
}


bool D3000::RetrieveCard(int ID)
{
	// 20120221 Tony mark char buff[16],data[32];
//	int n,i;
	char buff[16],data[BufferSize]; // 20120221 Tony add
	
	memset(data,'\0',BufferSize);   // 20120221 Tony add
	memset(buff,'\0',sizeof(buff));
	
	usleep(200000L);
	SendCommand(ID,(char *)"CP1",buff);
	
	//// ========================================================= //
	//// nick add s 20150511 Ver:000-000-GIO_V2-13B251-0003-13C241 //
	//if (G_ReadType == 2)
	//	SendCommand(ID,(char *)"CP0",buff);
	//else
	//	SendCommand(ID,(char *)"CP1",buff);
	//// nick add e 20150511 Ver:000-000-GIO_V2-13B251-0003-13C241 //
	//// ========================================================= //
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);

		if (G_ReadType == 1)
			sleep(2);
		
		if(GetResponse(data)==true)
		{ 
			if(data[5]=='P')
			{
				return true;
			}
			
			printf("Retrieve Error Code: %02X\n",(unsigned char)data[8]);
		}
	}
	
	return false;
}

int D3000::GetMachineID(int ID)
{
	char buff[16];
	// 20120221 Tony mark char data[24];
	char data[BufferSize];  // 20120221 Tony add
	int n=-1;
	
	memset(data,'\0',BufferSize);   // 20120221 Tony add
	memset(buff,'\0',sizeof(buff));
	
	usleep(200000L);
	SendCommand(ID,(char *)"GA0",buff);
	
	if( GetACK(ID)== true)
	{
		DoCommand(ID);
		
		if(GetResponse(data)==true)
		{
			if(data[5]=='P')
			{
				sscanf(data+9,"%2d",&n);
				return n;
			}
			
			printf("Get ID Error Code: %02x\n",(unsigned char)data[8]);
		}
	}
	
	return n;
}

// 20120221 Tony add s
void D3000::ClearBuff(void)
{
	comport.Clear232Bufer();
}
// 20120221 Tony add e