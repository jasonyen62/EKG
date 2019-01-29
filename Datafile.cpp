/* Datafile.cpp 
    All read/write file function(include SQLite db).
*/
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sqlite3.h>
#include "CommonDef.h"
#include "Datafile.h"
#include "Reader.h"
#include "traceLog.h"
#include "Network.h"					//Frank add 20120514
#include "ServerTalk.h"					//Frank add 20120514

#define LineWordMax 200

#define RETRY 3
#define TICKET_SN_FILE "TicketSN.txt"
#define TICKET1_FILE "Ticket1.txt"
#define TICKET2_FILE "Ticket2.txt"
#define TICKET_SN_TEMP "TicketSN.tmp"

ParkingConfig G_ParkingConfig;
FixConfig G_FixConfigSetting; // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
unsigned long FileSystemWaitTime = 50000; 	// 50ms

bool INIReadLine(char* lineData,FILE* fh)
{
	bool bComment = false;
	short index = 0;
	char c = 0x00;
	
	do
	{
		while(true)
		{
			c = fgetc(fh);
			
			if(c == EOF)
				return true;
			
			//if(c == 0x0d /*|| c == ' '*/)
			if(c == 0x0d || c == 0x09 )
				continue;
			
			if(c == 0x0a)
			{
				lineData[index]=0;
				//printf("%d\n",index);
				break;
			}
			
			if(bComment == true)
				continue;
			
			if(c == ';' || c == '#')
			{
				bComment = true;
				continue;
			}
			
			lineData[index] = c;
			index ++;
			//printf(".",index);
			
			if(index >= LineWordMax)
			{
				lineData[LineWordMax] = 0;
				break;
			}
		}
		
		//printf("%s\n",lineData);
		
		if(strlen(lineData) == 0)
		{
			index = 0;
			bComment = false;
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}while(strlen(lineData) == 0);
	
	return false;
}

bool ReadIni(char* data,char* filename,char* Section,char* key)
{
	int   len=0,i;
	bool  bFindKey = false;
	FILE* fh = NULL;
	char* SectionPoint = NULL;
	char* keyPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer[LineWordMax+1];
	
	fh = fopen(filename,"rb");
	
	if(fh == NULL)
		return false;
		
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek ( fh , 0L , SEEK_SET );
	
	while(true)
	{
		if(INIReadLine(LineData,fh)==true)
		{ //true :is EOF
			break;
		}
		
		//printf("Line: %s\n",LineData);
		sprintf(buffer,"[%s]",Section);
		//trim right
		
		len = strlen(LineData);
		
		for(i=len-1;i>0;i--)
		{
			printf("c:[%c]",LineData[i]);
			if(LineData[i] == ' ')
			{
				LineData[i] = 0;
			}
			else
			{
				break;
			}
		}
		
		if( SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			sprintf(buffer,"%s=",key);
			keyPoint = strstr(LineData,buffer);
			if(keyPoint != NULL)
			{
				bFindKey = true;
				strcpy(data,LineData+strlen(buffer));
			}
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	fclose(fh);
	return bFindKey;
}

unsigned long GetTicketSerial()
{
	FILE *fh = NULL;
	char SN[10];
	long serialNo = 1L;
	
	memset(SN,'\0',sizeof(SN));
	
	// Read Serial number
	fh = fopen(TICKET_SN_FILE,"r+");
	
	if(fh == NULL)
	{	//write a new file
		fh = fopen(TICKET_SN_FILE,"w");
		
		if(fh == NULL)
		{   //write fail
			return 0L;
		}
		
		fprintf(fh,"000001\n");
		fclose(fh);
	}
	else
	{
		if(fread (SN,1,6L,fh)>0)
		{
			serialNo = atol(SN);
		}
		serialNo++;

		// 20120925 Tony add s
		if(ReaderCFG.HourlyReaderType == 5 && G_ParkingConfig.BarCodeType == 0)
		{
			if(serialNo > 99999L)
			{
				serialNo =1;
			}
		}
		else
		{
		// 20120925 Tony add  e
			if(serialNo > 999999L)
			{
				serialNo =1;
			}
			
		}// 20120925 Tony add 
		
		fseek(fh,0L,SEEK_SET);
		fprintf(fh,"%06ld\n",serialNo);
		fclose(fh);
	}
	
	return serialNo;
}

unsigned short GetReTicket()
{
	FILE *fh = NULL;
	char Number[10];
	unsigned short Tickets = 0;
	
	memset(Number,'\0',sizeof(Number));
	
	// Read Serial number
	fh = fopen("ReTicket.txt","r+");
	
	if(fh == NULL)
	{	//write a new file
		fh = fopen("ReTicket.txt","w");
		
		if(fh == NULL)
		{   //write fail
			return 0;
		}
		
		fprintf(fh,"0000\n");
		fclose(fh);
	}
	else
	{
		if(fread(Number,1,4L,fh)>0)
		{
			Tickets = atoi(Number);
		}		
		fclose(fh);
	}	
	
	return Tickets;
}

void SaveReTicket(unsigned short ReTickets)
{
	FILE *fh = NULL;
	
	// Read Serial number
	fh = fopen("ReTicket.txt","w");
	
	if(fh == NULL)
	{   //write fail
		return;
	}
	
	fprintf(fh,"%04d\n",ReTickets);
	fclose(fh);
}

unsigned short GetTicket1()
{
	FILE *fh = NULL;
	char Number[10];
	unsigned short serialNo = 1;
	
	memset(Number,'\0',sizeof(Number));
	
	// Read Serial number
	fh = fopen(TICKET1_FILE,"r+");
	
	if(fh == NULL)
	{	//write a new file
		fh = fopen(TICKET1_FILE,"w");
		
		if(fh == NULL)
		{   //write fail
			return 0L;
		}
		
		fprintf(fh,"5000\n");
		serialNo = 5000;
		fclose(fh);
	}
	else
	{
		if(fread (Number,1,4L,fh)>0)
		{
			serialNo = atoi(Number);
		}		
		
		fclose(fh);
	}
	
	return serialNo;
}

unsigned short GetTicket2()
{
	FILE *fh = NULL;
	char Number[10];
	unsigned short serialNo = 1;

	memset(Number,'\0',sizeof(Number));
	
	// Read Serial number
	fh = fopen(TICKET2_FILE,"r+");
	
	if(fh == NULL)
	{	//write a new file
		fh = fopen(TICKET2_FILE,"w");
		
		if(fh == NULL)
		{   //write fail
			return 0L;
		}
		
		fprintf(fh,"5000\n");
		serialNo = 5000;
		fclose(fh);
	}
	else
	{
		if(fread (Number,1,4L,fh)>0)
		{
			serialNo = atoi(Number);
		}		
		
		fclose(fh);
	}
	
	return serialNo;
}

void SaveTicketNumber()
{
	FILE *fh = NULL;
	
	// Read Serial number
	fh = fopen(TICKET1_FILE,"w");
	
	if(fh == NULL)
	{   //write fail
		return;
	}
	
	fprintf(fh,"%04d\n",ReaderCFG.Ticket1);
	fclose(fh);
	fh = fopen(TICKET2_FILE,"w");
	
	if(fh == NULL)
	{   //write fail
		return;
	}
	
	fprintf(fh,"%04d\n",ReaderCFG.Ticket2);
	fclose(fh);
}

void SaveTicket2()
{
	FILE *fh = NULL;
	
	fh = fopen(TICKET2_FILE,"w");
	
	if(fh == NULL)
	{   //write fail
		return;
	}
	
	fprintf(fh,"%04d\n",ReaderCFG.Ticket2);
	fclose(fh);
}

void SaveTicket1()
{
	FILE *fh = NULL;
	
	// Read Serial number
	fh = fopen(TICKET1_FILE,"w");
	
	if(fh == NULL)
	{   //write fail
		return;
	}
	
	fprintf(fh,"%04d\n",ReaderCFG.Ticket1);	
	fclose(fh);	
}

void GetKeyName(char *key,char *value,char LineString[])
{
	char *pEqu = NULL;
	short len = 0,i;

	len = strlen(LineString);
	
	for(i=len-1;i>0;i--)
	{	
		if(LineString[i] == ' ')
		{
			LineString[i] = 0;
		}
		else
		{
			break;
		}
	}
	
	pEqu = strchr(LineString,'=');
	
	if(pEqu)
	{
		memcpy(key,LineString,pEqu-LineString);
		len = strlen(LineString) - (pEqu+1-LineString);
		memcpy(value,pEqu+1,len);
	}
}

bool ReadINI_Common()
{
	bool bFindKey = false;
	FILE* fh = NULL;
	char* SectionPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	int iMaintain_Hour = -1; // nick add 20160418 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	int iMaintain_Min = -1; // nick add 20160418 Ver:000-000-GIO_V2-13B251-0010-13C241 //

	fh = fopen("Parking.ini","rb");
	
	if(fh == NULL)
	{
		return false;
	}

	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek ( fh , 0L , SEEK_SET );
	
	while(true)
	{
		if(INIReadLine(LineData,fh) == true) //true :is EOF
			break;
		
		if(SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,"[COMMON]");
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section (is next section)
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
						
			GetKeyName(key,value,LineData);
			
			if(strcmp(key,"ParkingID")==0)
			{
				G_ParkingConfig.ParkingID = atoi(value);
				//printf("Parking ID = %d\n",G_ParkingConfig.ParkingID);
			}
			
			if(strcmp(key,"ParkingName")==0)
			{
				sprintf(G_ParkingConfig.ParkingName,"%s",value);
				//printf("Parking ID = %d\n",G_ParkingConfig.ParkingID);
			}
			
			if(strcmp(key,"ServerIP")==0)
			{
				sprintf(G_ParkingConfig.ServerIP,"%s" ,value);
				//printf("Server IP = %s\n",G_ParkingConfig.ServerIP);
			}
			
			if(strcmp(key,"ServerPort")==0)
			{
				G_ParkingConfig.ServerPort = atoi(value);
				//printf("Server port = %d\n",G_ParkingConfig.ServerPort);
			}
			
			if(strcmp(key,"QuietTime")==0)
			{
				G_ParkingConfig.QuietTime = atoi(value);
			}
			
			// For Barcode
			if(strcmp(key,"Type")==0)
			{
				//printf("Type:[%s]\n",value);
				G_ParkingConfig.BarCodeType = atoi(value);
			}
			
			if(strcmp(key,"Palstn")==0)
			{
				sprintf(G_ParkingConfig.BarCodeDetail ,"%s",value);
			}
			
			if(strcmp(key,"DateFormat")==0)
			{
				sprintf(G_ParkingConfig.DateFormat ,"%s",value);
			}
			
			// ========================================================= //
			// nick add s 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
			if(strcmp(key,"DateTimeFormat")==0)
			{
				sprintf(G_ParkingConfig.DateTimeFormat, "%s", value);
			}
			// nick add e 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
			// ========================================================= //
			
			if(strcmp(key,"Note")==0)
			{
				sprintf(G_ParkingConfig.Note ,"%s",value);
				//printf("Note:[%s]\n",G_ParkingConfig.Note);
			}
			
			if(strcmp(key,"USEMQ")==0)
			{
				G_ParkingConfig.UseMQ = atoi(value);		
			}
			
			if(strcmp(key,"Button_Timeout") == 0)					//Frank add s 20111020
			{
				G_ParkingConfig.WaitTime.Button_Timeout = atoi(value);
			}
			
			if(strcmp(key,"TakeTicket_Timeout") == 0)
			{
				G_ParkingConfig.WaitTime.TakeTicket_Timeout = atoi(value);
			}
			
			//Frank mark s 20120903 有重復設定
			/*
			if(strcmp(key,"WaitLoop2_Timeout") == 0)
			{
				G_ParkingConfig.WaitTime.WaitLoop2_Timeout = atoi(value);
			}
			*/
			//Frank mark e 20120903
			
			if(strcmp(key,"CloseBarrier_Timeout") == 0)
			{
				G_ParkingConfig.WaitTime.CloseBarrier_Timeout = atoi(value);
			}
			
			if(strcmp(key,"WaitCardIn_Timeout") == 0)
			{
				G_ParkingConfig.WaitTime.WaitCardIn_Timeout = atoi(value);
			}					//Frank add e 20111020
			
			//Frank add s 20120906
			if(strcmp(key , "VISOpenBarrier") == 0)
			{
				G_ParkingConfig.VISOpenBarrier = atoi(value);
			}
			
			if(strcmp(key , "VISforHourly") == 0)
			{
				G_ParkingConfig.VISforHourly = atoi(value);
			}
			
			if(strcmp(key , "VISforSeason") == 0)
			{
				G_ParkingConfig.VISforSeason = atoi(value);
			}
			//Frank add e 20120906
			//Frank add s 20130123
			G_ParkingConfig.MaxDiscountHours = 99999; // nick add 20130318 //
			
			if(strcmp(key , "MaxDiscountHours") == 0)
			{
				G_ParkingConfig.MaxDiscountHours = atoi(value);
				if (G_ParkingConfig.MaxDiscountHours < 0) G_ParkingConfig.MaxDiscountHours = 0; // nick add 20130318 //
			}
			//Frank add e 20130123
			
			// ========================================================= //
			// nick add s 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
			if (strcmp(key, "maintain_time") == 0)
			{
				sscanf(value, "%d:%d", &iMaintain_Hour, &iMaintain_Min);
				
				if ((iMaintain_Hour >= 0 && iMaintain_Hour <= 23) && (iMaintain_Min >= 0 && iMaintain_Min <= 59))
					sprintf(G_ParkingConfig.MaintainTime, "%s", value);
			}
			// nick add e 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
			// ========================================================= //
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	printf("Load MaintainTime:[%s], Hour:[%d], Mins:[%d].\n", G_ParkingConfig.MaintainTime, iMaintain_Hour, iMaintain_Min); // nick add 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	fclose(fh);
	return bFindKey;
}

bool ReadINI_Location()
{
	bool bFindKey= false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	
	fh = fopen("Parking.ini","rb");
	
	if(fh == NULL)
	{
		return false;
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek (fh , 0L , SEEK_SET);
	
	sprintf(buffer,"[Location%02d]",G_ParkingConfig.AreaID);
	//printf("Section:%s \n",buffer);
	
	while(true)
	{
		if(INIReadLine(LineData,fh)==true) //true :is EOF
			break;
		// printf("ini:%s",LineData);
		
		if(SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			//printf("%s \n",SectionPoint);
			if(strchr(LineData,'[') != NULL)
			{	//End section (is next section)
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			GetKeyName(key,value,LineData);
			//sprintf(keyName,"FreeTime");
			
			//Frank mark s 20120907
			/*if(strcmp(key,"FreeTime")==0)
			{
				G_ParkingConfig.FreeTime = atoi(value);
				//printf("FreeTime= %d\n",G_ParkingConfig.FreeTime);
			}*/
			//Frank mark e 20120907
			
			sprintf(keyName,"Space");
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.ParkingSpace = atoi(value);
				//printf("Space = %d\n",G_ParkingConfig.ParkingSpace);
			}
			
			sprintf(keyName,"Fee");
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.Fee = atoi(value);
			}
			
			sprintf(keyName,"FreeTime");
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.FreeTime = atoi(value);
			}
			
			sprintf(keyName,"PaidTime");
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.PaidTime = atoi(value);
			}
			
			sprintf(keyName,"FeeTime"); //計費單位
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.FeeTime = atoi(value);
			}
			
			sprintf(keyName,"ParkingFullCanEntry"); //車位滿仍可進車
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.ParkingFullCanEntry = atoi(value);
			}
			
			sprintf(keyName,"RFCheckPlate"); //RF in 要不要檢查車牌
			
			if(strcmp(key,keyName)==0)
			{
				G_ParkingConfig.RFCheckPlate = atoi(value);
			}
			
			//Frank add s 20120907
			if(strcmp(key , "Hourly_Use") == 0)
			{
				G_ParkingConfig.Hourly_Use = atoi(value);
			}
			
			if(strcmp(key , "Season_Use") == 0)
			{
				G_ParkingConfig.Season_Use = atoi(value);
			}
			//Frank add e 20120907

			//Frank add s 20120919			
			if(strcmp(key , "MoneyRate") == 0)
			{
				G_ParkingConfig.MoneyRate = atoi(value);
			}			
			//Frank add e 20120919
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	// 20110520 Tony add s
		
	//printf("UpLayer ID: %d\n",G_ParkingConfig.UpLayerID);
	
	if (G_ParkingConfig.UpLayerID > 0)
	{
		SectionPoint = NULL;
		memset(LineData,0,sizeof(LineData));
		memset(buffer,0,sizeof(buffer));
		fseek ( fh , 0L , SEEK_SET );
		
		sprintf(buffer,"[Location%02d]",G_ParkingConfig.UpLayerID);
		
		while(true)
		{
			if(INIReadLine(LineData,fh)==true) //true :is EOF
				break;
				
			if( SectionPoint == NULL)
			{
				SectionPoint = strstr(LineData,buffer);
			}
			else
			{
				if(strchr(LineData,'[') != NULL)
				{	
					break;
				}
								
				memset(value,0,sizeof(value));
				memset(key,0,sizeof(key));
				memset(keyName,0,sizeof(keyName));
				GetKeyName(key,value,LineData);
				
				sprintf(keyName,"FreeTime");
				
				if(strcmp(key,keyName)==0)
				{
					G_ParkingConfig.UpLayerFreeTime= atoi(value);
					printf("IN UpLayer Free Time: %d\n",G_ParkingConfig.UpLayerFreeTime);
					break;
				}
			}

			usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		}
	}
	// 20110520 Tony add e
	
	fclose(fh);
	return bFindKey;
}

bool ReadINI_INMachine(short sn)
{
	bool bFindKey= false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
//	char* keyPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	
	fh = fopen("Parking.ini","rb");
	
	if(fh == NULL)
	{
		//printf("Read Ini file fail.\n");
		return false;
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek (fh , 0L , SEEK_SET );
	
	sprintf(buffer,"[IN%02d]",sn);
	
	while(true)
	{
		if(INIReadLine(LineData,fh)==true) //true :is EOF
			break;
			
		if(SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			
			GetKeyName(key,value,LineData);
			
			if(strcmp(key,"AreaID")==0)
			{
				G_ParkingConfig.AreaID = atoi(value);
				//printf("AreaID:%d\n",G_ParkingConfig.AreaID);
			}
			
			// 20110505 Tony add s
			if(strcmp(key,"UpLayerID")==0)
			{
				G_ParkingConfig.UpLayerID= atoi(value);
				//printf("in PreLayer ID: %d\n",G_ParkingConfig.PreLayerID);
			}
			// 20110505 Tony add e
			
			if(strcmp(key,"MachineID")==0)
			{
				G_ParkingConfig.MachineID = atoi(value);
				//printf("in Machine ID: %d\n",G_ParkingConfig.MachineID);
			}
			
			if(strcmp(key,"ClientID")==0)
			{			
				sscanf(value,"%08lX",&G_ParkingConfig.ClientID);
				//printf("ClientID = %X\n",G_ParkingConfig.ClientID);
			}
			
			if(strcmp(key,"PlateClientID")==0)
			{
				sscanf(value,"%08lX",&G_ParkingConfig.PlateClientID);
				//printf("PlateClientID = %X\n",G_ParkingConfig.PlateClientID);
			}
			
			if(strcmp(key,"PlateServerIP")==0)
			{
				if(strlen(value) >0)
				{
					sprintf(G_ParkingConfig.PlateServerIP,"%s" ,value);
				}
			}
			
			if(strcmp(key,"PlateServerPort")==0)
			{
				G_ParkingConfig.PlateServerPort = atoi(value);
			}
			
			if(strcmp(key,"PlateTimeout")==0)
			{
				G_ParkingConfig.PlateTimeout = atoi(value);
			}
			
			if(strcmp(key,"LED888Port")==0)
			{
				G_ParkingConfig.LED888port = SerialPort[atoi(value)];
			}
			
			if(strcmp(key,"LED888Type")==0)
			{
				G_ParkingConfig.LED888Type = atoi(value);
			}
			
			if(strcmp(key,"HReaderPort")==0)
			{
				ReaderCFG.HourlyReaderComPort = SerialPort[atoi(value)];
				ReaderCFG.HourlyReaderComPort2 = SerialPort[atoi(value)+1];
			}
			
			if(strcmp(key,"HReaderType")==0)
			{
				ReaderCFG.HourlyReaderType = atoi(value);
			}
			
			if(strcmp(key,"SReaderPort")==0)
			{
				ReaderCFG.SeasonReaderComPort = SerialPort[atoi(value)];
			}
			
			if(strcmp(key,"SReaderType")==0)
			{
				ReaderCFG.SeasonReaderType = atoi(value);
			}
			
			if(strcmp(key,"SeasonReadTag")==0)
			{
				G_ParkingConfig.SeasonReadTag = atoi(value);
			}
			
			if(strcmp(key,"AutoIssueTicket")==0)
			{
				if(atoi(value) == 1)
					G_ParkingConfig.AutoIssueTicket = true;
				else
					G_ParkingConfig.AutoIssueTicket = false;
			}
			
			if(strcmp(key,"MifareDispenser") == 0)
			{
				ReaderCFG.MifareDispenserComPort = SerialPort[atoi(value)];
			}
			
			if(strcmp(key,"DispenserQuantity") == 0)
			{
				ReaderCFG.DispenserQuantity = atoi(value);
			}
			
			if(strcmp(key,"BinsSize") == 0)
			{
				G_ParkingConfig.BinsSize = atoi(value);
			}
			
			if(strcmp(key,"PreTicket") == 0)
			{
				if(atoi(value)==0)
					G_ParkingConfig.PreTicket = false;
				else
					G_ParkingConfig.PreTicket = true;
			}
				
			if(strcmp(key,"Loop2Timeout") == 0)
			{
				G_ParkingConfig.Loop2Timeout = atoi(value);
				//Frank add s 20120903
				if(G_ParkingConfig.Loop2Timeout < 10)
					G_ParkingConfig.Loop2Timeout = 10;
				//Frank add e 20120903
			}
			
			if(strcmp(key,"EasyCardIOControl") == 0)
			{
				//ReaderCFG.MifareDispenser = atoi(value);
			}
			
			// ====================================================================================================================== //
			// nick add s 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
			if(strcmp(key,"ImageServerIP")==0)
			{
				if(strlen(value) >0)
				{
					sprintf(G_ParkingConfig.ImageServerIP,"%s" ,value);
				}
			}
			
			if(strcmp(key,"ImageServerPort")==0)
			{
				G_ParkingConfig.ImageServerPort = atoi(value);
			}
			// nick add e 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
			// ====================================================================================================================== //
		}
		
		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}	
	
	fclose(fh);
	return bFindKey;
}

bool ReadINI_OutMachine(short sn)
{
	bool bFindKey= false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
//	char* keyPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	
	fh = fopen("Parking.ini","rb");
	
	if(fh == NULL)
	{
		printf("Read Ini file fail.\n");
		return false;
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek ( fh , 0L , SEEK_SET );
	
	sprintf(buffer,"[OUT%02d]",sn);
	
	while(true)
	{
		if(INIReadLine(LineData,fh)==true) //true :is EOF
			break;
		 //printf("%s\n",LineData);
		
		if( SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section 
				//printf("%s \n",LineData);
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			
			GetKeyName(key,value,LineData);
			
			//printf("read:%s   key:%s value:%s\n",LineData,key,value);
			if(strcmp(key,"AreaID")==0)
			{
				G_ParkingConfig.AreaID = atoi(value);
				//printf("Area:%d\n",G_ParkingConfig.AreaID );
			}
			
			// 20110505 Tony add s
			if(strcmp(key,"UpLayerID")==0)
			{
				G_ParkingConfig.UpLayerID= atoi(value);
				//printf("in PreLayer ID: %d\n",G_ParkingConfig.PreLayerID);
			}
			// 20110505 Tony add e
			
			if(strcmp(key,"MachineID")==0)
			{
				G_ParkingConfig.MachineID = atoi(value);
				//printf("out Machine ID: %d\n",G_ParkingConfig.MachineID);
			}
			
			if(strcmp(key,"ClientID")==0)
			{			
				sscanf(value,"%08lX",&G_ParkingConfig.ClientID);
				//printf("ClientID = %X\n",G_ParkingConfig.ClientID);
			}
			
			if(strcmp(key,"HReaderPort")==0)
			{
				ReaderCFG.HourlyReaderComPort = SerialPort[atoi(value)];
				ReaderCFG.HourlyReaderComPort2 = SerialPort[atoi(value)+1];
				//printf("HReaderPort:%d\n",ReaderCFG.HourlyReaderComPort );
			}
			
			if(strcmp(key,"PlateServerIP")==0)
			{
				if(strlen(value) >0)
				{
					sprintf(G_ParkingConfig.PlateServerIP,"%s" ,value);
				}
			}
			
			if(strcmp(key,"PlateTimeout")==0)
			{
				G_ParkingConfig.PlateTimeout = atoi(value);
			}
			
			if(strcmp(key,"PlateServerPort")==0)
			{
				G_ParkingConfig.PlateServerPort = atoi(value);
			}
			
			if(strcmp(key,"HReaderType")==0)
			{
				ReaderCFG.HourlyReaderType = atoi(value);
				//printf("HReaderType:%d\n",ReaderCFG.HourlyReaderType);
			}
			
			if(strcmp(key,"SReaderPort")==0)
			{
				ReaderCFG.SeasonReaderComPort = SerialPort[atoi(value)];
				//printf("SReaderPort:%d\n",ReaderCFG.SeasonReaderComPort);
			}
			
			if(strcmp(key,"SReaderType")==0)
			{
				ReaderCFG.SeasonReaderType = atoi(value);
				//printf("SReaderType:%d\n",ReaderCFG.SeasonReaderType);
			}
			
			if(strcmp(key,"SeasonReadTag")==0)
			{
				G_ParkingConfig.SeasonReadTag = atoi(value);
			}
			
			if(strcmp(key,"MifareDispenser")==0)
			{
				ReaderCFG.MifareDispenserComPort = SerialPort[atoi(value)];
				//printf("MifareDispenser:%d\n",ReaderCFG.MifareDispenserComPort);
			}
			
			if(strcmp(key,"DispenserQuantity")==0)
			{
				ReaderCFG.DispenserQuantity = atoi(value);
				//printf("DispenserQuantity:%d\n",ReaderCFG.DispenserQuantity);
			}
			
			if(strcmp(key,"BinsSize") == 0)
			{
				G_ParkingConfig.BinsSize = atoi(value);
			}
			
			if(strcmp(key,"Loop2Timeout") == 0)
			{
				G_ParkingConfig.Loop2Timeout = atoi(value);
				//Frank add s 20120903
				if(G_ParkingConfig.Loop2Timeout < 10)
					G_ParkingConfig.Loop2Timeout = 10;
				//Frank add e 20120903
			}
			
			if(strcmp(key,"EasyCardIOControl") == 0)
			{
				//ReaderCFG.MifareDispenser = atoi(value);
			}

			// ====================================================================================================================== //
			// nick add s 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
			if(strcmp(key,"ImageServerIP")==0)
			{
				if(strlen(value) >0)
				{
					sprintf(G_ParkingConfig.ImageServerIP,"%s" ,value);
				}
			}
			
			if(strcmp(key,"ImageServerPort")==0)
			{
				G_ParkingConfig.ImageServerPort = atoi(value);
			}
			// nick add e 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
			// ====================================================================================================================== //
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	fclose(fh);
	return bFindKey;
}

bool ReadINI_ApsMachine(short sn)
{
	bool bFindKey= false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
//	char* keyPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	
	fh = fopen("Parking.ini","rb");
	
	if(fh == NULL)
	{
		//printf("Read Ini file fail.\n");
		return false;
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek ( fh , 0L , SEEK_SET );
	
	sprintf(buffer,"[APS%02d]",sn);
	
	while(true)
	{
		if(INIReadLine(LineData,fh) == true) //true :is EOF
			break;
			
		if( SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section (is next section)
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			
			GetKeyName(key,value,LineData);
			
			if(strcmp(key,"AreaID") == 0)
			{
				G_ParkingConfig.AreaID = atoi(value);
				//printf("AreaID:%d \n",G_ParkingConfig.AreaID);
			}
			
			if(strcmp(key,"MachineID") == 0)
			{
				G_ParkingConfig.MachineID = atoi(value);
				//printf("aps Machine ID: %d\n",G_ParkingConfig.MachineID);
			}
			
			if(strcmp(key,"HReaderPort") == 0)
			{
				ReaderCFG.HourlyReaderComPort = SerialPort[atoi(value)];
			}
			
			if(strcmp(key,"HReaderType") == 0)
			{
				ReaderCFG.HourlyReaderType = atoi(value);
			}
			
			if(strcmp(key,"SReaderPort") == 0)
			{
				ReaderCFG.SeasonReaderComPort = SerialPort[atoi(value)];
			}
			
			if(strcmp(key,"SReaderType") == 0)
			{
				ReaderCFG.SeasonReaderType = atoi(value);
			}		
		}

		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	fclose(fh);
	return bFindKey;
}

bool ReadINI_FixConfig() // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
{
	bool bRtn = false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
	char	LineData[LineWordMax+1];
	char	buffer  [LineWordMax+1];
	char	key[128],value[128],keyName[128];

	memset(&G_FixConfigSetting, 0, sizeof(FixConfig));
	fh = fopen("/Data/LocalSeting/FixConfig.ini","rb");

	if (fh == NULL)
	{
		printf("Can't Find FixConfig.ini\n");
		return(bRtn);
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek ( fh , 0L , SEEK_SET );
	
	sprintf(buffer,"[COMMON]");

	while(true)
	{
		if(INIReadLine(LineData,fh) == true) //true :is EOF
			break;
			
		if( SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section (is next section)
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			
			GetKeyName(key,value,LineData);
			
			if(strcmp(key, "Allow_Area") == 0)
			{
				strncpy(G_FixConfigSetting.ch_Allow_Area, value, 9);
				printf("G_FixConfigSetting.ch_Allow_Area=[%s]\n", G_FixConfigSetting.ch_Allow_Area);
			}
		}

		usleep(10);
	}
	
	fclose(fh);
	return(bRtn);
}

bool ReadINI_NewTerminalConnectSetting()
{
	bool bFindKey= false;
	FILE* fh=NULL;
	char* SectionPoint = NULL;
//	char* keyPoint = NULL;
	char  LineData[LineWordMax+1];
	char  buffer  [LineWordMax+1];
	char  key[128],value[128],keyName[128];
	
	fh = fopen("/Data/LocalSeting/NewTerminal.ini","rb");
	
	if(fh == NULL)
	{
		//printf("Read Ini file fail.\n");
		return false;
	}
	
	memset(LineData,0,sizeof(LineData));
	memset(buffer,0,sizeof(buffer));
	fseek (fh , 0L , SEEK_SET );
	
	sprintf(buffer,"[COMMON]");
	
	while(true)
	{
		if(INIReadLine(LineData,fh)==true) //true :is EOF
			break;
			
		if(SectionPoint == NULL)
		{
			SectionPoint = strstr(LineData,buffer);
		}
		else
		{
			if(strchr(LineData,'[') != NULL)
			{	//End section
				break;
			}
			
			memset(value,0,sizeof(value));
			memset(key,0,sizeof(key));
			memset(keyName,0,sizeof(keyName));
			
			GetKeyName(key,value,LineData);
						
			if(strcmp(key,"NewTerminalIP")==0)
			{
				if(strlen(value) >0)
				{
					sprintf(G_ParkingConfig.NewTerminalIP,"%s" ,value);
				}
			}
			
			if(strcmp(key,"NewTerminalPort")==0)
			{
				G_ParkingConfig.NewTerminalPort = atoi(value);
			}
			
			if(strcmp(key,"LocalPort")==0)
			{
				G_ParkingConfig.LocalPort = atoi(value);
			}			
		}
		
		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}	
	
	fclose(fh);
	return bFindKey;
}


bool SQLiteCheckTableExist(char* TableName)
{
	bool bRet = false;
	int rows, cols;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if(sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		sprintf(qSQL,"SELECT name FROM sqlite_master WHERE name='%s'",TableName);
		
		if(sqlite3_get_table(db , qSQL, &result , &rows, &cols, &errMsg)==SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				bRet = true;
			}
			
			sqlite3_free_table(result);
		}
		else
		{
			printf("Chekc Exist Error:%s\n",errMsg);
		}
		
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
// nick mark 20130202 //		return bRet;
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool GetTempHourlyData(HTicketData* data)
{
	bool bRet = false;
	int rows, cols;
	int FieldCnt;		//nick add 20111219
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(qSQL,"SELECT TicketID,InDateTime,PlateNumber,InStaus FROM TempHourly");
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL,"SELECT [TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID] FROM [TempHourly]"); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
		sprintf(qSQL,"SELECT [TicketID], [InDateTime], [PlateNumber], [InStaus], [TagID], [AreaID] FROM TempHourly");	// Tony add 20150720 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		FieldCnt = 4;					//Frank add 20120206
		
		if(sqlite3_get_table(db , qSQL, &result , &rows, &cols, &errMsg)==SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				/*
				//nick mark s 20111219
				data->TicketID = atol(result[4]);
				data->in_time = atol(result[5]);
				sprintf(data->Plate,"%s", result[6]);
				
				if(strstr(result[7],"Y") != NULL)
					data->InStatus = true;
				else
					data->InStatus = false;
				//nick mark e 20111219
				*/

				FieldCnt = cols - 1;		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				
				//nick add s 20111219
				data->TicketID = atol(result[FieldCnt+1]);
				data->in_time = atol(result[FieldCnt+2]);
				sprintf(data->Plate,"%s", result[FieldCnt+3]);
				
				if(strstr(result[FieldCnt+4],"Y") != NULL)
					data->InStatus = true;
				else
					data->InStatus = false;
				//nick add e 20111219

				sprintf(data->TagID,"%s", result[FieldCnt+5]);	// Tony add 20150720 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				data->AreaID = atol(result[FieldCnt+6]);		// Tony add 20150720 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) data->AreaID = atol(result[FieldCnt+5]); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
				bRet = true;
			}
			
			sqlite3_free_table(result);
			usleep(FileSystemWaitTime);
		}
		else
		{
			printf("GetTempHourly Error:%s\n",errMsg);
		}
		
		sqlite3_close(db);
// nick mark 20130202 //		return bRet;
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool DeleteTempHourlyData(HTicketData data)
{
	bool bRet = false;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db = NULL;

	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
	{	//delete data
		
		sprintf(qSQL,"DELETE FROM TempHourly WHERE TicketID = %ld AND InDateTime= %ld",data.TicketID,data.in_time);
		
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		bRet = true;	
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

void DeleteTempHourlyOldData(time_t in_time)
{
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db = NULL;

	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
	{	//delete data	
		sprintf(qSQL,"DELETE FROM TempHourly WHERE InDateTime < %ld",in_time);
		
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
}


bool WriteTempHourlyData(HTicketData data,bool bIsOut)
{
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	bool bRet = false; // nick add 20130202 //

	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
	
		if(data.InStatus == true)
		{
			if(bIsOut==true)
			{
				// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','Y');",data.TicketID, data.out_time, data.Plate);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID]) VALUES (%ld, %ld, '%s', 'Y', %d);", data.TicketID, data.out_time, data.Plate, data.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
				sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID], [TagID]) VALUES (%ld, %ld, '%s', 'Y', %d, '%s');", data.TicketID, data.out_time, data.Plate, data.AreaID, data.TagID); // Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
			else
			{
				// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','Y');",data.TicketID, data.in_time, data.Plate);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID]) VALUES (%ld, %ld, '%s', 'Y', %d);", data.TicketID, data.in_time, data.Plate, data.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
				sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID], [TagID]) VALUES (%ld, %ld, '%s', 'Y', %d, '%s');", data.TicketID, data.in_time, data.Plate, data.AreaID, data.TagID); // Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
		}
		else
		{
			if(bIsOut==true)
			{
				// nick mark 20140728 Ver:000-000-GIO_V2-135101-0005-13B251 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','N');",data.TicketID, data.out_time, data.Plate);
				// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','N');",data.TicketID, data.out_time + 1, data.Plate); // nick add 20140728 Ver:000-000-GIO_V2-135101-0005-13B251 //
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID]) VALUES (%ld, %ld, '%s', 'N', %d);", data.TicketID, data.out_time + 1, data.Plate, data.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
				sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID], [TagID]) VALUES (%ld, %ld, '%s', 'N', %d, '%s');", data.TicketID, data.out_time + 1, data.Plate, data.AreaID, data.TagID); // Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
			else
			{
				// nick mark 20140728 Ver:000-000-GIO_V2-135101-0005-13B251 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','N');",data.TicketID, data.in_time, data.Plate);
				// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(qSQL,"INSERT INTO TempHourly VALUES(%ld,%ld,'%s','N');",data.TicketID, data.in_time + 1, data.Plate); // nick add 20140728 Ver:000-000-GIO_V2-135101-0005-13B251 //
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID]) VALUES (%ld, %ld, '%s', 'N', %d);", data.TicketID, data.in_time + 1, data.Plate, data.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
				sprintf(qSQL, "INSERT INTO [TempHourly] ([TicketID], [InDateTime], [PlateNumber], [InStaus], [AreaID], [TagID]) VALUES (%ld, %ld, '%s', 'N', %d, '%s');", data.TicketID, data.in_time + 1, data.Plate, data.AreaID, data.TagID); // Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
		}
		
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);	
// nick mark 20130202 //		return true;
		bRet = true; // nick add 20130202 //
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
// nick mark 20130202 //	return false;
	
	return bRet; // nick add 20130202 //
}

//Frank mark 20130114 static char *createSql = (char *)"CREATE TABLE TempDiscount(AID INTEGER,DiscountTime INTEGER,Type  INTEGER,DiscountMinute INTEGER,TicketID INTEGER,SID INTEGER,VID INTEGER,RealDiscountMinute INTEGER)";
static char *createSql = (char *)"CREATE TABLE TempDiscount(AID INTEGER,DiscountTime INTEGER,Type  INTEGER,DiscountMinute INTEGER,TicketID INTEGER,SID INTEGER,VID INTEGER,RealDiscountMinute INTEGER,Point INTEGER)";//Frank add 20130114
bool WriteTempDiscountData(HDiscountData data)
{
	//nick mark 20111219 char qSQL[256];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db  = NULL;
	bool bRet = false; // nick add 20130202 //
	
	if (data.bolIsDisct == false) return(false);		//nick add 20111219

	// =============== //
	// nick add s 20130222 //
	if(SQLiteCheckTableExist((char *)"TempDiscount") == false)
	{	//create table
		sqlite3_exec(db, createSql, 0, 0, &errMsg);
	}
	// nick add e 20130222 //
	// =============== //
	
	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		// ================ //
		// nick mark s 20130222 //
		//if(SQLiteCheckTableExist((char *)"TempDiscount") == false)
		//{	//create table
		//	sqlite3_exec(db, createSql, 0, 0, &errMsg);
		//}
		// nick mark s 20130222 //
		// ================ //

		//Frank mark s 20130114
		/*sprintf(qSQL,"INSERT INTO TempDiscount VALUES(%d,%ld,%d,%d,%ld,%d,%d,%d);",
			data.AID, data.DiscountTime, data.Type,
			data.DiscountMinute,data.TicketID,data.SID,data.VID,data.RealDiscountMinute);*/
		//Frank mark e 20130114
		//Frank add s 20130114
		sprintf(qSQL , "INSERT INTO TempDiscount VALUES(%d , %ld , %d , %d , %ld , %d , %d , %d , %ld);",
					data.AID , data.DiscountTime , data.Type , data.DiscountMinute , data.TicketID , data.SID,
					data.VID , data.RealDiscountMinute , data.Point);
		//Frnak add e 20130114
		
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);	
// nick mark 20130202 //		return true;
		bRet = true; // nick add 20130202 //
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}
	
// nick mark 20130202 //	return false;

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool DeleteTempDiscountData(HDiscountData data)
{
	bool bRet = false;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db  = NULL;
	
	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
	{	//delete data
		sprintf(qSQL,"DELETE FROM TempDiscount WHERE TicketID = %ld AND DiscountTime= %ld",data.TicketID, data.DiscountTime);
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		bRet = true;
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool GetTempDiscountData(HDiscountData* data)
{
	bool bRet = false;
	int rows, cols;
	int FieldCnt;		//nick add 20111219
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	char **result=NULL;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		//Frank mark 20130114 sprintf(qSQL,"SELECT AID,DiscountTime,Type,DiscountMinute,TicketID,SID,VID,RealDiscountMinute FROM TempDiscount");
		//Frank mark 20130114 FieldCnt = 7;		//8個Fields   //nick add 20111219
		//Frank add s 20130114
		sprintf(qSQL , "SELECT AID,DiscountTime,Type,DiscountMinute,TicketID,SID,VID,RealDiscountMinute,Point FROM TempDiscount");
		FieldCnt = 8;
		//Frank add e 20130114
		
		if(sqlite3_get_table(db , qSQL, &result , &rows, &cols, &errMsg)==SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				/*
				//nick add s 20111219
				data->AID				= atoi(result[8]);
				data->DiscountTime		= atol(result[9]);
				data->Type 				= atoi(result[10]);
				data->DiscountMinute 	= atoi(result[11]);
				data->TicketID			= atol(result[12]);
				data->SID				= atoi(result[13]);
				data->VID				= atoi(result[14]);
				data->RealDiscountMinute = atoi(result[15]);
				//nick add e 20111219
				*/
				
				FieldCnt = cols - 1;
				
				//nick add s 20111219
				data->AID				= atoi(result[FieldCnt+1]);
				data->DiscountTime		= atol(result[FieldCnt+2]);
				data->Type 				= atoi(result[FieldCnt+3]);
				data->DiscountMinute 	= atoi(result[FieldCnt+4]);
				data->TicketID			= atol(result[FieldCnt+5]);
				data->SID				= atoi(result[FieldCnt+6]);
				data->VID				= atoi(result[FieldCnt+7]);
				data->RealDiscountMinute = atoi(result[FieldCnt+8]);
				data ->Point				= atoi(result[FieldCnt+9]);					//Frank add 20130114
				//nick add e 20111219
				
				bRet = true;
				//printf("AID:%d VID:%d SID:%d \n",data->AID,data->VID,data->SID);
				//printf("TicketID:%ld Discount Minute:%d Type:%d Real:%d \n",data->TicketID,data->DiscountMinute,data->Type,data->RealDiscountMinute);
			}
			
			sqlite3_free_table(result);
			usleep(FileSystemWaitTime);
		}
		else
		{
			printf("Get Temp Discount Error:%s\n",errMsg);
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		
		sqlite3_close(db);
// nick mark 20130202 //		return bRet;
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

static char *createTempPayment = (char *)"CREATE TABLE [TempPayment] ([tick_id] INTEGER NOT NULL, [InDateTime] INTEGER NOT NULL DEFAULT 0, [PayDateTime] INTEGER NOT NULL DEFAULT 0, [PlateNumber] VARCHAR(10), [DisctType] INTEGER NOT NULL DEFAULT 0, [payment_type_id] INTEGER NOT NULL DEFAULT 1, [Deduct_Value] INTEGER DEFAULT 0, [VC_RemainingValue] INTEGER DEFAULT 0);";		//nick add 20111219

//Frank mark 20130115 bool DeleteTempPaymentData(HTicketData* t_data)		//nick add 20111219
bool DeleteTempPaymentData(HTicketData t_data)					//Frank add 20130115
{
	bool bRet = false;
	char qSQL[SQLLength+1];
	char *errMsg = NULL;
	sqlite3 *db  = NULL;
	
	memset(qSQL, 0, SQLLength+1);
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
	{	//delete data
		//Frank mark 20130115 sprintf(qSQL,"DELETE FROM [TempPayment] WHERE [tick_id] = %ld AND [PayDateTime] = %ld",t_data->TicketID, t_data->pay_time);
		sprintf(qSQL , "DELETE FROM [TempPayment] WHERE [tick_id] = %ld AND [PayDateTime] = %ld" , t_data.TicketID , t_data.pay_time);//Frank add 20130115
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		bRet = true;
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool GetTempPaymentData(HTicketData* t_data)		//nick add 20111219
{
	bool bRet = false;
	int rows, cols;
	int FieldCnt;
	char qSQL[SQLLength+1];
	char *errMsg = NULL;
	char **result=NULL;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(qSQL,"SELECT [tick_id], [InDateTime], [PayDateTime], [PlateNumber], [DisctType], [payment_type_id], [Deduct_Value], [VC_RemainingValue] FROM [TempPayment];");
		sprintf(qSQL,"SELECT [tick_id], [InDateTime], [PayDateTime], [PlateNumber], [DisctType], [payment_type_id], [Deduct_Value], [VC_RemainingValue], [TagID] FROM [TempPayment];");	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		FieldCnt = 7;		//8個Fields
		
		if(sqlite3_get_table(db , qSQL, &result , &rows, &cols, &errMsg)==SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				FieldCnt = cols-1;
				
				t_data->TicketID			= atol(result[FieldCnt+1]);
				t_data->in_time			= atol(result[FieldCnt+2]);
				t_data->pay_time			= atol(result[FieldCnt+3]);
				sprintf(t_data->Plate, "%s", result[FieldCnt+4]);
				
				if (strlen(t_data->Plate) == 0)
					sprintf(t_data->Plate, "********");
				
				t_data->Disct_Type		= atoi(result[FieldCnt+5]);
				t_data->payment_subject = atoi(result[FieldCnt+6]);
				t_data->Deduct_Value		= atoi(result[FieldCnt+7]);
				t_data->Remaining_Value	= atoi(result[FieldCnt+8]);
				sprintf(t_data->TagID, "%s", result[FieldCnt+9]);		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

				bRet = true;
			}
			
			sqlite3_free_table(result);
			usleep(FileSystemWaitTime);
		}
		else
		{
			printf("Get Temp Payment Error:%s\n",errMsg);
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		
		sqlite3_close(db);
// nick mark 20130202 //		return bRet;
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

bool WriteTempPaymentData(HTicketData t_data)		//nick add 20111219
{
	char qSQL[SQLLength+1];
	char *errMsg = NULL;
	sqlite3 *db  = NULL;
	bool bRet = false;
	
	memset(qSQL, 0, SQLLength+1);

	// =============== //
	// nick add s 20130222 //
	if(SQLiteCheckTableExist((char *)"TempPayment") == false)
	{	//create table
		sqlite3_exec(db, createTempPayment, 0, 0, &errMsg);
	}
	// nick add e 20130222 //
	// =============== //
	
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	sprintf(qSQL, "Write DB Payment: [%ld] [%ld]", t_data.TicketID, t_data.pay_time);
	ShowMessage(qSQL, 0);
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		// ================ //
		// nick mark s 20130222 //
		//if(SQLiteCheckTableExist((char *)"TempPayment") == false)
		//{	//create table
		//	sqlite3_exec(db, createTempPayment, 0, 0, &errMsg);
		//}
		// nick mark e 20130222 //
		// ================ //

		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		//sprintf(qSQL, "INSERT INTO [TempPayment] VALUES (%ld,%ld,%ld,'%s',%d,%d,%ld,%ld);",
		//	t_data.TicketID, t_data.in_time, t_data.pay_time,
		//	t_data.Plate, t_data.Disct_Type, t_data.payment_subject, t_data.Deduct_Value, t_data.Remaining_Value);
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

		sprintf(qSQL, "INSERT INTO [TempPayment] VALUES (%ld,%ld,%ld,'%s',%d,%d,%ld,%ld,'%s');",
			t_data.TicketID, t_data.in_time, t_data.pay_time,
			t_data.Plate, t_data.Disct_Type, t_data.payment_subject, t_data.Deduct_Value, t_data.Remaining_Value,t_data.TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(qSQL, 0);
		ShowMessage(qSQL, 3); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		bRet = true;
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}


void GetStatus()
{
	FILE* fh = NULL;
	bool  bReadOK = false;
	short retry = 0;
	size_t Fsize;
	
	do
	{
		fh = fopen("/tmp/Status","rb");
		
		if(fh == NULL)
		{
			printf("Read status file fail.\n");
			usleep(50000L);
			if(retry++ >3)
			{
				break;
			}
			continue;
		}
		
		Fsize = fread(&G_ParkingStatus,sizeof(struct strParkingStatus), 1, fh);
		fclose(fh);
		bReadOK = true;
	}while(bReadOK==false);
}

void StoreStatus()
{
	FILE* fh = NULL;
	bool  bReadOK = false;
	short retry=0;
	
	do
	{
		fh = fopen("/tmp/Status","wb");
		
		if(fh == NULL)
		{
			printf("write status file fail.\n");
			usleep(50000L);
			
			if(retry++ > 3)
			{
				break;
			}
			
			continue;
		}
		
		fwrite(&G_ParkingStatus.status,sizeof(unsigned int),1,fh);
		fclose(fh);
		bReadOK = true;
	}while(bReadOK==false);
	
	//test
	//G_ParkingStatus.status = 0xaa;
	//G_ParkingStatus.ParkingOccupied = 200;
}

void WriteLog(enum LOG_LEVEL llv,char logMessage[])
{
	char space[80],filename[80];	
	short level,i;
	
	time_t now;
	struct tm *tm_ptr = NULL;
	now = time((time_t *)0);	
	tm_ptr = localtime(&now);
	sprintf(filename,"./log/C%04d%02d%02d.log",tm_ptr->tm_year + 1900,tm_ptr->tm_mon + 1,tm_ptr->tm_mday);
	
	memset(space,'\0',sizeof(space));
	char* logLV = getenv("LOG_LEVEL");
	level = atoi(logLV);
	
	if(level == 0)
	{
		return;
	}
	
	if(llv < level)
	{
		return;
	}
	
	//Write to Message Log
	FILE* fh=NULL;
	
	for(i=0;i<3;i++)
	{
		fh = fopen("L.log","a+");
		
		if(fh == NULL)
		{
			usleep(50000L);
			continue;
		}
		
		fprintf(fh,"%04d-%02d-%02d %02d:%02d:%02d  [%s]",tm_ptr->tm_year + 1900,tm_ptr->tm_mon + 1,tm_ptr->tm_mday,
			tm_ptr->tm_hour + 1900,tm_ptr->tm_min + 1,tm_ptr->tm_sec,logMessage);
		
		break;
	}
	
	if( i==3)
		printf("Can't write log\n");
		
	fclose(fh);
}

//nick mark 20110209 char CheckSeasonIsValid(unsigned long ticketID,char* areaCode,char* TagID)
// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //char CheckSeasonIsValid(TicketData Ticket, char* areaCode)
char CheckSeasonIsValid(TicketData* Ticket, char* areaCode) // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
{	//return 0:無資料 -1:未開始 -2:己過期 -3:未在停車時段內 -4:無月票種類設定資料 -5:月票版本錯誤 -6:已出場 1:可使用,管制一進一出, 2: 可使用,不管一進一出
	char bRet = 0;  //無資料
	int rows, cols;
//nick mark 20110214	char qSQL[128];
//nick mark 20111219	char qSQL[1024];	//nick add 20110214
	char qSQL[SQLLength+1];	//nick add 20111219
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	time_t now;
	long startT=0,endT=0;
//	struct tm* nowTm_ptr=NULL;
//	struct tm tm_Start,tm_End;
//	time_t t_start,t_end;
//	int year,month,day;
	short len;
	int AccessControl=1;
	int SeasonType=0;	//nick add 20110209
	// nick mark 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //char buff[128];		//nick add 20110209
	char buff[512]; // nick add 20140417 Ver:000-000-GIO_V2-135101-0003-13B251 //
	struct tm *StartTime = NULL, *EndTime = NULL;					//Frank add 20120206
	struct tm *IDCardEntryTime = NULL;
	//Frank mark 20120907 long SeasonVersion;		// 20120206 Frank add
	unsigned long SeasonVersion;		// 20120907 Frank add
	IDCardStatus TicketStatus; // nick add 20150429 Ver:000-000-GIO_V2-13B251-0004-13C241 //

	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	now = time((time_t *)0);
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		//errMsg = (char*)malloc(1024 * sizeof(char));
		//memset(errMsg,'\0',1024);
		if(G_ParkingConfig.SeasonReadTag==1)
		{
//nick mark 20110209			sprintf(qSQL,"SELECT Tag,StartTime,EndTime,area_code,AccessControl FROM Season WHERE Tag=%s;",TagID);
			// nick mark 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //sprintf(qSQL,"SELECT Tag,StartTime,EndTime,area_code,AccessControl,Type FROM Season WHERE Tag=%s;",Ticket.TagID);	//nick add 20110209
			sprintf(qSQL,"SELECT TicketID,StartTime,EndTime,area_code,AccessControl,Type,Version,Tag FROM Season WHERE Tag='%s' or TicketID=%ld;", Ticket->TagID, Ticket->ticketID); // nick add 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		}
		else
		{
//nick mark 20110209			sprintf(qSQL,"SELECT TicketID,StartTime,EndTime,area_code,AccessControl FROM Season WHERE TicketID=%ld;",ticketID);
// 20120206 Frank mark			sprintf(qSQL,"SELECT TicketID,StartTime,EndTime,area_code,AccessControl,Type FROM Season WHERE TicketID=%ld;",Ticket.ticketID);	//nick add 20110209
			// nick mark 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //sprintf(qSQL,"SELECT TicketID,StartTime,EndTime,area_code,AccessControl,Type,Version FROM Season WHERE TicketID=%ld;",Ticket.ticketID);	// 20120206 Frank add
			sprintf(qSQL,"SELECT TicketID,StartTime,EndTime,area_code,AccessControl,Type,Version,Tag FROM Season WHERE TicketID=%ld;",Ticket->ticketID); // nick add 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		}
		
		//printf("SQL:%s \n",qSQL);
		
		if(sqlite3_get_table(db , qSQL, &result, &rows, &cols, &errMsg) == SQLITE_OK)
		{
			if(rows > 0)
			{	//Get first record data				
				//printf("Start:[%s] end:[%s]\n",result[6],result[7]);
				//printf("ver:[%s] \n",result[8]);
				//printf("tag:[%s] \n",result[9]);
				
				/*
				startT = atol(result[6]);
				endT   = atol(result[7]);
				len    = strlen(result[8]);
				strncpy(areaCode,result[8],len);
				AccessControl = atoi(result[9]);
				*/
				
				/*
				sprintf(buff, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s", result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7]);
				ShowMessage(buff);
				sprintf(buff, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s", result[cols], result[cols+1], result[cols+2], result[cols+3], result[cols+4], result[cols+5], result[cols+6], result[cols+7]);
				ShowMessage(buff);
				*/
				
				startT 			= atol(result[cols+1]);			//nick add 20110208
				endT   			= atol(result[cols+2]);			//nick add 20110208
				len    			= strlen(result[cols+3]);		//nick add 20110208
				strncpy(areaCode,result[cols+3],len);			//nick add 20110208
				AccessControl 	= atoi(result[cols+4]);			//nick add 20110208
				SeasonType 		= atoi(result[cols+5]);			//nick add 20110209
				SeasonVersion	= atoi(result[cols+6]);			// 20120206 Frank add
				
				// ========================================================== //
				// nick mark s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				//sprintf(buff, "Season Card Info STime:%ld ETime:%ld AreaCode:%s AccessCtrl:%d Type:%d", startT, endT, areaCode, AccessControl, SeasonType);
				//ShowMessage(buff);
				//
				////Frank add s 20120206
				//sprintf(buff, "Season Card Info Version:%ld , Ticket Info Version:%ld", SeasonVersion, Ticket.seasonVersion);
				//ShowMessage(buff);
				// nick mark e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				// ========================================================== //
				
				// ========================================================= //
				// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				// nick mark 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //sprintf(buff, "Season Card Info STime:[%ld] ETime:[%ld] AreaCode:[%s] AccessCtrl:[%d] Type:[%d] SeasonVersion:[%ld] TicketVersion:[%ld]", startT, endT, areaCode, AccessControl, SeasonType, SeasonVersion, Ticket.seasonVersion);
				// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //sprintf(buff, "NowTick:%ld, Season Card Info STime:[%ld] ETime:[%ld] AreaCode:[%s] AccessCtrl:[%d] Type:[%d] SeasonVersion:[%ld] TicketVersion:[%ld]", now, startT, endT, areaCode, AccessControl, SeasonType, SeasonVersion, Ticket.seasonVersion); // nick add 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //
				sprintf(buff, "NowTick:%ld, Season Card Info STime:[%ld] ETime:[%ld] AreaCode:[%s] AccessCtrl:[%d] Type:[%d] SeasonVersion:[%ld] TicketVersion:[%ld]", now, startT, endT, areaCode, AccessControl, SeasonType, SeasonVersion, Ticket->seasonVersion); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				ShowMessage(buff);
				// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				// ========================================================= //
				
				// ==================== //
				// nick mark s 20130311 //
				//StartTime=localtime((time_t*)&startT);
				//
				//sprintf(buff,"Season Card Info STime:%04d/%02d/%02d",
				//		StartTime->tm_year+1900, StartTime->tm_mon+1, StartTime->tm_mday);
				//ShowMessage(buff);
				//
				//EndTime=localtime((time_t*)&endT);
				//
				//sprintf(buff,"Season Card Info ETime:%04d/%02d/%02d",
				//		EndTime->tm_year+1900, EndTime->tm_mon+1, EndTime->tm_mday);
				//ShowMessage(buff);
				// nick mark e 20130311 //
				// ==================== //
				
				//Frank add e 20120206
				
				// ========================================================== //
				// nick mark s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				//// nick add s 20130311 //
				//StartTime = gmtime((time_t *)&startT);
				//sprintf(buff, "Season Card Info Start Time:%04d/%02d/%02d %02d:%02d:%02d",
				//	StartTime->tm_year + 1900, StartTime->tm_mon + 1, StartTime->tm_mday, StartTime->tm_hour, StartTime->tm_min, StartTime->tm_sec);
				//ShowMessage(buff);
				//
				//EndTime = gmtime((time_t *)&endT);
				//sprintf(buff, "Season Card Info End Time:%04d/%02d/%02d %02d:%02d:%02d",
				//	EndTime->tm_year + 1900, EndTime->tm_mon + 1, EndTime->tm_mday, EndTime->tm_hour, EndTime->tm_min, EndTime->tm_sec);
				//ShowMessage(buff);
				//// nick add e 20130311 //
				// nick mark e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				// ========================================================== //
				
				// ========================================================= //
				// nick add s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				StartTime = gmtime((time_t *)&startT);

				sprintf(buff, "Season Card Info Start Time:[%04d/%02d/%02d %02d:%02d:%02d], ",
					StartTime->tm_year + 1900, StartTime->tm_mon + 1, StartTime->tm_mday, StartTime->tm_hour, StartTime->tm_min, StartTime->tm_sec);
				
				EndTime = gmtime((time_t *)&endT);
				
				sprintf(buff + strlen(buff), "End Time:[%04d/%02d/%02d %02d:%02d:%02d]",
					EndTime->tm_year + 1900, EndTime->tm_mon + 1, EndTime->tm_mday, EndTime->tm_hour, EndTime->tm_min, EndTime->tm_sec);
				ShowMessage(buff);
				// nick add e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
				// ========================================================= //
				
				//Frank add s 20120907
				// nick mark 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(SeasonVersion != Ticket.seasonVersion)
				if(SeasonVersion != Ticket->seasonVersion && G_ParkingConfig.SeasonReadTag != 1) // nick add 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				{
					ShowMessage((char *)"Season Ticket Version error!");
					bRet = -5;
					goto EXITFUNC;
				}
				//Frank add e 20120907
				
				/*
				//nick mark s 20110208
				if(startT <= now && endT >= now)
				{
					if(AccessControl ==1)
					{
						bRet = 1;
					}
					else
					{
						bRet = 2;
					}
				}
				else if(startT > now)
				{
					bRet = -1; //未開始
					printf("Start:%ld \n",startT);
				}
				else if(endT < now)
				{
					bRet = -2; //己過期
					printf("End:%ld \n",endT);
				}
				//nick mark e 20110208
				*/
				
				//nick add s 20110208
				if(AccessControl == 1)
				{
					bRet = 1;
					
					// ========================================================= //
					// nick add s 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					if (G_ParkingConfig.SeasonReadTag == 1 && G_ParkingConfig.UpLayerID == 0)
					{
						// 傳送通訊給Server問狀態 //
						if (QuestIDCardStatus(*Ticket, &TicketStatus) == true)
						{
							printf("Ticket Status:[%ld], [%d], [%ld]\n", TicketStatus.TicketID, TicketStatus.Status, TicketStatus.Last_Entry_Exit_Time);
							Ticket->status = TicketStatus.Status;

							if (IsINMachine == false)
							{
								IDCardEntryTime = localtime(&TicketStatus.Last_Entry_Exit_Time);
								
								Ticket->in_year = IDCardEntryTime->tm_year + 1900;
								Ticket->in_month = IDCardEntryTime->tm_mon + 1;
								Ticket->in_day = IDCardEntryTime->tm_mday;
								Ticket->in_hour = IDCardEntryTime->tm_hour;
								Ticket->in_min = IDCardEntryTime->tm_min;
							}
						}
						else
							Ticket->status = 4; // 問不到狀態等於第一次進出場
						//
					}
					// nick add e 20150427 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					// ========================================================= //
				}
				else
					bRet = 2;
					
				//判斷是否已經繳費
				// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //if (CheckSeasonTicketPaid(Ticket))     //nick add 20120106
				if (CheckSeasonTicketPaid(*Ticket)) // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					goto EXITFUNC;                      //nick add 20120106
				
				// ========================================================== //
				// nick mark s 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
				////未到起始時間可入場但須補費
				//if(Ticket.status == 1 || Ticket.status == 4 || Ticket.ticketID % 10 == 5)
				//{
				//	if((startT > now) || (endT < now))
				//	{
				//		bRet = -2; //己過期
				//		printf("End:%ld  Now:%ld \n", endT, now);
				//		goto EXITFUNC;
				//	}
				//}
				// nick mark e 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
				// ========================================================== //
				//nick add e 20110208
				
				//nick add s 20110209
				if(IsINMachine == false)
				{ //為出口設備時，判斷時段月票
					// ========================================================= //
					// nick add s 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
					//未到起始時間可入場但須補費
					if((startT > now) || (endT < now))
					{
						bRet = -2; //己過期
						// nick mark 20140624 Ver:000-000-GIO_V2-135101-0005-13B251 //printf("Start:%ld  End:%ld  Now:%ld \n", startT, endT, now);
						goto EXITFUNC;
					}
					// nick add e 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
					// ========================================================= //
					
					// ========================================================= //
					// nick add s 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					if (Ticket->status == 3 && bRet == 1)
					{
						bRet = -6; // 已出場
						goto EXITFUNC;
					}
					// nick add e 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					// ========================================================= //
					
					sprintf(qSQL, "SELECT [Type], Mon_StartTime, Mon_EndTime, Tue_StartTime, Tue_EndTime, "
								  "Wed_StartTime, Wed_EndTime, Thu_StartTime, Thu_EndTime, "
								  "Fri_StartTime, Fri_EndTime, Sat_StartTime, Sat_EndTime, "
								  "Sun_StartTime, Sun_EndTime, ChargeDate FROM [SeasonConfig] WHERE Type = %d", SeasonType);
					//ShowMessage(qSQL);
					
					if(sqlite3_get_table(db , qSQL, &result, &rows, &cols, &errMsg) == SQLITE_OK)
					{
						if(rows > 0)
						{	//判斷是否在可停車時段內
							time_t entry;
							time_t VstartT;
							time_t VendT = 0;
							struct tm EntryTime;
							struct tm *GetTime;
							struct tm TmpTime;
							int CalcCnt=0;
							int CmpRst;
							const char *wday[7] = {"Sun_", "Mon_", "Tue_", "Wed_", "Thu_", "Fri_", "Sat_"};
							
							// ========================================================== //
							// nick mark s 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
							//EntryTime.tm_year = Ticket.in_year - 1900;
							//EntryTime.tm_mon = Ticket.in_month - 1;
							//EntryTime.tm_mday = Ticket.in_day;
							//EntryTime.tm_hour = Ticket.in_hour;
							//EntryTime.tm_min = Ticket.in_min;
							// nick mark e 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
							// ========================================================== //
							// ========================================================= //
							// nick add s 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
							EntryTime.tm_year = Ticket->in_year - 1900;
							EntryTime.tm_mon = Ticket->in_month - 1;
							EntryTime.tm_mday = Ticket->in_day;
							EntryTime.tm_hour = Ticket->in_hour;
							EntryTime.tm_min = Ticket->in_min;
							// nick add e 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
							// ========================================================= //
							EntryTime.tm_sec = 59;
							entry = mktime(&EntryTime);
							
							// ========================================================= //
							// nick add s 20131127 Ver:000-000-GIO_V2-133181-0104-135101 //
							// 檢查是否在前一天的時段內 //
							time_t tmp_entry;
							// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //EntryTime.tm_mday = Ticket.in_day - 1;
							EntryTime.tm_mday = Ticket->in_day - 1; // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
							tmp_entry = mktime(&EntryTime);
							
							GetTime = localtime(&tmp_entry);
							memcpy(&TmpTime, GetTime, sizeof(TmpTime));
							sprintf(buff, "%d", TmpTime.tm_wday);
							printf("Check yesterday season card zone. tm_wday:%s\n", buff);
							
							for(int colsCnt = 0; colsCnt < cols; colsCnt++)
							{
								if (strncmp(result[colsCnt], wday[TmpTime.tm_wday], strlen(wday[TmpTime.tm_wday])) == 0)
								{
									CmpRst = strcmp(result[colsCnt+cols], result[colsCnt+cols+1]);
									
									TmpTime.tm_hour = atoi(result[colsCnt + cols]);
									TmpTime.tm_min = atoi(result[colsCnt + cols] + 3);
									VstartT = mktime(&TmpTime);
									
									if (CmpRst < 0)
									{
										TmpTime.tm_hour = atoi(result[colsCnt + cols + 1]);
										TmpTime.tm_min = atoi(result[colsCnt + cols + 1] + 3);
										VendT = mktime(&TmpTime);
									}
									else if (CmpRst > 0)
									{
										TmpTime.tm_mday++;
										TmpTime.tm_hour = atoi(result[colsCnt + cols + 1]);
										TmpTime.tm_min = atoi(result[colsCnt + cols + 1] + 3);
										VendT = mktime(&TmpTime);
									}
									else
									{
										VendT = VstartT + 86400;
									}
									
									printf("Compare Zone Result:%d, entry:%ld, StartTime:%ld, EndTime:%ld\n", CmpRst, entry, VstartT, VendT);
									
									if (entry >= VstartT && entry <= VendT)
										entry = VendT + 1;
									
									break;
								}
							}
							//
							// nick add e 20131127 Ver:000-000-GIO_V2-133181-0104-135101 //
							// ========================================================= //
							
							while(entry < now)
							{
								char *found;
								
								//取得星期幾
								GetTime = localtime(&entry);
								memcpy(&TmpTime, GetTime, sizeof(TmpTime));
								sprintf(buff, "%d", TmpTime.tm_wday);
								printf("Check season card zone. tm_wday:%s\n", buff);
								
								// 檢查ChargeDate //
								if (result[2*cols-1] != NULL)
								{
									found=strstr(result[2*cols-1], buff);
									
									if (found != NULL)
									{
										bRet=-3;
										goto EXITFUNC;
									}
								}
								//
								
								for(int colsCnt = 0; colsCnt < cols; colsCnt++)
								{	// 尋找當天時段 //
									if (strncmp(result[colsCnt], wday[TmpTime.tm_wday], strlen(wday[TmpTime.tm_wday])) == 0)
									{
										CmpRst = strcmp(result[colsCnt+cols], result[colsCnt+cols+1]);
										
										TmpTime.tm_hour=atoi(result[colsCnt + cols]);
										TmpTime.tm_min=atoi(result[colsCnt + cols] + 3);
										VstartT=mktime(&TmpTime);
										
										if (CmpRst < 0)
										{	//日間月票時段
											TmpTime.tm_hour = atoi(result[colsCnt + cols + 1]);
											TmpTime.tm_min = atoi(result[colsCnt + cols + 1] + 3);
											VendT = mktime(&TmpTime);
										}
										else if(CmpRst > 0)
										{	//夜間月票時段
											TmpTime.tm_mday++;
											TmpTime.tm_hour = atoi(result[colsCnt + cols + 1]);
											TmpTime.tm_min = atoi(result[colsCnt + cols + 1] + 3);
											VendT = mktime(&TmpTime);
										}
										else
										{	//全日月票時段
											VendT = VstartT + 86400;
										}
										
										printf("Compare Zone Result:%d, entry:%ld, StartTime:%ld, EndTime:%ld\n", CmpRst, entry, VstartT, VendT);
										
										if(entry >= VstartT && entry <= VendT)
											entry = VendT + 1;
										else
										{
											bRet=-3;
											goto EXITFUNC;
										}
										
										break;
									}
								}
								
								if (++CalcCnt > 7) break;
								usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
							}
						}
						else
						{	//無月票基本設定資料
							bRet=-4;
						}
						
						sqlite3_free_table(result);
					}
					else
					{
						printf("ERROR:%s\n",errMsg);
						
						if(errMsg)
							sqlite3_free((void*)errMsg);
					}
				}
				//nick add e 20110209
				// ========================================================= //
				// nick add s 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
				else
				{ //為入口設備時，判斷是否超過月票截止日期
					if (endT < now)
					{
						bRet = -2; //己過期
						printf("End:%ld  Now:%ld \n",endT,now);
						goto EXITFUNC;
					}
				}
				// nick add e 20140529 Ver:000-000-GIO_V2-135101-0004-13B251 //
				// ========================================================= //
			}
			else
				sqlite3_free_table(result);
		}
		else
		{
			printf("ERROR:%s\n",errMsg);
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
		// nick mark 20130202 //return bRet;
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
	
//nick add s 20110209
EXITFUNC:

	sqlite3_free_table(result);
	sqlite3_close(db);
	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
//nick add e 20110209
}

bool CheckValueIsValid(TicketData *ticketData)
{
	bool bRet = false;
	
	if(G_ParkingConfig.FeeType==0)
	{	// 以每(or半)小時金額計
		if(G_ParkingConfig.FeeTime == 1)
		{
		}
	}
	
	return bRet;
}

bool CheckBlackList(unsigned long ticketID)
{
	bool bRet = false;
	int rows, cols;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	time_t now;
//	long startT = 0, endT = 0;
//	struct tm* nowTm_ptr=NULL;
//	struct tm tm_Start,tm_End;
//	time_t t_start,t_end;
//	int year,month,day;
//	short len;

	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	now = time((time_t *)0);
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		//errMsg = (char*)malloc(1024 * sizeof(char));
		//memset(errMsg,'\0',1024);
		sprintf(qSQL,"SELECT ticket_id FROM Black_List WHERE ticket_id=%ld;",ticketID);
		
		if(sqlite3_get_table(db , qSQL, &result, &rows, &cols, &errMsg) == SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				bRet = true;
			}
			
			sqlite3_free_table(result);
		}
		else
		{
			printf("ERROR:%s\n",errMsg);
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
// nick mark 20130202 //		return bRet;
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

void WriteOpenBarrier(OpenBrData openData)
{
	//nick mark  20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		//errMsg = (char*)malloc(1024 * sizeof(char));
		//memset(errMsg,'\0',1024);
		sprintf(qSQL,"INSERT INTO OpenBarrier(OpenTime)  VALUES(%ld);",openData.OpenTime);	
		//printf("SQL:%s \n",qSQL);
		sqlite3_exec(db, qSQL, 0, 0, &errMsg);
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
		
		if(errMsg)
			sqlite3_free((void*)errMsg);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
}

bool ReadOpenBarrier(OpenBrData *openData)
{
	int rows, cols;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	bool bRet = false;	
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	
	//nick mark 20111219 memset(qSQL,'\0',sizeof(qSQL));
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		sprintf(qSQL,"SELECT Serial,OpenTime FROM OpenBarrier ;");
		//printf("SQL:%s\n",qSQL);
		
		if(sqlite3_get_table(db , qSQL, &result , &rows, &cols, &errMsg)==SQLITE_OK)
		{
			if(rows >0)
			{	//Get first record data
				openData->sn = atol(result[2]);
				openData->OpenTime  = atol(result[3]);
				bRet = true;
			}
			
			sqlite3_free_table(result);
			sqlite3_close(db);
		}
		else
		{
			printf("ERROR:[%s]\n",errMsg);
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
			
			bRet = false;
		}

		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't Open parking.s3db\n");
		bRet = false;
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

void DelOpenBarrier(OpenBrData openData)
{
//	bool bRet = false;
	//nick mark 20111219 char qSQL[128];
	char qSQL[SQLLength+1];		//nick add 20111219
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	
	memset(qSQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		sprintf(qSQL,"DELETE  FROM OpenBarrier WHERE  Serial = %ld;",openData.sn);
		
		if(sqlite3_exec(db, qSQL, 0, 0, &errMsg) != SQLITE_OK)
		{
			printf("BR open delete Error:[%s] \n",errMsg);
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		
		sqlite3_close(db);
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
}

bool RunSQL(char SQL[])
{
	bool bRet = false;
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	char buf[4096];
	
	memset(buf, 0, 4096);					//Frank add 20120206
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		memset(buf, '\0', sizeof(buf));
		// nick mark 20130202 //sprintf(buf,"Run SQL:[%s]",SQL);
		// nick mark 20130202 //ShowMessage(buf,0);
		
		if(sqlite3_exec(db, SQL, 0, 0, &errMsg) != SQLITE_OK)
		{
			// nick mark 20130202 //printf("Run SQL Error:[%s]  SQL:[%s]\n",errMsg,SQL);
          //  bRet = true;					//Frank mark 20111020
			sprintf(buf,"Run SQL Error:[%s] SQL:[%s]", errMsg, SQL); // nick add 20130202 //
			ShowMessage(buf,0); // nick add 20130202 //
		 				
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		else
		{
			bRet = true;
		}
		
		sqlite3_close(db);		
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet;
}

void SQLiteAddColumn(char* tblName,char* colName,char* Type)
{
	//nick mark 20111219 char SQL[256];
	char SQL[SQLLength+1];		//nick add 20111219
	
	//nick mark 20111219 memset(SQL,'\0',sizeof(SQL));
	memset(SQL, 0, SQLLength+1);		//nick add 20111219
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	bool bRet = false;
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	
	if (sqlite3_open_v2("./data/parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{		
		sprintf(SQL,"ALTER TABLE %s ADD %s %s DEFAULT '1'",tblName,colName,Type);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(SQL,0);
		
		if(sqlite3_exec(db, SQL, 0, 0, &errMsg) != SQLITE_OK)
		{
			printf("Run SQL Error:[%s]  SQL:[%s]\n",errMsg,SQL);
			bRet = true;
			
			if(errMsg)
				sqlite3_free((void*)errMsg);
		}
		else
		{
			bRet = true;
		}
		
		sqlite3_close(db);		
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
}

void PopCommandDataQue(CMD_msg* CMD)
{
}

void PushCommandDataQue(CMD_msg CMD)
{
}

//Frank add s 20120206
bool CheckSeasonTicketPaid(TicketData ticketData)
{
	time_t now;
	time_t EntryTime;          //nick add 20120106
	struct tm tm_PayTime;      //nick add 20120106
	char buf[256];
	
	if (ticketData.status == 4)      //第一次進出
		return true;
   
	if (ticketData.status == 3) // 已出場票卡 // // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		return false; // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	
	memset(buf,'\0',sizeof(buf));
	// Get Time at the moment
	now = time((time_t *)0);
	
	//nick add s 20120106
	tm_PayTime.tm_year = ticketData.in_year - 1900;
	tm_PayTime.tm_mon = ticketData.in_month - 1;
	tm_PayTime.tm_mday = ticketData.in_day;
	tm_PayTime.tm_hour = ticketData.in_hour;
	tm_PayTime.tm_min = ticketData.in_min;
	tm_PayTime.tm_sec = 59;
	EntryTime = mktime(&tm_PayTime);
  	
	if (ticketData.status != 1)
	{
		if(((unsigned int)EntryTime + (G_ParkingConfig.PaidTime * 60)) > (unsigned int)now)
		{
			sprintf(buf,"Season Paid:%4d%02d%02d %02d:%02d ", ticketData.in_year,ticketData.in_month,ticketData.in_day,
			ticketData.in_hour,ticketData.in_min);
			ShowMessage(buf);
			return true;
		}
		
		sprintf(buf,"Season Paid:%4d%02d%02d %02d:%02d  G_ParkingConfig.PaidTime:%d", ticketData.in_year,ticketData.in_month,ticketData.in_day,
		ticketData.in_hour,ticketData.in_min, G_ParkingConfig.PaidTime);
	}
	else
	{
		sprintf(buf,"Season In Status ### EntryTime:%4d%02d%02d %02d:%02d  G_ParkingConfig.FreeTime:%d", ticketData.in_year,ticketData.in_month,ticketData.in_day,
		ticketData.in_hour,ticketData.in_min, G_ParkingConfig.FreeTime);
	}
   	
	ShowMessage(buf);
	return false;
	//nick add e 20120106
}
//Frank add e 20120206

//Frank add s 20120523
static char *createTempValue = (char *)"CREATE TABLE TempValue (TicketID INTEGER KEY NOT NULL ,OutDateData DATETIME,InDateData DATETIME,Value INTEGER NOT NULL  DEFAULT (0) ,ValuePay INTEGER NOT NULL  DEFAULT (0) )";

bool WriteTempValueData(TicketData *ticketData , int ValuePay)
{
	char qSQL[SQLLength + 1];
	char *errMsg = NULL;
	sqlite3 *db = NULL;
	time_t InDateData , OutDateData;
	struct tm tm_Time;
	bool bRet = false; // nick add 20130202 //
	
	memset(qSQL , 0 , SQLLength + 1);

	// =============== //
	// nick add s 20130222 //
	if(SQLiteCheckTableExist((char *)"TempValue") == false)
	{
		sqlite3_exec(db , createTempValue , 0 , 0 , &errMsg);
	}
	// nick add e 20130222 //
	// =============== //
	
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	tm_Time.tm_year	= ticketData->in_year - 1900;
	tm_Time.tm_mon	= ticketData->in_month - 1;
	tm_Time.tm_mday	= ticketData->in_day;
	tm_Time.tm_hour	= ticketData->in_hour;
	tm_Time.tm_min	= ticketData->in_min;
	tm_Time.tm_sec	= ticketData->in_sec;					//Frank add 20120725
	InDateData = mktime(&tm_Time);
	
	tm_Time.tm_year	= ticketData->out_year - 1900;
	tm_Time.tm_mon	= ticketData->out_month - 1;
	tm_Time.tm_mday	= ticketData->out_day;
	tm_Time.tm_hour	= ticketData->out_hour;
	tm_Time.tm_min	= ticketData->out_min;
	tm_Time.tm_sec	= ticketData->out_sec;					//Frank add 20120725
	OutDateData = mktime(&tm_Time);
	
	if (sqlite3_open_v2(".//data//parking.s3db", &db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
	{
		// ================ //
		// nick mark s 20130222 //
		//if(SQLiteCheckTableExist((char *)"TempValue") == false)
		//{
		//	sqlite3_exec(db , createTempValue , 0 , 0 , &errMsg);
		//}
		// nick mark e 20130222 //
		// ================ //
		
		sprintf(qSQL , "INSERT INTO TempValue VALUES(%ld , %ld , %ld , %ld , %d);" , 
			ticketData->ticketID , OutDateData , InDateData , ticketData->value , ValuePay);
			
		sqlite3_exec(db , qSQL , 0 , 0 , &errMsg);
		sqlite3_close(db);
// nick mark 20130202 //		return true;
		bRet = true; // nick add 20130202 //
		usleep(FileSystemWaitTime);
	}
	else
	{
		printf("Can't open //data//parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	return bRet; // nick add 20130202 //
// nick mark 20130202 //	return false;
}
//Frank add e 20120523
