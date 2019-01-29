#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "../CommonDef.h"
#include "D1000.h"
#include "../traceLog.h"

D1000::~D1000(void)
{
	Close();
}

D1000::D1000(void)
{
	BufferSize=16;  // 20120221 Tony add
}

// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //void D1000::init(enum COMPORT port)
void D1000::init(enum COMPORT port, int Dispense) // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
{
	//comport.PortInit(port,9600,'N',8,1);
	comport.PortInit2(port,9600,'N',8,1);
	my_port = port;
	//20130904 KARATE add s D1000 Type
	// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //int iIssueDispense = 1;
	char sRet = 0;
	// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //sRet = GetVer(iIssueDispense);
	sRet = GetVer(Dispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	//20130904 KARATE add e
}

void D1000::Close()
{
	//char buffMsg[BufferSize];
	
	comport.PortClose();
	//sprintf(buffMsg, "D1000 Port Close Port:[ COM%d ].", my_port);
	//ShowMessage(buffMsg);
}

//Frank mark 20121102 void D1000::Reset(int ID)
bool D1000::Reset(int ID)					//Frank add 20121102
{
	// 20120221 Tony mark char buff[16];
	char buff[BufferSize];  // 20120221 Tony add
	int iRet;
	
	memset(buff,'\0',sizeof(buff));
	
	SendCommand(ID,(char *)"RS",buff);
	usleep(200000);
	iRet = GetACK(ID);
	
	if(iRet == 1)
	{
		DoCommand(ID);
		return true;					//Frank add 20121102
	}
	else if(iRet == -1)
	{
		printf("D1000 Reset no ack\n");
	}
	else
	{
		printf("D1000 Reset error.\n");
	}
	
	return false;					//Frank add 20121102
}

//20130904 KARATE add s Get Ver
char D1000::GetVerResponse(void)
{
	char sData[BufferSize];
	int iLen=0;
	iLen = BufferSize;
	char sName[20];

	usleep(200000);
	memset(sData, '\0', sizeof(sData));
	
	if(comport.PortRead2(&iLen, sData, 500) == true)
	{
		printf("D1000 GetVerResponse = [%s]\n", sData);
		memcpy(sName, sData + 3, sizeof(sData) - 5);
		
		if(memcmp(sName, "TTCE_D1801", 10) == 0)
		{
			printf("Module = [%s]\n", sName);
			G_ReadType = 1;
			return 1;
		}
		else if (memcmp(sName, "ACT_F1", 6) == 0)
		{
			printf("Module = [%s]\n", sName);
			G_ReadType = 2;
			return 2;
		}
	}
	else
	{
		printf("D1000 No GetVerResponse\n");
	}
	
	return 0;
}	

char D1000::GetVer(int iID)
{
	char sBuff[BufferSize];
	char sRet = 0;

	memset(sBuff, '\0', sizeof(sBuff));
	SendCommand(iID,(char*)"GV", sBuff);
	usleep(200000);
	sRet = GetACK(iID);
	
	if(sRet == 1)
	{
		DoCommand(iID);
		sRet = GetVerResponse();
		return sRet;
	}
	else if(sRet == -1)
	{
		printf("D1000 Get Status no ack\n");
		return -1;
	}
	return 0;
}
//20130904 KARATE add e

char D1000::GetStatus(int ID,char *status)
{
	// 20120221 Tony mark char buff[16];
	char buff[BufferSize];  // 20120221 Tony add
	char ret =0;
	
	usleep(200000L);
	
	memset(buff,'\0',sizeof(buff));

	// nick mark 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //SendCommand(ID,(char *)"RF",buff);
	
	// ========================================================= //
	// nick add s 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
	if (G_ReadType == 3)
		SendCommand(ID,(char *)"AP",buff);
	else
		SendCommand(ID,(char *)"RF",buff);
	// nick add e 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================= //
	
	ret = GetACK(ID);
	
	if(ret == 1)
	{
		DoCommand(ID);
		
		if(GetResponse(status)==true)
		{
			return 1;
		}
	}
	else if(ret == -1)
	{
		printf("D1000 Get Status no ack\n");
		return -1;
	}
	
	return 0;
}

bool D1000::IsEmpty(int ID)
{
	// 20120221 Tony mark char status[16];
	char status[BufferSize];    // 20120221 Tony add
	
	memset(status,'\0',sizeof(status));
	usleep(100000L);
	
	if(GetStatus(ID,status) == 1)
	{
		if(status[7] == '8')
		{
			//printf("D1000_%d is Empty.\n",ID);
			return true;
		}
	}
	return false;
}

bool D1000::IsCardInMachine(int ID)
{
	// 20120221 Tony mark char status[16];
	char status[BufferSize];    // 20120221 Tony add
	
	memset(status,'\0',sizeof(status));
	
	usleep(200000L);
	
	if(GetStatus(ID , status) == 1)
	{
		if(status[7] == '0' || status[7] == '8' || status[7] == '4')
			return false;
		else
		{
			//printf("status:%c\n",status[7]);	
		}
	}
	else
		return false;
		
	return true;
}

bool D1000::IsCardIn(int ID)
{
	// 20120221 Tony mark char status[16];
	char status[BufferSize];    // 20120221 Tony add
	
	usleep(200000L);
	
	memset(status,'\0',sizeof(status));
	
	if(GetStatus(ID,status) == 1)
	{
		// nick mark 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //if(status[7] == '1')
		if(status[7] == '1' || status[7] == '3') // nick add 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		{
			return true;
		}
		else
		{
			//printf("s7:%c\n",status[7]);
		}
	}
	else
	{
		printf("D1000 Get status CardIn error.\n");
	}
	//printf("return true\n");
	return false;
}

