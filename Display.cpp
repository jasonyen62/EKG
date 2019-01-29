
/* Display Control */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <sys/msg.h>
#include "Dio.h"
#include "Display.h"
#include "CommonDef.h"
#include "IPMS_Driver/ra8822.h"
#include "traceLog.h" // nick add 20130219 //

#pragma pack(1) /// force alignment to 1 byte
typedef struct bitmapfileheader{
	unsigned short	type;
	unsigned long	fsize;
	unsigned short	reserved1;
	unsigned short	reserved2;
	unsigned long	offsetbits;
} BITMAPFILEHEADER;

typedef struct bitmapinfoheader{
	unsigned long	hsize;
	unsigned long	width;
	unsigned long	height;
	unsigned short	planes;
	unsigned short	bitcount;
	unsigned long	compression;
	unsigned long	sizeimage;
	long			xpelspermeter;
	long			ypelspermeter;
	unsigned long	colorsused;
	unsigned long	colorsimportant;
} BITMAPINFOHEADER; 

typedef struct single_pixel{
	unsigned char nouse;
	unsigned char blue; 
	unsigned char green; 
	unsigned char red;
} SINGLE_PIXEL;

typedef struct LCMScreenFileFormat
{
	unsigned short width;
	unsigned short height;
	char  bitData[1920]; //30 byte * 64 line = 1920
}LCMFile;

long lShowLocalTime; // nick add 20130208 //

char LCM_Preshow[128]; // nick add 20130409 //

#pragma pack()  /// set alignment back to default

void VGA_ShowBMP(char filename[]);

/*  ========= LCM 顯示文字 ==========
	車道封閉中\nLane closed
	歡迎光臨\n請按鈕取票或持票卡感應\nPlease push button\nor insert monthly card
	請插入票卡\nPlease insert ticket
	請取回票卡\nPlease take your ticket
	限用月票卡\nMonthly card only
	您的票卡已過期\n請取回票卡\nMonthly card expired\nPlease take your ticket
	您的票卡無效\n請取回票卡\nTicket not valid\nPlease take your ticket
	請稍候\nPlease wait..
	請進場\nPlease enter
	柵欄機無法動作\n請按對講機呼叫管理員\nBarrier not function\nPlease press  intercom
	設備故障\n請按對講機呼叫管理員\nDevice not functio\nPlease press intercom
	車位已滿 請稍候\nParking lot full\nPlease wait..
	您尚未繳費\n請至全自動收費站繳費\nNot paid\nPlease pay at APS
	請離場 謝謝光臨\nPlease leave\nHave a nice day!
	票卡無法寫入\n請重新製作票卡\nTicket not writable\nPlease renew the ticket
	票卡重複進場 請洽管理員\nRepeat access\nPlease push intercom
	場地編號不符\n請確認入/出境場地\nWrong area\nPlease check area
	無法讀取票卡資料\nTicket not readable
	停車場免費開放中\nParking lot free open..
	封閉車道中\n請由其他入口進場\nLane closed\nPlease select other lane
	票卡處理中 請稍候\nTicket is processing\nPlease wait..
	車牌號碼不符\n請按對講機呼叫管理員\nPlate number not match\nPlease press  intercom
	車牌無法辨識 請稍候\nPlate number is processing\nPlease wait..
	重新啟動中 請稍候\nRestarting..Please wait
*/
void InitialScreen()
{
	LCM_Initial();
	LCM_FillScreen(0x00);
}

void ClearScreen()
{
	LCM_FillScreen(0x00);
	VGA_ShowBMP((char *)"clear");
}

void DisplayOn(bool bOn)
{
	LCM_Light(bOn);
	
	if(bOn==false)
		ClearScreen();
}

void Boot_ClearScreen()
{
	Boot_LCM_FillScreen(0x00);
	VGA_ShowBMP((char *)"clear");
}


void Boot_DisplayOn(bool bOn)
{
	Boot_LCM_Light(bOn);
	
	if(bOn==false)
		Boot_ClearScreen();
}


void LCM_PrintString(unsigned char x,unsigned char y,char* showString)
{
	short i;

	LCM_GotoXY(x,y);

	for(i=0;i<(short)strlen(showString);i++)
	{
		LCM_WriteData(showString[i]);
	}
}

