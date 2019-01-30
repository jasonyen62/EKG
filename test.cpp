/* Test Program */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <dirent.h>
#include <termios.h>
#include <time.h>
#include <linux/kd.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <SDL/SDL.h>

#include "CommonDef.h"
#include "IPMS_Driver/rs232.h"
#include "IPMS_Driver/MF700.h"
#include "IPMS_Driver/CRT350.h"
#include "IPMS_Driver/Eltra1000.h"
#include "IPMS_Driver/ra8822.h"
#include "IPMS_Driver/D1000.h"
#include "IPMS_Driver/D3000.h"
#include "IPMS_Driver/LED888.h"
#include "IPMS_Driver/BarCode.h"
#include "IPMS_Driver/Tup500.h"
#include "IPMS_Driver/MCP210.h"
#include "IPMS_Driver/HF320.h" // nick add 20150114 Ver:000-000-GIO_V2-13B251-0001-13C241 //
#include "test.h"
#include "IC8255.h"
#include "Network.h"
#include "Datafile.h"
#include "Reader.h"
#include "Voice.h"
#include "Dio.h"
#include "Display.h"
#include "traceLog.h"					//Frank add 20121112

#define outp(a, b)  outb(b, a)
#define inp(a)      inb(a)

static char *createsql = (char *)"CREATE TABLE Season("
               "SeasonID VARCHAR(11) PRIMARY KEY,"
               "StartDate DATE,"
               "EndDate DATE);";

static char *insertsql = (char *)"INSERT INTO Contact VALUES(1, 'Jason', '0926557381');";
static char *querysql  = (char *)"SELECT * FROM Season where SeasonID='parktron';";
int getch (void);

void Beep(int freq,int msec)
{
	int fd = open("/dev/tty10", O_RDONLY);
	
	if (fd > 0)
	{
		ioctl(fd, KDMKTONE, (msec<<16)+(1193180/freq));
	}
}

