/* TUP500 Termal Printer control */
/* TUP500 Termal Printer control */

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "../CommonDef.h"
#include "Tup500.h"
#include "../traceLog.h"					//Frank add 20111122
#include<stdlib.h>					//Frank add 20121119

TUP500::TUP500(void)
{
	lineSn = 0;
	characterSn = 0;
	barcodeSn = 0;
	BufferSize = 256;   // 20120221 Tony add
}

TUP500::~TUP500(void)
{
	Close();
}

void TUP500::init(enum COMPORT port,char type)
{	
	m_Type	= type;
	if(type ==0)
		return;
	if(type == 1 || type == 2)
	{
		//comport.PortInit(port,9600,'N',8,1);
		comport.PortInit(port,9600,'N',8,1);
	}

	my_port = port;
	ClearBuff();    // 20120313 Tony add 
}

void TUP500::Close()
{
	char buffMsg[BufferSize];
	
	comport.PortClose();
	sprintf(buffMsg, "TUP500 Port Close Port:[ COM%d ].", my_port);
	ShowMessage(buffMsg);

}

void TUP500::Reset()
{	//Headware Reset
	// 20120221 Tony mark char data[64],recvData[256];
	char data[64],recvData[BufferSize];    // 20120221 Tony add
	// 20120223 Tony mark int len;
//	int sLen=0,i;
	ShowMessage((char *)"TUP500 :: Reset()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add
	
	sprintf(data,"\x1B\x3F\x0A");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data));

	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,50) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::ClearSetting()
{	//Clears all format memory and image memory data. 
	// 20120221 Tony mark char data[64],recvData[256];
	char data[64],recvData[BufferSize];    // 20120221 Tony add
	// 20120223 Tony mark int len;
//	int sLen=0;,i;
	
	ShowMessage((char *)"TUP500 :: ClearSetting()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add
	
	if(m_Type == 1)
	{	
		sprintf(data,"\x1B""C\x0A%c", 0);	
	}
	
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,4);
	// 20121102 Tony mark usleep(150000L);
	TUP500WaitTime(strlen(data));// 20121102 Tony add
	
	// 20120223 Tony mark s
	/*
	if(comport.GetDTR()==true)
	if(comport.PortRead2(&len,recvData,50) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::SetPrintArea(int length)
{
	// 20120221 Tony mark char data[64],recvData[256];
	char data[64],recvData[BufferSize];    // 20120221 Tony add
	// 20120223 Tony mark int len;
	
	ShowMessage((char *)"TUP500 :: SetPrintArea()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add

	sprintf(data,"\x1B""D%04d\x0A%c",length,0);
	
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,8);
	// 20121102 Tony mark  usleep(150000L);
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,50) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::SetHLine(int x1,int x2,int y,int width)
{	//畫橫線
	char data[128];
	
	ShowMessage((char *)"TUP500 :: SetHLine()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B""L%02d;%04d,%04d,%04d,%04d,0,%1d,0\x0A%c",lineSn,x1,y,x2,y,width,0);
	comport.PortWrite(data,strlen(data)+1);
	//if(comport.GetDTR()==true)
	usleep(30000L);
	lineSn++;
}

void TUP500::SetBarCodePosition(int x, int y, int mode, int type, int rotation,int height)
{
	// 20120221 Tony mark char data[256],recvData[256];
	char data[256],recvData[BufferSize];   // 20120221 Tony add
	// 20120223 Tony mark int len;
	
	ShowMessage((char *)"TUP500 :: SetBarCodePosition()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add

	sprintf(data,"\x1B""PB%02d;%04d,%04d,%1d,%1d,%1d,%04d,0\x0A%c",barcodeSn,x,y,mode,type,rotation,height,0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	//20120313 Tony mark usleep(150000L);
	// 20121102 Tony mark usleep(500000L);	// 20120313 Tony add
	TUP500WaitTime(strlen(data)); // 20121102 Tony add
	ClearBuff	();		// 20120313 Tony add  
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,60) == true)
	{	

	}
	*/
	// 20120223 Tony mark e
}