void LCM_PrintStringRightAlign(unsigned char y,char* showString)
{
	short i;
	short len;
	unsigned char x;
	
	len = strlen(showString);
	x= 30 - len;
	
	LCM_GotoXY(x,y);

	for(i=0;i<(short)strlen(showString);i++)
	{
		LCM_WriteData(showString[i]);
	}
}

void LCM_LoadBitmap(char* bitmapData,unsigned long* xByte,unsigned long* yByte,char filename[])
{
	unsigned long i,j,lineByte;
	char c;
	bool bitLight[256]; // it would only use 256 color
	FILE *fh;
	BITMAPINFOHEADER *bmpInfo=NULL;
	SINGLE_PIXEL     *color=NULL;
	char* bfInfo=NULL;
	char* bfColor = NULL;
	char screenFile[80];
	size_t Fsize;
	
	sprintf(screenFile,"./screen/%s",filename);
	
	fh = fopen(screenFile,"r");
	
	if(fh == NULL)
	{
		printf("Can't Open %s .\n",screenFile);
		return;
	}

	bfInfo = new char[sizeof(BITMAPINFOHEADER)];
	bfColor = new char[sizeof(SINGLE_PIXEL)];
	bmpInfo = (BITMAPINFOHEADER*)bfInfo;
	color   = (SINGLE_PIXEL*)bfColor;
	
	fseek(fh , 14L , SEEK_SET );

	Fsize = fread(bfInfo,1,sizeof(BITMAPINFOHEADER),fh);
	lineByte = bmpInfo->width + ( 4 - (bmpInfo->width % 4));
	
	// Read Palette
	if(bmpInfo->colorsused >256)
	{
		printf("BMP file Color too much\n");
	}
	
	for(i=0;i<bmpInfo->colorsused;i++)
	{
		Fsize = fread(bfColor,1,sizeof(SINGLE_PIXEL),fh);
		 
		if(color->blue == 0x00)
		{
			bitLight[i] = true;
		}
		else
		{
			bitLight[i] = false;
		}		
	}
	
	if( (bmpInfo->width%8) >0)
	{
		(*xByte) = bmpInfo->width/8+1;
	}
	else
	{
		(*xByte) = bmpInfo->width/8;
	}
	
	(*yByte) = bmpInfo->height;

	for(i=0;i<bmpInfo->height;i++)
	{	
		for(j=0;j<lineByte;j++)
		{
			if(j>=bmpInfo->width) continue;
			Fsize = fread(&c,1,1,fh);
			
			if(bitLight[(unsigned char)c] == true)
			{
				bitmapData[(i*(*xByte)) + (j/8)] |= (0x80 >> (j%8));			
			}			
		}
	}
	fclose(fh);
	
	delete bfInfo;
	delete bfColor;
}

void LCM_ShowImage(short pX,short pY,unsigned long xByte,unsigned long yByte,char* image)
{
	long i,p;
	unsigned long j;
	
	LCM_SetGraphMode(true);
	
	for( i=(yByte-1L); i>=0L; i--)
	{
		LCM_WriteCommand(0x60,pX);
		LCM_WriteCommand(0x70,pY);
	
		for(j=0L;j<xByte;j++)
		{
			p = i*xByte+ j;
			
			if(p > 1920L)
			{
				break;
			}
			
			LCM_WriteData(image[p]);
		}
		pY++;
	}
	
	LCM_SetGraphMode(false);
}

