/* Bar code Reader Control  */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "BarCode.h"
#include "../traceLog.h"

BarCode::BarCode(void)
{
}

BarCode::~BarCode(void)
{
	Close();
}

void BarCode::init(enum COMPORT port)
{
	// nick mark 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //comport.PortInit(port,9600,'N',8,1);
	//comport.PortInit(port, 115200, 'N', 8, 1); // nick add 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //
	comport.PortInit2(port, 115200, 'N', 8, 1); // nick add 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //
	my_port = port;
}

void BarCode::Close()
{
	//char buffMsg[200];
	
	comport.PortClose();
	//sprintf(buffMsg, "Barcode Port Close Port:[ COM%d ].", my_port);
	//ShowMessage(buffMsg);
}

void BarCode::ReadStart()
{
	char sendData[128];
	int sLen;

	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x1B""Z\x0d");
	sLen = strlen(sendData);
	comport.PortWrite(sendData,sLen);
	usleep(100000L);
}

void BarCode::ReadEnd()
{
	char sendData[128];
	int sLen;

	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x1B""Y\x0d");
	sLen = strlen(sendData);
	comport.PortWrite(sendData,sLen);
    usleep(100000L);
}

bool BarCode::GetData(char* datas)
{
	int i;					//Frank add 20111122
	int iCnt = 0;
	
	//int len=0;					//Frank mark 20111122
	//if( comport.PortRead2(&len,datas,600) == true)					//Frank mark 20111116
	//if( comport.PortRead3(&len,datas,1,127) > 0 )				//Frank mark 20111122
	if(datas !=0)						//Frank add s 20111122
	{
		//if(comport.PortRead3(datas,10,127) > 0 )
		iCnt = comport.PortRead3(datas,10,127);

		if (iCnt > 0)
		{
			printf("Barcode Get Data:%d\n", iCnt);
			
			for(i=0; i<iCnt; i++)
			{
				if(datas[i]=='\n')
					break;
			}
			
			printf("len:%d\n",i);
			
			if(i==20)
				return true;
		}
	}					//Frank add e 20111122
	return false;
}

void BarCode::ClearBuff(void)					//Frank add s 20111116
{
	comport.Clear232Bufer();
}					//Frank add e 20111116