void TUP500::SetBarCodeData(char datas[])
{
	char data[256];
	// 20120223 Tony mark int len,i;
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
	
	ShowMessage((char *)"TUP500 :: SetBarCodeData()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add

	sprintf(data,"\x1B""RB%02d;%s\x0A%c",barcodeSn,datas,0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	usleep(30000L);
	ClearBuff();	// 20120313 Tony add
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,60) == true)
	{
		printf("Set Barcode data recv: ");
		for(i=0;i<len;i++)
		{
			printf("%02x ",recvData[i]);
		}
		
		printf("\n");
		//return recvData[0];
		//10if((recvData[5] & 0x04) >0) return true;
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::SetCharactPosition(int x, int y, int width, int height, int type,int rotation)
{
	// 20120221 Tony mark char data[256],recvData[256];
	char data[256],recvData[BufferSize];   // 20120221 Tony add
	// 20120223 Tony mark int len;
	
	ShowMessage((char *)"TUP500 :: SetCharactPosition()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add

	sprintf(data,"\x1B""PC%02d;%04d,%04d,%1d,%1d,%01d,%02d,00,00,0\x0A%c",characterSn,x,y,width,height,type,rotation,0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	//20121102 Tony mark usleep(150000L);
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,60) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
	
	characterSn++;
}

void TUP500::SetCharactData(int ID,char datas[])
{
	// 20120221 Tony mark char data[256],recvData[256];
	char data[BufferSize],recvData[BufferSize];   // 20120221 Tony add
	// 20120223 Tony mark int len;
	
	ShowMessage((char *)"TUP500 :: SetCharactData()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	memset(recvData,'\0',BufferSize);	// 20120221 Tony add

	sprintf(data,"\x1B""RC%02d;%s\x0A%c",ID,datas,0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	usleep(35000L);
	ClearBuff();	// 20120313 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*

	if(comport.PortRead2(&len,recvData,60) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::SetArrowImage(int x,int y)
{
	// 20120221 Tony mark char data[2048], recvData[512],bmp[80];					//Frank add 20111116
	char data[2048], recvData[BufferSize],bmp[80];					// 20120221 Tony add
	char buffer[512];
	// 20120223 Tony mark int i,index,len;
	int index,len;  // 20120223 Tony add
	size_t result;
	
	ShowMessage((char *)"TUP500 :: SetArrowImage()" , 4);	// 20120427 Tony add
	
	FILE* fh=NULL;
	
	memset(data,'\0',sizeof(data));
	memset(bmp,'\0',sizeof(bmp));					//Frank add 20111116
	
	sprintf(data,"\x1B""H;%04d,%04d,",x,y);
	index = (long)strlen(data);
	//printf("index:%ld \n",index);
	
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)					//Frank add s 20111116
		sprintf(bmp,"./jpg/%s/arrowr.bmp", G_ParkingConfig.ParkLanguage);
	else
		sprintf(bmp,"./jpg/arrowr.bmp" );
	fh =fopen(bmp,"r");					//Frank add e 20111116
	
	if(fh == NULL)
	{
		printf("no arrow.bmp file\n");
		return;
	}
	fseek (fh, 0, SEEK_END);
	len = ftell (fh);
	//printf("len:%ld \n",len);
	fseek (fh, 0, SEEK_SET);
	result = fread(buffer,1,len,fh);
	memcpy (data+index, buffer, len);
	fclose (fh);

	//Frank add s 20121119
	int i;
	printf("len = %d \n",len);
	printf("SetArrowImage :: buffer : ");
	for(i=0;i<len;i++)	
	{
		printf("%02X ",(unsigned char)buffer[i]);
	}
	printf("\n");
	//Frank add e 20121119
	
	index+=len;
	//printf("index:%ld \n",index);
	data[index++] = 0x0A;
	data[index++] = 0;
	//printf("index:%ld \n",index);

	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	
	ClearBuff();    // 20120221 Tony add
	
	//Frank add 20121119
	printf("SetArrowImage :: Send CMD 1 : ");
	for(i=0;i<=index;i++)
	{
		printf("%02X ",(unsigned char)data[i]);
	}
	printf("\n");
	//Frank add e 20121119
	
	comport.PortWrite(data,index);
	usleep(300000L);	// 20120313 Tony add
	ClearBuff();	// 20120313 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,80) == true)
	{	
		printf("arrow rcv:");
		for(i=0;i<len;i++)
		{
			printf("%02x ",recvData[i]);
		}
		printf("\n");
	}
	*/
	// 20120223 Tony mark e
	
	index =0; len=0;
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B""H;%04d,%04d,",x + 46,y);
	index = (long)strlen(data);
	//printf("index:%ld \n",index);
	
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)					//Frank add s 20111116
		sprintf(bmp,"./jpg/%s/arrowl.bmp", G_ParkingConfig.ParkLanguage);
	else
		sprintf(bmp,"./jpg/arrowl.bmp" );
	
	fh =fopen(bmp,"r");					//Frank add e 20111116
	
	if(fh == NULL)
	{
		printf("no arrow.bmp file\n");
		return;
	}
	fseek (fh, 0, SEEK_END);
	len = ftell (fh);
	//printf("len:%ld \n",len);
	fseek (fh, 0, SEEK_SET);
	result = fread(buffer,1,len,fh);
	memcpy (data+index, buffer, len);
	fclose (fh);
	index+=len;
	//printf("index:%ld \n",index);
	data[index++] = 0x0A;
	data[index++] = 0;
	//printf("index:%ld \n",index);

	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,index);
	usleep(300000L);	// 20120313 Tony add
	ClearBuff();	// 20120313 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,80) == true)
	{	
		printf("arrow rcv:");
		for(i=0;i<len;i++)
		{
			printf("%02x ",recvData[i]);
		}
		printf("\n");
	}
	*/
	// 20120223 Tony mark e
}

void TUP500::ClearBitmap()
{
	// 20120221 Tony mark char data[128],recvData[128];
	char data[128],recvData[BufferSize];    // 20120221 Tony add
	// 20120223 Tony mark int len;
	
	ShowMessage((char *)"TUP500 :: ClearBitmap()" , 4);	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);	    // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B""X\x0A%c", 0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	//Frank mark 20121119 usleep(200000L);	// 20120313 Tony add
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
	ClearBuff();	// 20120313 Tony add
	
	//if(comport.GetDTR()==true)
	// 20120223 Tony mark s
	/*
	if(comport.PortRead2(&len,recvData,60) == true)
	{	
		
	}
	*/
	// 20120223 Tony mark e
}

bool TUP500::CheckOffline()
{
	char data[64];
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
//	int i,len;
	int len;
	
	ShowMessage((char *)"TUP500 :: CheckOffline()" , 4);	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x06\x01");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,3);
	TUP500WaitTime(strlen(data)); // 20121102 Tony add
	len=BufferSize;    // 20120221 Tony add
	
	if(comport.PortRead2(&len,recvData,50) == true)
	{
		//printf("Status: ");
		//for(i=0;i<len;i++)
		//{
		//	printf("%02x ",recvData[i]);	
		//}		
		//printf("\n");
		//return recvData[0];
		
		if((recvData[2] & 0x08) >0) return true;
	}
	else if(len==0)					//Frank add s 20111122
	{
		// 20120427 Tony mark  ShowMessage((char *)"Printer Offline!");
		ShowMessage((char *)"TUP500 :: CheckOffline : Printer Offline!");	// 20120427 Tony add
		return true;
	}					//Frank add e 20111122
	
	return false;
}

bool TUP500::CheckNearEnd()
{
	char data[64];
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
//	int i,len;
	int len;
	
	ShowMessage((char *)"TUP500 :: CheckNearEnd()" , 4);	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x06\x01");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,3);
	len=BufferSize;    // 20120221 Tony add

	if(comport.PortRead2(&len,recvData,50) == true)
	{
//		printf("CheckNearEnd Status: ");
		
//		for(i=0;i<len;i++)
//		{
//			printf("recvData[%d] : %02X \n",i,recvData[i]);	
//		}
		
		//printf("\n");
		//return recvData[0];
		if((recvData[5] & 0x04) >0) return true;
	}
	
	return false;
}

bool TUP500::CheckPaperEnd()
{
	char data[64];
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add 
//	int i,len;
	int len;
	
	ShowMessage((char *)"TUP500 :: CheckPaperEnd()" , 4);	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x06\x01");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,3);
	len=BufferSize;    // 20120221 Tony add

	if(comport.PortRead2(&len,recvData,50) == true)
	{	
		if((recvData[5] & 0x08) >0) return true;
	}
	else if(len==0)					//Frank add s 20111122
	{
		ShowMessage((char *)"TUP500 :: CheckPaperEnd : Printer Offline!");	// 20120427 Tony add
		// 20120427 Tony mark ShowMessage((char *)"Printer Offline!");
		return true;
	}					//Frank add e 20111122
	
	return false;
}

bool TUP500::CheckPaperJam()
{
	char data[64];
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
//	int i,len;
	int len;
	
	ShowMessage((char *)"TUP500 :: CheckPaperJam()");	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x06\x01");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,3);
	len=BufferSize;    // 20120221 Tony add

	if(comport.PortRead2(&len,recvData,50) == true)
	{
		if((recvData[4] & 0x04) >0) return true;
	}
	
	return false;
}

bool TUP500::CheckPaperOnOutlet()
{
	char data[64];
	// 20120221 Tony mark char recvData[128];
	char recvData[BufferSize];  // 20120221 Tony add
//	int i,len;
	int len;
	
	ShowMessage((char *)"TUP500 :: CheckPaperOnOutlet()" , 4);	// 20120427 Tony add
	
	memset(recvData,'\0',BufferSize);   // 20120221 Tony add
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x06\x01");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,3);
	len=BufferSize;    // 20120221 Tony add

	if(comport.PortRead2(&len,recvData,50) == true)
	{
		if((recvData[8] & 0x02) >0) return true;
		//printf("position:%02x\n ",recvData[8]);
	}
	
	return false;
}

void TUP500::SetCutter()
{
	char data[128];
	
	ShowMessage((char *)"TUP500 :: SetCutter()" , 4);	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x42\x2B\x32\x32\x0A%c", 0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	//if(comport.GetDTR()==true)
		// 20121102 Tony mark usleep(10000L);
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
}

void TUP500::SetPrintDensity(int n)
{
	char data[128];

	ShowMessage((char *)"TUP500 :: SetPrintDensity()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x1E\x64%1d",n);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	
	// 20121102 Tony mark usleep(10000L);
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
}

void TUP500::IssuePaper()
{	//Prints one page according to the print area setting command
	char data[128];
	
	ShowMessage((char *)"TUP500 :: IssuePaper()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x49\x0A%c", 0);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data)+1);
	//if(comport.GetDTR()==true)
		//Frank mark 20121119 usleep(10000L);
	TUP500WaitTime(strlen(data)); // 20121102 Tony add
}

void TUP500::Receive()
{	//回收
	char data[128];

	ShowMessage((char *)"TUP500 :: Receive()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x5C\x30\x30");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data));
	//if(comport.GetDTR()==true)
		// 20121102 Tony mark usleep(10000L);
	TUP500WaitTime(strlen(data));	// 20121102 Tony add
}