void ShowLCMFile(short pX,short pY,char imageFile[])
{
	LCMFile *lcmFile = NULL;
	FILE *fh = NULL;	
	short i, j, lineByte;    
	char* bfInfo = NULL;	
	char* subName = NULL;
	char screenFile[80];
	char bmpFile[80];
	char newimageFile[80];		//nick add 20110705
	size_t Fsize;
	
	memset(bmpFile,'\0',sizeof(bmpFile));
	memset(screenFile,'\0',sizeof(screenFile));
	memset(newimageFile, '\0', sizeof(newimageFile));		//nick add 20110705

	if (strcmp(LCM_Preshow, imageFile) == 0) return; // nick add 20130409 //
	
//nick mark 20110705	sprintf(screenFile,"./screen/%s",imageFile);
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)
		sprintf(screenFile,"./screen/%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
	else
		sprintf(screenFile,"./screen/%s", imageFile);
	
	fh = fopen(screenFile,"r");
	
	if(fh == NULL)
	{
//nick mark 20110705		printf("Can't Open %s .\n",screenFile);
//nick mark 20110705		return;

	//nick add s 20110705
		sprintf(screenFile,"./screen/%s",imageFile);
		fh = fopen(screenFile,"r");

		if (fh == NULL)
		{	//若兩個檔案皆無法開啟時,使用新格式
			sprintf(newimageFile, "%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
			goto SHOWVGA;
		}
		else
			memcpy(newimageFile, imageFile, sizeof(newimageFile));
	}
	else
	{
		if (strlen(G_ParkingConfig.ParkLanguage) != 0)
			sprintf(newimageFile, "%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
		else
			memcpy(newimageFile, imageFile, sizeof(newimageFile));
	}
	//nick add e 20110705

	bfInfo = new char[sizeof(LCMFile)];
	lcmFile = (LCMFile*)bfInfo;
	fseek(fh , 0L, SEEK_SET);
	Fsize = fread(bfInfo,1,sizeof(LCMFile),fh);
	fclose(fh);
		
	LCM_SetGraphMode(true);
	lineByte = lcmFile->width / 8 + (lcmFile->width % 8 ? 1:0);
	
	for(i=0; i<lcmFile->height; i++)
	{
		LCM_WriteCommand(0x60,pX);
		LCM_WriteCommand(0x70,pY+i);
	
		for(j=0;j<lineByte;j++)
		{			
			LCM_WriteData(lcmFile->bitData[i* lineByte + j]);
		}	
	}
	
	LCM_SetGraphMode(false);
	
//nick mark 20110705	subName = strstr(imageFile,".lcm");

	//nick add s 20110705
SHOWVGA:

	subName = strstr(newimageFile, ".lcm");
	//nick add e 20110705

	if(subName != NULL)
	{
//nick mark 20110705		strncpy(bmpFile,imageFile,strlen(imageFile)-4);
		strncpy(bmpFile,newimageFile,strlen(newimageFile)-4);		//nick add 20110705
		strcat(bmpFile,".bmp");
		//printf("Show BMP Image:%s\n", bmpFile);
		VGA_ShowBMP(bmpFile);
	}
	
	delete bfInfo;
	
	sprintf(LCM_Preshow, "%s", imageFile); // nick add 20130409 //
}

void Boot_ShowLCMFile(short pX,short pY,char imageFile[])
{
	LCMFile *lcmFile = NULL;
	FILE *fh = NULL;	
	short i, j, lineByte;    
	char* bfInfo = NULL;	
	char* subName = NULL;
	char screenFile[80];
	char bmpFile[80];
	char newimageFile[80];		//nick add 20110705
	size_t Fsize;
	
	memset(bmpFile,'\0',sizeof(bmpFile));
	memset(screenFile,'\0',sizeof(screenFile));
	memset(newimageFile, '\0', sizeof(newimageFile));		//nick add 20110705

	if (strcmp(LCM_Preshow, imageFile) == 0) return; // nick add 20130409 //
	
//nick mark 20110705	sprintf(screenFile,"./screen/%s",imageFile);
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)
		sprintf(screenFile,"./screen/%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
	else
		sprintf(screenFile,"./screen/%s", imageFile);
	
	fh = fopen(screenFile,"r");
	
	if(fh == NULL)
	{
//nick mark 20110705		printf("Can't Open %s .\n",screenFile);
//nick mark 20110705		return;

	//nick add s 20110705
		sprintf(screenFile,"./screen/%s",imageFile);
		fh = fopen(screenFile,"r");

		if (fh == NULL)
		{	//若兩個檔案皆無法開啟時,使用新格式
			sprintf(newimageFile, "%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
			goto SHOWVGA;
		}
		else
			memcpy(newimageFile, imageFile, sizeof(newimageFile));
	}
	else
	{
		if (strlen(G_ParkingConfig.ParkLanguage) != 0)
			sprintf(newimageFile, "%s/%s", G_ParkingConfig.ParkLanguage, imageFile);
		else
			memcpy(newimageFile, imageFile, sizeof(newimageFile));
	}
	//nick add e 20110705

	bfInfo = new char[sizeof(LCMFile)];
	lcmFile = (LCMFile*)bfInfo;
	fseek(fh , 0L, SEEK_SET);
	Fsize = fread(bfInfo,1,sizeof(LCMFile),fh);
	fclose(fh);
		
	Boot_LCM_SetGraphMode(true);
	lineByte = lcmFile->width / 8 + (lcmFile->width % 8 ? 1:0);
	
	for(i=0; i<lcmFile->height; i++)
	{
		Boot_LCM_WriteCommand(0x60,pX);
		Boot_LCM_WriteCommand(0x70,pY+i);
	
		for(j=0;j<lineByte;j++)
		{			
			Boot_LCM_WriteData(lcmFile->bitData[i* lineByte + j]);
		}	
	}
	
	Boot_LCM_SetGraphMode(false);

	
//nick mark 20110705	subName = strstr(imageFile,".lcm");

	//nick add s 20110705
SHOWVGA:

	subName = strstr(newimageFile, ".lcm");
	//nick add e 20110705

	if(subName != NULL)
	{
//nick mark 20110705		strncpy(bmpFile,imageFile,strlen(imageFile)-4);
		strncpy(bmpFile,newimageFile,strlen(newimageFile)-4);		//nick add 20110705
		strcat(bmpFile,".bmp");
		//printf("Show BMP Image:%s\n", bmpFile);
		VGA_ShowBMP(bmpFile);
	}
	
	delete bfInfo;
	
	sprintf(LCM_Preshow, "%s", imageFile); // nick add 20130409 //
}


void LCM_ShowTime()
{
	char LineString[40];
	time_t now;
	struct tm *tm_ptr = NULL;
	static time_t rec_time = 0L; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	if (CheckTimeout(lShowLocalTime, 500L) == false) return; // nick add 20130219 //
	now = time((time_t *)0);

	if (now == rec_time) return; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	memset(LineString,'\0',sizeof(LineString));
	tm_ptr = localtime(&now);
    
	// nick mark 20130409 //if(b_GServerConnect == true)
	if(b_GServerConnect == false) // nick add 20130409 //
	{
// ======================================================= //
// nick mark s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
//		sprintf(LineString,"%04d-%02d-%02d %02d:%02d:%02d ",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
//			tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
// nick mark e 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
// ======================================================= //
		// ====================================================== //
		// nick add s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
		if (G_ParkingConfig.DateTimeFormat[0] == 'd')
		{
			sprintf(LineString, "%02d-%02d-%04d %02d:%02d:%02d ", tm_ptr->tm_mday, tm_ptr->tm_mon + 1, tm_ptr->tm_year + 1900,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		else if (G_ParkingConfig.DateTimeFormat[0] == 'y')
		{
			sprintf(LineString, "%04d-%02d-%02d %02d:%02d:%02d ", tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		else
		{
			sprintf(LineString, "%02d-%02d-%04d %02d:%02d:%02d ", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_year + 1900,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		// nick add e 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
		// ====================================================== //
	}
	else
	{
// ======================================================= //
// nick mark s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
//		sprintf(LineString,"%04d-%02d-%02d %02d:%02d:%02d.",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
//			tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
// nick mark e 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
// ======================================================= //
		// ====================================================== //
		// nick add s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
		if (G_ParkingConfig.DateTimeFormat[0] == 'd')
		{
			sprintf(LineString,"%02d-%02d-%04d %02d:%02d:%02d.", tm_ptr->tm_mday, tm_ptr->tm_mon + 1, tm_ptr->tm_year + 1900,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		else if (G_ParkingConfig.DateTimeFormat[0] == 'y')
		{
			sprintf(LineString, "%04d-%02d-%02d %02d:%02d:%02d.", tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		else
		{
			sprintf(LineString, "%02d-%02d-%04d %02d:%02d:%02d.", tm_ptr->tm_mon + 1, tm_ptr->tm_mday, tm_ptr->tm_year + 1900,
				tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		}
		// nick add e 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
		// ====================================================== //
	}
	
	LCM_PrintString(0,3,LineString);
	// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	lShowLocalTime = GetTickCount(); // nick add 20130219 //
	rec_time = now; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
}

void VGA_ShowBMP(char filename[])
{
	struct cmd_msg_st cmddata;
	int msgid;
	
	if(G_ParkingConfig.UseMQ == 0) return;
	
	msgid = msgget((key_t)1234,0666 | IPC_CREAT);
	
	if(msgid == -1)
	{
		printf("create mq error \n");
		return;
	}
	
	cmddata.msg_type = 2;
	//Frank mark 20120206 printf("<<  VGA_ShowBMP()  :  filename = %s\n", filename);
	sprintf(cmddata.fileName,"%s",filename);
	
	if(msgsnd(msgid,(void*)&cmddata,80,0) == -1)
	{
		printf("Disp Send MQ error\n");
		return;
	}
}