char D1000::DispenseCard(int ID,enum D1000_DISPENSE place)
{
	// 20120221 Tony mark char buff[16];
	char buff[BufferSize];  // 20120221 Tony add
	char ret =0;
    
	memset(buff,'\0',sizeof(buff));
	
	sprintf(buff,"%1d",place);
	SendCommand(ID,(char *)"FC",buff);
	ret = GetACK(ID);
	
	if(ret== 1)
	{
		DoCommand(ID);
		return 1;
	}
	else if(ret == -1)
	{
		printf("D1000 Dispense Card no ack\n");
		return -1;
	}
	return 0;
}

int D1000::SendCommand(int ID,char command[],char parm[])
{
	// 20120221 Tony mark char sendData[16];
	char sendData[BufferSize];  // 20120221 Tony add
	int sLen = 0;
	int i = 0;

	memset(sendData, '\0', sizeof(sendData));
	
	sprintf(sendData, "%c%02d%s%s%c",STX,ID,command,parm,ETX);

	sLen = strlen(sendData);
	
	printf("D1000 Send:\n");
	
	for(i = 0; i < sLen; i++)
	{
		printf("%02X ", (unsigned char)sendData[i]);
	}
	
	printf("\n");
	
	ClearBuff();  // 20120221 Tony add 
	comport.PortWriteWithBCC(sendData,sLen);

	return (sLen + 1);
}

bool D1000::GetResponse(char* rData)
{
//	int i,len=0;
//	char* response = NULL;
//	int rID,Func;
	int i;
	int len=0;
	
	len = BufferSize;   // 20120221 Tony add

	if(comport.PortRead2(&len,rData,500) == true)
	{
		printf("D1000 Revc:");
		
		for(i = 0; i < len;i++)
		{
			printf("%02X ",(unsigned char)rData[i]);
		}
		
		return true;
	}
	else
	{
		printf("D1000 No resp\n");
	}
	return false;
}

void D1000::DoCommand(int ID)
{	//Send ENQ+ADDR
	// 20120221 Tony mark char sendData[16];
	char sendData[BufferSize];  // 20120221 Tony add
//	int sLen=0;
	
	memset(sendData,'\0',sizeof(sendData));
	
	if (G_ReadType == 3) return; // nick add 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	usleep(200000L);	
	sprintf(sendData,"%c%02d",ENQ,ID);
	ClearBuff();  // 20120221 Tony add
	comport.PortWrite(sendData,3);
}

char D1000::GetACK(int ID)
{
	// 20120221 Tony mark char recvData[16];
	char recvData[BufferSize]; // 20120221 Tony add
	int len,id;//,i;
	
	memset(recvData,'\0',sizeof(recvData));
	
	// nick mark 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //len = BufferSize; // 20120221 Tony add
	
	// ========================================================= //
	// nick add s 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
	if (G_ReadType == 3)
		len = 4;
	else
		len = BufferSize;
	// nick add e 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================= //
	
	if(comport.PortRead2(&len,recvData,600) == true)
	{
	//	//printf("R:");
	//	//for(i=0;i<len;i++)
	//	{
	//		printf("%02x ",recvData[i]);
	//	}
	//	printf(".\n");
		
		if(recvData[0] == ACK)
		{
			sscanf(recvData + 1,"%2d",&id);
			
			if(ID == id)
			{
				// ========================================================= //
				// nick add s 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
				if (len > 3)
				{
					G_ReadType = 3;
				}
				// nick add e 20140528 Ver:000-000-GIO_V2-135101-0004-13B251 //
				// ========================================================= //
				
				return 1;
			}
			else
				printf("id=%d ID= %d",id,ID);
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

char D1000::DispenseCardOut(int ID)
{	// Dispense Card to TAKE Ticket
	// 20120221 Tony mark char buff[16];
	char buff[BufferSize];  // 20120221 Tony add
	char ret;
	
	memset(buff,'\0',sizeof(buff));
	
	SendCommand(ID,(char *)"DC",buff);
	ret = GetACK(ID);
	
	if(ret == 1)
	{
		DoCommand(ID);
		return 1;
	}
	else if(ret == -1)
	{
		printf("Dispense Card Out no ack\n");
		return -1;
	}
	return 0;
}

char D1000::RetrieveCard(int ID)
{
	// 20120221 Tony mark char buff[16];
	char buff[BufferSize];  // 20120221 Tony add
	char ret =0;
	
	memset(buff,'\0',sizeof(buff));
	
	usleep(200000L);
	SendCommand(ID,(char *)"CP",buff);
	ret = GetACK(ID);
	
	if(ret == 1)
	{
		DoCommand(ID);
		return 1;
	}
	else if(ret == -1)
	{
		printf("RetrieveCard no ack\n");
		return -1;
	}
	return 0;
}

// 20120221 Tony add s
void D1000::ClearBuff(void)
{
	comport.Clear232Bufer();
}
// 20120221 Tony add e

//Frank add s 20120821
char D1000::GetStatusD2(int ID , char *status)
{
	char buf[BufferSize] , iRet = 0;
	
	usleep(200000L);
	
	memset(buf , '\0' , sizeof(buf));
	
	SendCommand(ID , (char *)"AP" , buf);
	
	if(GetResponse(status)==true)
	{
		iRet = 1;
	}
	else
	{
		printf("D1000 Get Status no ack\n");
		iRet = -1;
	}
	
	return iRet;
}
//Frank add e 20120821