void TUP500::IssuePaper(int start,int len)
{	//Prints one page according to the print area setting command

}

void TUP500::SelectFont(enum STAR_FONT font)
{
	char data[128];
	
	ShowMessage((char *)"TUP500 :: SelectFont()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	
	sprintf(data,"\x1B\x1E\x46""%d\x0A",font);
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,strlen(data));
	if(comport.GetDTR()==true)
		// 20121102 Tony mark usleep(10000L);
	TUP500WaitTime(sizeof(data));	// 20121102 Tony add
}

void TUP500::FormFeed()
{
	char data[128];
	
	ShowMessage((char *)"TUP500 :: FormFeed()");	// 20120427 Tony add
	
	memset(data,'\0',sizeof(data));
	sprintf(data,"\x0C");
	ClearBuff();    // 20120221 Tony add
	comport.PortWrite(data,1);
	if(comport.GetDTR()==true)
		// 20121102 Tony mark usleep(10000L);
	TUP500WaitTime(sizeof(data));	// 20121102 Tony add
}

// 20120221 Tony add s
void TUP500::ClearBuff(void)
{
	comport.Clear232Bufer();
}					
// 20120221 Tony add e

// 20121029 Tony add s

void TUP500::SetPrintImage(int x,int y, char FileName[])
{
	char bmp[80];

	int index,len;
	size_t result;
	
	ShowMessage((char *)"TUP500 :: SetPrintImage()",4);
	
	FILE* fh=NULL;
	memset(bmp,'\0',sizeof(bmp));
	
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)
		sprintf(bmp,"./jpg/%s/%s", G_ParkingConfig.ParkLanguage,FileName);
	else
		sprintf(bmp,"./jpg/%s", FileName);
	
	fh =fopen(bmp,"r");
	
	if(fh == NULL)
	{
		sprintf(bmp,"./jpg/%s", FileName);		
		fh =fopen(bmp,"r");
		
		if(fh == NULL)
		{
			printf("no arrow.bmp file\n");
			return;
		}
	}
	
	fseek (fh, 0, SEEK_END);
	len = ftell (fh);

	char *buffer = new char[len];
	memset(buffer,'\0',sizeof(buffer));
	
	fseek (fh, 0, SEEK_SET);
	result = fread(buffer,1,len,fh);

	fclose (fh);

	//printf("buffer size = %d\n",sizeof(buffer));

	char *data = new char[len + 1000];		
	memset(data,'\0',sizeof(data));
	sprintf(data,"\x1B""H:%04d,%04d,",x,y);
	index = (long)strlen(data);
	
	memcpy (data+index, buffer, len);	
	index+=len;
	data[index++] = 0x2C;
	data[index++] = 0x0A;
	data[index++] = 0;
	
	//printf("SetPrintImage :: index : %d\n",index);

	ClearBuff();
	comport.PortWrite(data,index);

	
	if(index < 7000)
	{
		TUP500WaitTime(7000);
	}else{
		TUP500WaitTime(index);
	}
	

	//TUP500WaitTime(index);
	ClearBuff();

	delete [] buffer;
	delete [] data;
}