int getch ()
{
	int ch;
	struct termios oldt, newt;
	
	tcgetattr(STDIN_FILENO, &oldt);
	memcpy(&newt, &oldt, sizeof(newt));
	newt.c_lflag &= ~( ECHO | ICANON | ECHOE | ECHOK | 
							ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	
	return ch;
}

void Test232()
{
	char sCmd[128];
	char sendData[64];
	char c;
	int len=0;
	int port=0;
	int baudrate=0;
	int i;
	RS232 comport;

	memset(sCmd,'\0',sizeof(sCmd));
	memset(sendData,'\0',sizeof(sendData));
	printf("Select COM port:");

	c = getchar();
	putchar(c);
	port = atoi(&c);
	
	if(port > 8)
	{
		port =0;
	}
	
	printf("1. 9600\n");
	printf("2. 4800\n");
	printf("3. 19200\n");
	printf("4. 38400\n");
	printf("5. 115200\n");
	printf("Select Baud Rate:");

	c = getchar();
	putchar(c);
	
	switch( atoi(&c))
	{
		default:
		case 1:
			baudrate = 9600;
			break;
		case 2:
			baudrate = 4800;
			break;
		case 3:
			baudrate = 19200;
			break;
		case 4:
			baudrate = 38400;
			break;
		case 5:
			//baudrate = 115200;
			break;
	}
	
	baudrate = 9600;
	printf("Open COM%d,Baudrate:%d.\n",SerialPort[port]+1,baudrate);
	//comport.PortInit(SerialPort[port],baudrate,'N',8,1);
	comport.PortInit2(SerialPort[port],baudrate,'N',8,1);

	while(1)
	{
		//comport.PortOpen();
		len = sizeof(sCmd); // 20120221 Tony add
		if(comport.PortRead2(&len,sCmd,1000) == true)
		{
			for (i=0; i<len; i++)
			{
				printf("%c ",sCmd[i]);
			}
			
			printf("\n");
			usleep(100000L);
			//comport.PortWrite(sCmd,len);
			//printf("[%s]",sCmd);
			sprintf(sendData,"Receive:");
			comport.PortWrite(sendData,strlen(sendData));
			comport.PortWrite(sCmd,strlen(sCmd));
			len =0;

			if(strcmp(sCmd,"QUIT\n")==0)
			{
				break;
			}
			
			memset(sCmd,'\0',sizeof(sCmd));
		}
		usleep(100000);
	}
	
	comport.PortClose();
	printf("End RS232 TEST.\n");
}

void TestLED888()
{
	int i,Number;
	int j; // nick add 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	LED888 led888;

	// nick mark 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //led888.init(COM4);
	// nick mark 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //printf("LED888 in COM1\n");
	
	for (j = 1; j <= 2; j++)
	{
		led888.init(COM4, j);
		printf("LED888 in COM4, type:%d\n", j);
		
		for(i=0;i<10;i++)
		{	
			Number = i * 1111;
			printf("Num : %d \n",Number);
			led888.Send(1,Number);
			led888.Send(2,Number);
			led888.Send(3,Number);
			sleep(1);
		}
		
		led888.Close();
	}
}

//Frank add s 20121113
void WriteData(sectorData sdata)
{
	int i;
	char logbuf[100] , TmpBuf[5];
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 1
	sprintf(logbuf , "WriteData:: Block1:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)sdata.block1[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 2
	sprintf(logbuf , "WriteData:: Block2:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)sdata.block2[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 3
	sprintf(logbuf , "WriteData:: Block3:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)sdata.block3[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
}

void ReadData(sectorData rsdata)
{
	int i;
	char logbuf[100] , TmpBuf[5];
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 1
	sprintf(logbuf , "ReadData:: Block1:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)rsdata.block1[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 2
	sprintf(logbuf , "ReadData:: Block2:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)rsdata.block2[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
	
	memset(logbuf , '\0' , sizeof(logbuf));
	memset(TmpBuf , '\0' , sizeof(TmpBuf));
	
	// Block 3
	sprintf(logbuf , "ReadData:: Block3:");
	
	for(i = 0 ; i < 16 ; i++)
	{
		sprintf(TmpBuf , "%02X " , (unsigned char)rsdata.block3[i]);
		strcat(logbuf , TmpBuf);
	}
	
	ShowMessage(logbuf);
}

void TestMF700()
{
	char Tag[24];
	time_t now;
	struct tm *tm_ptr = NULL;
	sectorData sdata , rsdata;
	
	MF700 *mf700 = new MF700();
	mf700->init(COM2);  //COM2
	
	while(1)
	{
		if(mf700->CacheCard() == true)
		{
			memset(&sdata , 0xFF , sizeof(sdata));
			memset(&rsdata , 0xFF , sizeof(rsdata));
			
			if(mf700->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata) == 1)
			{
				ReadData(rsdata);
				ShowMessage((char*)"ReadSector OK!");
			}
			else
			{
				ShowMessage((char*)"ReadSector NG!");
				continue;
			}
			
			now    = time((time_t *)0);
			tm_ptr = localtime(&now);
			Int2BCD(tm_ptr ->tm_year + 1900 , 4 , &sdata.block1[0]);
			Int2BCD(tm_ptr ->tm_mon + 1 , 2 , &sdata.block1[2]);
			Int2BCD(tm_ptr ->tm_mday , 2 , &sdata.block1[3]);
			Int2BCD(tm_ptr ->tm_hour , 2 , &sdata.block1[4]);
			Int2BCD(tm_ptr ->tm_min , 2 , &sdata.block1[5]);
			Int2BCD(tm_ptr ->tm_sec , 2 , &sdata.block1[6]);
			
			memcpy(&sdata.block2,&sdata.block1,sizeof(sdata.block1));
			memcpy(&sdata.block3,&sdata.block1,sizeof(sdata.block1));
			
			if(mf700->WriteSector(Tag , PARKING_DATA_SECTOR , sdata) == 1)
			{
				WriteData(sdata);
				ShowMessage((char*)"WriteSector OK!");
			}
			else
			{
				ShowMessage((char*)"WriteSector NG!");
				continue;
			}
			
			if(mf700->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata) == 1)
			{
				ReadData(rsdata);
				ShowMessage((char*)"ReadSector OK!");
			}
			else
			{
				ShowMessage((char*)"ReadSector NG!");
				continue;
			}
			
			if(memcmp(&sdata , &rsdata , sizeof(rsdata)) == 0)
				ShowMessage((char*)"The Same!");
			else
				ShowMessage((char*)"Different!");
		}
		
		sleep(5);
	}
}
//Frank add e 20121113
//Frank mark s 20121112
/*void TestMF700()
{
	int SectorNumber[4] = {PARKING_DATA_SECTOR,PARKING_DATA_BAK_SECTOR,PARKING_DISCOUNT_SECTOR,PARKING_DISCOUNT_BAK_SECTOR};
	int i,j,sector;
//	int index =0;
//	char  ticketID[20];
	//char  writeData[20];
//	char  plate[10];
	char  Tag[24];
//	long  lTicketid;
	time_t now;
	struct tm *tm_ptr = NULL;
	unsigned char *SectorData = NULL;

//	sectorData sdata;
	sectorData rsdata;
	Block1 *block1 = NULL;
	Block2 *block2 = NULL;
	Block3 *block3 = NULL;

	MF700 *mf700 = new MF700();
	mf700->init(COM2);  //COM2

	mf700->mfHalt();
	sector = 1;

	SectorData = new unsigned char[50];
	memset(SectorData, 0xFF, 49);
	SectorData[49] = 0;
	block1 = (Block1*)(SectorData+0);
	block2 = (Block2*)(SectorData+16);
	block3 = (Block3*)(SectorData+32);

	while(1)
	{
		//printf("wait.. \n");
		if(mf700->CacheCard() == true)
		{
			printf(".\n");
			
			VoiceOn(index++);

			block1->format = 1;
			block1->parkingId = 5;//(unsigned char)G_ParkingConfig.ParkingID & 0xFF;
			block1->areaId = 1;//(unsigned char)(((G_ParkingConfig.ParkingID & 0x0300) >> 4 ) | (G_ParkingConfig.AreaID & 0x3F));
			block1->langId = 1;
			sprintf(ticketID,"%02d%06ld1",G_ParkingConfig.MachineID,999999L);
			lTicketid = atol(ticketID);
			printf("TicketID:%09ld\n",lTicketid);
			block1->ticketID = lTicketid;
			sprintf(plate,"**CA1234");
			memcpy(block1->plate,&plate,8);

			block2->status = 0x01;
			Int2BCD(tm_ptr->tm_year+1900,4,block2->in_year);
			Int2BCD(tm_ptr->tm_mon+1,2,&block2->in_month);
			Int2BCD(tm_ptr->tm_mday,2,&block2->in_day);
			Int2BCD(tm_ptr->tm_hour,2,&block2->in_hour);
			Int2BCD(tm_ptr->tm_min,2,&block2->in_min);

			memset(block2->Value,0,sizeof(long));
			memset(block2->seasonVersion,0,sizeof(long));
			block2->empty = (unsigned char)0xFF;

			Int2BCD(tm_ptr->tm_year+1900,4,block3->out_year);
			Int2BCD(tm_ptr->tm_mon+1,2,&block3->out_month);
			Int2BCD(tm_ptr->tm_mday,2,&block3->out_day);
			Int2BCD(tm_ptr->tm_hour,2,&block3->out_hour);
			Int2BCD(tm_ptr->tm_min,2,&block3->out_min);
			Int2BCD(tm_ptr->tm_sec,2,&block3->out_sec);
			block3->optime[0] = 11;
			block3->optime[1] = 22;
			block3->optime[2] = 33;
			memset(block3->empty,0xFF,sizeof(block3->empty));
			block3->randon = (unsigned char)(rand() % 256);
			block3->bcc = (unsigned char) 79; // initial value

			block3->bcc = INIT_DATA_BCC;
			
			for(i=0;i<47;i++)
			{	// check sum
				block3->bcc ^= (unsigned char)SectorData[i];
				//printf("%2X ",(unsigned char)SectorData[i]);
			}
			
			printf("BCC:%02x\n",block3->bcc);
			
			memcpy(&sdata.block1,block1,sizeof(Block1));
			memcpy(&sdata.block2,block2,sizeof(Block2));
			memcpy(&sdata.block3,block3,sizeof(Block3));
			
			
			//if(mf700->WriteSector(sector,sdata)==true)
			{
				//mf700->WriteSector(sector+1,sdata);
			}
			
			if(++sector >15)
				sector =2;
			
			for(j=0;j<4;j++)
			{
				printf("Sector:%d\n",SectorNumber[j]);
				now = time((time_t *)0);
				tm_ptr = gmtime(&now);
				
				if(mf700->ReadSector(Tag,SectorNumber[j],&rsdata)==1)
				{
					printf("Block1:");
					
					for(i=0;i<16;i++)
					{
						printf("%2X ",(unsigned char)rsdata.block1[i]);
					}
					
					printf("\n");
					printf("Block2:");
					
					for(i=0;i<16;i++)
					{
						printf("%02X ",(unsigned char)rsdata.block2[i]);
					}
					
					printf("\n");
					printf("Block3:");
					
					for(i=0;i<16;i++)
					{
						printf("%2X ",(unsigned char)rsdata.block3[i]);
					}
					
					printf("%04d-%02d-%02d\n",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
					Int2BCD(tm_ptr->tm_year+1900,4,block2->in_year);
					Int2BCD(tm_ptr->tm_mon+1,2,&block2->in_month);
					Int2BCD(tm_ptr->tm_mday,2,&block2->in_day);
					Int2BCD(tm_ptr->tm_hour,2,&block2->in_hour);
					Int2BCD(tm_ptr->tm_min,2,&block2->in_min);
					rsdata.block1[0] = 1;
					rsdata.block1[1] = 0;
					rsdata.block1[2] = 0;
					//printf("write %02X%02X-%02x-%02X\n",block2->in_year[0],block2->in_year[1],block2->in_month,block2->in_day);
					//if(mf700->WriteSector(SectorNumber[j],rsdata)==false)
					{
						
					}
				}
			}
		}
		
		usleep(100000L);
		mf700->mfHalt();
	}
	
	if(SectorData)
		delete SectorData;
}*/
//Frank mark e 20121112

void TestCRT350()
{
	char data[128];

	CRT350 reader;

	reader.init(SerialPort[1]); //COM2
	memset(data,'\0',sizeof(data));

	reader.GetStatus(STATUS_SENSOR,data);
	printf("%02X %02X %02X ",data[0],data[1],data[2]);
}


void WriteCMD(CMD_msg CMD)
{
    FILE* fh = NULL;
    char * buffer = NULL;
    
    fh = fopen("/tmp/cmd.txt","wb");
    
    if(fh != NULL)
    {
        buffer = (char*)(&CMD);
        fwrite(buffer,1,sizeof(Madia_msg_st),fh);
        fclose(fh);
    }
}

void TestBMP()
{
//	char data[128];
	CMD_msg CMD;
	DIR *dp=NULL;
	struct dirent *entry;
	struct stat statbuf;
	int iRst;
    
	for(int i=0;i<10;i++)
	{
		if((dp = opendir("jpg"))==NULL)
		{
			printf("cannot open directory: %s\n","jpg");
			return ;
		}
		
		iRst = chdir((char *)"jpg");
		
		while((entry = readdir(dp)) != NULL)
		{
			lstat(entry->d_name,&statbuf);
			
			if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
			{
				continue;
			}
			
			CMD.msg_type=0;
			sprintf(CMD.fileName, "%s" ,entry->d_name);
			WriteCMD(CMD);
			//Pause
			usleep(500000L);
		}
		
		sprintf(CMD.fileName,"end");
		WriteCMD(CMD);
		iRst = chdir((char *)"..");
		closedir(dp);
	}
}

void TestD1000()
{
//	bool bReadOK = false;
	char data[16];
//	int SectorNumber[4] = {PARKING_DATA_SECTOR,PARKING_DATA_BAK_SECTOR,PARKING_DISCOUNT_SECTOR,PARKING_DISCOUNT_BAK_SECTOR};
	
	int sector;
	int mID=1;
	//char  CardID[20];
//	char  ticketID[20];
	//char  writeData[20];
//	char  plate[10];
//	long  lTicketid=0L;
	time_t now;
	struct tm *tm_ptr=NULL;
	unsigned char *SectorData=NULL;
//	unsigned char *SectorDatar=NULL;
//	sectorData sdata;
//	sectorData rsdata;
	Block1 *block1=NULL;
	Block2 *block2=NULL;
	Block3 *block3=NULL;

	now = time((time_t *)0);
	tm_ptr = gmtime(&now);

	MF700 *mf700 = new MF700();
	mf700->init(SerialPort[3]);  //COM3

	mf700->mfHalt();
	sector = 1;

	SectorData = new unsigned char[50];
	memset(SectorData, 0x00, 49);
	SectorData[49] = 0;
	block1 = (Block1*)(SectorData+0);
	block2 = (Block2*)(SectorData+16);
	block3 = (Block3*)(SectorData+32);
	
	D1000 Dispenser;

	Dispenser.init(SerialPort[1]); //COM1
	memset(data,'\0',sizeof(data));
	
	Dispenser.Reset(mID);
	printf("Get Status:\n");
	
	while(1)
	{
		Dispenser.GetStatus(mID,data);
		usleep(500000L);
		printf("data:%s \n",data);
	}
	
	printf("Dispense \n");
	
	//Dispenser.DispenseCard(mID,D1000_DISPENSE_TO_TAKE);
	//sleep(1);
	
	/*
	for(k=0;k<60;k++)
	{
		bReadOK = true;
		for(i=0;i<3;i++)
		{
			Dispenser.DispenseCard(i,D1000_DISPENSE_TO_READER);
			
			for(i=0;i<30;i++)
			{
				usleep(100000L);
				
				if(mf700->CacheCard() == true)
					break;
			}
			
			if(i == 30)
			{
				printf("Time out\n");
				
				if(Dispenser.IsCardInMachine(mID))
				{
					Dispenser.RetrieveCard(mID);
				}
			}
			else
			{
				for(j=0;j<4;j++)
				{
					printf("Sector:%d\n",SectorNumber[j]);
					
					if(mf700->ReadSector(SectorNumber[j],&rsdata)==true)
					{
					}
					else
					{
						if(Dispenser.IsCardInMachine(mID))
						{
							Dispenser.RetrieveCard(mID);
						}
						
						bReadOK = false;
						break;
					}
				}			
			}
			
			sleep(1);
			Dispenser.DispenseCard(i,D1000_DISPENSE_TO_OUT); // 不會咬住的位置
		}
		
		sleep(1);
	}
	
	if(Dispenser.IsCardInMachine(mID))
	{
		Dispenser.RetrieveCard(mID);
	}

	Dispenser.DispenseCard(mID,D1000_DISPENSE_TO_OUT); // 不會咬住的位置
	*/
}

void TestD3000()
{
//	bool bReadOK = false;
	char data[16],c;
//	int SectorNumber[4] = {PARKING_DATA_SECTOR,PARKING_DATA_BAK_SECTOR,PARKING_DISCOUNT_SECTOR,PARKING_DISCOUNT_BAK_SECTOR};
	
	int sector;
//	int index =0;
//	char  ticketID[20];	
//	char  plate[10];
//	long  lTicketid=0L;
	time_t now;
	struct tm *tm_ptr=NULL;
	unsigned char *SectorData=NULL;
//	unsigned char *SectorDatar=NULL;
//	sectorData sdata;
//	sectorData rsdata;
	Block1 *block1=NULL;
	Block2 *block2=NULL;
	Block3 *block3=NULL;

	now = time((time_t *)0);
	tm_ptr = gmtime(&now);

	MF700 *mf700 = new MF700();
	mf700->init(SerialPort[1]);  //COM1

	mf700->mfHalt();
	sector = 1;

	SectorData = new unsigned char[50];
	memset(SectorData, 0x00, 49);
	SectorData[49] = 0;
	block1 = (Block1*)(SectorData+0);
	block2 = (Block2*)(SectorData+16);
	block3 = (Block3*)(SectorData+32);
	
	D3000 Dispenser;

	Dispenser.init(SerialPort[2]); //COM2
	memset(data,'\0',sizeof(data));
	
	Dispenser.Reset(0);
	printf("Get Status:\n");
	Dispenser.GetStatus(0,data);
	printf("Status:%02x \n",(unsigned char)data[8]);
		
	while(1)
	{
	/*
		for(i=0;i<30;i++)
		{
			usleep(100000L);
			
			if(mf700->CacheCard() == true)
				break;
		}
		if(i == 30)
		{
			continue;
		}
		else
		{
			for(j=0;j<4;j++)
			{
				printf("Sector:%d\n",SectorNumber[j]);
				
				if(mf700->ReadSector(SectorNumber[j],&rsdata)==true)
				{
				}
				else
				{
					if(Dispenser.IsCardInMachine(0))
					{
						Dispenser.RetrieveCard(0);
					}
					
					bReadOK = false;
					break;
				}
			}
		}
		*/
		
		printf("1.GetStatus\n");
		printf("2.RetrieveCard\n");
		printf("3.RejectCard\n");
		printf("4.Enable\n");
		printf("5.Disable\n");
		
		c=getch();
		printf("c=%c",c);
		
		if(c=='1')
		{
			printf("Status:");
			Dispenser.GetStatus(0,data);
			printf("Status:%6s \n",data);
		}
		else if(c=='2')
		{
			printf("RetrieveCard:");
			
			if(Dispenser.IsCardInMachine(0))
			{
				Dispenser.RetrieveCard(0);
			}
		}
		else if(c=='3')
		{
			printf("RejectCard:");
			
			if(Dispenser.IsCardInMachine(0))
			{
				Dispenser.RejectCard(0);
			}
		}
		else if(c=='4')
		{
			Dispenser.EnableWork(0,D3000_ENABLE);
		}
		else if(c=='5')
		{
			Dispenser.EnableWork(0,D3000_DISABLE);
		}
		else if(c=='0')
		{
			break;
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		//Dispenser.DispenseCard(0,D1000_DISPENSE_TO_READER);
		//mf700->mfHalt();
	}	
}

void TestLCM()
{
//	char* bmpFile[11] = {
//		"Fault.lcm","NoPay.lcm","TicketDataError.lcm","InsertTicket.lcm","ParkingFull.lcm",
//		"TicketReadError.lcm","PushButton.lcm","Leave.lcm","TakeTicket.lcm","parktron.lcm","LCMt1.lcm"
//	};
//	char filename[80];
	char LineString[40];
	char bitmapData[1920];
	time_t now;
	struct tm *tm_ptr;
//	int i;
//	unsigned long xByte,yByte;
		
	memset(bitmapData,'\0',sizeof(bitmapData));
	memset(LineString,'\0',sizeof(LineString));
	IOInitial();
	
	printf("Reset LCM\n");
	LCM_Reset();
	LCM_Light(true);
	printf("initial LCM\n");
	LCM_Initial();
	//usleep(1000);
	printf("Clear LCM\n");
	LCM_FillScreen(0x00);

	sprintf(LineString,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	LCM_PrintString(0,0,LineString);
	
	sprintf(LineString,"012345678901234567890123456789");
	LCM_PrintString(0,1,LineString);
	
	sprintf(LineString,"博辰科技");  //中文	
	LCM_PrintString(20,3,LineString);
	
	now = time((time_t *)0);
	
	tm_ptr = localtime(&now);
	sprintf(LineString,"%04d-%02d-%02d %02d:%02d:%02d",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
		tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	printf("%s\n",LineString);
	
	LCM_PrintString(0,3,LineString);
	//LCM_LoadBitmap(bitmapData,&xByte,&yByte,"LCMt1.bmp");
	
	//printf("X:%ld  Y:%ld\n",xByte,yByte);
	//LCM_ShowImage(0,0, xByte,yByte,bitmapData);
    
	// Show all Picture
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	
	if((dp=opendir("screen"))==NULL)
	{
		fprintf(stderr,"cannot open screen Directory\n");
		return;
	}
	
	//chdir("screen");
	
	while((entry=readdir(dp)) != NULL)
	{
		sleep(6);
		LCM_ShowTime();
		lstat(entry->d_name,&statbuf);
		
		if(strstr(entry->d_name,".lcm") >0)
		{
			printf("Filename:[%s]\n",entry->d_name);
			ShowLCMFile(0,0,entry->d_name);		
		}
      
		//LCM_LoadBitmap(bitmapData,&xByte,&yByte,bmpFile[i%10]);
		//LCM_ShowImage(0,0, xByte,yByte,bitmapData);
		//ShowLCMFile(0,0,bmpFile[i%10]);
	}
	
	//chdir("..");
	closedir(dp);
}

void SqliteTest()
{
	int i,j;
	int rows, cols;
	sqlite3 *db;
	char *errMsg = NULL;
	char **result;
	
	/*  }   database    */
	if (sqlite3_open_v2("example.db3", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
		printf("Open example.db3 fail!\n");
		return;
	}

	printf("Open Success!\n");
	
	/*      Table */
	sqlite3_exec(db, createsql, 0, 0, &errMsg);

	/*  s W @       */
	sqlite3_exec(db, insertsql, 0, 0, &errMsg);
	/*    o           ID */
	//printf("ID:%ld\n", sqlite3_last_insert_rowid(db));

	/*    o database              */
	if(sqlite3_get_table(db , querysql, &result , &rows, &cols, &errMsg)==SQLITE_OK)
	{
		printf("List Data:\n");
		
		/*  C X         */
		for ( i=0;i<=rows;i++)
		{
			for (j=0;j<cols;j++)
			{
				printf("%-10s\t", result[i*cols+j]);
			}
			printf("\n");
		}
	}

	/*      */
	sqlite3_free_table(result);

	/*      database */
	sqlite3_close(db);
}

void WatchDogEnable(bool yes)
{
	char c = 0x00;
	
	if(yes)
	{	// Enable watchdog timer
		c = inp(0x68);
		c |= 0x40;
	}
	else
	{   // Disable watchdog timer
		c = inp(0x68);
		c &= ~(0x40);
	}
	
	outp(0x68, c);
}

void WatchDogSetTimer(unsigned long msec)
{
	iopl(3);
	unsigned long lTime;

	lTime = 0x20L * msec;
	outp(0x6c, (unsigned char)(lTime >> 16) & 0xff);
	outp(0x6b, (unsigned char)(lTime >> 8) & 0xff);
	outp(0x6a, (unsigned char)(lTime >> 0) & 0xff);
	outp(0x69, 0xd0); //Set: When Timeout ,do Reset.
}

void WatchDogClearTimer(void)
{   //clear watchdog timer value
	outp(0x67, 0x00);
}

void Test8255()
{
//	int i=0;
	unsigned char data=0x01;
	unsigned char portA=0x00,portB=0x01;
	IC8255 io8255_2;
	IC8255 io8255_1;
	io8255_2.Init8255(0x0304,false,false,false,false);
	io8255_1.Init8255(0x0300,true,true,false,false);

	while(1)
	{	
		//io8255_1.Out8255Byte(IC8255_PORTA,0x55);
		//io8255_1.Out8255Byte(IC8255_PORTB,0x55);
		//portB=io8255_1.Get8255Byte(IC8255_PORTB);
		portA=io8255_1.Get8255Byte(IC8255_PORTA);
		printf("A:[%02X] B:[%02X] \n",portA,portB);
		/*
		io8255_1.Out8255Byte(IC8255_PORTC,0x55);
		io8255_2.Out8255Byte(IC8255_PORTA,0x55);
		io8255_2.Out8255Byte(IC8255_PORTB,0x55);
		io8255_2.Out8255Byte(IC8255_PORTC,0x55);
		*/
		usleep(500000L);
		portB=io8255_1.Get8255Byte(IC8255_PORTB);
		//portA=io8255_1.Get8255Byte(IC8255_PORTA);
		printf("A:[%02X] B:[%02X] \n",portA,portB);
		//io8255_1.Out8255Byte(IC8255_PORTA,0xAA);
		//io8255_1.Out8255Byte(IC8255_PORTB,0xAA);
		/*
		io8255_1.Out8255Byte(IC8255_PORTC,0xAA);
		io8255_2.Out8255Byte(IC8255_PORTA,0xAA);
		io8255_2.Out8255Byte(IC8255_PORTB,0xAA);
		io8255_2.Out8255Byte(IC8255_PORTC,0xAA);
		*/
		printf("%02x \n",~data);
		io8255_1.Out8255Byte(IC8255_PORTC,~data);
		data<<=1;
		usleep(500000L);
		
		if(data == 0x80) 
			data = 0x01;
	}
}

void TestVoice()
{
	int i=0;
	enum VOICEDEF voicedef[15]=
	{
		VOICE_NONE,
		VOICE_WELCOME,
		VOICE_TAKETICKET,
		VOICE_PLSENTER,
		VOICE_PFULL ,
		VOICE_FAULT,
		VOICE_INVTICKET,
		VOICE_PROCESSING,
		VOICE_PLSEXIT,
		VOICE_BRCLOSE,
		VOICE_INSERT ,
		VOICE_READERROR,
		VOICE_PLSWAIT 
	};
    
	IOInitial();

	while(1)
	{
		for(i=0;i<12;i++)
		{			
			//io8255_2.Out8255HByte(IC8255_PORTC,~((unsigned char)((i+1)<<4)));
			//usleep(50000L); //最少50ms
			printf("%d \n",i);
			//io8255_2.Out8255HByte(IC8255_PORTC,0xFF);
			
			VoiceOn(voicedef[i+1]);
			sleep(8);
		}
	}
}

void UdpTest()
{
	int i;
	char CarPlate[10];
	char SendData[256];
	char recvData[UDPRecvBuffSize];
	int len;
	UDPSocket udpPlateServer;
	txParktron tx;
	//myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial((char *)"192.168.0.164",2393, 1994);
	//printf("plate IP:%s port:%d myport:%d\n",G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort,myport+1);
	
	memset(CarPlate,'\0',sizeof(CarPlate));
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	
	while(1)
	{
		printf("Receive:\n");
		
		if((len=udpPlateServer.udpRecv(recvData)) > 0)
		{ 	//Check format.  Do server command
			//printf("UDP recv: %d",len);
			
			for(i=0;i<len;i++)
			{
				printf("%02x ",(unsigned char)recvData[i]);
			}
			
			printf("\n");	
			udpPlateServer.parseCommand(&tx, recvData);
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
}

#define IO_PERMOFF  0
#define IO_PERMON   3
#define VORTEX86SX 0x31504d44
#define VORTEX86DX 0x32504d44
#define VORTEX86MX 0x33504d44
 // Vortex86SX: 0x31504d44
  // Vortex86DX: 0x32504d44
  // Vortex86MX: 0x33504d44
  // We check Vortex86SX here
unsigned int IsVortex86SX()
{
	unsigned long val;
	iopl(IO_PERMON);
	outl(0x80000090, 0xcf8);
	val = inl(0xcfc);
	iopl(IO_PERMOFF);

	if(val == VORTEX86SX)
		return 1;
	else
		return 0;
}

void PlateTest()
{
	int i;
	char CarPlate[10];
	char SendData[256];
	char recvData[UDPRecvBuffSize];
	int len;
	UDPSocket udpPlateServer;
	txParktron tx;
	//myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial((char *)"192.168.0.115",1792, 1792);
	//printf("plate IP:%s port:%d myport:%d\n",G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort,myport+1);
	
	memset(CarPlate,'\0',sizeof(CarPlate));
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	
	//sprintf(plate,"********");
	//Command ID = 1046
	
	sprintf(SendData,"\x04""%d\x1c""%X""\x1c""%d""\x1c""%ld""i\x1F",1,0x02010700,1075,812312311L);
	printf("%s\n",SendData);
	udpPlateServer.udpSendWithBccEnd(SendData,strlen(SendData));
	usleep(100000L);
	
	printf("Send plate Receive:\n");

	if((len=udpPlateServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		//printf("UDP recv: %d",len);
		if(recvData[0] == 0x06)
		{
			if((len=udpPlateServer.udpRecv(recvData)) > 0)
			{
				for(i=0;i<len;i++)
				{
					printf("%02x ",(unsigned char)recvData[i]);
				}
				
				printf("\n");
				udpPlateServer.parseCommand(&tx, recvData);
				
				if(recvData[0] == 0x04)
				{
					printf("ACK!\n");
					memset(SendData,'\0',sizeof(SendData));
					sprintf(SendData,"\x06""1");	
					udpPlateServer.udpSend(SendData,2);
				}
			}
		}
	}
	//memcpy(plate, CarPlate,strlen(CarPlate));
}

void CommandTest()
{
	int i;
	char CarPlate[10];
	char SendData[256];
	char recvData[UDPRecvBuffSize];
	int len;
	UDPSocket udpPlateServer;
//	txParktron tx;
	//myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial((char *)"192.168.0.11",1993, 1792);
	//printf("plate IP:%s port:%d myport:%d\n",G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort,myport+1);
	//  02 30 30 30 30 30 31 1c 30 30 30 30 30 37 30 30 1c 31 30 30 31 1c 33 69 31 41 03
	memset(CarPlate,'\0',sizeof(CarPlate));
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	
	//sprintf(plate,"********");
	//Command ID = 1046
	
	sprintf(SendData,"\x02\x30\x30\x30\x30\x30\x31\x1c\x30\x30\x30\x30\x30\x37\x30\x30\x1c\x31\x30\x30\x31\x1c\x33\x69\x31\x41\x03");
	
	udpPlateServer.udpSend(SendData,strlen(SendData));
	usleep(100000L);
	printf("Send plate Receive:\n");
	
	if((len=udpPlateServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		//printf("UDP recv: %d",len);
		if(recvData[0] == 0x06)
		{		
			for(i=0;i<len;i++)
			{
				printf("%02x ",(unsigned char)recvData[i]);
			}
			printf("\n");			
		}
	}
	
	//memcpy(plate, CarPlate,strlen(CarPlate));
}

void timetest()
{
	time_t now;
	time_t entry;
	struct tm EntryTime;

	//printf("TEST:\n");
	now = time((time_t *)0);
	EntryTime.tm_year = 2010-1900;
	EntryTime.tm_mon = 1;
	EntryTime.tm_mday = 21;
	EntryTime.tm_hour = 17;
	EntryTime.tm_min = 13;
	EntryTime.tm_sec = 40;
	EntryTime.tm_wday = 4;
	//EntryTime.tm_isdst = now.tm_isdst;
	
	printf("now:%ld \n",now);

	entry = mktime(&EntryTime);
	printf("entry:%ld \n",entry);
}

/*
#define WIDTH 640
#define HEIGHT 480
#define BPP 4
#define DEPTH 32

void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 *pixmem32;
	Uint32 colour;  
	
	colour = SDL_MapRGB( screen->format, r, g, b );
	
	pixmem32 = (Uint32*) screen->pixels  + y + x;
	*pixmem32 = colour;
}


void DrawScreen(SDL_Surface* screen, int h)
{
	int x, y, ytimesw;
	
	if(SDL_MUSTLOCK(screen)) 
	{
		if(SDL_LockSurface(screen) < 0) return;
	}
	
	for(y = 0; y < screen->h; y++ ) 
	{
		ytimesw = y*screen->pitch/BPP;
		
		for( x = 0; x < screen->w; x++ ) 
		{
			setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
		}
	}
	
	if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
	SDL_Flip(screen); 
}
*/

int SDLTest()
{
	int a,b;
	a=500;
	b=1000;
	printf("a/b=%.1f\n",(double)a/b);
//	SDL_Surface *screen;
//	SDL_Event event;
	
//	int keypress = 0;
//	int h=0; 

	//if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
	/* 
	if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_FULLSCREEN|SDL_HWSURFACE)))
	{
		SDL_Quit();
		return 1;
	}
	
	while(!keypress) 
	{
		DrawScreen(screen,h++);
		
		while(SDL_PollEvent(&event)) 
		{      
			switch (event.type) 
			{
				case SDL_QUIT:
					keypress = 1;
					break;
				case SDL_KEYDOWN:
					keypress = 1;
					break;
			}
		}
	}
	*/
	
	//  SDL_Quit();
	return 0;
}

void BarcodeTest()
{
	char data[128];
	D3000 Retrive;
	BarCode barcode;
	int success,fail;
	int i,count;
	
	success=0;
	fail=0;
	Retrive.init(COM1);
	barcode.init(COM4);
	
	Retrive.Reset(0);
	Retrive.EnableWork(0,D3000_ENABLE);
	while(1)
	{
		barcode.ReadStart();
		
		while(1)
		{
			usleep(50000L);
			
			if( Retrive.IsCardIn(0)==true)
			{                
				printf("Start.\n");
				break;
			}
			
			//if(Retrive.IsCardInMachine(0)==true)
			//	Retrive.RejectCard(0);
		}
        
		//while(1)
		{
			count =0;
			
			for(i=0;i<1;i++)
			{
				memset(data,'\0',sizeof(data));
				
				if(barcode.GetData((char*)(&data))==true)
				{
					count++;
					printf("%s\n",data);
				}
			}
			
			if(count >0)
			{
				success++;
			}
			else
			{
				fail++;
			}
			
			printf("Success:%d Fail:%d Count:%d \n",success,fail,count);
			sleep(1);
			Retrive.RejectCard(0);
		}
		
		barcode.ReadEnd();
		printf("End.\n");
	}
}

void TUP500Test()
{
	int i;
	TUP500 printer;
//	char status;
	char datas[128];
	
	char datetimeString[30];
	struct tm *in_DateTime = NULL;
	
	time_t now;
	
	printer.init(COM4,1);
	
	printer.SetPrintArea();
	printer.SetHLine(100,550,675,2);
	printer.SetBarCodePosition(160, 0, 1, 7, 0, 80);	
	printer.SetCharactPosition(110, 480, 1, 1, 3, 0);
	printer.SetCharactPosition(120, 680, 1, 1, 1, 0);
	printer.SetCutter();
	
	for(i=0;i<100;i++)
	{
		now = time((time_t *)0);
		memset(datetimeString,'\0',sizeof(datetimeString));

		in_DateTime = localtime(&now);
		sprintf(datetimeString,"%04d.%02d.%02d %02d:%02d:%02d",
			in_DateTime->tm_year+1900, in_DateTime->tm_mon+1, in_DateTime->tm_mday,
			in_DateTime->tm_hour,in_DateTime->tm_min,in_DateTime->tm_sec);

		memset(datas,'\0',sizeof(datas));
	
		printer.SetBarCodeData((char *)"09385886811");
		printer.SetCharactData(0,(char *)"Parktron 中文 Test");
		printer.SetCharactData(1,datetimeString);
		printer.SetPrintDensity(i%5+1);
		
		printer.IssuePaper();
		printer.CheckNearEnd();
		printf("%d\n",i);
		sleep(3);
		printer.CheckPaperOnOutlet();
		printer.Receive();
	}
}

void TestMCP210()
{
	char cls[] = "parktron";
	int mID = 1;
	int SectorNumber[4] = {PARKING_DATA_SECTOR , PARKING_DATA_BAK_SECTOR
						   , PARKING_DISCOUNT_SECTOR , PARKING_DISCOUNT_BAK_SECTOR};
	int i , j , sector;
	char Tag[24];
	time_t now;
	struct tm *tm_ptr = NULL;
	unsigned char *SectorData = NULL;
	
	MCP210 mcp210;
	mcp210.Init(COM1);
	mcp210.Reset();
	
	MF700 *mf700 = new MF700();
	mf700 ->init(COM2);
	
	D1000 Dispenser;
	Dispenser.init(SerialPort[3]);
	
	sectorData rsdata;
	Block1 *block1 = NULL;
	Block2 *block2 = NULL;
	Block3 *block3 = NULL;
	
	mf700->mfHalt();
	sector = 1;
	
	SectorData = new unsigned char[50];
	
	memset(SectorData , 0xFF , 49);
	
	SectorData[49] = 0;
	
	block1 = (Block1*)(SectorData + 0);
	block2 = (Block2*)(SectorData + 16);
	block3 = (Block3*)(SectorData + 32);
	
	while(1)
	{
		if(mcp210.GetStatus() != 2)
		{
			Dispenser.DispenseCardOut(mID);
			mcp210.InsertTicket();
			
			for(j = 0 ; j < 4 ; j++)
			{
				printf("Sector:%d\n" , SectorNumber[j]);
				now = time((time_t *)0);
				tm_ptr = gmtime(&now);
				
				if(mf700 ->ReadSector(Tag , SectorNumber[j] , &rsdata) == 1)
				{
					printf("Block1:");
					
					for(i = 0 ; i < 16 ; i++)
					{
						printf("%2X " , (unsigned char)rsdata.block1[i]);
					}
					
					printf("\n");
					printf("Block2:");
					
					for(i = 0 ; i < 16 ; i++)
					{
						printf("%02X " , (unsigned char)rsdata.block2[i]);
					}
					
					printf("\n");
					printf("Block3:");
					
					for(i = 0 ; i < 16 ; i++)
					{
						printf("%2X " , (unsigned char)rsdata.block3[i]);
					}
					
					printf("%04d-%02d-%02d\n" , tm_ptr ->tm_year+1900 , tm_ptr ->tm_mon+1 , tm_ptr ->tm_mday);
					Int2BCD(tm_ptr ->tm_year+1900 , 4 , block2 ->in_year);
					Int2BCD(tm_ptr ->tm_mon+1 , 2 , &block2 ->in_month);
					Int2BCD(tm_ptr ->tm_mday , 2 , &block2 ->in_day);
					Int2BCD(tm_ptr ->tm_hour , 2 , &block2 ->in_hour);
					Int2BCD(tm_ptr ->tm_min , 2 , &block2 ->in_min);
					rsdata.block1[0] = 1;
					rsdata.block1[1] = 0;
					rsdata.block1[2] = 0;
				}
			}
				
			if(mcp210.EjectCardOut(cls)==-2)
				mcp210.EjectCardOut(cls);
		}
		else
		{
			printf("wait take card!\n");
			while(1)
			{
				if(mcp210.GetStatus() != 2)
					break;

				usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
			}
		}
	}
}

void TestD1800()
{
	int mID = 1;
	
	D1000 Dispenser;
	
	Dispenser.init(SerialPort[3]);
	Dispenser.Reset(mID);
	
	while(1)
	{
		printf("cardout : %d\n" , Dispenser.DispenseCardOut(mID));
		sleep(5);
	}
}

void HF320Test() // nick add 20150114 Ver:000-000-GIO_V2-13B251-0001-13C241 //
{
	HF320 *reader = new HF320();
	char CardID[20];
	char BlockData[33];
	char buf[1024];
	sectorData mySectorData;
	double startTick = 0;
	int iWriteSector = 5;
	int iTestLoop = 15;
	int i = 0;
	
	memset(CardID, 0, sizeof(CardID));
	memset(buf, 0, sizeof(buf));
	
	if (reader->init(COM1))
	{
		printf("Open Success.\n");
		
		sleep(3);
		
		for (int iTestCnt = 1; iTestCnt <= iTestLoop; iTestCnt++)
		{
			printf("Start Read / Write One Sector. Cnt=[%d]\n", iTestCnt);
			startTick = GetTickCount();
			
			if (strlen(CardID) <= 0)
			{
				for (i = 0; i < 3; i++)
				{
					if (reader->CacheCard(CardID) == 1)
						break;
				}
				
				if (i >= 3)
					goto PORTCLOSE_FLAG;
			}
			
			printf("Get Card Tag:[%s]\n", CardID);
			memset(&mySectorData, 0, sizeof(sectorData));
			
			for (i = 0; i < 3; i++)
			{
				if (reader->ReadSector(CardID, iWriteSector, &mySectorData) == 1)
					break;
			}
			
			if (i >= 3)
				goto PORTCLOSE_FLAG;
			
			printf("ReadSector Finished.\n");
			// Print Read Data
			sprintf(buf, "Get Card Sector Data -> Block1:[");
			memset(BlockData, 0, sizeof(BlockData));

			for (i = 0; i < (int)sizeof(mySectorData.block1); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block1[i]);
				mySectorData.block1[i] = (mySectorData.block1[i] + 1) & 0xFF;
			}

			sprintf(buf + strlen(buf), "%s], Block2:[", BlockData);
			memset(BlockData, 0, sizeof(BlockData));

			for (i = 0; i < (int)sizeof(mySectorData.block2); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block2[i]);
				mySectorData.block2[i] = (mySectorData.block2[i] + 1) & 0xFF;
			}

			sprintf(buf + strlen(buf), "%s], Block3:[", BlockData);
			memset(BlockData, 0, sizeof(BlockData));

			for (i = 0; i < (int)sizeof(mySectorData.block3); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block3[i]);
				mySectorData.block3[i] = (mySectorData.block3[i] + 1) & 0xFF;
				printf("%d ", mySectorData.block3[i]);
			}
			
			printf("\n");

			sprintf(buf + strlen(buf), "%s]", BlockData);
			printf("%s\n", buf);
			//
			G_MemBlocksWriteSuc[iWriteSector] = 0;
			
			for (i = 0; i < 3; i++)
			{
				if (reader->WriteSector(CardID, iWriteSector, mySectorData) == 1)
					break;
			}
			//

			if (i >= 3)
				goto PORTCLOSE_FLAG;
			
			printf("Finish Read / Write One Sector. [%d]->Spend time : [%lf]\n", iTestCnt, GetTickCount() - startTick);
			
			if (iTestCnt < iTestLoop)
				sleep(2);
		}
		
		PORTCLOSE_FLAG:
		
		reader->Close();
	}
	
	reader = NULL;
}

void HF320_Visual_Card_Test()
{
	HF320 *reader = new HF320();
	char CardID[20];
	char BlockData[33];
	char buf[1024];
	sectorData mySectorData;
	double startTick = 0;
	int iWriteSector = 5;
	int iTestLoop = 15;
	int i = 0;
	long l_DisplayData = 0;
	
	memset(CardID, 0, sizeof(CardID));
	memset(buf, 0, sizeof(buf));
	
	if (reader->init(COM1))
	{
		printf("Open Success.\n");
		reader->SetAPDUMode(true);
		
		sleep(3);
		
		for (int iTestCnt = 1; iTestCnt <= iTestLoop; iTestCnt++)
		{
			memset(CardID, 0, sizeof(CardID));
			printf("Start Read / Write One Sector. Cnt=[%d]\n", iTestCnt);
			startTick = GetTickCount();
			
			if (strlen(CardID) <= 0)
			{
				for (i = 0; i < 3; i++)
				{
					if (reader->CacheCard(CardID) == 1)
						break;
				}
				
				if (i >= 3)
					goto PORTCLOSE_FLAG;
			}
			
			printf("Get Card Tag:[%s]\n", CardID);
			memset(&mySectorData, 0, sizeof(sectorData));
			
			for (i = 0; i < 3; i++)
			{
				if (reader->ReadSector(CardID, iWriteSector, &mySectorData) == 1)
					break;
			}
			
			if (i >= 3)
				goto PORTCLOSE_FLAG;
			
			printf("ReadSector Finished.\n");
			
			// Print Read Data
			sprintf(buf, "Get Card Sector Data -> Block1:[");
			memset(BlockData, 0, sizeof(BlockData));
         
			for (i = 0; i < (int)sizeof(mySectorData.block1); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block1[i]);
				mySectorData.block1[i] = (mySectorData.block1[i] + 1) & 0xFF;
			}
         
			sprintf(buf + strlen(buf), "%s], Block2:[", BlockData);
			memset(BlockData, 0, sizeof(BlockData));
         
			for (i = 0; i < (int)sizeof(mySectorData.block2); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block2[i]);
				mySectorData.block2[i] = (mySectorData.block2[i] + 1) & 0xFF;
			}
         
			sprintf(buf + strlen(buf), "%s], Block3:[", BlockData);
			memset(BlockData, 0, sizeof(BlockData));
         
			for (i = 0; i < (int)sizeof(mySectorData.block3); i++)
			{
				sprintf(BlockData + 2 * i, "%02X", mySectorData.block3[i]);
				mySectorData.block3[i] = (mySectorData.block3[i] + 1) & 0xFF;
				printf("%d ", mySectorData.block3[i]);
			}
			
			printf("\n");
         
			sprintf(buf + strlen(buf), "%s]", BlockData);
			printf("%s\n", buf);
			//
			// Display Datetime
			l_DisplayData = 10281200L + iTestCnt;
			sprintf(buf, "%ld", l_DisplayData);
			
			for (i = 0; i < 3; i++)
			{
				if (reader->ShowDisplay(buf, strlen(buf)) != false)
					break;
			}
			//
			// Write Sector Data
			G_MemBlocksWriteSuc[iWriteSector] = 0;
			
			for (i = 0; i < 3; i++)
			{
				if (reader->WriteSector(CardID, iWriteSector, mySectorData) == 1)
					break;
			}
			//
         
			if (i >= 3)
				goto PORTCLOSE_FLAG;
			
			printf("Finish Read / Write One Sector. [%d]->Spend time : [%lf]\n", iTestCnt, GetTickCount() - startTick);
			
			if (iTestCnt < iTestLoop)
				sleep(2);
		}
		
		PORTCLOSE_FLAG:
		
		printf("Close RS232 start!\n");
		reader->Close();
		printf("Close RS232 end!\n");
	}
	
	reader = NULL;
	printf("Set reader NULL end!\n");
}