void TUP500::TUP500WaitTime(unsigned int DataSize)
{
	long WaitTime = 0;

	int TmpV1 = DataSize / 2;		
	int TmpValSize = 0;
	int DoVal=0;
	DoVal = TmpV1;

	printf("DataSize : [%d]\n",DataSize);
	printf("DataSize/2 : [%d]\n",TmpV1);
	
	while(1)
	{
		//DoVal = DoVal / 10;
			
		if (DoVal < 10)
		{
			TmpValSize++;
			break;
		}
		else if(DoVal > 0)
		{
			TmpValSize++;
		}		

		DoVal = DoVal / 10;
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}

	char *StrVal = new char[TmpValSize];		
	memset(StrVal,'\0',sizeof(StrVal));

	/*
	if(TmpValSize > 3)
	{
		sprintf(StrVal,"1%0*d",TmpValSize,0);					
	}else{
		sprintf(StrVal,"5%0*d",TmpValSize-1,0);				
	}*/
	
	printf("TmpValSize = %d\n",TmpValSize);
	
	if(TmpValSize > 3)
	{
		sprintf(StrVal,"1%0*d",TmpValSize-1,0);					
	}
	else if(TmpValSize > 2)
	{
		sprintf(StrVal,"5%0*d",TmpValSize-1,0);				
	}
	else if(TmpValSize > 1)
	{
		sprintf(StrVal,"100");				
	}
	else
	{
		sprintf(StrVal,"5");				
	}

	printf("StrVal = %s\n",StrVal);

	WaitTime = (TmpV1 + atoi(StrVal)) * 1000;	

	printf("WaitTime = %ld\n",WaitTime);
	usleep(WaitTime);
	delete [] StrVal;
}

// 20121029 Tony add e

// 20130117 Tony add s
bool TUP500::CheckFE()
{
	int len,i;
	bool RtnBln;
	char recvData[BufferSize];

	memset(recvData,'\0',BufferSize);
	len=BufferSize;
	RtnBln = false;
	
	if(comport.PortRead2(&len,recvData,50) == true)
	{
		for(i=0;i<len;i++)
		{
			//printf("%02x\n",recvData[i]);	
			
			if((recvData[i] & 0xFE) == 0xFE) 
			{	
				RtnBln=true;
				goto EndDo;
			}
		}	
	}	

EndDo:	
	if (RtnBln==true)
	{
		ShowMessage((char *)"TUP500 :: CheckFE : Power reopen.");
	}
	
	return RtnBln;
}
// 20130117 Tony add e

