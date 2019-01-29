/************************************************************
File:	main.c
Use:	main Program
*************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <sqlite3.h>
#include <typeinfo>
#include <netinet/in.h>
#include <sys/time.h>

#include "CommonDef.h"
#include "Network.h"
#include "ServerTalk.h"
#include "rs232.h"
#include "MF700.h"
#include "Dio.h"
#include "IPMS_Driver/ra8822.h"
#include "IPMS_Driver/LED888.h"
#include "Datafile.h"
#include "Display.h"
#include "Voice.h"
#include "Reader.h"
#include "test.h"
#include "traceLog.h"
#include "PassingIniFile.h"					//Frank add 20110817

#define VERSION					"0.8"
#define TIMEOUT_TAKETICKET		10
//-----------------------------------------------------------
// Frank add s 20110817
//-----------------------------------------------------------
#define UPS_DELAY_MAXTIME				  65535					//Frank modify 20111020
#define UPS_DELAY_MINTIME 				  10						//Frank modify 20111020
#define UPS_MS_TO_SEC					  1000
//Frank mark 20111020#define UPS_SEC_TO_MIN  				  UPS_MS_TO_SEC * 60
#define UPS_SHUTDOWN_PULSE_TIME_MAX   UPS_MS_TO_SEC * 100
#define UPS_SHUTDOWN_PULSE_TIME_MIN   UPS_MS_TO_SEC * 0.3
//-----------------------------------------------------------
// Frank add e 20110817
//-----------------------------------------------------------

// define Globe variable ___________________________________
bool G_bSystemFault 					= false;
bool G_ButtonPress					= false;
bool G_bolWaitLoop2					= false;		//nick add 20101221
bool G_bolOverLoop2					= false;		//nick add 20101221
bool volatile G_ParkingFull		= false;
bool volatile G_RFin					= false;
bool volatile G_POSIN					= false; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //

bool volatile G_bThreadRun			= true;
bool G_bErrHold						= false;

bool volatile G_bStopService		= false;
bool volatile b_GReset				= false;
//nick mark 20120223 int  iRunTime = 0;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long iRunTime							= 0;     //nick add 20120223
unsigned long iRunTime = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
int G_iFullLevel						= 0;
//nick mark 20130121 //bool volatile G_bMaintain		= false;	//nick add 20110216
bool bHasNewINI						= false;	//nick add 20110217

parkingStatus G_ParkingStatus;
queue <serverCommand> G_SvrCmdQue;
queue <serverCommand> G_NewClientCmdQue;
queue <serverAck> G_SvrAckQue;
queue <IDCardStatus> G_SvrIDCardStatusQue; // nick add 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
pthread_t SendPlateData; // nick add 20130219 //
pthread_t tdPreissue;
pthread_t td_SendVisSoftTrigger; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
pthread_mutex_t Data_mutex;
pthread_mutex_t Log_mutex; // nick add 20130318 //
pthread_mutex_t NewTerminal_mutex;

TicketData LastTicketData; // nick add 20130222 //


//-----------------------------------------------------------
// Frank add s 20110817
//-----------------------------------------------------------
int					ShutdownUPSDelayTime;
int					UPSShutdownPulseTime;
unsigned int		ShutdownUPSDefDelayTime;					//Frank add 20111020
bool					PowerFailState;
bool					PowerFailSignalIn;
bool					UPSChange;
bool					UPSstartstate; 
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long		PowerFailStartTime;
double		PowerFailStartTime; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
st_IOSycn G_Loop1IOSycn;
st_IOSycn G_CarFullStatusSycn;

//-----------------------------------------------------------
// Frank add e 20110817
//-----------------------------------------------------------

// Alarm 開關
bool G_bTicketLow						= false;
bool G_bTicketZero					= false;
bool G_bRetriveFull					= false;

// define Custom data type

// define Thread Function __________________________________
void *UDPThread(void *arg);
void *UpdateThread(void *arg);		//nick add 20110215
void *GetDI(void *arg); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
void *CheckDIThread(void *arg); // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //

int ThreadEntryPoint(void  *data);
void *ThreadSyncTriggerToNewTerimal(void *arg);	// Tony add 20170718
void *ThreadPollingTriggerFromNewTerimal(void *arg);


// define local function ____________________________________
int MainProcessIn(void);
void MainProcessOut(void);
void IdleProcess(void);
enum TicketType IdleProcessForNewTerminel(ThirdPartyTicketData* thirdPartyTicketData);

void ReadConfig(void);
void ProcessIOState(void);
void ProcessManulIO(void);
void LoadLanguage(int LangID, char* Langauge);		//nick add 20110705
void UpdateDataToServer(void);
void WriteHourlyInData(TicketData ticketData,bool bIn);
void WriteHourlyOutData(TicketData ticketData, bool bIn);
void *SendDataToPlateSVR(void *HourlyticketData);
void CheckParkingINI(void);					//nick add 20110216
void UPSControl(void); // Frank add 20110817
void UPSGetsetting(void); // Frank add 20110817
void Waitmsec(unsigned int msec);					//Frank add 20120907

enum TicketType DetectUserCardType(TicketData* ticketData);
enum TicketType ReadOutTicketData(TicketData* ticketData);

// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) bool ProcessHourlyIn(TicketData ticketData);
bool ProcessHourlyIn(TicketData* ticketData);// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

bool ProcessSeasonCardIn(TicketData ticketData);
bool ProcessEasyCardIn(TicketData ticketData);
bool ProcessValueCardIn(TicketData ticketData);
bool WaitPlateAllow(TicketData* ticketData,int WaitTime=0);
bool WaitCarTraverse(int sec = 30); // nick edit 20131218 from 0 to 30 Ver:000-000-GIO_V2-133181-0107-135101 //
bool ProcessHourlyOut(TicketData ticketData);
bool ProcessSeasonCardOut(TicketData ticketData);
bool ProcessEasyCardOut(TicketData ticketData);
bool ProcessValueCardOut(TicketData *ticketData);
bool WaitTakeTicket(enum TicketType ticketType,int sec=0);
bool CheckUpdateSeasonConfig(void);		//nick add 20130121 //
bool SystemReboot();          // 20120314 Tony add
bool AlreadyRun(void);					//Frank add 20120206
void MakeThirdPartyTicketData(TicketData* ticketData,ThirdPartyTicketData thirdPartyTicketData); // Tony add 20170307
void MakeINHTicketData(HTicketData* hTicketData, TicketData ticketData);
void MakeOUTHTicketData(HTicketData* hTicketData, TicketData ticketData);

bool CommDataGetParm(char* str ,short index,char commData[]);
bool CheckHourlyTicketPaid(TicketData ticketData);

void SendLoop1Status(bool loop1ON);
void SendCarFullStatus();
void SendCarEnterStatus(int carInStatus);	// Tony add 20170710
void SendCarEnterStart(TicketType ticketType);
bool SyncLoop1Status(st_IOSycn* buttonIOSycn);
bool SyncLoop2Status(st_IOSycn* buttonIOSycn);
bool SyncButtonStatus(st_IOSycn* buttonIOSycn);
bool SyncCarFullStatus(st_IOSycn* carFullStatusSycn);
bool SyncRFInStatus(st_IOSycn* rFInIOSycn);
void SyncSystemFaultToNewTerminal(void);
bool G_Passage = false;
bool G_NoTicketSystem = false;

static char LastTagId[24];
unsigned long G_MemBlocksWriteSuc[16]; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //

//_______________________________________________________________

enum TicketType DetectUserCardType(TicketData* ticketData)
{	//Get plate number and ticket Data, Easy card is check input signal
	int iRet,retRead = 0;
	int res;				// 20110629 Tony add 
	enum TicketType result = NONE_TICKET;
	char readerStatus = 0;
	char ticketID[16];
	char  buf[256];
	char  mfTag[1024];
//20110512 Tony mark 	time_t now, outDeadline;
	struct tm *tm_ptr = NULL;
	time_t now;
	// nick mark 20130219 //pthread_t SendPlateData;	// 20110629 Tony add
	unsigned long TickSerial = 0L; // nick add 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	memset(ticketID,'\0',sizeof(ticketID));
	memset(buf,'\0',sizeof(buf));
	memset(mfTag,'\0',sizeof(mfTag));
	
	//現在時間 ==> 入場時間
	now    = time((time_t *)0);
	tm_ptr = localtime(&now);
	ticketData->in_year  = tm_ptr->tm_year + 1900;
	ticketData->in_month = tm_ptr->tm_mon + 1;
	ticketData->in_day   = tm_ptr->tm_mday;
	ticketData->in_hour  = tm_ptr->tm_hour;
	ticketData->in_min   = tm_ptr->tm_min;
	ticketData->in_sec   = tm_ptr->tm_sec;	// 20110506 Tony add
	ticketData->areaId = G_ParkingConfig.AreaID; // nick add 20160720 Ver:000-000-GIO_V2-13B251-0014-13C241 //
	
	if(G_RFin == true)
	{
		ticketData->ticketID = 0; // nick add 20140418 Ver:000-000-GIO_V2-135101-0003-13B251 //
		result = EASY_CARD;
	}
	// ========================================================= //
	// nick add s 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	else if (G_POSIN == true)
	{
		ticketData->ticketID = 0;
		result = POS_IN;
	}
	// nick add e 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	// ========================================================= //
	else if(G_ButtonPress == true || G_ParkingConfig.AutoIssueTicket == true)
	{	//檢查到按鈕取票
		CalcTmpTime = GetTickCount();
		EasyCardEnable(false); // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
		printf("DetectUserCardType :: Satrt : %ld\n",CalcTmpTime);
		VoiceOn(VOICE_PROCESSING);            //票卡處理中
		AudioOn(AUDIO_PROCESSING);
		ShowLCMFile(0,0,(char *)"TicketProcess.lcm");   ////票卡處理中
			
		if(G_ParkingConfig.AutoIssueTicket == true)
		{
			ShowMessage((char *)"Auto Issue Ticket.");
		}
		else
		{
			ShowMessage((char *)"Detect Button.");
			G_ButtonPress = false;		//	讓無卡時不會一直發卡
		}
		
		if(G_iFullLevel >= 1 && G_ParkingConfig.ParkingFullCanEntry == 0)
		{
			// ========================================================== //
			// nick mark s 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
			//AudioOn(AUDIO_PFULL);
			//ShowMessage((char *)"Parking is Full!!");
			//VoiceOn(VOICE_PFULL);    /// 車位己滿
			//ShowLCMFile(0,0,(char *)"ParkingFull.lcm");
			// nick mark e 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
			// ========================================================== //
			
			// ========================================================= //
			// nick add s 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
			switch (G_iFullLevel)
			{
				case 1:
					AudioOn(AUDIO_HOURLYFULL);
					VoiceOn(VOICE_PFULL);
					ShowLCMFile(0,0,(char *)"HourlyParkingFull.lcm");
					ShowMessage((char *)"Hourly Parking is Full!!");
					break;
				case 2:
					AudioOn(AUDIO_PFULL);
					VoiceOn(VOICE_PFULL);
					ShowLCMFile(0,0,(char *)"ParkingFull.lcm");
					ShowMessage((char *)"Parking is Full!!");
					break;
				default:
					break;
			}
			// nick add e 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
			// ========================================================= //
			
			sleep(2);
			LCM_FillScreen(0x00);
			return NONE_TICKET;
		}

		if (strlen(G_ParkingConfig.NewTerminalIP) > 7 && G_ParkingConfig.NewTerminalPort > 0)	// Tony add 20170904
		// Tony mark 20170904 if(ReaderCFG.HourlyReaderType == 0) 
		{			
			return NONE_TICKET;
		}
		
		//出場期限 = 入場時間 + 入場免費時間
		/* 
		outDeadline = now + (G_ParkingConfig.FreeTime * 60);
		tm_ptr = localtime(&outDeadline);
		ticketData->out_year  = tm_ptr->tm_year + 1900;
		ticketData->out_month = tm_ptr->tm_mon + 1;
		ticketData->out_day   = tm_ptr->tm_mday;
		ticketData->out_hour  = tm_ptr->tm_hour;
		ticketData->out_min   = tm_ptr->tm_min;
		*/
		
		// 20110506 Tony add s
		ticketData->out_year  = 2000;
		ticketData->out_month = 1;
		ticketData->out_day   = 1;
		ticketData->out_hour  = 0;
		ticketData->out_min   = 0;
		ticketData->out_sec   = 0; 
		// 20110506 Tony add e

// ========================================================== //
// nick mark s 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
//		ShowMessage((char *)"DetectUserCardType :: GetTicketSerial");	// 20120427 Tony add
//		
//		if(ReaderCFG.HourlyReaderType == 5 && G_ParkingConfig.BarCodeType == 0)
//		{	//簡易型
//			// 20120925 Tony mark sprintf(ticketID,"%08ld",GetTicketSerial());
//			sprintf(ticketID,"%07ld1",GetTicketSerial());	// 20120925 Tony add
//		}
//		else
//		{
//			sprintf(ticketID,"%02d%06ld1",G_ParkingConfig.MachineID,GetTicketSerial());
//		}
// nick mark e 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
// ========================================================== //

		// ========================================================= //
		// nick add s 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
// nick mark 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //		TickSerial = GetTicketSerial();

		// ========================================================= //
		// nick add s 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		for (iRet = 0; iRet < 3; iRet++)
		{
			TickSerial = GetTicketSerial();

			if (TickSerial == 0)
			{
				res = system((char *)"mount-o,remount rw /usr/Ipms");
				usleep(100000);
			}
			else
				break;
		}
		// nick add e 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		// ========================================================= //
		
		if (TickSerial == 0)
		{
			ShowMessage((char *)"Hourly Ticket Serial is 0. Auto Reboot!"); // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
			iRet = system((char *)"reboot");
		}
		else
		{
			if(ReaderCFG.HourlyReaderType == 5 && G_ParkingConfig.BarCodeType == 0)
			{	//簡易型
				sprintf(ticketID, "%07ld1", TickSerial);	// 20120925 Tony add
			}
			else
			{
				sprintf(ticketID, "%02d%06ld1", G_ParkingConfig.MachineID, TickSerial);
			}
		}
		// nick add e 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
		// ========================================================= //
		
		ticketData->ticketID  = atol(ticketID);
		
		// 20110506 Tony add s
		if(G_ParkingConfig.ParkingID > 999)
		{
			ticketData->parkingId =999;
		}
		else
		{
		// 20110506 Tony add e
			ticketData->parkingId = G_ParkingConfig.ParkingID;
		}// 20110506 Tony add
		
		// nick mark 20160720 Ver:000-000-GIO_V2-13B251-0014-13C241 //ticketData->areaId = G_ParkingConfig.AreaID;
		
		printf("CheckTicketIssue :: Start : %ld\n",GetTickCount()-CalcTmpTime);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage((char *)"DetectUserCardType :: CheckTicketIssue");	// 20120427 Tony add
		iRet = CheckTicketIssue();
		printf("CheckTicketIssue :: End : %ld\n",GetTickCount()-CalcTmpTime);
		
		if(iRet == 1)
		{   //發卡機裡面有票
			result = HOURLY_TICKET;
			
			//SendDataToPlateSVR(*ticketData);	// 20110627 Tony add
			// ================ //
			// nick mark s 20130219 //
			//res = pthread_create(&SendPlateData,NULL,SendDataToPlateSVR,(void *)ticketData);
			//
			//if(res != 0)
			//{
			//	perror("Thread creation failed !");
			//	exit(EXIT_FAILURE);
			//}
			// nick mark s 20130219 //
			// ================ //
		}
		// nick mark 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //else if(IssueTicket() == false)
		else if(IssueTicket() != 1) // nick add 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //
		{   //發出票卡
			result = DESPENSE_ERROR;
			ShowMessage((char *)"Dispense Error!");
			ShowLCMFile(0,0,(char *)"TicketOver.lcm"); //票卡用完					//Frank add 20111116
		}
		else
		{
			result = HOURLY_TICKET;
			
			//SendDataToPlateSVR(*ticketData);	// 20110627 Tony add
			// ================ //
			// nick mark s 20130219 //
			//res = pthread_create(&SendPlateData,NULL,SendDataToPlateSVR,(void *)ticketData);
			//
			//if(res != 0)
			//{
			//	perror("Thread creation failed !");
			//	exit(EXIT_FAILURE);
			//}
			// nick mark e 20130219 //
			// ================ //
		}

		printf("DetectUserCardType :: End : %ld\n",GetTickCount()-CalcTmpTime);
		// nick mark 20130219 //return result;					//Frank add 20121225
	}
	else if((readerStatus = GetSeasonTicketInsert((char*)(&mfTag))) == 1)
	{
		EasyCardEnable(false); // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
		
		// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(G_ParkingConfig.SeasonReadTag == 1)
		// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //{
		// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //	result = SEASON_TICKET;
		// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //}
		// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //else
		{
			ShowLCMFile(0,0,(char *)"TicketProcess.lcm");   ////票卡處理中
			retRead = ReadTicketData(ticketData,SEASON_TICKET);
			sprintf(buf,"Read Season Tag:[%s]",ticketData->TagID);
			ShowMessage(buf);
			
			if(retRead == 1)
			{
				if (ticketData->ticketID % 10L != 1 && G_ParkingConfig.SeasonReadTag == 1) // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					AudioOn(AUDIO_SEASON_W_SUC); // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				
				if((ticketData->ticketID % 10L) == 2 && ticketData->seasonVersion > 0)
				{
					result = SEASON_TICKET;
					ShowMessage((char *)"Is Season.");
				}
				else if((ticketData->ticketID % 10L) == 5 && ticketData->seasonVersion > 0)
				{
					ShowMessage((char *)"Is Hotel Season.");
					result = SEASON_TICKET;
				}
				//Frank mark 20120709 //else if((ticketData->ticketID % 10L) == 3 && ticketData->value > 0)
				else if((ticketData->ticketID % 10L) == 3)					//Frank add 20120709
				{
					result = VALUE_TICKET;
					sprintf(buf,"Is Value Card, the value:[%ld].",ticketData->value);
					ShowMessage(buf);
				}
				else if((ticketData->ticketID % 10L) == 1)
				{
					//owMessage((char *)"Is Hourly.");
					sprintf(buf,"DetectUserCardType->GetSeasonTicketInsert ->IS_Hourly :: G_ParkingConfig.UpLayerID = %d , ReaderCFG.HourlyReaderType = %d",G_ParkingConfig.UpLayerID,ReaderCFG.HourlyReaderType);
					ShowMessage(buf);
					// 20110505 Tony add s
					if(G_ParkingConfig.UpLayerID!= 0 && ReaderCFG.HourlyReaderType == 0)
					{
						result = SEASON_TICKET;
						ShowMessage((char *)"SEASON_TICKET");
					}
					else
					{
						AudioOn(AUDIO_READERROR);
						VoiceOn(VOICE_READERROR);
						ShowMessage((char *)"HOURLY_S");
					// 20110505 Tony add e
						result = HOURLY_S;
					}// 20110505 Tony add
				}
				else if(ticketData->ticketID == 99999999L)
				{
					result = PARKTRON_CARD;
				}
				else
				{
					AudioOn(AUDIO_INVTICKET);
					VoiceOn(VOICE_INVTICKET);
					result = UNKNOW_TICKET;
					sprintf(buf,"Ticket:%ld Unknow. Ver:%ld",ticketData->ticketID,ticketData->seasonVersion);
					ShowMessage(buf);
				}
			}
			else if(retRead == -1)
			{
				result = UNKNOW_TICKET;
				AudioOn(AUDIO_READERROR);
				VoiceOn(VOICE_READERROR);
				sprintf(buf,"Read Error Ticket:%ld.",ticketData->ticketID);
				ShowMessage(buf);
			}
			else if(retRead == 0)
			{
				result = NONE_TICKET;
			}
		}
	}

	// =================== //
	// nick add s 20130219 //
	if (result == HOURLY_TICKET || result == SEASON_TICKET || result == VALUE_TICKET)
	{
		if (SendPlateData != 0)
		{
			pthread_cancel(SendPlateData);
			//usleep(3000000L);
			//pthread_join(SendPlateData, NULL);
			SendPlateData = 0;
		}
		
		for (res = 0; res < 3; res++)
		{
			if (pthread_create(&SendPlateData,NULL,SendDataToPlateSVR,(void *)ticketData) == 0)
				break;
			
			usleep(1000L);
		}

		if (res == 3) SendPlateData = 0;
		printf("Man OCR pthread_id = 0x%lx\n", SendPlateData);
	}
	// nick add e 20130219 //
	// =================== //
	
	if(readerStatus == -1)
	{
		G_ParkingStatus.status &= (~STATUS_READER_CONNECT);
	}
	else
	{
		G_ParkingStatus.status |= STATUS_READER_CONNECT;
	}
	
	return result;
}

enum TicketType ReadOutTicketData(TicketData* ticketData)
{	//Get  ticket Data, Easy card is check input signal
	enum TicketType result = NONE_TICKET;
	int readRet = 0;
	time_t now;
	struct tm *tm_ptr = NULL;
	char mfTag[80];
	char buf[256];
	long Type = 0L;
	
	memset(buf,'\0',sizeof(buf));
	memset(mfTag,'\0',sizeof(mfTag));
	
	if(G_RFin == true)
	{
		now = time((time_t *)0);
		tm_ptr = localtime(&now);
		
		ticketData->out_year  = tm_ptr->tm_year + 1900;
		ticketData->out_month = tm_ptr->tm_mon + 1;
		ticketData->out_day   = tm_ptr->tm_mday;
		ticketData->out_hour  = tm_ptr->tm_hour;
		ticketData->out_min   = tm_ptr->tm_min;
		ticketData->ticketID = 0; // nick add 20140418 Ver:000-000-GIO_V2-135101-0003-13B251 //
		ticketData->areaId = G_ParkingConfig.AreaID; // nick add 20160720 Ver:000-000-GIO_V2-13B251-0014-13C241 //
		result = EASY_CARD;
	}
	// ========================================================= //
	// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	else if (G_POSIN == true)
	{
		now = time((time_t *)0);
		tm_ptr = localtime(&now);
		
		ticketData->out_year  = tm_ptr->tm_year + 1900;
		ticketData->out_month = tm_ptr->tm_mon + 1;
		ticketData->out_day   = tm_ptr->tm_mday;
		ticketData->out_hour  = tm_ptr->tm_hour;
		ticketData->out_min   = tm_ptr->tm_min;
		ticketData->ticketID = 1;
		ticketData->areaId = G_ParkingConfig.AreaID; // nick add 20160720 Ver:000-000-GIO_V2-13B251-0014-13C241 //
		result = POS_IN;
	}
	// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	// ========================================================= //	
	else if(GetHourlyTicketInsert() == true)
	{
		EasyCardEnable(false); // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
		ShowMessage((char *)"Hourly ticket Insert. ");
		ShowLCMFile(0,0,(char *)"TicketProcess.lcm");   ////票卡處理中
		readRet = ReadTicketData(ticketData,HOURLY_TICKET);
		
		if(readRet == 1)
		{
			Type = ticketData->ticketID % 10L;
			
			if(Type == 2 && ticketData->seasonVersion > 0)
			{
				result = SEASON_TICKET_H;
				ShowMessage((char *)"Is Season.");
			}
			else if(Type == 5)
			{
				result = SEASON_TICKET_H;
				ShowMessage((char *)"Is Hotel Season.");
			}
			//Frank mark 20120709 else if(Type == 3 && ticketData->value > 0)
			else if(Type == 3)					//Frank add 20120709
			{
				result = VALUE_TICKET;
				sprintf(buf,"Is Value Card, the value:[%ld].",ticketData->value);
				ShowMessage(buf);
			}
			else if(Type == 1)
			{
				result = HOURLY_TICKET;
				ShowMessage((char *)"Is Hourly .");
			}
			else
			{
				AudioOn(AUDIO_INVTICKET);
				VoiceOn(VOICE_INVTICKET);
				result = UNKNOW_TICKET;
				sprintf(buf,"Ticket:%ld Unknow %ld.",ticketData->ticketID,Type);
				ShowMessage(buf);
			}
		}
		else if(readRet == -1)
		{
			result = TICKET_READ_ERROR;
			ShowMessage((char *)"Read Ticket Error! ");
		}
		else
		{
			result = NONE_TICKET;
		}
	}
	else if(GetSeasonTicketInsert((char*)(&mfTag)) == true)
	{
		EasyCardEnable(false); // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
		ShowLCMFile(0,0,(char *)"TicketProcess.lcm");   ////票卡處理中
		
		// ========================================================== //
		// nick mark s 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		//if(G_ParkingConfig.SeasonReadTag == 1)
		//{
		//	result = SEASON_TICKET;
		//	ShowMessage((char *)"Is season Tag.");
		//}
		//else
		// nick mark e 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		// ========================================================== //
		{
			readRet = ReadTicketData(ticketData,SEASON_TICKET);
			sprintf(buf,"Read Season Tag:[%s]",ticketData->TagID);
			ShowMessage(buf);
			
			if(readRet == 1)
			{
				Type = ticketData->ticketID % 10L;
				
				if (Type != 1 && G_ParkingConfig.SeasonReadTag == 1) // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					AudioOn(AUDIO_SEASON_W_SUC); // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				
				if(Type == 2 && ticketData->seasonVersion != 0)
				{
					result = SEASON_TICKET;
					ShowMessage((char *)"Is Season .");
				}
				else if(Type == 5 && ticketData->seasonVersion != 0)
				{
					result = SEASON_TICKET;
					ShowMessage((char *)"Is Hotel Season .");
				}
				//Frank mark 20120709 else if(Type == 3 && ticketData->value > 0)
				else if(Type == 3)					//Frank add 20120709
				{
					result = VALUE_TICKET;
					sprintf(buf,"Is Value Card, the value:[%ld] .",ticketData->value);
					ShowMessage(buf);
				}			
				else if(Type == 1)
				{					
					// 20110511 Tony add s
					// ========================================================== //
					// nick mark s 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
					//sprintf(buf,"DetectUserCardType ->GetSeasonTicketInsert ->IS_Hourly.");
					//ShowMessage(buf);
					//sprintf(buf,"G_ParkingConfig.PreLayerID = %d , ReaderCFG.HourlyReaderType = %d.",G_ParkingConfig.UpLayerID,ReaderCFG.HourlyReaderType );
					// nick mark e 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
					// ========================================================== //
					
					sprintf(buf,"GetSeasonTicketInsert()->IS_Hourly. G_ParkingConfig.PreLayerID = [%d], ReaderCFG.HourlyReaderType = [%d].",G_ParkingConfig.UpLayerID,ReaderCFG.HourlyReaderType ); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
					ShowMessage(buf);
					
					if (ReaderCFG.HourlyReaderType == 0 && G_ParkingConfig.UpLayerID!= 0)
					{
						result = SEASON_TICKET;
					}
					else
					{
						AudioOn(AUDIO_READERROR);
						VoiceOn(VOICE_READERROR);
					// 20110511 Tony add e				
						result = HOURLY_S;
						ShowMessage((char *)"Is Hourly . Can't process at this Reader");
					}// 20110511 Tony add
				}
				else if(ticketData->ticketID == 99999999L)
				{
					result = PARKTRON_CARD;
				}
				else
				{
					AudioOn(AUDIO_INVTICKET);
					VoiceOn(VOICE_INVTICKET);
					result = UNKNOW_TICKET;
					sprintf(buf,"Ticket:[%ld] Unknow %ld . Ver:[%ld]",ticketData->ticketID,Type,ticketData->seasonVersion);
					ShowMessage(buf);
				}
			}
			else if( readRet == -1)
			{
				AudioOn(AUDIO_READERROR);
				//nick mark 20101207 result = TICKET_READ_ERROR;
				result = TICKET_SEASON_READ_ERROR;	//nick add 20101207
				ShowMessage((char *)"Read Season Ticket Error! ");
				
				sleep(1);
			}
			else if(readRet == 0)
			{
				result = NONE_TICKET;
			}
		}
	}
	
	return result;
}

// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) bool ProcessHourlyIn(TicketData ticketData)
bool ProcessHourlyIn(TicketData* ticketData)	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
{	//Issue and write ticket Data to Ticket
	//Frank mark 20120821 int timeout = 0;
	int retry = 0;
	int i,iRet=1;
	char buf[256];
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Ticks = GetTickCount();					//Frank add 20120821
	unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	unsigned long IssueTimeout = 0; // nick add 20140909 Ver:000-000-GIO_V2-135101-0006-13B251 //
	
	memset(buf,'\0',sizeof(buf));
	
	for(i=0; i < 5; i++)
	{
		for (int j = 0; j < 16; j++) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
			G_MemBlocksWriteSuc[j] = 0; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		
//Frank mark	20120511	if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4)
		// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || ReaderCFG.HourlyReaderType == 6)					//Frank add 20120508
		if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || ReaderCFG.HourlyReaderType == 6 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
		{
			// ========================================================= //
			// nick add s 20140909 Ver:000-000-GIO_V2-135101-0006-13B251 //
			// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //if (ReaderCFG.HourlyReaderType == 3)
			if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
				IssueTimeout = 20000;
			else
				IssueTimeout = 8000;
			// nick add e 20140909 Ver:000-000-GIO_V2-135101-0006-13B251 //
			// ========================================================= //
		
			while(1)
			{
				iRet = CheckTicketIssue();
				
				if(iRet == 1)
				{   //檢查到票了 .
					HopperReset();
					break;
				}
				else if(iRet == -1)
				{
					G_ParkingStatus.status &= (~STATUS_READER_CONNECT);
				}
				else
				{
					G_ParkingStatus.status |= STATUS_READER_CONNECT;
				}
				
				//Frank mark 20120717 usleep(600000L);   // 600ms
				//Frank mark 20120821 usleep(100000L);					//Frank add 20120717
				
				if(GetLoop1() == false)
				{
					HopperReset();
					RecycleTicket();   //回收票卡.
					return false;
				}
				
				//Frank mark 20120717 if(timeout++ > 5)
				//Frank mark 20120821 if(timeout++ > 25)					//Frank add 20120717
				// nick mark 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //if(CheckTimeout(Ticks, (double)8000))					//Frank add 20120821
				// nick mark 20140909 Ver:000-000-GIO_V2-135101-0006-13B251 //if(CheckTimeout(&Ticks, (double)20000)) // nick add 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
				if(CheckTimeout(&Ticks, IssueTimeout)) // nick add 20140909 Ver:000-000-GIO_V2-135101-0006-13B251 //
				{	// 超過8秒,沒感應到票卡,再發一張 (Chip coin)
					// 先回收
					if(ReaderCFG.HourlyReaderType == 6)					//Frank add 20120821
						ReaderReset();					//Frank add 20120821
					
					HopperReset();
					DrawChipCoin(false);
					RecycleTicket();   //回收票卡.
					sleep(1);
					
					if(retry++ > 3)
					{
						return false;
					}
					
					if(CheckTicketEmpty()==true)
						return false;
					
					sprintf(buf,"Issue again %d .",retry);
					ShowMessage(buf);

					for (int j = 0; j < 16; j++) // nick add 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
						G_MemBlocksWriteSuc[j] = 0; // nick add 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
					
					//Frank add s 20120821
					if (ReaderCFG.HourlyReaderType == 6)
					{
						IssueTicket(true);
					}
					// ========================================================= //
					// nick add s 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
					// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //else if (ReaderCFG.HourlyReaderType == 3)
					else if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
					{
						G_bSystemFault = true;
						SendAlarm(2, (char *)"No Ticket!");
						return (false);
					}
					// nick add e 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
					// ========================================================= //
					else
					{
						// ========================================================= //
						// nick add s 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
						if(ReaderCFG.DispenserQuantity > 1)
						{
						if (IssueDispense == 1)
							IssueDispense = 2;
						else
							IssueDispense = 1;
						}
						// nick add e 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
						// ========================================================= //
					//Frank add e 20120821
						IssueTicket();
					//Frank mark 20120821 timeout = 0;
					}
					
					Ticks = GetTickCount();					//Frank add 20120821
				}
				
				LCM_ShowTime();
				usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
			}
		}
		else if(ReaderCFG.HourlyReaderType == 2)
		{
		}
		else if(ReaderCFG.HourlyReaderType == 5)
		{
		//Frank add s 20111116
			if(CheckTicketEmpty() == true)
			{
				return false;
			}
		//Frank add e 20111116
		}
		else if(ReaderCFG.HourlyReaderType == 1)
		{
		}
		
		printf("WriteIssueData :: Satrt : %ld\n",GetTickCount()-CalcTmpTime);
		iRet = WriteIssueData(ticketData);  // 20120131 Tony add
		printf("WriteIssueData :: End : %ld\n",GetTickCount()-CalcTmpTime);

		sprintf(buf,"WriteIssueData :: Tciket id : %ld , TagID : %s\n" ,ticketData->ticketID,ticketData->TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		ShowMessage(buf);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
		if (iRet != 1)                      // 20120131 Tony add
		// 20120131 Tony mark if((iRet = WriteIssueData(ticketData)) != 1)
		{
			if (iRet == -1)
			{
				G_ParkingStatus.status &= (~STATUS_READER_CONNECT);
				
				// ========================================================= //
				// nick add s 20140724 Ver:000-000-GIO_V2-135101-0005-13B251 //
				RecycleTicket();
				
				// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //if (ReaderCFG.HourlyReaderType == 3)
				if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
				{
					i = 5;
					// nick mark 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //G_bSystemFault = true;
					break;
				}
				// nick add e 20140724 Ver:000-000-GIO_V2-135101-0005-13B251 //
				// ========================================================= //
			}
			
// nick mark 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //			RecycleTicket();   //回收票卡.
			ReceiveChipCoin(); // nick add 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
			sprintf(buf,"Write Issue Ticket Error %d.",iRet);
			SendAlarm(61,buf);
			ShowMessage(buf);
			LCM_ShowTime();
			
			if(i < 2)
			{
				sleep(1);
				
				if(CheckTicketEmpty() == true)
					return false;
					
				IssueTicket();
				Ticks = GetTickCount();					//Frank add 20120921
			}
			// ========================================================= //
			// nick add s 20160712 Ver:000-000-GIO_V2-13C241-0001-166241 //
			else
			{ // 超過三次寫卡失敗
				G_bSystemFault = true;
				return (false);
			}
			// nick add e 20160712 Ver:000-000-GIO_V2-13C241-0001-166241 //
			// ========================================================= //

			continue;
		}
		else
		{   //寫入成功離開
//Frank mark	20120508		if(TicketToOutlet() == false)
			TicketData sticketData;	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			memcpy(&sticketData,ticketData,sizeof(sticketData));	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

			// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) if(TicketToOutlet(ticketData) == false)					//Frank add 20120508
			if(TicketToOutlet(sticketData) == false)	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				continue;
			
			break;
		}
	}
	
	if(i >= 5)
	{
		// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
		if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
			HopperReset();
		
		return false;
	}
	
	return true;
}

bool ProcessSeasonCardIn(TicketData *ticketData)					//Frank add 20120208
//20120208 Frank mark  bool ProcessSeasonCardIn(TicketData ticketData)
{	//check Season Ticket is valid and write in out status to Ticket
	int iRet = 1,i,len;
	unsigned int Area;
	char buf[256];
	char area_code[10];
	
	//20110519 Tony add s
	struct tm *tm_ptr2 = NULL;
	time_t now;
	now    = time((time_t *)0);
	tm_ptr2 = localtime(&now);
	//20110519 Tony add e
	
	TicketData sticketData;	// 20120208 Frank add
	memcpy(&sticketData,ticketData,sizeof(sticketData));	// 20120208 Frank add
	
	memset(buf,'\0',sizeof(buf));
	memset(area_code,'\0',sizeof(area_code));
	
	sprintf(buf,"Season TicketID:%ld",ticketData->ticketID); // 20120117 Tony add
	ShowMessage(buf);                                       // 20120117 Tony add
	
	printf("TicketTag:[%s], LastTag:[%s]\n", ticketData->TagID, LastTagId); // nick add 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	
	if(strncmp(ticketData->TagID,LastTagId,8) != 0)
	{
		for (int rows = 0; rows < 16; rows++) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
			G_MemBlocksWriteSuc[rows] = 0; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		
		// 20110506 Tony add s
		//if((ticketData.ticketID % 10L) == 1)
		if(G_ParkingConfig.UpLayerID != 0)
		{ // 內層設備判斷邏輯 //
			sprintf(buf,"Ticket.ParkingID:%d, Cfg.ParkingID :%d  ;Ticket.Area:%d, Cfg.UpLayerID:%d",
				ticketData->parkingId, G_ParkingConfig.ParkingID, ticketData->areaId, G_ParkingConfig.UpLayerID);
			ShowMessage(buf);
			
			if(ticketData->parkingId != G_ParkingConfig.ParkingID)
			{
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //				sprintf(buf,"In Parking Id Error.");
				sprintf(buf,"Ticket ID:%ld In Parking Id Error.", ticketData->ticketID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
				ShowMessage(buf);
				AudioOn(AUDIO_AREAERR);
				SendAlarm(60,buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}
			
			// 20120113 Tony mark if((ticketData.ticketID % 10L) == 1)
			// 20120113 Tony mark {
			if(ticketData->areaId != G_ParkingConfig.UpLayerID)
			{ //判斷Ticket.AreaID和PreLayerID的值是否相同，不相同時表示該票卡的進出流程錯誤 //
        		// 20120117 Tony add s
				if(ticketData->areaId == G_ParkingConfig.AreaID)
				{ // 票卡ID和區域ID相同時表示已進場 //
					AudioOn(AUDIO_REENTER);
					ShowLCMFile(0,0,(char *)"AlreadyIn.lcm");
					sprintf(buf,"TicketID:%ld  Re-Enter.",ticketData->ticketID);
					ShowMessage(buf);
					//Frank mark 20120709 SendAlarm(26,buf);
					SendAlarm(19 , buf);					//Frank add 20120709
				}
				else
				{
					// 20120117 Tony add e
					// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //sprintf(buf,"In Area Error.");
					sprintf(buf,"TicketID:%ld In Area Error.", ticketData->ticketID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
					ShowMessage(buf);
					AudioOn(AUDIO_AREAERR);
					SendAlarm(60,buf);
					ShowLCMFile(0,0,(char *)"AreaError.lcm");
				}	// 20120117 Tony add

				return false;
			}
			// 20120113 Tony mark }
			
			// 20111202 Tony add 月票判斷 s
			if((ticketData->ticketID % 10L) == 2)
			{
				// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = CheckSeasonIsValid(sticketData, area_code);	// 20120208 Frank add
				iRet = CheckSeasonIsValid(&sticketData, area_code); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// 20120208 Frank mark iRet = CheckSeasonIsValid(ticketData, area_code);
				sprintf(buf, "CheckSeasonIsValid() Result:[%d]", iRet);
				ShowMessage(buf);
				
				if(iRet < 1)
				{	//檢查使用期限		
					AudioOn(AUDIO_SEASONEXPIRE);
					ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
					
					if(iRet == -2)
					{
						sprintf(buf,"TicketID:%ld is Expired!",ticketData->ticketID);
					}
					else if(iRet == -1)
						sprintf(buf,"TicketID:%ld not Start.",ticketData->ticketID);
					else if(iRet == 0)
						sprintf(buf,"TicketID:%ld No Data.",ticketData->ticketID);
					//Frank add s 20120907
					else if(iRet == -5)
						sprintf(buf , "TicketID : %ld Version error." , ticketData ->ticketID);
					//Frank add e 20120907
					
					ShowMessage(buf);
					SendAlarm(32,buf);
					return false;
				}
				
				len = strlen(area_code);
				
				for( i=0; i<len; i++)
				{
					buf[0] = area_code[i];
					buf[1] = 0;
					Area =(atoi(buf));
					
					if( Area == G_ParkingConfig.AreaID || Area == 9)
					{
						break;
					}
					else
						printf("Area:%d not there\n",atoi(buf));
				}
				
				memset(buf,'\0',sizeof(buf));
				
				if(i == len)
				{	//AREA 未定義
					AudioOn(AUDIO_AREAERR);
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //					sprintf(buf,"Area:%d, %d not Allow", ticketData->areaId,G_ParkingConfig.AreaID);
					sprintf(buf,"TicketID:%ld Area:%d, %d not Allow", ticketData->ticketID, ticketData->areaId,G_ParkingConfig.AreaID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
					ShowMessage(buf);
					SendAlarm(60,buf);
					ShowLCMFile(0,0,(char *)"AreaError.lcm");
					return false;
				}
			}
			// 20111202 Tony add 月票判斷 e 
			
			sprintf(buf,"Check BlackList.");
			ShowMessage(buf);
			
			// 檢查黑名單
			if(CheckBlackList(ticketData->ticketID))
			{
				AudioOn(AUDIO_INVTICKET);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				sprintf(buf,"TicketID:%ld is Black",ticketData->ticketID);
				ShowMessage(buf);
				SendAlarm(22,buf);
				return false;
			}
			
			sprintf(buf,"Get Staytime & Optime start");
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			//time_t now,tm_intime;
			time_t tm_intime;
			double TmpSec;
			unsigned long ulOptime;
			struct tm tm_ptr1;
			//struct tm *tm_ptr2 = NULL;
			
			ulOptime = (unsigned long)BCD2Int((unsigned char*)ticketData->optime,3);		// 20120208 Frank add
			// 20120208 Frank mark ulOptime = (unsigned long)BCD2Int((unsigned char*)ticketData.optime,3);		// 20111205 Tony add
			printf("ulOptime : %ld \n",ulOptime);										// 20111205 Tony add
			
			tm_ptr1.tm_year = ticketData->in_year - 1900;
			//sprintf(buf,"tm_ptr1.tm_year %d",tm_ptr1.tm_year);
			//ShowMessage(buf);
			
			tm_ptr1.tm_mon = (int)ticketData->in_month - 1;
			//sprintf(buf,"tm_ptr1.tm_mon : %d",tm_ptr1.tm_mon);
			//ShowMessage(buf);
			
			tm_ptr1.tm_mday = (int)ticketData->in_day;
			//sprintf(buf,"tm_ptr1.tm_mday : %d",tm_ptr1.tm_mday);
			//ShowMessage(buf);
			
			tm_ptr1.tm_hour = (int)ticketData->in_hour;
			//sprintf(buf,"tm_ptr1.tm_hour : %d",tm_ptr1.tm_hour);
			//ShowMessage(buf);
			
			tm_ptr1.tm_min = (int)ticketData->in_min;
			//sprintf(buf,"tm_ptr1.tm_min : %d",tm_ptr1.tm_min);
			//ShowMessage(buf);
			
			tm_ptr1.tm_sec = (int)ticketData->in_sec;
			//sprintf(buf,"tm_ptr1.tm_sec : %d",tm_ptr1.tm_sec);
			//ShowMessage(buf);
			
			tm_intime=mktime(&tm_ptr1);
			
			sprintf(buf,"asctime(tm_intime)");
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			sprintf(buf,"In time : %s",asctime(&tm_ptr1));
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			//now    = time((time_t *)0);
			//tm_ptr2 = localtime(&now);
			
			sprintf(buf,"asctime(tm_ptr2)");
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			sprintf(buf,"Now time : %s",asctime(tm_ptr2));
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			TmpSec = difftime(now,tm_intime);
			
			sprintf(buf,"Diff time : %lf",TmpSec);
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			//ticketData.in_year  = tm_ptr2->tm_year + 1900;
			//ticketData.in_month = tm_ptr2->tm_mon + 1;
			//ticketData.in_day   = tm_ptr2->tm_mday;
			//ticketData.in_hour  = tm_ptr2->tm_hour;
			//ticketData.in_min   = tm_ptr2->tm_min;
			//ticketData.in_sec   = tm_ptr2->tm_sec;
			
			sprintf(buf,"Before staytime : %ld", ticketData->staytime );
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			sprintf(buf,"Parking config -> UpLayer free time : %d",G_ParkingConfig.UpLayerFreeTime);
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			sprintf(buf,"In min : %f",(TmpSec / 60));
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			if((ticketData->ticketID % 10L) == 1)	// 20111202 Tony add
			{										// 20111202 Tony add
				sprintf(buf,"TicketData.staytime Before: %ld",ticketData->staytime);	// 20111205 Tony add
				ShowMessage(buf);											// 20111205 Tony add
					
				if (ticketData->staytime == 0)										// 20111205 Tony add
				{
					if ((TmpSec / 60) > G_ParkingConfig.UpLayerFreeTime)
					{
						ticketData->staytime += (unsigned long)TmpSec;
						sprintf(buf,"Intime > Freetime , TmpSec / 60: %ld",(unsigned long)(TmpSec / 60));
						ShowMessage(buf);
						
						/* 20111202 Tony mark s
						ticketData.in_year	= tm_ptr2->tm_year + 1900;
						ticketData.in_month = tm_ptr2->tm_mon + 1;
						ticketData.in_day	= tm_ptr2->tm_mday;
						ticketData.in_hour	= tm_ptr2->tm_hour;
						ticketData.in_min	= tm_ptr2->tm_min;
						ticketData.in_sec	= tm_ptr2->tm_sec;
						20111202 Tony mark e*/
					}
				// 20111205 Tony add s
				}
				else	
				{
					ticketData->staytime += (unsigned long)TmpSec;
					sprintf(buf,"TmpSec / 60 : %ld",(unsigned long)(TmpSec / 60));
					// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
				}
				
				sprintf(buf,"TicketData.staytime After: %ld",ticketData->staytime);
				// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
				// 20111205 Tony add e
			}										// 20111202 Tony add
			
			// 20111202 Tony add s
			/*20120208 Frank mark 
			ticketData.in_year	= tm_ptr2->tm_year + 1900;
			ticketData.in_month = tm_ptr2->tm_mon + 1;
			ticketData.in_day	= tm_ptr2->tm_mday;
			ticketData.in_hour	= tm_ptr2->tm_hour;
			ticketData.in_min	= tm_ptr2->tm_min;
			ticketData.in_sec	= tm_ptr2->tm_sec;
			*/
			// 20111202 Tony add e
			
			sprintf(buf,"After staytime : %ld",ticketData->staytime );
			ShowMessage(buf);
			
			// 20111205 Tony mark ulOptime = (unsigned long)(TmpSec / 60);
			ulOptime += ((unsigned long)(TmpSec / 60));	// 20111205 Tony add 
			sprintf(buf,"Optime : %ld",ulOptime);
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			Int2BCD(ulOptime,6,(unsigned char *)ticketData->optime);	// 20120208 frank add
			// 20120208 Frank mark Int2BCD(ulOptime,6,(unsigned char *)&ticketData.optime);
			
			sprintf(buf,"Get Staytime & Optime End");
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			// 20120130 Tony mark ticketData.areaId = G_ParkingConfig.AreaID;
		}
		else
		{
		// 20110506 Tony add e	
			if(ticketData->parkingId != G_ParkingConfig.ParkingID)	// 20110519 Tony add
			//20110519 Tony mark if(ticketData.parkingId != G_ParkingConfig.ParkingID || ticketData.areaId != G_ParkingConfig.AreaID)
			{	//檢查場區
				AudioOn(AUDIO_AREAERR);
				// 20111205 Tony mark sprintf(buf,"ParkingID:%d, %d  ;Area:%d, %d",
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //				sprintf(buf,"IN ParkingID:%d, %d  ;Area:%d, %d",	// 20111205 Tony add
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //					ticketData->parkingId, G_ParkingConfig.ParkingID, ticketData->areaId, G_ParkingConfig.AreaID);
				sprintf(buf,"TicketID:%ld IN ParkingID:%d, %d  ;Area:%d, %d",
					ticketData->ticketID, ticketData->parkingId, G_ParkingConfig.ParkingID, ticketData->areaId, G_ParkingConfig.AreaID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
				ShowMessage(buf);
				SendAlarm(60,buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}
			
			//20110519 Tony add s
			/*	20111205 Tony mark
			if(ticketData.areaId != G_ParkingConfig.AreaID)
			{	//檢查場區
				AudioOn(AUDIO_AREAERR);
				sprintf(buf,"ParkingID:%d, %d  ;Area:%d, %d",
					ticketData.parkingId, G_ParkingConfig.ParkingID, ticketData.areaId, G_ParkingConfig.AreaID);
				ShowMessage(buf);
				SendAlarm(60,buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}*/
			//20110519 Tony add s
			
			// 檢查黑名單
			if(CheckBlackList(ticketData->ticketID))
			{
				AudioOn(AUDIO_SEASONEXPIRE);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				sprintf(buf,"TicketID:%ld is Black",ticketData->ticketID);
				ShowMessage(buf);
				SendAlarm(22,buf);
				return false;
			}
			
			// Get Time at the moment
	//nick mark 20110209		iRet = CheckSeasonIsValid(ticketData.ticketID,area_code);
			
			// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = CheckSeasonIsValid(sticketData, area_code);	// 20120208 Frank add
			iRet = CheckSeasonIsValid(ticketData, area_code); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			// 20120208 Frank mark iRet = CheckSeasonIsValid(ticketData, area_code);	//nick add 20110209			
			sprintf(buf, "CheckSeasonIsValid() Result:[%d]", iRet);
			ShowMessage(buf);
			
			if(iRet < 1)
			{	//檢查使用期限		
				AudioOn(AUDIO_SEASONEXPIRE);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				
				if(iRet == -2)
				{
					sprintf(buf,"TicketID:%ld is Expired!",ticketData->ticketID);
				}
				else if(iRet == -1)
					sprintf(buf,"TicketID:%ld not Start.",ticketData->ticketID);
				else if(iRet == 0)
					sprintf(buf,"TicketID:%ld No Data.",ticketData->ticketID);
				//Frank add s 20120907
				else if(iRet == -5)
					sprintf(buf , "TicketID : %ld Version error." , ticketData ->ticketID);
				//Frank add e 20120907
				
				ShowMessage(buf);
				SendAlarm(32,buf);
				return false;
			}
			
			len = strlen(area_code);
			
			for( i=0; i<len; i++)
			{
				buf[0] = area_code[i];
				buf[1] = 0;
				Area =(atoi(buf));
				
				if( Area == G_ParkingConfig.AreaID || Area == 9)
				{
					break;
				}
				else
					printf("Area:%d not there\n",atoi(buf));
			}
			
			memset(buf,'\0',sizeof(buf));
			
			if(i == len)
			{	//AREA 未定義
				AudioOn(AUDIO_AREAERR);
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //				sprintf(buf,"Area:%d, %d not Allow", ticketData->areaId,G_ParkingConfig.AreaID);
				sprintf(buf,"TicketID:%ld Area:%d, %d not Allow", ticketData->ticketID, ticketData->areaId,G_ParkingConfig.AreaID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
				ShowMessage(buf);
				SendAlarm(60,buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}

			printf("TicketStatus:[%d]\n", ticketData->status);
			
			if(iRet==1 && ticketData->status == 1)
			{	//檢查進出狀態
				AudioOn(AUDIO_REENTER);
				ShowLCMFile(0,0,(char *)"AlreadyIn.lcm");
				sprintf(buf,"TicketID:%ld  Re-Enter.",ticketData->ticketID);
				ShowMessage(buf);
				//Frank mark 20120709 SendAlarm(26,buf);
				SendAlarm(19 , buf);					//Frank add 20120709
				return false;
			}	
			
			//20110519 Tony add s
			ticketData->staytime = 0;
			Int2BCD(0,6,(unsigned char *)ticketData->optime);	// 20120208 Frank add
			// 20120208 Frank mark Int2BCD(0,6,(unsigned char *)&ticketData.optime);
			
			/* 20120208 Frank mark
			ticketData.in_year	= tm_ptr2->tm_year + 1900;
			ticketData.in_month = tm_ptr2->tm_mon + 1;
			ticketData.in_day	= tm_ptr2->tm_mday;
			ticketData.in_hour	= tm_ptr2->tm_hour;
			ticketData.in_min	= tm_ptr2->tm_min;
			ticketData.in_sec	= tm_ptr2->tm_sec;
			*/
		}
		// 20110519 Tony add e
	}
	else
	{
		ShowMessage((char *)"Last Card Write.");
		memcpy(ticketData, &LastTicketData, sizeof(TicketData)); // nick add 20130222 //
	}
	
	// 20120208 Frank add s
	ticketData->in_year	= tm_ptr2->tm_year + 1900;
	ticketData->in_month = tm_ptr2->tm_mon + 1;
	ticketData->in_day	= tm_ptr2->tm_mday;
	ticketData->in_hour	= tm_ptr2->tm_hour;
	ticketData->in_min	= tm_ptr2->tm_min;
	ticketData->in_sec	= tm_ptr2->tm_sec;
	// 20120208 Frank add e
	
	ticketData->areaId = G_ParkingConfig.AreaID; //20120130 Tony add

	// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = WriteCarOutData(ticketData,false,false);    // 20120208 Frank add
	// 20120208 Frank mark iRet = WriteCarOutData(&ticketData,false,false);    // 20120131 Tony add
	
	// ========================================================= //
	// nick add s 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	if (G_ParkingConfig.SeasonReadTag == 0)
		iRet = WriteCarOutData(ticketData,false,false);    // 20120208 Frank add
	else if (G_ParkingConfig.SeasonReadTag == 1)
		iRet = 1;
	// nick add e 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	// ========================================================= //
	
	if(iRet != 1)                                       // 20120131 Tony add
	// 20120131 Tony mark if((iRet = WriteCarOutData(&ticketData,false,false)) != 1)
	{	//寫入進出狀態
		// nick mark 20130222 //AudioOn(AUDIO_READERROR);
		sprintf(buf,"Write Season Ticket Error %d",iRet);
		ShowMessage(buf);
		// nick mark 20130222 //ShowLCMFile(0,0,(char *)"TicketReadError.lcm");
		// nick mark 20130222 //SendAlarm(61,buf);
		sprintf(LastTagId,"%s",ticketData->TagID);
		return false;
	}
	
// nick mark 20130202 //	if (ReaderCFG.SeasonReaderType == 3)		//nick add 20120420
// nick mark 20130202 //		AudioOn(AUDIO_SEASON_W_SUC);		//nick add 20120420
	
	sprintf(buf,"ProcessSeasonCardIn End:%04d.%02d.%02d %02d:%02d:%02d",ticketData->in_year,ticketData->in_month,ticketData->in_day,
	ticketData->in_hour,ticketData->in_min,ticketData->in_sec);					//Frank add 20120207
	ShowMessage(buf);						//Frank add 20120207
	
	memset(LastTagId, '\0', sizeof(LastTagId));
	//memset(&LastTicketData, 0, sizeof(TicketData)); // nick add 20130222 //
	return true;
}

bool ProcessEasyCardIn(TicketData ticketData)
{	//reserved
	usleep(100000L);
	return true;
}

bool ProcessValueCardIn(TicketData ticketData)
{	//check Value Ticket is valid and write in out status to Ticket
	int iRet = 1;//,i,len;
//	int Area;
	char buf[256];
	
	memset(buf,'\0',sizeof(buf));
	
	if( strncmp(ticketData.TagID,LastTagId,8) != 0)
	{
		for (int i = 0; i < 16; i++) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
			G_MemBlocksWriteSuc[i] = 0; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		
		// 檢查黑名單
		if(CheckBlackList(ticketData.ticketID))
		{
			AudioOn(AUDIO_SEASONEXPIRE);
			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			sprintf(buf,"TicketID:%ld is Black",ticketData.ticketID);
			ShowMessage(buf);
			SendAlarm(22,buf);
			return false;
		}
		
		if(ticketData.parkingId != G_ParkingConfig.ParkingID || ticketData.areaId != G_ParkingConfig.AreaID)
		{	//檢查場區
			AudioOn(AUDIO_AREAERR);
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //			sprintf(buf,"ParkingID:%d, %d  ;Area:%d, %d",
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //				ticketData.parkingId, G_ParkingConfig.ParkingID, ticketData.areaId, G_ParkingConfig.AreaID);
			sprintf(buf,"TicketID:%ld ParkingID:%d, %d  ;Area:%d, %d",
				ticketData.ticketID, ticketData.parkingId, G_ParkingConfig.ParkingID, ticketData.areaId, G_ParkingConfig.AreaID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
			ShowMessage(buf);
			SendAlarm(60,buf);
			ShowLCMFile(0,0,(char *)"AreaError.lcm");
			return false;
		}
		
		if(ticketData.status == 1)
		{	//檢查進出狀態
			AudioOn(AUDIO_REENTER);
			ShowLCMFile(0,0,(char *)"AlreadyIn.lcm");
			sprintf(buf,"TicketID:%ld  Re-Enter.",ticketData.ticketID);
			ShowMessage(buf);
			//Frank mark 20120709 SendAlarm(26,buf);
			SendAlarm(19 , buf);					//Frank add 20120709
			return false;
		}
	}
	else
	{
		ShowMessage((char *)"Last VCard Write.");
		memcpy(&ticketData, &LastTicketData, sizeof(TicketData)); // nick add 20130222 //
	}
	
	sprintf(buf,"Ticket:%ld Value:[%ld].",ticketData.ticketID,ticketData.value);
	ShowMessage(buf);
	
	iRet = WriteCarOutData(&ticketData,false,false);    //20120131 Tony add
        
	if(iRet != 1)	// 20120131 Tony add
	// 20120131 Tony mark if((iRet = WriteCarOutData(&ticketData,false,false)) != 1)
	{	//寫入進出狀態
		// ================ //
		// nick mark s 20130222 //
		//AudioOn(AUDIO_READERROR);
		//sprintf(buf,"Write Value Card Error %d",iRet);
		//ShowMessage(buf);
		//ShowLCMFile(0,0,(char *)"TicketReadError.lcm");
		//SendAlarm(61,buf);
		// nick mark s 20130222 //
		// ================ //

		sprintf(buf,"Write Value Card Error %d",iRet); // nick add 20130222 //
		ShowMessage(buf); // nick add 20130222 //
		sprintf(LastTagId,"%s",ticketData.TagID);
		return false;
	}

	memset(LastTagId, 0, sizeof(LastTagId)); // nick add 20130222 //
	//memset(&LastTicketData, 0, sizeof(TicketData)); // nick add 20130222 //
	return true;
}

bool CheckHourlyTicketPaid(TicketData ticketData)
{
	//Frank mark 20120914 int i,TotalDiscountHour=0;
	int i;					//Frank add 20120914
	unsigned long ParkSecond = 0L;//Frank add 20120914
	double TotalDiscountHour = 0.0 , DiscountSecond = 0.0 , TotalDiscountMinute = 0.0 , DiscountMinute = 0.0;//Frank add 20120917
	bool bHasLimit = false;
	long min =0;
	//Frank mark 20120914 long DiscountSecond = 0L;		//nick add 20111220
	//Frank mark 20120914 long ParkSecond = 0L, TotalDiscountMinute = 0L, DiscountMinute = 0L;					//Frank add 20120206
	char buf[256];
	time_t now;
//	time_t OutLimit,EntryTime,LimitTime;
	time_t EntryTime,LimitTime;
	struct tm tm_EntryTime,tm_LimitTime;
//	struct tm tm_OutLimit,tm_EntryTime,tm_LimitTime,tm_now;
	//nick mark 20111219	HDiscountData DiscountData;
	
	HDiscountData DiscountData[20];		//nick add 20111219
	
	memset(DiscountData, 0, sizeof(DiscountData));		//nick add 20111219
	memset(buf,'\0',sizeof(buf));
	// Get Time at the moment
	now = time((time_t *)0);
	//tm_now = mktime(&now);
	//compare Limit out time is over?
/*
	tm_OutLimit.tm_year = ticketData.out_year - 1900;
	tm_OutLimit.tm_mon = ticketData.out_month - 1;
	tm_OutLimit.tm_mday = ticketData.out_day;
	tm_OutLimit.tm_hour = ticketData.out_hour;
	tm_OutLimit.tm_min = ticketData.out_min;
	tm_OutLimit.tm_sec = ticketData.out_sec;
	OutLimit = mktime(&tm_OutLimit);
*/
	tm_EntryTime.tm_year = ticketData.in_year - 1900;
	tm_EntryTime.tm_mon = ticketData.in_month - 1;
	tm_EntryTime.tm_mday = ticketData.in_day;
	tm_EntryTime.tm_hour = ticketData.in_hour;
	tm_EntryTime.tm_min = ticketData.in_min;
	//Frank mark 20130115 tm_EntryTime.tm_sec = 59;
	tm_EntryTime.tm_sec = ticketData.in_sec;					//Frank add 20130115
	EntryTime = mktime(&tm_EntryTime);
	LimitTime = EntryTime;	// 20110714 Nick Add
	
	//nick add s 20111220
	HTicketData t_data;
	
	t_data.TicketID = ticketData.ticketID;
	t_data.in_time = (unsigned long)EntryTime;
	t_data.pay_time = (unsigned long)now;
	t_data.out_time = (unsigned long)now;
	sprintf(t_data.Plate, "%s", ticketData.plate);
	t_data.Amount = 0;
	t_data.Deduct_Value = 0;
   t_data.payment_subject = 1;
	t_data.Pay_Type = 1;
	t_data.Disct_Type = 0;
	//nick add e 20111220
	ParkSecond = now - EntryTime;					//Frank add 20120206
	
	if(((unsigned int)ticketData.status == 1) || ((unsigned int)ticketData.status == 2) || ((unsigned int)ticketData.status == 6))
	{   //未付費 判斷入場緩衝(免費)時間
	
		//Frank add s 20120206
		if((ticketData.staytimetype > 0) && (ticketData.staytime > 0))
		{
			return false;
		}
		//Frank add e 20120206
		
		if((unsigned int)ticketData.status == 1)
		{	//進場狀態，判斷進場免費時間
			if (((unsigned int)EntryTime + (G_ParkingConfig.FreeTime *60)) > (unsigned int)now)
			{
				return true;
			}
			
			sprintf(buf,"Entry:%4d%02d%02d %02d:%02d  G_ParkingConfig.FreeTime:%d",ticketData.in_year,ticketData.in_month,ticketData.in_day,
				ticketData.in_hour,ticketData.in_min, G_ParkingConfig.FreeTime);
			ShowMessage(buf);
		}
		else
		{	//繳費狀態，判斷出場免費時間
			if(((unsigned int)EntryTime + (G_ParkingConfig.PaidTime * 60)) > (unsigned int)now)
			{
				sprintf(buf,"Paid:%4d%02d%02d %02d:%02d ", ticketData.in_year,ticketData.in_month,ticketData.in_day,
				ticketData.in_hour,ticketData.in_min);
				ShowMessage(buf);
				return true;
			}
			
			sprintf(buf,"Paid:%4d%02d%02d %02d:%02d  G_ParkingConfig.PaidTime:%d", ticketData.in_year,ticketData.in_month,ticketData.in_day,
				ticketData.in_hour,ticketData.in_min, G_ParkingConfig.PaidTime);
			ShowMessage(buf);
		}
		
		//折扣
		//nick mark 20111221 DiscountData.DiscountTime = now;
		
		if(ticketData.discount_year == ticketData.in_year &&
			ticketData.discount_month == ticketData.in_month &&
			ticketData.discount_day == ticketData.in_day)
		{
			//Frank add s 20120914
			if(ticketData.TicketVer == 2)
			{
				if(ticketData.discount_hour != ticketData.in_hour ||
					ticketData.discount_min != ticketData.in_min ||
					ticketData.discount_sec != ticketData.in_sec)
				{
					return false;
				}
			}
			//Frank add e 20120914
			// 判斷限時折扣
//nick mark 20110128			if(ticketData.LimitDiscount == 9)
			//Frank mark 20120910 if(ticketData.LimitDiscount >= 1)
			if((ticketData.LHDisctCnt & 0x0F) != 0)					//Frank add 20120910
			{
				tm_LimitTime.tm_year = ticketData.LimitYear - 1900;
				tm_LimitTime.tm_mon = ticketData.LimitMon - 1;
				tm_LimitTime.tm_mday = ticketData.LimitDay;
				tm_LimitTime.tm_hour = ticketData.LimitHour;
				tm_LimitTime.tm_min = 00;
				tm_LimitTime.tm_sec = 59;
				LimitTime = mktime(&tm_LimitTime);
				
//				printf("Discount Limit date:%4d%02d%02d %02d \n",ticketData.LimitYear,ticketData.LimitMon,ticketData.LimitDay,ticketData.LimitHour);
				
				if(LimitTime > EntryTime)
				{
					bHasLimit = true;
				}
				
//nick mark 20111219				if( LimitTime > now)
				if (bHasLimit)		//nick add 20111221
				{	//DiscountData[0]:固定限時折扣用
					/*
					//nick mark s 20111219
					DiscountData.TicketID = ticketData.ticketID;
					DiscountData.AID = ticketData.discounts[7].AID;
					DiscountData.VID = ticketData.discounts[7].VID;
					DiscountData.SID = ticketData.discounts[7].SID;
					DiscountData.Type = 45;
					//nick mark e 20111219 
					*/
					
					//nick add s 20111219
					DiscountData[0].DiscountTime = now;
					DiscountData[0].TicketID = ticketData.ticketID;
					
					if(ticketData.TicketVer == 1)					//Frank add 20120912
					{					//Frank add 20120912
						DiscountData[0].AID = ticketData.discounts[7].AID;
						DiscountData[0].VID = ticketData.discounts[7].VID;
						DiscountData[0].SID = ticketData.discounts[7].SID;
						//Frank add s 20120912
					}
					else if (ticketData.TicketVer == 2)
					{
						// ==================== //
						// nick mark s 20130318 //
						//DiscountData[0].AID = ticketData.Discounts2[0].AID;
						//DiscountData[0].VID = ticketData.Discounts2[0].VID;
						//DiscountData[0].SID = ticketData.Discounts2[0].SID;
						// nick mark e 20130318 //
						// ==================== //
						
						// nick add s 20130318 //
						DiscountData[0].AID = ticketData.LimitAID;
						DiscountData[0].VID = ticketData.LimitSID;
						DiscountData[0].SID = ticketData.LimitVID;
						// nick add e 20130318 //
					}
					//Frank add e 20120912
					
					DiscountData[0].Type = 45;
					//nick add e 20111219
					
					min = (now - EntryTime) / 60;
					
					if(((now - EntryTime) % 60) >0)
						min= min + 1;
						
					//nick mark 20111219 DiscountData.RealDiscountMinute=min;
					DiscountData[0].RealDiscountMinute = min;		//nick add 20111219
					min = (LimitTime - EntryTime) / 60;
					
					if(((LimitTime - EntryTime) % 60) >0)
						min= min + 1;
						
					//nick mark 20111219 DiscountData.DiscountMinute=min;
					DiscountData[0].DiscountMinute = min;		//nick add 20111219
					DiscountData[0].bolIsDisct = true;			//nick add 20111219
					sprintf(buf,"Limit Discount: %04d-%02d-%02d %02d",ticketData.LimitYear,ticketData.LimitMon,ticketData.LimitDay,ticketData.LimitHour);
					
					if(LimitTime > now)		//nick add 20111219
					{
						//nick mark 20111219 WriteTempDiscountData(DiscountData);
						WriteTempDiscountData(DiscountData[0]);		//nick add 20111219
						ShowMessage(buf);
						t_data.Disct_Type = DiscountData[0].Type;		//nick add 20111220
						WriteTempPaymentData(t_data);					//nick add 20111220
						return true;
					}
					
					EntryTime = LimitTime;		//nick add 20111219	
				}
				
				//printf("Limit: %ld ,now:%ld \n",LimitTime,now);
			}
			
			//sprintf(buf,"Discount %d \n",ticketData.DiscountCount);
			//ShowMessage(buf);
			
			// =================== //
			// nick add s 20130318 //
			min = ParkSecond /60;
			
			if((ParkSecond % 60) > 0)
				min = min + 1;
			// nick add e 20130318 //
			// =================== //
			
			printf("CheckHourlyTicketPaid() -> Discount Count:%d\n", ticketData.DiscountCount); // nick add 20130318 for debug //
			int iDisctType = 0; // nick add 20130318 //
			unsigned long DisctPoint = 0; // nick add 20161020 Ver:000-000-GIO_V2-13B251-0005-13C241 //
			unsigned char POINT[4]; // nick add 20161020 Ver:000-000-GIO_V2-13B251-0005-13C241 //
			
			for(i = 0 ; i < (ticketData.DiscountCount) ; i++)
			{ //DiscountData   1 ~ 19 : 小時折扣使用
				// ================== //
				//nick mark s 20111219
				//DiscountData.TicketID = ticketData.ticketID;
				//DiscountData.AID = ticketData.discounts[i].AID;
				//DiscountData.VID = ticketData.discounts[i].VID;
				//DiscountData.SID = ticketData.discounts[i].SID;
				//nick mark e 20111219
				// =================== //
				
				//nick add s 20111219
				DiscountData[1 + i].DiscountTime = now;
				DiscountData[1 + i].TicketID = ticketData.ticketID;
				
				if(ticketData.TicketVer == 1)					//Frank add 20120912
				{					//Frank add 20120912
					DiscountData[1 + i].AID = ticketData.discounts[i].AID;
					DiscountData[1 + i].VID = ticketData.discounts[i].VID;
					DiscountData[1 + i].SID = ticketData.discounts[i].SID;
					
					if (ticketData.discounts[i].AID > 124) // nick add 20130318 //
						iDisctType = 1; // nick add 20130318 //
				//Frank add s 20120912
				}
				else if (ticketData.TicketVer == 2)
				{
					DiscountData[1 + i].AID = ticketData.Discounts2[i].AID;
					DiscountData[1 + i].VID = ticketData.Discounts2[i].VID;
					DiscountData[1 + i].SID = ticketData.Discounts2[i].SID;
					// nick mark 20161020 Ver:000-000-GIO_V2-13C241-0001-166241 //iDisctType = (ticketData.Discounts2[1 + i].DiscountHour[0] >> 4); // nick add 20130318 //
					iDisctType = (ticketData.Discounts2[i].DiscountHour[0] >> 4); // nick add 20161020 Ver:000-000-GIO_V2-13C241-0001-166241 //
				}

				sprintf(buf, "Disct [%d] in ticket, Type:[%d]", i, iDisctType);
				ShowMessage(buf);
				
				//Frank add e 20120912
				//nick add e 20111219
				
				//Frank mark 20120917 if(ticketData.discounts[i].AID > 124)
				// nick mark 20130318 //if((ticketData.discounts[i].AID > 124) || ((ticketData.Discounts2[i].DiscountHour[0] >> 4) == 1))//Frank add 20120917
				// nick mark 20130318 //if((ticketData.discounts[i].AID > 124) || ((ticketData.Discounts2[1 + i].DiscountHour[0] >> 4) == 1))//Frank add 20120917
				if (iDisctType == 1) // nick add 20130318 //
				{   //百分比折扣
					memcpy(POINT, ticketData.Discounts2[i].DiscountHour, sizeof(POINT));
					POINT[0] &= 0xF;
					DisctPoint = BCD2Ulong(POINT);
					
					//Frank mark 20120917 if(ticketData.discounts[i].DiscountHour == 100)
					// nick mark 20130318 //if((ticketData.discounts[i].DiscountHour == 100) || (ticketData.Discounts2[i].DiscountHour[2] == 1 && 
					// nick mark 20130318 //	ticketData.Discounts2[i].DiscountHour[3] == 0))//Frank add 20121009
					if((ticketData.discounts[i].DiscountHour == 100) || DisctPoint >= 100) // nick add 20130318 // // nick edit 20161020 Ver:000-000-GIO_V2-13C241-0001-166241 //
					{
						//nick mark 20111219 DiscountData.Type = 47;
						//nick mark 20111219 DiscountData.DiscountMinute = 0;
						DiscountData[1 + i].Type = 47;					//nick add 20111219
						DiscountData[1 + i].DiscountMinute = 0;		//nick add 20111219
						min = (now - EntryTime)/60;
						
						if( ((now - EntryTime)%60) >0)
							min = min + 1;
						
						//nick mark 20111219 DiscountData.RealDiscountMinute=min;
						//nick mark 20111219 WriteTempDiscountData(DiscountData);
						DiscountData[1 + i].bolIsDisct = true;				//nick add 20111219
						WriteTempDiscountData(DiscountData[1 + i]);		//nick add 20111219
						sprintf(buf,"Percent Discount 100");
						ShowMessage(buf);
						t_data.Disct_Type = DiscountData[1 + i].Type;		//nick add 20111220
						WriteTempPaymentData(t_data);					//nick add 20111220
						return true;
					}

					if(ticketData.TicketVer == 1)
						sprintf(buf,"Discount [%d] Percent",ticketData.discounts[i].DiscountHour);
					else
						sprintf(buf,"Discount [%ld] Percent", DisctPoint);
					
					ShowMessage(buf);
				}
				//Frank add s 20120917
				// nick mark 20130318 //else if((ticketData.Discounts2[i].DiscountHour[0] >> 4) == 2)
				// ========================================================= //
				// nick add s 20161020 Ver:000-000-GIO_V2-13C241-0001-166241 //
				else if (iDisctType == 2) // 100% Discount
				{
					DiscountData[1 + i].Type = 47;
					DiscountData[1 + i].DiscountMinute = 0;
					min = (now - EntryTime)/60;
					
					if( ((now - EntryTime)%60) >0)
						min = min + 1;

					DiscountData[1 + i].bolIsDisct = true;
					WriteTempDiscountData(DiscountData[1 + i]);
					sprintf(buf,"Percent Discount 100");
					ShowMessage(buf);
					t_data.Disct_Type = DiscountData[1 + i].Type;
					WriteTempPaymentData(t_data);
					return true;
				}
				// nick add e 20161020 Ver:000-000-GIO_V2-13C241-0001-166241 //
				// ========================================================= //
				else if(iDisctType == 3) // nick add 20130318 //
				{//變更費率
					ShowMessage((char *)"Change rate!");
					return false;
				}
				//Frank add e 20120917
				else
				{   //小時折扣
					//nick mark 20111219 if(bHasLimit == true)
					{	//有限時折扣,從限時的時間開始算
						//Frank mark if (LimitTime>EntryTime) EntryTime = LimitTime;	// 20110712 Nick add
					}
					
					//nick mark 20111220 long DiscountSecond = 0L;
					// nick mark 20130318 //if(TotalDiscountHour == G_ParkingConfig.MaxDiscountHours && G_ParkingConfig.MaxDiscountHours !=0) break;					//Frank add 20130128
					if(TotalDiscountHour >= G_ParkingConfig.MaxDiscountHours) break; // nick add 20130318 //
					
					if(ticketData.TicketVer == 1)					//Frank add 20120912
					{					//Frank add 20120912
						TotalDiscountHour += ticketData.discounts[i].DiscountHour;
						//Frank add s 20120912
					}
					else if (ticketData.TicketVer == 2)
					{
						//Frank add s 20120919
						//Frank mark 20130123if((ticketData.Discounts2[i].DiscountHour[0] >> 4) == 3)
						// nick mark 20130306 //if(G_ParkingConfig.MoneyRate != 1)					//Frank add 20130123
						if (G_ParkingConfig.MoneyRate > 1) // nick add 20130306 //
						{//金額折扣
							DiscountData[1 + i].Point += BCD2Ulong(ticketData.Discounts2[i].DiscountHour);					//Frank add 20130128
							
							if(G_ParkingConfig.FeeTime == 2 || G_ParkingConfig.FeeTime == 3)
							{
								//Frank mark 20130125 TotalDiscountHour += BCD2Ulong(ticketData.Discounts2[i].DiscountHour) / (G_ParkingConfig.MoneyRate * 2);
								TotalDiscountHour += (double)(DiscountData[1 + i].Point / (double)(G_ParkingConfig.MoneyRate * 2));//Frank add 20130128
							}
							else
							{
								//Frank mark 20130125 TotalDiscountHour += BCD2Ulong(ticketData.Discounts2[i].DiscountHour) / G_ParkingConfig.MoneyRate;
								TotalDiscountHour += (double)DiscountData[1 + i].Point / (double)G_ParkingConfig.MoneyRate;//Frank add 20130128
							}
							
							//Frank mark 20130125 DiscountData[1 + i].Point += BCD2Ulong(ticketData.Discounts2[i].DiscountHour);				//Frank add 20130114
						}
						//Frank add s 20130123
						else
						{
							TotalDiscountHour += BCD2Ulong(ticketData.Discounts2[i].DiscountHour);
						}
						//Frank add e 20130123
						
						printf("Discount Data[%d], AID:%03d, VID:%03d, SID:%03d, Point:%ld\n", i, DiscountData[1 + i].AID, DiscountData[1 + i].VID, DiscountData[1 + i].SID, DiscountData[1 + i].Point); // nick add 20130318 for debug //
						
						// ================ //
						// nick mark s 20130318 //
						//Frank add s 20130128
						//if(TotalDiscountHour >= G_ParkingConfig.MaxDiscountHours && G_ParkingConfig.MaxDiscountHours !=0)
						//{
						//	TotalDiscountHour = G_ParkingConfig.MaxDiscountHours;
						//}
						//Frank add e 20130128
						// nick mark e 20130318 //
						// ================ //
						
						//Frank mark 20130123 else if((ticketData.Discounts2[i].DiscountHour[0] >> 4) != 0)
						//Frank add e 20120919
						//Frank mark 20120919 if((ticketData.Discounts2[i].DiscountHour[0] >> 4) != 0)
						// nick mark 20141006 Ver:000-000-GIO_V2-135101-0007-13B251 //if((ticketData.Discounts2[i].DiscountHour[0] >> 4) != 0)					//Frank add 20130123
						if((iDisctType != 0)) // nick add 20141006 Ver:000-000-GIO_V2-135101-0007-13B251 //
						{
							sprintf(buf , "Discount type error : [%d] " , i);
							ShowMessage(buf);
							continue;
						}
						//Frank mark s 20130123
						/*else					//Frank add 20120919
						{					//Frank add 20121023
							TotalDiscountHour += BCD2Ulong(ticketData.Discounts2[i].DiscountHour);
						}					//Frank add 20121023*/
						//Frank mark e 20130123
						
					}
					else
					{ // unknow mifare type format
						continue;
					}
					//Frank add e 20120912

					// =============== //
					// nick add s 20130318 //
					if(TotalDiscountHour >= G_ParkingConfig.MaxDiscountHours)
					{
						TotalDiscountHour = G_ParkingConfig.MaxDiscountHours;
					}
					// nick add e 20130318 //
					// =============== //
					
					//nick mark 20111219 DiscountData.Type = 46;
					DiscountData[1 + i].Type = 46;				//nick add 20111219
					DiscountData[1 + i].bolIsDisct = true;		//nick add 20111219
					
					//nick mark 20111220 sprintf(buf,"Total Discount Hour:%d ",TotalDiscountHour);
					//Frank mark 20120914 sprintf(buf,"Total Discount Points:%d ",TotalDiscountHour);		//nick add 20111220
					//Frank mark 20130128 sprintf(buf , "Total Discount Points : %.0f" , TotalDiscountHour);					//Frank add 20120914
					sprintf(buf , "Total Discount Points : %.1f" , TotalDiscountHour);					//Frank add 20130128
					ShowMessage(buf);
					//nick add s 20110712
					//Frank mark 20120914 sprintf(buf, "DiscountSecond:%d; ParkSecond:%ld, AfterDiscountSec:%ld, FeeTime:%d", TotalDiscountHour * 3600, ParkSecond, (EntryTime + (TotalDiscountHour * 3600)), G_ParkingConfig.FeeTime);
					// Nick mark s 20170620 
					//sprintf(buf , "DiscountSecond:%.0f; ParkSecond:%lu, AfterDiscountSec:%.0f, FeeTime:%d" , TotalDiscountHour * 3600
					//			, ParkSecond , ((double)EntryTime + (TotalDiscountHour * 3600)) , G_ParkingConfig.FeeTime);//Frank add 20120914
					//ShowMessage(buf);
					// Nick mark e 20170620 
					//nick add e 20110712
					
					// ==================== //
					// nick mark s 20130318 //
					////Frank add s 20120206
					//min = ParkSecond /60;
					//
					//if((ParkSecond % 60) > 0)
					//	min = min + 1;
					////Frank add e 20120206
					// nick mark s 20130318 //
					// ==================== //
					
					// ==================== //
					//Frank mark s 20120206
					//if(G_ParkingConfig.FeeTime == 1)
					//{   //計費單位:1小時
					//	DiscountSecond = TotalDiscountHour *3600;
					//	DiscountData.DiscountMinute = DiscountSecond/60;
					//	min = (now - EntryTime)/60;
					//	if( ((now - EntryTime)%60) >0)
					//	min = min + 1;
					//	DiscountData.RealDiscountMinute=min;
					//	
					//	if( (EntryTime+DiscountSecond) > now)
					//	{
					//		sprintf(buf,"Discount Hour: %d",TotalDiscountHour);
					//		ShowMessage(buf);
					//		WriteTempDiscountData(DiscountData);
					//		return true;
					//	}
					//}
					//else if(G_ParkingConfig.FeeTime == 2 || G_ParkingConfig.FeeTime==3)
					//{   //計費單位:半小時
					//	DiscountSecond = TotalDiscountHour * 1800;
					//	DiscountData.DiscountMinute=DiscountSecond / 60;
					//	min = (now - EntryTime)/60;
					//	
					//	if(((now - EntryTime)%60) >0)
					//		min = min + 1;
					//	
					//	DiscountData.RealDiscountMinute=min;
					//	
					//	if( (EntryTime+DiscountSecond) > now)
					//	{
					//		sprintf(buf,"Discount half Hour: %d",TotalDiscountHour);
					//		ShowMessage(buf);
					//		WriteTempDiscountData(DiscountData);
					//		return true;
					//	}
					//}
					//Frank mark e 20120206
					// ==================== //
					
					//Frank add s 20120206
					if(G_ParkingConfig.FeeTime == 2 || G_ParkingConfig.FeeTime == 3)
					{   //計費單位:半小時
						DiscountSecond = TotalDiscountHour * 1800;
						DiscountMinute = TotalDiscountHour * 30;

						// ================ //
						// nick mark s 20130306 //
						////Frank add  s 20120914
						//if(ticketData.TicketVer == 1)
						//	DiscountData[1 + i].DiscountMinute = ticketData.discounts[i].DiscountHour * 30;
						//else if(ticketData.TicketVer == 2)
						//	DiscountData[1 + i].DiscountMinute = BCD2Ulong(ticketData.Discounts2[i].DiscountHour) * 30;
						////Frank add e 20120914
						// nick mark e 20130306 //
						// ================ //

						// =============== //
						// nick add s 20130306 //
						if(ticketData.TicketVer == 1)
							DiscountData[1 + i].DiscountMinute = (ticketData.discounts[i].DiscountHour / G_ParkingConfig.MoneyRate) * 30;
						else
							DiscountData[1 + i].DiscountMinute = (BCD2Ulong(ticketData.Discounts2[i].DiscountHour) / G_ParkingConfig.MoneyRate) * 30;
						// nick add e 20130306 //
						// =============== //
					}
					else
					{   //計費單位:1小時
						DiscountSecond = TotalDiscountHour * 3600;
						DiscountMinute = TotalDiscountHour * 60;

						// ================ //
						// nick mark s 20130306 //
						//Frank add  s 20120914
						//if(ticketData.TicketVer == 1)
						//	DiscountData[1 + i].DiscountMinute = ticketData.discounts[i].DiscountHour * 60;
						//else if(ticketData.TicketVer == 2)
						//	DiscountData[1 + i].DiscountMinute = BCD2Ulong(ticketData.Discounts2[i].DiscountHour) * 60;
						//Frank add  e 20120914
						// nick mark e 20130306 //
						// ================ //

						// =============== //
						// nick add s 20130306 //
						if(ticketData.TicketVer == 1)
							DiscountData[1 + i].DiscountMinute = (ticketData.discounts[i].DiscountHour / G_ParkingConfig.MoneyRate) * 60;
						else if(ticketData.TicketVer == 2)
							DiscountData[1 + i].DiscountMinute = (BCD2Ulong(ticketData.Discounts2[i].DiscountHour) / G_ParkingConfig.MoneyRate) * 60;
						// nick add e 20130306 //
						// =============== //
					}

					// Nick add s 20170620
					sprintf(buf , "DiscountSecond:%.0f; ParkSecond:%lu, AfterDiscountSec:%.0f, FeeTime:%d" , DiscountSecond
						, ParkSecond , ((double)EntryTime + DiscountSecond) , G_ParkingConfig.FeeTime);
					ShowMessage(buf);
					// Nick add e 20170620
					
					// nick mark 20130318 //if(DiscountSecond > ParkSecond)
					if (DiscountSecond >= ParkSecond) // nick add 20130318 //
					{
						//Frank mark 20120914 if((DiscountMinute - min) < DiscountData[1 + i].DiscountMinute)
						// nick mark 20130318 //if((DiscountMinute - min) < (unsigned long)DiscountData[1 + i].DiscountMinute)					//Frank add 20120914
						if ((unsigned long)DiscountData[1 + i].DiscountMinute >= (min - DiscountMinute)) // nick add 20130318 //
						{
							DiscountData[1 + i].RealDiscountMinute = min - TotalDiscountMinute;
							if (DiscountData[1 + i].RealDiscountMinute < 0) DiscountData[1 + i].RealDiscountMinute = 0; // nick add 20130318 //
						}
						else
						{
							DiscountData[1 + i].RealDiscountMinute = 0;
						}
					}
					else
					{
						DiscountData[1 + i].RealDiscountMinute = DiscountData[1 + i].DiscountMinute;
						// nick mark 20130318 //TotalDiscountMinute += DiscountData[1 + i].DiscountMinute;
					}
					
					TotalDiscountMinute += DiscountData[1 + i].DiscountMinute; // nick add 20130318 //
					//Frank add e 20120206
				}
			}
			
			//nick add s 20111220
			//Frank mark 20120914 if ((EntryTime+DiscountSecond) > now)
			if ((EntryTime + DiscountSecond) > (unsigned long)now)					//Frank add 20120914
			{
				for (int iDisctCnt = 0 ; iDisctCnt < 20 ; iDisctCnt++)
				{
					if (DiscountData[iDisctCnt].bolIsDisct)
					{
						if (t_data.Disct_Type <= 0) t_data.Disct_Type = DiscountData[iDisctCnt].Type;
						WriteTempDiscountData(DiscountData[iDisctCnt]);
					}
				}
				
				WriteTempPaymentData(t_data);
				return true;
			}
			//nick add e 20111220
		}
	}
/*	else if((unsigned int)ticketData.status == 2)
	{   //己付費 判斷出場緩衝
		if(((unsigned int)EntryTime + (G_ParkingConfig.PaidTime *60)) > (unsigned int)now)
		{
			sprintf(buf,"Paid:%4d%02d%02d %02d:%02d ", ticketData.in_year,ticketData.in_month,ticketData.in_day,
			ticketData.in_hour,ticketData.in_min);
			ShowMessage(buf);
			return true;
		}
		
		sprintf(buf,"Paid:%4d%02d%02d %02d:%02d  G_ParkingConfig.PaidTime:%d", ticketData.in_year,ticketData.in_month,ticketData.in_day,
			ticketData.in_hour,ticketData.in_min, G_ParkingConfig.PaidTime);
		ShowMessage(buf);
	}*/
	else if(ticketData.status == 4)
	{
		return true;
	}
	
	return false;
}

/*
//nick move to Datafile.cpp 20120110
bool CheckSeasonTicketPaid(TicketData ticketData)
{
	time_t now;
//	time_t OutLimit;
//	struct tm tm_OutLimit;
	char buf[256];
	
	memset(buf,'\0',sizeof(buf));
	// Get Time at the moment
	now = time((time_t *)0);
	
	return true;
}
//nick move to Datafile.cpp 20120110
*/

long CalculateFeeType0(TicketData ticketData)
{
	time_t now;
	time_t EntryTime; //,LimitTime;
	struct tm tm_EntryTime; //,tm_LimitTime,tm_now;
	long hour=0; //,min;
	long Fee=0;
	unsigned int min=0;
	
	// Get Time at moment
	now = time((time_t *)0);
	
	tm_EntryTime.tm_year = ticketData.in_year - 1900;
	tm_EntryTime.tm_mon  = ticketData.in_month - 1;
	tm_EntryTime.tm_mday = ticketData.in_day;
	tm_EntryTime.tm_hour = ticketData.in_hour;
	tm_EntryTime.tm_min  = ticketData.in_min;
	tm_EntryTime.tm_sec  = 59;
	EntryTime = mktime(&tm_EntryTime);
	
	if(G_ParkingConfig.FeeTime == 1)
	{	//每一小時計
		hour = (now-EntryTime)/3600;
		min  = ((now-EntryTime) % 3600) / 60;
		
		if(min > G_ParkingConfig.deviation) 
			hour++;
	}
	else if(G_ParkingConfig.FeeTime == 2)
	{	//每30分計
		hour = (now-EntryTime) / 1800;
		min  = ((now-EntryTime) % 1800) / 60;
		
		if(min > G_ParkingConfig.deviation) 
			hour++;
	}
	
	Fee = (hour * G_ParkingConfig.Fee);
	return Fee;
}

int CalculateFeeType1(TicketData ticketData)
{
	return 0;
}

int CheckValueFee(TicketData ticketData)
{
//	int i;
	int  Fee = 0;
//	bool bHasLimit = false;
//	long min = 0L;
	time_t now;
	time_t EntryTime; //,LimitTime;
	struct tm tm_EntryTime; //,tm_LimitTime,tm_now;
	
	// Get Time at the moment
	now = time((time_t *)0);
	
	tm_EntryTime.tm_year = ticketData.in_year - 1900;
	tm_EntryTime.tm_mon  = ticketData.in_month - 1;
	tm_EntryTime.tm_mday = ticketData.in_day;
	tm_EntryTime.tm_hour = ticketData.in_hour;
	tm_EntryTime.tm_min  = ticketData.in_min;
	tm_EntryTime.tm_sec  = 59;
	EntryTime = mktime(&tm_EntryTime);
	
	if(ticketData.status == 1)
	{   //未付費 判斷入場緩衝(免費)時間
		if(((unsigned int)EntryTime + (G_ParkingConfig.FreeTime * 60)) > (unsigned int)now)
		{
			return 0;
		}
	}
	
	if(G_ParkingConfig.FeeTime == 0)
	{
		Fee = G_ParkingConfig.Fee;
	}
	else
	{
		switch(G_ParkingConfig.FeeType)
		{
			default:
			case 0:
				Fee = CalculateFeeType0(ticketData);
				break;
			case 1:
				Fee = CalculateFeeType1(ticketData);
				break;
		}
	}
	
	return Fee;
}

bool ProcessHourlyOut(TicketData ticketData)
{	//Issue and write ticket Data to Ticket
	char buf[256];
	
	memset(buf,'\0',sizeof(buf));

	for (int i = 0; i < 16; i++) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		G_MemBlocksWriteSuc[i] = 0; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
	
	if(ticketData.status == 3)
	{	//檢查票卡入出場狀態
		AudioOn(AUDIO_REEXIT);
		ShowLCMFile(0,0,(char *)"AlreadyOut.lcm");
		ShowMessage((char *)"Already OUT !!");
		sprintf(buf, "Ticket ID:%ld", ticketData.ticketID); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
		SendAlarm(23, buf); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
		sleep(3); // nick add 20130202 //
		return false;
	}

	// ========================================================= //
	// nick add s 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	sprintf(buf,"Check BlackList.");
	ShowMessage(buf);
	
	// 檢查黑名單
	if(CheckBlackList(ticketData.ticketID))
	{
		AudioOn(AUDIO_INVTICKET);
		ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
		sprintf(buf,"TicketID:%ld is Black",ticketData.ticketID);
		ShowMessage(buf);
		SendAlarm(22,buf);
		return false;
	}
	// nick add e 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	// ========================================================= //
	
	// ========================================================== //
	// nick mark s 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	//if(ticketData.parkingId != G_ParkingConfig.ParkingID ||
	//	ticketData.areaId != G_ParkingConfig.AreaID)
	//{	// 檢查場區
	//	AudioOn(AUDIO_AREAERR);
	//	sprintf(buf,"out Area Error. TicketParkingID:%d TicketArea:%d ",
	//		ticketData.parkingId,ticketData.areaId);
	//	ShowLCMFile(0,0,(char *)"AreaError.lcm");
	//	ShowMessage(buf);
	//	sleep(3); // nick add 20130202 //
	//	return false;
	//}
	// nick mark e 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	// ========================================================== //
	
	// ========================================================= //
	// nick add s 20150821 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	if (strlen(G_FixConfigSetting.ch_Allow_Area) > 0)
	{
		sprintf(buf, "%d", ticketData.areaId);
		
		if(ticketData.parkingId != G_ParkingConfig.ParkingID ||
			strstr(G_FixConfigSetting.ch_Allow_Area, buf) == NULL)
		{	// 檢查場區
			AudioOn(AUDIO_AREAERR);
			sprintf(buf,"out Area Error. TicketParkingID:%d TicketArea:%d ",
				ticketData.parkingId,ticketData.areaId);
			ShowLCMFile(0,0,(char *)"AreaError.lcm");
			ShowMessage(buf);
			sleep(3); // nick add 20130202 //
			return false;
		}
	}
	else
	{
		if(ticketData.parkingId != G_ParkingConfig.ParkingID ||
			ticketData.areaId != G_ParkingConfig.AreaID)
		{	// 檢查場區
			AudioOn(AUDIO_AREAERR);
			sprintf(buf,"out Area Error. TicketParkingID:%d TicketArea:%d ",
				ticketData.parkingId,ticketData.areaId);
			ShowLCMFile(0,0,(char *)"AreaError.lcm");
			ShowMessage(buf);
			sleep(3); // nick add 20130202 //
			return false;
		}
	}
	
	// nick add e 20150821 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	// ========================================================= //	
	if(CheckHourlyTicketPaid(ticketData) == false)
	{	//檢查付費狀況
		AudioOn(AUDIO_NOPAY);
		sprintf(buf,"Over Time. Entry:%04d.%02d.%02d %02d:%02d",ticketData.in_year,ticketData.in_month,ticketData.in_day,
			ticketData.in_hour,ticketData.in_min);
		ShowMessage(buf);
		ShowLCMFile(0,0,(char *)"NoPay.lcm");
		sleep(3); // nick add 20130202 //
		return false;
	}
	
	return true;
}

bool ProcessSeasonCardOut(TicketData *ticketData)	// 20120208 Frank add
// 20120208 Frank mark bool ProcessSeasonCardOut(TicketData *outticketData)
//20110516 Tony mark bool ProcessSeasonCardOut(TicketData ticketData)
{	//check Season Ticket is valid and write in out status to Ticket
	int len,i,iRet=0;
	unsigned int Area;
	char buf[256];
	char area_code[10];
	
	//20110519 Tony add s
	struct tm *tm_ptr2 = NULL;
	time_t now;
	now = time((time_t *)0);
	tm_ptr2 = localtime(&now);
	
	//20120208 Frank mark TicketData ticketData;
	//20120208 Frank mark memcpy(&ticketData,outticketData,sizeof(TicketData));
	//20110519 Tony add e
	
	TicketData sticketData;	// 20120208 Frank add
	memcpy(&sticketData,ticketData,sizeof(sticketData));	// 20120208 Frank add
	
	sprintf(buf,"Season TicketID:%ld",ticketData->ticketID);
	ShowMessage(buf);
	
	// nick mark 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //if (strncmp(LastTagId,ticketData->TagID, 8) == 0)
	if (strncmp(LastTagId,ticketData->TagID, 8) == 0 && sticketData.ticketID > 0) // nick add 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	{
		ShowMessage((char *)"Last Re-Write");
		memcpy(ticketData, &LastTicketData, sizeof(TicketData)); // nick add 20130222 //
		return true;
	}

	for (int imyCnt = 0; imyCnt < 16; imyCnt++) // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
		G_MemBlocksWriteSuc[imyCnt] = 0; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //

	//20110511 Tony add s	
	//if((ticketData.ticketID % 10L) == 1)
	if(G_ParkingConfig.UpLayerID != 0)
	{
		sprintf(buf,"Ticket.ParkingID:%d, Cfg.ParkingID :%d  ;Ticket.Area:%d, Cfg.AreaID:%d",
			ticketData->parkingId, G_ParkingConfig.ParkingID, ticketData->areaId, G_ParkingConfig.AreaID);
		ShowMessage(buf);
		
		if(ticketData->parkingId != G_ParkingConfig.ParkingID)
		{
			AudioOn(AUDIO_AREAERR);
			// 20111205 Tony mark sprintf(buf,"out Area Error.");
			sprintf(buf,"Out Parking ID Error.");	// 20111205 Tony add
			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			ShowMessage(buf);
			return false;
		}
		
		// 20120113 Tony mark if((ticketData.ticketID % 10L) == 1)	// 20111205 Tony add
		// 20120113 Tony mark {										// 20111205 Tony add
		if(ticketData->areaId != G_ParkingConfig.AreaID)
		{	//判斷Ticket.AreaID和AreaID的值是否相同
			// 20120117 Tony add s
			if(ticketData->areaId == G_ParkingConfig.UpLayerID)
			{
				AudioOn(AUDIO_REEXIT);
				ShowMessage((char *)"Season card already Exit!");
				ShowLCMFile(0,0,(char *)"AlreadyOut.lcm");
				sprintf(buf, "Ticket ID:%ld", ticketData->ticketID); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
				SendAlarm(23, buf); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
			}
			else
			{
				// 20120117 Tony add e
    				AudioOn(AUDIO_AREAERR);
    				sprintf(buf,"Out Area Error.");
    				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
    				ShowMessage(buf);
			}                                                       // 20120117 Tony add
                	
			return false;
		}
		// 20120113 Tony mark  }										// 20111205 Tony add
		
		// 20111205 Tony add s
		if((ticketData->ticketID % 10L) == 2)
		{
			memset(area_code,'\0',sizeof(area_code));
			
			// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = CheckSeasonIsValid(sticketData, area_code);	// 20120208 Frank add
			iRet = CheckSeasonIsValid(&sticketData, area_code); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			// 20120208 Frank mark iRet = CheckSeasonIsValid(ticketData, area_code);
			sprintf(buf, "CheckSeasonIsValid() Result:[%d]", iRet);
			ShowMessage(buf);
			
			if( iRet < 1)
			{	//檢查使用期限
				if(iRet == -2)
				{
					if (ticketData->status != 2)
						sprintf(buf,"out TicketID:%ld is Expired!",ticketData->ticketID);
					else
					{
						iRet=1;
						goto Uplayer_Out_SeasonValid;
					}
				}
				else if(iRet == -1)
					sprintf(buf,"out TicketID:%ld not Start.",ticketData->ticketID);
				else if(iRet == 0)
					sprintf(buf,"out TicketID:%ld No Data.",ticketData->ticketID);
				//Frank add s 20120907
				else if(iRet == -5)
					sprintf(buf , "out TicketID : %ld Version error." , ticketData ->ticketID);
				//Frank add e 20120907
				
				AudioOn(AUDIO_SEASONEXPIRE);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				
				ShowMessage(buf);
				SendAlarm(32,buf);
				return false;
			}
			
	Uplayer_Out_SeasonValid:
			
			len = strlen(area_code);
			
			for( i=0; i<len; i++)
			{
				buf[0] = area_code[i];
				buf[1] = 0;
				Area = atoi(buf);
				
				if(Area == G_ParkingConfig.AreaID || Area == 9)
					break;
			}
			
			memset(buf,'\0',sizeof(buf));
			
			if(i == len)
			{
				AudioOn(AUDIO_AREAERR);
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //				sprintf(buf,"Area:%d, %d not Allow", ticketData->areaId,G_ParkingConfig.AreaID);
				sprintf(buf,"TicketID:%ld Area:%d, %d not Allow", ticketData->ticketID, ticketData->areaId,G_ParkingConfig.AreaID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
				ShowMessage(buf);
				SendAlarm(60,buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}
			
			if(ticketData->status == 4)
			{
				ShowMessage((char *)"New Season Card");
				return true;
			}
			
			if(iRet == 1 && ticketData->status == 3)
			{
				AudioOn(AUDIO_REEXIT);
				ShowMessage((char *)"Season card already Exit!");
				ShowLCMFile(0,0,(char *)"AlreadyOut.lcm");
				sprintf(buf, "Ticket ID:%ld", ticketData->ticketID); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
				SendAlarm(23, buf); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
				return false;
			}
		}
		// 20111205 Tony add e
		
		sprintf(buf,"Check BlackList.");
		ShowMessage(buf);
		
		if(CheckBlackList(ticketData->ticketID))
		{
			AudioOn(AUDIO_INVTICKET);
			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			sprintf(buf,"TicketID:%ld is Black",ticketData->ticketID);
			ShowMessage(buf);
			SendAlarm(22,buf);
			return false;
		}
		
		sprintf(buf,"Get Staytime & Optime start");
		ShowMessage(buf);
		
		time_t tm_Outtime;
		//time_t now,tm_Outtime;
		double TmpSec;
		unsigned long ulOptime;
		struct tm tm_ptr1;
		//struct tm *tm_ptr2 = NULL;
		
		ulOptime = (unsigned long)(BCD2Int((unsigned char*)ticketData->optime,3));	// 20111205 Tony add
		printf("ulOptime : %ld \n",ulOptime);										// 20111205 Tony add
		
		tm_ptr1.tm_year = ticketData->in_year - 1900;
		tm_ptr1.tm_mon = ticketData->in_month - 1;
		tm_ptr1.tm_mday = ticketData->in_day;
		tm_ptr1.tm_hour = ticketData->in_hour;
		tm_ptr1.tm_min = ticketData->in_min;
		tm_ptr1.tm_sec = ticketData->in_sec;
		
		tm_Outtime=mktime(&tm_ptr1);
		
		sprintf(buf,"In time : %s",asctime(&tm_ptr1));
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		//now    = time((time_t *)0);
		//tm_ptr2 = localtime(&now);
		
		//ticketData.in_year  = tm_ptr2->tm_year + 1900;
		//ticketData.in_month = tm_ptr2->tm_mon + 1;
		//ticketData.in_day	 = tm_ptr2->tm_mday;
		//ticketData.in_hour  = tm_ptr2->tm_hour;
		//ticketData.in_min	 = tm_ptr2->tm_min;
		//ticketData.in_sec	 = tm_ptr2->tm_sec;
		
		sprintf(buf,"Now time : %s",asctime(tm_ptr2));
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		TmpSec = difftime(now,tm_Outtime);
		
		sprintf(buf,"Diff time : %lf",TmpSec);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		sprintf(buf,"Before staytime : %ld",ticketData->staytime);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		if((ticketData->ticketID % 10L) == 1)			// 20111205 Tony add
		{												// 20111205 Tony add
			if ((TmpSec / 60) > G_ParkingConfig.FreeTime)
			{
				ticketData->staytime += (unsigned long)TmpSec;
				sprintf(buf,"Intime > Freetime : %ld",(unsigned long)(TmpSec / 60));
				ShowMessage(buf);
				
				/* 20111202 Tony mark s
				ticketData.in_year  = tm_ptr2->tm_year + 1900;
				ticketData.in_month = tm_ptr2->tm_mon + 1;
				ticketData.in_day	 = tm_ptr2->tm_mday;
				ticketData.in_hour  = tm_ptr2->tm_hour;
				ticketData.in_min	 = tm_ptr2->tm_min;
				ticketData.in_sec	 = tm_ptr2->tm_sec;
				20111202 Tony mark e*/
			}
		}												// 20111205 Tony add 
		
		// 20111202 Tony add s
		/* 20120208 Frank mark
		ticketData.in_year  = tm_ptr2->tm_year + 1900;
		ticketData.in_month = tm_ptr2->tm_mon + 1;
		ticketData.in_day	 = tm_ptr2->tm_mday;
		ticketData.in_hour  = tm_ptr2->tm_hour;
		ticketData.in_min	 = tm_ptr2->tm_min;
		ticketData.in_sec	 = tm_ptr2->tm_sec;
		*/
		// 20111202 Tony add e
		
		sprintf(buf,"After staytime : %ld",ticketData->staytime);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		// 20111205 Tony mark ulOptime=(unsigned long)(TmpSec / 60);
		ulOptime += ((unsigned long)(TmpSec / 60));	// 20111205 Tony add
		sprintf(buf,"Optime : %ld",ulOptime);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		Int2BCD(ulOptime,6,(unsigned char *)ticketData->optime);	// 20120208 Frank add
		// 20120208 Frank mark Int2BCD(ulOptime,6,(unsigned char *)&ticketData.optime);
		
		sprintf(buf,"Get Staytime & Optime End");
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		// 20120130 Tony mark ticketData.areaId = G_ParkingConfig.UpLayerID;
	}
	else
	{
	//20110511 Tony add e	
		if(G_ParkingConfig.SeasonReadTag == 0)
		{
			// compare area
			if(G_ParkingConfig.ParkingID > 0 || G_ParkingConfig.AreaID > 0)
			{
				if(ticketData->parkingId != G_ParkingConfig.ParkingID)	//20110519 Tony add
				// 20110519 Tony mark if(ticketData.parkingId != G_ParkingConfig.ParkingID || ticketData.areaId != G_ParkingConfig.AreaID)
				{	//場區錯誤
					AudioOn(AUDIO_AREAERR);
					// 20111205 Tony mark sprintf(buf,"out Area Error. ParkingID:%d %d Area:%d %d",
					sprintf(buf,"Parking ID Error. ParkingID:%d %d Area:%d %d",	// 20111205 Tony add
						ticketData->parkingId,G_ParkingConfig.ParkingID,ticketData->areaId,G_ParkingConfig.AreaID);
					ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
					ShowMessage(buf);
					return false;
				}
				
				// 20110519 Tony add s
				/*	20111205 Tony mark
				if(ticketData.areaId != G_ParkingConfig.AreaID)
				{	//場區錯誤
					AudioOn(AUDIO_AREAERR);
					sprintf(buf,"Out Area Error. ParkingID:%d %d Area:%d %d",
						ticketData.parkingId,G_ParkingConfig.ParkingID,ticketData.areaId,G_ParkingConfig.AreaID);
					ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
					ShowMessage(buf);
					return false;
				}
				*/
				// 20110519 Tony add e
			}
			
			//printf("check black\n");
			
			if(CheckBlackList(ticketData->ticketID))
			{
				AudioOn(AUDIO_SEASONEXPIRE);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				sprintf(buf,"TicketID:%ld is Black",ticketData->ticketID);
				ShowMessage(buf);
				SendAlarm(22,buf);
				return false;
			}
			
			//20110518 Tony add s
			/*	20111205 Tony mark
			if (ticketData.staytime > 0)
			{
				AudioOn(AUDIO_NOPAY);
				ShowLCMFile(0,0,(char *)"NoPay.lcm");
				return false;
			}
			*/
			//20110518 Tony add e
			
			// Get season data from DB
			memset(area_code,'\0',sizeof(area_code));
			//printf("check invalid\n");
			
	//nick mark 20110209		iRet = CheckSeasonIsValid(ticketData.ticketID,area_code);
			// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = CheckSeasonIsValid(sticketData, area_code);	// 20120208 Frank add
			iRet = CheckSeasonIsValid(&sticketData, area_code); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			// 20120208 Frank mark iRet = CheckSeasonIsValid(ticketData, area_code);	//nick add 20110209
			sprintf(buf, "CheckSeasonIsValid() Result:[%d]", iRet);
			ShowMessage(buf);
			
			if( iRet < 1)
			{	//檢查使用期限
	//nick mark 20110208			AudioOn(AUDIO_SEASONEXPIRE);
	//nick mark 20110208			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				
				if(iRet == -2)
				{
	//nick mark 20110208				sprintf(buf,"out TicketID:%ld is Expired!",ticketData.ticketID);
					//nick add s 20110208
					if (ticketData->status != 2)
						sprintf(buf,"out TicketID:%ld is Expired!",ticketData->ticketID);
					else
					{
						iRet=1;
						goto Out_SeasonValid;
					}
					//nick add e 20110208
				}
				//Frank add s 20120628
				else if(iRet == -3)
				{
					if((ticketData ->ticketID % 10L) == 3)
					{
						int ValueRet;
						
						ValueRet = CheckValueTicketPaid(ticketData);
						
						if(ValueRet < 1)
						{
							if(ValueRet == -1)
							{
								AudioOn(AUDIO_VALUE_LACK);
								sprintf(buf , "Value is not Enough! TicketID:%ld value:%ld " , ticketData->ticketID , ticketData->value);
								ShowMessage(buf);
								ShowLCMFile(0 , 0 , (char *)"NoPay.lcm");
								SendAlarm(58 , buf);
							}
							else
								ShowMessage((char *)"Value can't exit!");
							
							return false;
						}
						
						goto Out_SeasonValid;
					}
					else
						//Frank mark 20120709 sprintf(buf , "out TicketID : %ld not In Park Time." , ticketData->ticketID);
					//Frank add s 20120709
					{
						AudioOn(AUDIO_NOPAY);
						sprintf(buf , "NoPay TicketID:%ld is not in park time." , ticketData->ticketID);
						ShowMessage(buf);
						ShowLCMFile(0 , 0 , (char *)"NoPay.lcm");
						SendAlarm(55 , buf);
						return false;
					}
					//Frank add e 20120709
				}
				//Frank add e 20120628
				//nick add s 20110214
				/* 20111205 Tony mark s
				else if(iRet == -3)
				{
					AudioOn(AUDIO_NOPAY);
					sprintf(buf,"Over Time. Entry:%04d.%02d.%02d %02d:%02d",ticketData.in_year,ticketData.in_month,ticketData.in_day,
						ticketData.in_hour,ticketData.in_min);
					ShowMessage(buf);
					ShowLCMFile(0,0,(char *)"NoPay.lcm");
					return false;
				}
				20111205 Tony mark e*/
				//nick add e 20110214
				else if(iRet == -1)
					sprintf(buf,"out TicketID:%ld not Start.",ticketData->ticketID);
				else if(iRet == 0)
					sprintf(buf,"out TicketID:%ld No Data.",ticketData->ticketID);
				//Frank add s 20120907
				else if(iRet == -5)
					sprintf(buf , "out TicketID : %ld Version error." , ticketData ->ticketID);
				//Frank add e 20120907
				
				AudioOn(AUDIO_SEASONEXPIRE);						//nick add 20110208
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");		//nick add 20110208
				
				ShowMessage(buf);
				SendAlarm(32,buf);
				return false;
			}
			
	Out_SeasonValid:
			
			len = strlen(area_code);
			
			for(i = 0; i < len; i++)
			{
				buf[0] = area_code[i];
				buf[1] = 0;
				Area = atoi(buf);
				
				if(Area == G_ParkingConfig.AreaID || Area == 9)
					break;
				//else
				//	printf("Area:%d not there",atoi(buf));
			}

			sprintf(buf, "%d", ticketData->areaId); // nick add 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
			
			if (strlen(G_FixConfigSetting.ch_Allow_Area) > 0 && strstr(G_FixConfigSetting.ch_Allow_Area, buf) != NULL) // nick add 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
				i = 0; // nick add 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
			
			memset(buf,'\0',sizeof(buf));
			
			if(i == len)
			{	//AREA 未定義
				AudioOn(AUDIO_AREAERR);
				//Frank mark 20120709 sprintf(buf,"Area:%d, %d not Allow", ticketData->areaId,G_ParkingConfig.AreaID);
				// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //sprintf(buf , "Area:%d, %d not Allow TicketID:%ld" , ticketData->areaId , G_ParkingConfig.AreaID , ticketData->ticketID);
				sprintf(buf , "TicketID:%ld Area:%d, %d not Allow TicketID:%ld" , ticketData->ticketID, ticketData->areaId , G_ParkingConfig.AreaID , ticketData->ticketID); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
				ShowMessage(buf);
				SendAlarm(60, buf);
				ShowLCMFile(0,0,(char *)"AreaError.lcm");
				return false;
			}
			
			if(ticketData->status == 4)
			{
				ShowMessage((char *)"New Season Card");
				return true;
			}
					
			if(iRet == 1 && ticketData->status == 3)
			{
				AudioOn(AUDIO_REEXIT);
				ShowMessage((char *)"Season card already Exit!");
				ShowLCMFile(0,0,(char *)"AlreadyOut.lcm");
				sprintf(buf, "Ticket ID:%ld", ticketData->ticketID); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
				SendAlarm(23, buf); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
				return false;
			}
			
			/* 改到判斷完 車牌才做
			if((iRet=WriteCarOutData(&ticketData,false,true)) != 1)
			{	//寫入進出狀態
				AudioOn(AUDIO_READERROR);
				sprintf(buf,"Out Write Season Ticket Error %d",iRet);
				ShowMessage(buf);
				ShowLCMFile(0,0,"TicketReadError.lcm");
				SendAlarm(61,buf);
				return false;
			}*/
		}
		else
		{	//Tag  判斷方式
	//nick mark 20110209 		iRet = CheckSeasonIsValid(ticketData.ticketID,area_code,ticketData.TagID);
			// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = CheckSeasonIsValid(sticketData, area_code);	// 20120208 Frank add
			iRet = CheckSeasonIsValid(&sticketData, area_code); // nick add 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			// 20120208 Frank mark iRet = CheckSeasonIsValid(ticketData, area_code);	//nick add 20110209
			
			if(iRet < 1)
			{	//檢查使用期限
				AudioOn(AUDIO_SEASONEXPIRE);
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
				
				if(iRet == -2)
					sprintf(buf,"TicketID:%ld is Expired!",ticketData->ticketID);
				else if(iRet == -1)
					sprintf(buf,"TicketID:%ld not Start.",ticketData->ticketID);
				else if(iRet == 0)
					sprintf(buf,"TicketID:%ld No Data.",ticketData->ticketID);
				//Frank add s 20120907
				else if(iRet == -5)
					sprintf(buf , "TicketID : %ld Version error." , ticketData ->ticketID);
				//Frank add e 20120907
				
				ShowMessage(buf);
				SendAlarm(32,buf);
				return false;
			}
		}
		
	//20110519 Tony add s
		/* 20120208 Frank mark 
		ticketData.in_year  = tm_ptr2->tm_year + 1900;
		ticketData.in_month = tm_ptr2->tm_mon + 1;
		ticketData.in_day	 = tm_ptr2->tm_mday;
		ticketData.in_hour  = tm_ptr2->tm_hour;
		ticketData.in_min	 = tm_ptr2->tm_min;
		ticketData.in_sec	 = tm_ptr2->tm_sec;
		*/
	}	
	//20110519 Tony add e	
	tm_ptr2 = localtime(&now);					//Frank add 20120724
	//Frank add s 20120208
	ticketData->in_year  = tm_ptr2->tm_year + 1900;
	ticketData->in_month = tm_ptr2->tm_mon + 1;
	ticketData->in_day = tm_ptr2->tm_mday;
	ticketData->in_hour  = tm_ptr2->tm_hour;
	ticketData->in_min = tm_ptr2->tm_min;
	ticketData->in_sec = tm_ptr2->tm_sec;
	//Frank add e 20120208
	
	ticketData->areaId = G_ParkingConfig.UpLayerID; // 20120130 Tony add
	// 20120208 Frank mark memcpy(outticketData,&ticketData,sizeof(TicketData));	// 20110511 Tony add
	
	sprintf(buf,"ProcessSeasonCardOUT End:%04d.%02d.%02d %02d:%02d:%02d" , ticketData->in_year , ticketData->in_month ,
		ticketData->in_day , ticketData->in_hour , ticketData->in_min , ticketData->in_sec);					//Frank add 20120208
	ShowMessage(buf);						//Frank add 20120208
	
	return true;
}

bool ProcessEasyCardOut(TicketData ticketData)
{	//reserved
	return true;
}

bool ProcessValueCardOut(TicketData *ticketData)
{	//check Value Ticket is valid and write in out status to Ticket
	char buf[256];
	unsigned long Fee=0L;
	
	sprintf(buf,"Value TicketID:%ld",ticketData->ticketID);
	ShowMessage(buf);
	
	if(CheckBlackList(ticketData->ticketID))
	{
		AudioOn(AUDIO_SEASONEXPIRE);
		ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
		sprintf(buf,"TicketID:%ld is Black",ticketData->ticketID);
		ShowMessage(buf);
		SendAlarm(22,buf);
		return false;
	}
	
	// compare area
	if(G_ParkingConfig.ParkingID > 0 || G_ParkingConfig.AreaID > 0)
	{
		if(ticketData->parkingId != G_ParkingConfig.ParkingID ||
			ticketData->areaId != G_ParkingConfig.AreaID)
		{	//場區錯誤
			AudioOn(AUDIO_AREAERR);
			sprintf(buf,"out Parking Area Error ParkingID:%d %d Area:%d %d",
			ticketData->parkingId,G_ParkingConfig.ParkingID,ticketData->areaId,G_ParkingConfig.AreaID);
			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			ShowMessage(buf);
			return false;
		}
	}
	
	if(ticketData->status == 3)
	{
		AudioOn(AUDIO_REEXIT);
		ShowMessage((char *)"Value card already Exit!");
		ShowLCMFile(0,0,(char *)"AlreadyOut.lcm");
		sprintf(buf, "Ticket ID:%ld", ticketData->ticketID); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
		SendAlarm(23, buf); // nick add 20130918 Ver:000-000-GIO_V2-133181-0100-135101 //
		return false;
	}
	
	// Check value is enough
	Fee = CheckValueFee(*ticketData);
	
	if(Fee > ticketData->value)
	{
		AudioOn(AUDIO_VALUE_LACK);
		ShowMessage((char *)"Out Value not Enough !");
		ShowLCMFile(0,0,(char *)"NoPay.lcm");
		return false;
	}
	
	ticketData->value -= Fee;
	return true;
}

int MainProcessIn()
{   //return 0:沒入車 1:有入車 -1:故障
	int iRet=0,i;
	//Frank mark 20120904 int waitTime =30;
	int waitTime = 3;					//Frank add 20120904
	bool bRecvedPlate = false;
	bool bNoTicket=false;
	bool bNoCheckPlate=false;
	bool bOpenBr = false;
	bool bCarIn = false;
	bool bBlackTickt = false;
	bool alarm = true;					//Frank add 20111020
	bool AudioOnFlag = true;			//Frank add 20111020
	char Plate[16],buf[256];
	unsigned long Ticks = 1;					//Frank add 20120904
	TicketData ticketData;
	ThirdPartyTicketData thirdPartyTicketData;
	enum TicketType ticketType = NONE_TICKET;
	HTicketData hTicketData;
	int timeout = 0;
	int iRetryCnt = 0; // nick add 20150523 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	int res = 0; // nick add 20150523 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	
	time_t now=0;
	static time_t in_lastTime = 0,QuietTime = 0;
	static bool bSentVISTrigger = false; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	
	memset(&ticketData, 0, sizeof(TicketData)); // nick add 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
	memset(&thirdPartyTicketData, 0, sizeof(thirdPartyTicketData));	// Tony add 20170310 
	
	//Frank add s 20121225
	//if((G_ParkingStatus.status & STATUS_READER_CONNECT) == 0)
		//iRet = -1;
	//Frank add e 20121225

	//if(GetLoop1() == false && G_RFin == false)
	if(GetLoop1() == false && G_RFin == false && G_NewClientCmdQue.empty() == true)
	{        
		//nick mark 20120223 iRunTime++;
		return iRet;
	}
	
	if(G_RFin == true)
	{
		ShowMessage((char *)"--- Detect RF in! (no loop1) ---");
	}
	else
	{
		ShowMessage((char *)"--- Detect Loop1! ---");
		now = time((time_t *)0);
		in_lastTime = now;
	}
	
	memset(buf,'\0',sizeof(buf));
	memset(&ticketData,'\0',sizeof(TicketData));
	
	DrawChipCoin(false); // 關閉票卡位置Shutter // nick add 20140929 Ver:000-000-GIO_V2-135101-0006-13B251 //

	// ========================================================= //
	// nick add s 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	if (GetLoop1() && bSentVISTrigger == false)
	{
		bSentVISTrigger = true;

		if (td_SendVisSoftTrigger != 0)
		{
			pthread_cancel(td_SendVisSoftTrigger);
			td_SendVisSoftTrigger = 0;
		}
		
		for (iRetryCnt = 0; iRetryCnt < 3; iRetryCnt++)
		{
			res = pthread_create(&td_SendVisSoftTrigger, NULL, SendSoftTrigger2Vis, (void*)NULL);
			
			if(res == 0)
				break;
		}
		
		if (iRetryCnt >= 3)
			ShowMessage((char *)"VIS Software Trigger Thread Create Error.");
	}
	// nick add e 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	// ========================================================= //

	
	//while(GetLoop1() == true || G_RFin == true)
	while(GetLoop1() == true || G_RFin == true || G_NewClientCmdQue.empty() == false)
	{
		//SendLoop1Status(true);
		iRunTime = GetTickCount();      //nick add 20120223
		
		if(G_iFullLevel == 2)
		{ // 全場滿車 //
			if (AudioOnFlag == true)					//Frank add s 20111020
			{
				DisplayOn(true);					//Frank add 20111104
				ShowMessage((char *)"Parking is FULL!");
				AudioOn(AUDIO_PFULL);
				VoiceOn(VOICE_PFULL);    /// 車位己滿
				AudioOnFlag = false;
			}						//Frank add e 20111020
			
			ShowLCMFile(0,0,(char *)"ParkingFull.lcm");
			// nick mark 20110421 //sleep(3);
			
			//nick add s 20110421
			// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long FullSTick = GetTickCount();
			unsigned long FullSTick = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
			
			do
			{				
				IdleProcess();
				usleep(10000L);
			}while(G_iFullLevel == 2 && (CheckTimeout(&FullSTick, (double)3000) == false));
			//nick add e 20110421
			
			continue;
		}
		
		//Display
		if((timeout % 300) == 0 && G_RFin == false)
		{
			if((G_ParkingStatus.status & STATUS_TICKET_EMPTY) != 0)
			{
				if (bNoTicket==false) SendAlarm(2, (char *)"");		//nick add 20110415
				bNoTicket = true;
			}
			
			EasyCardEnable(true);
			DisplayOn(true);
			
			if(bNoTicket)
			{
				ShowLCMFile(0,0,(char *)"TicketOver.lcm"); //票卡用完
			}
			else
			{
				//Frank add s 20130123
				if((G_ParkingConfig.Hourly_Use == 0) && (G_ParkingConfig.Season_Use == 0))
				{
					AudioOn(AUDIO_STOPSERVICE);
					ShowLCMFile(0,0,(char *)"StopService.lcm");

					if (G_bStopService == false) // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
						ShowMessage((char *)"Hourly_Use = 0 and Season_Use = 0");
					
					G_bStopService = true;
					return iRet;
				}
				else if(G_ParkingConfig.Hourly_Use == 0)
				{
					AudioOn(AUDIO_INSERT);  //請插入票卡
					VoiceOn(VOICE_INSERT);  //請插入票卡
					ShowLCMFile(0,0,(char *)"InsertTicket.lcm");
					
					ShowMessage((char *)"--- Please Insert Ticket.(Hourly_Use = 0) ---");
				}
				else
				{
					// ========================================================== //
					// nick mark s 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
					//if(G_iFullLevel == 0 && bNoTicket == false)
					//{
					//	AudioOn(AUDIO_WELCOME);  //請按鈕取票
					//	VoiceOn(VOICE_WELCOME);  //請按鈕取票
					//}
					//
					//ShowLCMFile(0,0,(char *)"PushButton.lcm");
					// nick mark e 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
					// ========================================================== //
					
					// ========================================================= //
					// nick add s 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
					if (G_iFullLevel == 0)
					{ // 未滿車可取票 //
						AudioOn(AUDIO_WELCOME);  //請按鈕取票
						VoiceOn(VOICE_WELCOME);  //請按鈕取票
						ShowLCMFile(0, 0, (char *)"PushButton.lcm");
					}
					else if(G_iFullLevel == 1)
					{ // 計時票滿車 //
						AudioOn(AUDIO_HOURLYFULL);
						VoiceOn(VOICE_PFULL);    /// 計時車車位己滿
						ShowLCMFile(0, 0, (char *)"HourlyParkingFull.lcm");
						IdleProcess(); // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //						
						usleep(10); // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
						// nick mark 20160120 Ver:000-000-GIO_V2-13B251-0007-13C241 (2-Output) //continue; // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
					}
					// nick add e 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
					// ========================================================= //
				}
			}
		}
		
		//nick mark 20120223 iRunTime = 0;
		timeout++;
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		
		if(alarm == true)					//Frank add s 20111020
		{
			now = time((time_t *)0);
			QuietTime = now - in_lastTime ;
		
			if((unsigned int)QuietTime >= G_ParkingConfig.WaitTime.Button_Timeout && G_iFullLevel == 0) // nick add 20150409 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			{   //超過時間未動作
				//Frank mark 20121225 sprintf(buf,"Wait Push button too long:%d",(unsigned int)QuietTime);
				sprintf(buf , "Wait Push button too long:%d" , G_ParkingConfig.WaitTime.Button_Timeout);					//Frank add 20121225
				
				ShowMessage(buf);
				SendAlarm(4,buf);
				
				in_lastTime = now;
				QuietTime = 0;
				alarm = false;
			}
			else
			{
				//sprintf(buf,"Wait  Time:%d ,QuietTime:%d",QuietTime,G_ParkingConfig.QuietTime);
				//printf("%s \n",buf);
			}
		}					//Frank add e 20111020

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		LCM_ShowTime();
		ticketType = DetectUserCardType(&ticketData);

		// New Terminal Passage
		if(ticketType == NONE_TICKET)
		{
			memset(&thirdPartyTicketData, 0, sizeof(thirdPartyTicketData));
			ticketType = IdleProcessForNewTerminel(&thirdPartyTicketData);

			if((ticketType == ThirdParty_Ticket) || (ticketType == ThirdParty_Ticket_IssueHS))
			{
				ShowMessage((char *)"Is ThirdParty_Ticket.");
				MakeThirdPartyTicketData(&ticketData,thirdPartyTicketData);

				// 不需發卡
				if (ticketType == ThirdParty_Ticket)
				{
					break;
				}

				// Display 票卡處理中
				VoiceOn(VOICE_PROCESSING);			  
				AudioOn(AUDIO_PROCESSING);
				ShowLCMFile(0,0,(char *)"TicketProcess.lcm");	
									
				if(CheckTicketIssue() == 1)
				{
					ShowMessage((char *)"New terminal :: issue ticket success!");
					break;
				}
				
				if(IssueTicket() == 1)
				{   
					ShowMessage((char *)"New terminal :: issue ticket success!");
					break;				
				}
				
				// 發卡失敗
				ShowMessage((char *)"New terminal :: issue ticket failed !");
				SendCarEnterStart(ticketType);
				usleep(50000L);
				SendCarEnterStatus(2);				
				ticketType = NONE_TICKET;
			}
		}
		
		if(ticketType == NONE_TICKET)
		{
			// 檢查Server指令及手動開柵欄
			IdleProcess();
		}
		else if(ticketType == UNKNOW_TICKET || ticketType == HOURLY_S)
		{
			usleep(50000L);
			//Frank add s 20120508
			if(ReaderCFG.HourlyReaderType == 6)
			{
				usleep(500000L);
				RejectTicket();
				AudioOn(AUDIO_TAKETICKET);
				VoiceOn(VOICE_TAKETICKET);  //語音:請取票
				WaitTakeTicket(ticketType);
				G_ButtonPress = false; // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
				return iRet;
			}
			//Frank add e 20120508
		}
		else
		{
			break;
		}
		
		if(GetLoop1() == false)
		{
			RecycleTicket(true);   //回收票卡
			G_ParkingStatus.Out_Retrieved ++;
			SaveReTicket(G_ParkingStatus.Out_Retrieved);
			LCM_FillScreen(0x00);
			
			G_ButtonPress = false;					//Frank add 20121227
			bSentVISTrigger = false; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
			ShowMessage((char *)"Car Leave.");
			ShowLCMFile(0,0,(char *)"logo.lcm");
			return iRet;
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	AudioOnFlag = true;					//Frank add 20111020
	G_ButtonPress = false; // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
	
	if(ticketType == NONE_TICKET)
	{
		LCM_FillScreen(0x00);
		ShowLCMFile(0,0,(char *)"logo.lcm");
		return iRet;
	}
	
	SendCarEnterStart(ticketType);

	if(ticketType == ThirdParty_Ticket || ticketType == ThirdParty_Ticket_IssueHS)
	{
		bNoCheckPlate = true;
		goto ThirdPartyTicket;
	}
	
	//Frank add s 20120604
	if(ticketType == SEASON_TICKET || ticketType == VALUE_TICKET)
	{
		if((ticketData.ticketID % 10L) == 2 || (ticketData.ticketID % 10L) == 3 ||(ticketData.ticketID % 10L) == 5
			|| (G_ParkingConfig.UpLayerID!= 0 && ReaderCFG.HourlyReaderType == 0))
		{
			ShowMessage((char *)"Process Season.");
			bOpenBr = ProcessSeasonCardIn(&ticketData);
			bRecvedPlate = true;					//Frank add 20120830
			//Frank add s 20120904
			if(G_ParkingConfig.VISforSeason == false)
			{
				if(strlen(G_ParkingConfig.PlateServerIP) > 7  && ticketType != PARKTRON_CARD)					//Frank add 20130118
					ClearVIS();
				goto NO_VIS;
			}
			//Frank add e 20120904
		}
		else
		{
			ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			bOpenBr = false;
		}
		
		if (bOpenBr == false)					//Frank add 20120814
			goto SEASON_Noopen;					//Frank add 20120814
	}
	//Frank add e 20120604
	//Frank add s 20120904
	// nick mark 20150209 Ver:000-000-GIO_V2-13B251-0001-13C241 //else if((G_ParkingConfig.VISforHourly == false) && (ticketType == HOURLY_TICKET))
	// nick mark 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //else if ((G_ParkingConfig.VISforHourly == false && ticketType == HOURLY_TICKET) || ticketType == EASY_CARD) // nick add 20150209 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	else if ((G_ParkingConfig.VISforHourly == false && ticketType == HOURLY_TICKET) || ticketType == EASY_CARD || ticketType == POS_IN) // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	{
		// nick mark 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(strlen(G_ParkingConfig.PlateServerIP) > 7  && ticketType != PARKTRON_CARD)					//Frank add 20130118
		if(strlen(G_ParkingConfig.PlateServerIP) > 7) // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			ClearVIS();
		
		bNoCheckPlate = true; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		goto NO_VIS;
	}
	//Frank add e 20120904

	if(strlen(G_ParkingConfig.PlateServerIP) > 7  && ticketType != PARKTRON_CARD) //有設定車牌辨識Server
	//if(strlen(G_ParkingConfig.PlateServerIP) > 7  && ticketType != PARKTRON_CARD) //有設定車牌辨識Server
	{	//取得車牌號碼
		char iPlateRet = 0;		
		
		memset(Plate,'\0',sizeof(Plate));
		
		//Frank add s 20120806
		if(ticketType == SEASON_TICKET || ticketType == VALUE_TICKET)
		{
			//Frank mark 20120813 char qSQL[SQLLength + 1] , SeasonPlate[10] , *errMsg = NULL , **result;
			char qSQL[SQLLength + 1] , *errMsg = NULL , **result;					//Frank add 20120813
			sqlite3 *db = NULL;
			int rows , cols , len ;
			
			memset(qSQL , '\0' , SQLLength + 1);
			memset(ticketData.plate , '\0' , sizeof(ticketData.plate));					//Frank add 20120814
			//Frank mark 20120813 memset(SeasonPlate , '\0' , sizeof(SeasonPlate));
			pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
			
			if (sqlite3_open_v2(".//data//parking.s3db" , &db , SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
			{
				sprintf(qSQL , "SELECT TicketID,PlateNumber FROM Season WHERE TicketID=%ld;" , ticketData.ticketID);
				
				if(sqlite3_get_table(db , qSQL , &result , &rows , &cols , &errMsg) == SQLITE_OK)
				{
					if(rows > 0)
					{
						len = strlen(result[cols + 1]);
						//Frank mark 20120813 strncpy(SeasonPlate , result[cols + 1] , len);
						strncpy(ticketData.plate , result[cols + 1] , len);					//Frank add 20120813
					}
					else
						sqlite3_free_table(result);
					
					sqlite3_exec(db , qSQL , 0 , 0 , &errMsg);
					sqlite3_close(db);
				}
			}
			else
			{
				printf("Can't open //data//parking.s3db\n");
			}

			pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
			//Frank mark 20120813 strcpy(ticketData.plate , SeasonPlate);
		}
		//Frank add e 20120806
		
		if(G_ParkingConfig.RFCheckPlate == 0 && ticketType == EASY_CARD)
		{
			bNoCheckPlate = true;
		}
		else
		{
			ShowLCMFile(0,0,(char *)"PlateProcess.lcm");    //車牌辨識中
		}
		
		for(i=0;i<2;i++)
		{
			iPlateRet = GetPlateNumber(Plate,ticketData,false);
			printf("GetPlateNumber return value:[%d]\n.", iPlateRet);
			
			//Frank mark 20120814 if(iPlateRet == 1)
			//Frank mark 20120827 if((iPlateRet == 1) || (iPlateRet == -1))					//Frank add 20120814
			if((iPlateRet == 1) || (iPlateRet == -1) || (iPlateRet == -3))					//Frank add 20120827
			{
				break;
			}
			else if(iPlateRet == 0)
			{
				usleep(100000L);
			}
		}
		
		if(iPlateRet == 1)
		{
			bRecvedPlate = true;
			sprintf(buf,"Plate : [%s]",Plate);
			ShowMessage(buf);
			LCM_PrintStringRightAlign(3,Plate);
			memcpy(ticketData.plate,Plate,8);
		}
		else if(iPlateRet == -1)
		{
			//ShowLCMFile(0,0,(char *)"PlateError.lcm");    //車牌辨識錯誤
			ShowMessage((char *)"Plate Error!!");
			strcpy(ticketData.plate,"********");
			//sleep(1);
			//iRet = 0;
			//Frank add s 20120814
			if(ticketType == SEASON_TICKET || ticketType == VALUE_TICKET)
			{
				ShowLCMFile(0 , 0 , (char *)"PlateError.lcm");    //車牌辨識錯誤
				bOpenBr = false;
			}
			//Frank add e 20120814
		}
		else
		{	//車牌辨識 無回應
			ShowMessage((char *)"Plate no Response!!");
			strcpy(ticketData.plate,"********");
		}		
	}
	else
	{
		
NO_VIS:					//Frank add 20120904

		strcpy(ticketData.plate , "********");
	}
	
SEASON_Noopen:					//Frank add 20120814
	
	// ========================================================= //
	// nick add s 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	// 防止月票卡拿計時票
	if (G_ParkingConfig.VISOpenBarrier == true)
	{
		if (bRecvedPlate == false)
			waitTime = 2;
		else
			waitTime = 0.5;
		
		Ticks = GetTickCount();
		
		while (CheckTimeout(&Ticks, (waitTime * 1000)) == false)
		{
			if (G_POSIN == true)
			{
				ticketType = POS_IN;
				ticketData.ticketID = 1;
				G_ButtonPress = false;
				break;
			}
			else if (G_RFin == true)
			{
				ticketType = EASY_CARD;
				ticketData.ticketID = 0;
				break;
			}
	
			usleep(1);
		}
	}
	//
	// nick add e 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	// ========================================================= //

ThirdPartyTicket:
	
	if((ticketType == HOURLY_TICKET)||(ticketType == ThirdParty_Ticket_IssueHS))
	{
		//VoiceOn(VOICE_PROCESSING);            //票卡處理中
		//AudioOn(AUDIO_PROCESSING);
		ShowMessage((char *)"Process Hourly.");
		//ShowLCMFile(0,0,"TicketProcess.lcm");   ////票卡處理中
		printf("ProcessHourlyIn :: Satrt : %ld\n",GetTickCount()-CalcTmpTime);
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) bOpenBr = ProcessHourlyIn(ticketData);
		bOpenBr = ProcessHourlyIn(&ticketData);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		printf("ProcessHourlyIn :: End : %ld\n",GetTickCount()-CalcTmpTime);
		
		if(bOpenBr == false)
		{   //無法出票
			ShowLCMFile(0,0,(char *)"NoTicket.lcm");
			DrawChipCoin(false);
			sleep(2);
			iRet = 0;
			ClearScreen();
			
			if(ticketType == ThirdParty_Ticket_IssueHS)
			{
				ShowMessage((char *)"New terminal :: write ticket failed !");
				SendCarEnterStatus(3);		
			}
		}
		else
		{
			AudioOn(AUDIO_TAKETICKET);
			VoiceOn(VOICE_TAKETICKET);    //請取票
			
			ShowLCMFile(0,0,(char *)"TakeTicket.lcm");
			usleep(500000L);
			
			if(WaitTakeTicket(ticketType) == false)
			{	//沒有取票進場
				bOpenBr = false;
				RecycleTicket();
				G_ParkingStatus.Out_Retrieved ++;	// 回收數量加 1
				SaveReTicket(G_ParkingStatus.Out_Retrieved);
				LCM_FillScreen(0x00); // nick add s 20140210 Ver:000-000-GIO_V2-135101-0000-13B250 //
				G_ButtonPress = false;					//Frank add 20121224

				if(ticketType == ThirdParty_Ticket_IssueHS)
				{	
					ShowMessage((char *)"New terminal :: write ticket failed !");
					SendCarEnterStatus(4);		
				}
			}
			else
			{
				SaveTicketNumber();
			}
		}
	}
	else if(ticketType == EASY_CARD)
	{
		ShowMessage((char *)"Process RF-in (EasyCard).");
		bOpenBr = ProcessEasyCardIn(ticketData);
	}
	// Tony add 20170307 s
	else if(ticketType == ThirdParty_Ticket)	
	{
		ShowMessage((char *)"ThirdParty Ticket in .");
		MakeThirdPartyTicketData(&ticketData,thirdPartyTicketData);
		bNoCheckPlate = true;
		bOpenBr = true;	
	}
	// Tony add 20170307 e
	else if(ticketType == PARKTRON_CARD)
	{
		ShowMessage((char *)"Parktron Card in .");
		bOpenBr = true;
	}
	else if (ticketType == POS_IN) // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		bOpenBr = true; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		
	EasyCardEnable(false);
	
	if(bOpenBr == true)
	{
		// SendCarEnterStart(ticketType);
		
		// nick mark 20150407 Ver:000-000-GIO_V2-13B251-00044-13C241 //if(ticketType != HOURLY_TICKET && ticketType != PARKTRON_CARD)
		if(ticketType != HOURLY_TICKET) // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		{	//月票: 車號是否相符
			// Frank add s 20120507
			if(ReaderCFG.HourlyReaderType == 6)
			{
				usleep(300000L);
				TicketToOutlet(ticketData);
				usleep(300000L);
				
				AudioOn(AUDIO_TAKETICKET);
				VoiceOn(VOICE_TAKETICKET);    //請取票
				
				ShowLCMFile(0,0,(char *)"TakeTicket.lcm");
				usleep(500000L);
				
				if(WaitTakeTicket(ticketType) == false)
				{	//沒有取票
					RecycleTicket();
				}
			}
			// Frank add e 20120507
			
			if(bNoCheckPlate == false)
			{
				if(strcmp(ticketData.plate,"********") == 0)
				{	// ******直接過
					G_bolWaitLoop2 = true;		// 20111215 Tony add
					OpenBarrier(true);
				}
				else if(strcmp(ticketData.plate,Plate) == 0)
				{
					ShowMessage((char *)"Season Open barrier.");
					G_bolWaitLoop2 = true;		// 20111215 Tony add
					OpenBarrier(true);
				}
				else
				{
					ShowLCMFile(0,0,(char *)"PlateError.lcm");
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //					sprintf(buf,"VIS:%s ; TicketData:%s",Plate,ticketData.plate);
					sprintf(buf,"TicketID:%ld  VIS:%s ; TicketData:%s", ticketData.ticketID, Plate, ticketData.plate); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
					ShowMessage(buf);
					SendAlarm(26,buf);
					sleep(1);
					//return;
				}
			}
			else
			{
				// nick mark 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //ShowMessage((char *)"EasyCard Open Barrier.");
				//ShowMessage((char *)"EasyCard / POSIN / PCard / NoCheckPlate Open Barrier."); // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				ShowMessage((char *)"ThirdParty Card / EasyCard / POSIN / PCard / NoCheckPlate Open Barrier."); // Tony add 2070307
				G_bolWaitLoop2 = true;		// 20111215 Tony add
				OpenBarrier(true);
			}
		}
		else
		{
			// ========================================================= //
			// nick add s 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			void* thread_result = NULL;

			if (tdPreissue != 0)
			{
				pthread_join(tdPreissue, &thread_result);
				tdPreissue = 0;
			}
			
			if (pthread_create(&tdPreissue, NULL, PreIssueTicket, (void*)NULL) != 0)
				perror("Thread preissue creation failed !");
			else
				printf("Preissue thread PID:[0x%lX]\n", tdPreissue);
			// nick add e 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
			
			AudioOn(AUDIO_PLSENTER);
			VoiceOn(VOICE_PLSENTER);	 	//請進場
			G_bolWaitLoop2 = true;		// 20111215 Tony add
			OpenBarrier(true);
			ShowMessage((char *)"Hourly in Open Barrier.");
		}
		
		// nick mark 20150413 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(GetLoop1() == true)
		{
			if (ticketType != HOURLY_TICKET)
			{
				AudioOn(AUDIO_PLSENTER);
				VoiceOn(VOICE_PLSENTER);	 	//請進場
			}
			
			ShowLCMFile(0,0,(char *)"PleaseEnter.lcm");
			//OpenBarrier(true);
			
			if(G_bolOverLoop2 == false)		// 20111215 Tony add
			{								// 20111215 Tony add
				if(CheckBarrierClosed() == true)
				{
	//nick mark 20101220				SendAlarm(6,"In Barrier Can't Open.");
					ShowMessage((char *)"In Barrier Can't Open.");
					// 20111216 Tony mark usleep(500000);
					// nick mark 20130202 //OpenBarrier(true);
				}
			}	// 20111215 Tony add
		}
		
		if(ticketType == HOURLY_TICKET && G_RFin == true)
		{
			bBlackTickt = true;
			sprintf(buf,"Ticket:%ld Plate:[%s]. ",ticketData.ticketID, ticketData.plate);
			SendAlarm(22,buf);
		}
		
		// ========================================================= //
		// nick add s 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		// 先寫入進場資料 //
		// nick mark 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(ticketType != PARKTRON_CARD)
		if (ticketType != PARKTRON_CARD && ticketType != POS_IN) // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		{
			MakeINHTicketData(&hTicketData,ticketData);

			if(SendInCarData(hTicketData) == false)
			{
				sprintf(buf,"SendInCarData : false. Ticket:%ld Plate:[%s]. ",ticketData.ticketID, ticketData.plate);
				ShowMessage(buf);
				sprintf(buf,"DB Write In. Ticket:%ld Plate:[%s]. ",ticketData.ticketID, ticketData.plate);
				ShowMessage(buf);
				WriteHourlyInData(ticketData,true);
			}
		}

		//UpdateDataToServer();
		// nick add e 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		// ========================================================= //
		
		// nick mark 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //bCarIn = WaitCarTraverse(10);
		bCarIn = WaitCarTraverse(30); // nick add 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //
 		
		if(bCarIn == false)
		{
			sprintf(buf,"Ticket= %ld , Back off!!",ticketData.ticketID);
			ShowMessage(buf);
			SendAlarm(33,buf);
			iRet =0;
			G_RFin = false; // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
			
			if(ticketType == HOURLY_TICKET)
			{
				DrawChipCoin(true);
				RecycleTicket();   //回收票卡.
				DrawChipCoin(false);
			}
		}

		//usleep(50000L);
		//SendLoop1Status(false);
		usleep(50000L);

		if(bCarIn)
		{
			SendCarEnterStatus(1);
		}
		else
		{
			SendCarEnterStatus(0);
		}
		
		ClearScreen();
		G_ButtonPress = false;
		// nick mark 20140912 Ver:000-000-GIO_V2-135101-0006-13B251 //G_RFin = false;
		//CloseBarrier(true);
		
		// ========================================================== //
		// nick mark s 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		//if(ticketType != PARKTRON_CARD)
		//{
		//	//pthread_mutex_lock(&Data_mutex);
		//	
		//	if(bBlackTickt == true)
		//	{
		//		bCarIn = false;
		//	}
		//	
		//	//寫 入車資料
		//	//sprintf(buf,"DB Write In. Ticket:%ld Plate:[%s] \n    InTime:%04d-%02d-%02d %02d:%02d",
		//	//			ticketData.ticketID, ticketData.plate,ticketData.in_year ,ticketData.in_month ,ticketData.in_day,
		//	//			ticketData.in_hour ,ticketData.in_min);
		//	
		//	if(bCarIn)
		//		sprintf(buf,"DB Write In. Ticket:%ld Plate:[%s]. ",ticketData.ticketID, ticketData.plate);
		//	else
		//		sprintf(buf,"DB Write In. Ticket:%ld Plate:[%s]. Black",ticketData.ticketID, ticketData.plate);
		//	
		//	ShowMessage(buf);
		//	WriteHourlyInData(ticketData,bCarIn);
		//	//pthread_mutex_unlock(&Data_mutex);
		//}
		// nick mark e 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		// ========================================================== //
		
		// ========================================================= //
		// nick add s 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		// 若為黑名單再次傳送資料 //
		if (bBlackTickt == true || bCarIn == false)
		{
			if(ticketType != PARKTRON_CARD)
			{
				sprintf(buf,"DB Write Black Ticket In. Ticket:%ld Plate:[%s]. Black",ticketData.ticketID, ticketData.plate);
				ShowMessage(buf);
				WriteHourlyInData(ticketData, false);
			}
		}
		// nick add e 20140729 Ver:000-000-GIO_V2-135101-0005-13B251 //
		// ========================================================= //
		
		G_bolWaitLoop2 = false;		// 20111215 Tony add
		
		// ========================================================= //
		// nick add s 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		if (tdPreissue != 0)
		{
			pthread_join(tdPreissue, NULL);
			tdPreissue = 0;
		}
		// nick add e 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		// ========================================================= //
	}
	else
	{
		//Frank add s 20120508
		if(ReaderCFG.HourlyReaderType == 6)
		{
			if(ticketType == SEASON_TICKET || ticketType == VALUE_TICKET)
			{
				sleep(1);
				RejectTicket();
				AudioOn(AUDIO_TAKETICKET);
				VoiceOn(VOICE_TAKETICKET);  //語音:請取票
				WaitTakeTicket(ticketType);
			}
		}
		//Frank add e 20120508
		sleep(2);
		timeout = 0;
		ShowLCMFile(0,0,(char *)"logo.lcm");
		G_bolWaitLoop2 = false;		// 20111215 Tony add
		return 0;
		//continue;
	}

	G_bolWaitLoop2 = false;		// 20111215 Tony add
	bSentVISTrigger = false; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	ClearScreen();
	ShowLCMFile(0,0,(char *)"logo.lcm");
	//LCM_ShowTime();
	//End in process
	return iRet;
}

bool WaitCarTraverse(int sec)
{	// wait n second
	//int i;
	int Closealarm = 0;					//Frank add 20120903
//	int iResetTickLimit = 30; // nick add 20131120 Ver:000-000-GIO_V2-133181-0103-135101 //
	char buf[64];
	bool bRet = true;
	bool alarm = true;					//Frank add s 20111020
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Ticks = GetTickCount();					//Frank add 20120903
	unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	unsigned long dlWaitLoop2Timeout = 0; // nick add 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
	time_t now = 0;
	static time_t in_lastTime = 0 , QuietTime = 0;
	
	now = time((time_t *)0);
	in_lastTime = now;					//Frank add e 20111020
	
	memset(buf , '\0' , sizeof(buf));
	
	if(G_ParkingConfig.Loop2Timeout > sec)
	{
		sec = G_ParkingConfig.Loop2Timeout;					//Frank add 20120903
	}
	
	//Frank mark 20120903 sprintf(buf,"Wait Time:%d",waitTime);
	sprintf(buf , "Wait Time : %ds" , sec);					//Frank add 20120903
	ShowMessage(buf);
	dlWaitLoop2Timeout = sec * 1000; // nick add 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	while(1)
	{	//等走到 loop2 或倒車
		//Frank mark 20120903 usleep(100000L);      // 100ms
		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		LCM_ShowTime();
		
//nick mark 20101222		if( CheckBarrierClosed() == true || GetLoop2() == true)
		if(G_bolOverLoop2 == true || GetLoop2() == true)		//nick add 20101222
		{	// car in
			break;
		}
		
		if(GetLoop2() == false && GetLoop1() == false)
		{	//car back
			//Frank mark 20120903 for(i=0;i<waitTime;i++)
// nick mark 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //			while(CheckTimeout(Ticks, (double)(sec * 1000)) == false)					//Frank add 20120903
			while(1) // nick add 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
			{
//nick mark 20120131	usleep(100000L);
				//Frank mark 20120903 usleep(10000L);     //nick add 20120131

// ========================================================== //
// nick mark s 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
//					if(GetLoop1() == true && GetLoop2() == false)
//					{
////nick mark 20120131	if((i/10) > 2)
//						{	//2秒後有下一部進來(loop1)就當作前車己離開
//							bRet = false;
//						}
////nick mark 20120131	else
//						{
//						}
//						
//						usleep(500000L);
//						
//						if(GetLoop1() == true && GetLoop2() == false)
//						{
//							G_RFin = false;
//							return bRet;
//						}
//					}
// nick mark e 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
// ========================================================== //
				
				// ========================================================= //
				// nick add s 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
				LCM_ShowTime();
				
				if(CheckTimeout(&Ticks, dlWaitLoop2Timeout))
				{
					bRet =  false;
					// nick mark 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //G_RFin = false;
					ShowMessage((char *)"Wait Loop2 Detect Timeout Close Barrier.");
					goto LOOP1_SAVE;
				}

				//if (sec <= iResetTickLimit)
				//{
				//	if (GetLoop1())
				//		Ticks = GetTickCount();
				//}
				// nick add e 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
				// ========================================================= //

				if(GetLoop2() == false)
				{
					usleep(10000); // nick add 20131120 Ver:000-000-GIO_V2-133181-0103-135101 //
					continue;
				}
				else
				{
					break;
				}
			}
			
// ========================================================== //
// nick mark s 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //			
//			if(CheckTimeout(Ticks, (double)(sec * 1000)))					//Frank add 20120903
//			{
//				bRet =  false;
//				CloseBarrier(true);
//				ShowMessage((char *)"Close Barrier.");
//				G_RFin = false;
//				return bRet;
//			}
// nick mark e 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //
// ========================================================== //
		}
		else
		{
			if(alarm == true)						//Frank add s 20111020
			{
				now = time((time_t *)0);
				QuietTime = now - in_lastTime ;
				
				//Frank mark 201200903 if((unsigned int)QuietTime == G_ParkingConfig.WaitTime.WaitLoop2_Timeout)
				if((unsigned int)QuietTime > (unsigned int)G_ParkingConfig.Loop2Timeout)					//Frank add 20120903
				{
					// nick mark 20131129 Ver:000-000-GIO_V2-133181-0104-135101 //sprintf(buf , "Wait loop2 Too long :%d" , (unsigned int)QuietTime);
					sprintf(buf , "Wait loop2 Too long :%d" , (unsigned int)G_ParkingConfig.Loop2Timeout);
					ShowMessage(buf);
					SendAlarm(4 , buf);
					// ========================================================== //
					// nick mark s 20131129 Ver:000-000-GIO_V2-133181-0104-135101 //
					//alarm = false;
					//in_lastTime = now;
					//QuietTime = 0;
					//Ticks = GetTickCount();					//Frank add 20120903
					// nick mark e 20131129 Ver:000-000-GIO_V2-133181-0104-135101 //
					// ========================================================== //
					goto LOOP1_SAVE; // nick add 20131129 Ver:000-000-GIO_V2-133181-0104-135101 //
				}
			}					//Frank add e 20111020
			//Frank add s 20120903
			else if (CheckTimeout(&Ticks, (unsigned long)5000))
			{
				// ========================================================== //
				// nick mark s 20131127 Ver:000-000-GIO_V2-133181-0104-135101 //
				//if(Closealarm > 3)
				//{
				//	bRet =  false;
				//	CloseBarrier(true);
				//	ShowMessage((char *)"Barrier closed");
				//	G_RFin = false;
				//	return bRet;
				//}
				// nick mark e 20131127 Ver:000-000-GIO_V2-133181-0104-135101 //
				// ========================================================== //
				
				AudioOn(AUDIO_BRCLOSE);
				ShowMessage((char *)"Barrier will close!");
				Ticks = GetTickCount();
				Closealarm++;
			}
			//Frank add e 20120903
		}
	}
	
	ShowMessage((char *)"Wait loop2 leave.");
	
// nick mark 20131120 Ver:000-000-GIO_V2-133181-0103-135101 //	OutCounter();
// nick mark 20131202 Ver:000-000-GIO_V2-133181-0105-135101 //	G_RFin = false;
	alarm = true;					//Frank add 20111020
	
	while(1)
	{   //等待通過 loop2
		//Frank mark 20120903 usleep(500000L);      // 500ms
		//IdleProcess();
		
		LCM_ShowTime();
		
		if(GetLoop2() == false)
		{
			usleep(100000);
			//printf("no loop2 detect\n");
			
			if(GetLoop2() == false)
			{
				break;
			}
		}
		
		if(alarm == true)					//Frank add s 20111020
		{
			now = time((time_t *)0);
			QuietTime = now - in_lastTime ;
			
			//Frank mark 20120903 if((unsigned int)QuietTime == G_ParkingConfig.WaitTime.CloseBarrier_Timeout)
			if((unsigned int)QuietTime > G_ParkingConfig.WaitTime.CloseBarrier_Timeout)					//Frank add 20120903
			{ 
				//Frank mark 20121224 sprintf(buf , "Wait Close Barrier Too long :%d" , (unsigned int)QuietTime);
				sprintf(buf , "Wait Close Barrier Too long :%d" , G_ParkingConfig.WaitTime.CloseBarrier_Timeout);//Frank add 20121224
				ShowMessage(buf);
				SendAlarm(4 , buf);
				alarm = false;
			}
		}					//Frank add e 20111020
	}
	
	bRet = true;
// nick mark 20131120 Ver:000-000-GIO_V2-133181-0103-135101 //	CloseBarrier(true);
	ShowMessage((char *)"Close Barrier.");

LOOP1_SAVE: // nick add 20131119 Ver:000-000-GIO_V2-133181-0103-135101 //

	// nick mark 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //G_RFin = false; // nick add 20131202 Ver:000-000-GIO_V2-133181-0105-135101 //
	CloseBarrier(true);
	// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //OutCounter(); // nick add 20131120 Ver:000-000-GIO_V2-133181-0103-135101 //

	// 20171113 Clark : 決議取消防砸車
	/* 
	//保險 1.6秒內再有loop2 柵欄舉起
	for( i = 0; i < 8; i++)
	{
		usleep(200000L);
		
		if(GetLoop1() == true)
		{
			//printf("next car arrived!!\n");
			break;
		}
		
		if(GetLoop2() == true)
		{
			OpenBarrier(true , false);
			
			while(GetLoop2() == true)
			{
				usleep(100000L);
			}
			
			OpenBarrier(false,false);
			//CloseBarrier();
		}
	}
	*/
	
	// ========================================================== //
	// nick mark s 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	//// ========================================================= //
	//// nick add s 20131126 Ver:000-000-GIO_V2-133181-0103-135101 //
	//if (IsINMachine) // nick add 20131127 Ver:000-000-GIO_V2-133181-0104-135101
	//{
	//	if(G_ParkingConfig.PreTicket == true)
	//	{
	//		if (CheckTicketIssue() == 0) // nick add 20131127 Ver:000-000-GIO_V2-133181-0104-135101 //
	//		{   //發卡機裡面沒有票
	//			if(CheckTicketEmpty() == true)
	//			{
	//				G_ParkingStatus.status |= STATUS_TICKET_EMPTY;
	//			}
	//			
	//			IssueTicket();
	//			ShowMessage((char *)"@ Pre Issue Ticket.");
	//		}
	//	}
	//}
	//// nick add s 20131126 Ver:000-000-GIO_V2-133181-0103-135101 //
	//// ========================================================= //
	// nick mark e 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	// ========================================================== //
	
	return bRet;
}

void WriteHourlyInData(TicketData ticketData,bool bIn)
{
	time_t entry;
	struct tm EntryTime;
	char buf[128];	//nick add 20110124
	
	sprintf(buf,"WriteHourlyInData:: ticketData.EntryTime :%04d.%02d.%02d %02d:%02d:%02d",ticketData.in_year,ticketData.in_month,ticketData.in_day,
		ticketData.in_hour,ticketData.in_min,ticketData.in_sec);					//Frank add 20120207
	ShowMessage(buf);						//Frank add 20120207
	
	memset(buf, '\0', sizeof(buf));	//nick add 20110124
	
	EntryTime.tm_year = ticketData.in_year - 1900;
	EntryTime.tm_mon = ticketData.in_month - 1;
	EntryTime.tm_mday = ticketData.in_day;
	EntryTime.tm_hour = ticketData.in_hour;
	EntryTime.tm_min = ticketData.in_min;
	// 20120202 Tony mark EntryTime.tm_sec = 59;
   	EntryTime.tm_sec = ticketData.in_sec;   // 20120202 Tony add
	
	entry = mktime(&EntryTime);
	
	HTicketData htd;
	htd.TicketID = ticketData.ticketID;
	sprintf(htd.Plate,"%s",ticketData.plate);
	htd.in_time = (unsigned long)entry;
	htd.InStatus = bIn;
	htd.AreaID = ticketData.areaId; // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //

	sprintf(htd.TagID,"%s",ticketData.TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf, "Write In Hourly Temp DB:%ld  Time:%04d-%02d-%02d %02d:%02d  Ticks=%ld", htd.TicketID, EntryTime.tm_year+1900, EntryTime.tm_mon+1, EntryTime.tm_mday, EntryTime.tm_hour, EntryTime.tm_min, htd.in_time);	// 20120208 Frank add
	sprintf(buf, "Write In Hourly Temp DB:%ld  Time:%04d-%02d-%02d %02d:%02d  Ticks=%ld TagID = %s", htd.TicketID, EntryTime.tm_year+1900, EntryTime.tm_mon+1, EntryTime.tm_mday, EntryTime.tm_hour, EntryTime.tm_min, htd.in_time,htd.TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	// 20120208 Frank mark sprintf(buf, "Write In Hourly Temp DB:%ld  Time:%04d-%02d-%02d %02d:%02d  Ticks=%ld", htd.TicketID, EntryTime.tm_year, EntryTime.tm_mon, EntryTime.tm_mday, EntryTime.tm_hour, EntryTime.tm_min, htd.in_time);	//nick add 20110124
	ShowMessage(buf,2);		//nick add 20110124
	
	//write local car in data to tmp database
	WriteTempHourlyData(htd);
	//pthread_mutex_unlock(&Data_mutex);
}

void WriteHourlyOutData(TicketData ticketData,bool bOut)
{
	time_t leave;
//	struct tm EntryTime;
	char buf[128];	//nick add 20110124

	// 20120208 Tony add s
   	sprintf(buf,"WriteHourlyOutData :: Ticket info Exit Time :%04d.%02d.%02d %02d:%02d:%02d",ticketData.in_year,ticketData.in_month,ticketData.in_day,	ticketData.in_hour,ticketData.in_min,ticketData.in_sec);
   	ShowMessage(buf);
	// 20120208 Tony add e
	
	memset(buf, '\0', sizeof(buf));	//nick add 20110124
	
	//EntryTime.tm_year = ticketData.out_year - 1900;
	//EntryTime.tm_mon = ticketData.out_month - 1;
	//EntryTime.tm_mday = ticketData.out_day;
	//EntryTime.tm_hour = ticketData.out_hour;
	//EntryTime.tm_min = ticketData.out_min;
	//EntryTime.tm_sec = ticketData.out_sec;
	
	//leave = mktime(&EntryTime);
	leave = time((time_t *)0);
	
	//sprintf(buf, "Write In Hourly Temp DB TicketData:%ld  Time:%04d-%02d-%02d %02d:%02d  entry=%ld", ticketData.ticketID, ticketData.in_year, ticketData.in_month, ticketData.in_day, ticketData.in_hour, ticketData.in_min, leave);	//nick add 20110124
	//ShowMessage(buf,2);		//nick add 20110124
	
	HTicketData htd;
	htd.TicketID = ticketData.ticketID;
	sprintf(htd.Plate,"%s",ticketData.plate);
	htd.out_time = (unsigned long)leave;
	//printf("time value:%ld\n",(long)leave);
	htd.InStatus = bOut;
	htd.AreaID = ticketData.areaId; // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //

	sprintf(htd.TagID,"%s",ticketData.TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
	//sprintf(buf, "Write In Hourly Temp DB:%ld  Time:%04d-%02d-%02d %02d:%02d  Ticks=%ld", htd.TicketID, EntryTime.tm_year, EntryTime.tm_mon, EntryTime.tm_mday, EntryTime.tm_hour, EntryTime.tm_min, htd.out_time);	//nick add 20110124
	//ShowMessage(buf,2);		//nick add 20110124
	
	//write local car in data to tmp database
	WriteTempHourlyData(htd,true);
	//pthread_mutex_unlock(&Data_mutex);
}

bool WaitTakeTicket(enum TicketType ticketType, int sec)
{	// if sec == 0 ,wait forever
	char buf[256];
	int timeout = 0;
	time_t now = 0;
	static time_t in_lastTime = 0,QuietTime=0;
	bool alarm = true;					//Frank add 20111020
	bool bRet = false; // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
	
	memset(buf,'\0',sizeof(buf));
	
	//wait take ticket
	now = time((time_t *)0L);
	in_lastTime = now;
	
	ShowMessage((char *)"Check Take Ticket.", 1);
	
	// nick mark 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //while(CheckTakeTicket(ticketType) == false)
	while((bRet = CheckTakeTicket(ticketType)) == false) // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	{   //Check Take Ticket every 500ms
		LCM_ShowTime();
		
		if(sec > 0)
		{
			if(timeout > sec * 2 || GetLoop1() == false)
			{
				LCM_FillScreen(0x00);
				timeout =0;
				//bRet = true; // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
				break;
			}
		}
		
		if(GetLoop1() == false)
		{
			//LCM_FillScreen(0x00);
			//回收
			// nick mark 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //return false;
			bRet = false; // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
			// ncik mark 20150701 Ver:000-000-GIO_V2-13B251-0005-13C241 //break; // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
			goto FUNCEXIT; // nick add 20150701 Ver:000-000-GIO_V2-13B251-0005-13C241 //
		}
		
		usleep(500000L);      // 500ms
		timeout ++;
		
		if(alarm == true)					//Frank add s 20111020
		{
			now = time((time_t *)0);
			QuietTime = now - in_lastTime;
			
			//Frank mark 20120903 if((unsigned int)QuietTime == G_ParkingConfig.WaitTime.TakeTicket_Timeout)
			if((unsigned int)QuietTime >= G_ParkingConfig.WaitTime.TakeTicket_Timeout)					//Frank add 20120903
			{   //超過時間未動作
				sprintf(buf,"Wait take Ticket Too long :%d",(unsigned int)QuietTime);
				ShowMessage(buf);
				SendAlarm(4,buf);
				alarm = false;
				bRet = true; // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
				break; // 發過Alarm後需要開門 // // nick add 20140527 Ver:000-000-GIO_V2-135101-0004-13B251 //
			}
		}					//Frank add e 20111020
	}

FUNCEXIT: // nick add 20150701 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	
	DrawChipCoin(false); // nick add 20150304 Ver:000-000-GIO_V2-13B251-0002-13C241 //
	// nick mark 20150701 Ver:000-000-GIO_V2-13B251-0005-13C241 //return true;
	return (bRet); // nick add 20150701 Ver:000-000-GIO_V2-13B251-0005-13C241 //
}

void MainProcessOut()
{
	int counter=0, iRet=1;
	int iRetryCnt = 0; // nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	int res = 0; // nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	char Plate[16],buf[256];
	char iPlateRet=0; // nick add 20130226 //
	bool bOpenBr = false;
	bool bCarOut = false;
	bool alarm = true;						//Frank add s 20111020
	TicketData ticketData;
	ThirdPartyTicketData thirdPartyTicketData;	// Tony add 20170307
	enum TicketType ticketType = NONE_TICKET;
	HTicketData hTicketData;
	static bool bSentVISTrigger = false; // nick add 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	//int timeout = 0;
	
	time_t now=0;
	static time_t in_lastTime = 0,QuietTime = 0;
	now = time((time_t *)0);
	in_lastTime = now;						//Frank add e 20111020
	
	memset(buf,'\0',sizeof(buf));
	memset(&ticketData, 0, sizeof(TicketData)); // nick add 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	//if(GetLoop1() == false && G_RFin == false)
	if(GetLoop1() == false && G_RFin == false && G_NewClientCmdQue.empty() == true)
	{
		//nick mark 20120223 iRunTime++;
		return;
	}
	
	if(G_RFin == false)
	{
		//TurnOnCoinShutter(true); //打開票卡投入口
		DispenserEnable(true);
	}
	else
	{
		ShowMessage((char *)"Out RF in.");
		DispenserEnable(false);
	}
	
	// ========================================================= //
	// nick add s 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //	
	if (GetLoop1() && bSentVISTrigger == false)
	{
		bSentVISTrigger = true;

		if (td_SendVisSoftTrigger != 0)
		{
			pthread_cancel(td_SendVisSoftTrigger);
			td_SendVisSoftTrigger = 0;
		}
		
		for (iRetryCnt = 0; iRetryCnt < 3; iRetryCnt++)
		{
			res = pthread_create(&td_SendVisSoftTrigger, NULL, SendSoftTrigger2Vis, (void*)NULL);
		
			if(res == 0)
				break;
		}
		
		if (iRetryCnt >= 3)
			ShowMessage((char *)"VIS Software Trigger Thread Create Error.");
	}
	// nick add e 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	// ========================================================= //
	
	// while(GetLoop1() == true || G_RFin == true)
	while(GetLoop1() == true || G_RFin == true || G_NewClientCmdQue.empty() == false)
	{
		//SendLoop1Status(true);
		usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		iRunTime = GetTickCount();      //nick add 20120223
		
		memset(Plate,'\0',sizeof(Plate));
		// 20130117 Tony mark ClearReaderBuff();					//Frank add 20111116
		
		if((counter++ % 400 == 0 && G_RFin == false) || (iRet != 1))
		{
			//ReaderReset();//
			DrawChipCoin(false);
			AudioOn(AUDIO_INSERT);  ///請插入票卡
			VoiceOn(VOICE_INSERT);  ///請插入票卡
			DisplayOn(true);
			ShowLCMFile(0,0,(char *)"InsertTicket.lcm");
			
			memset(&ticketData,'\0',sizeof(TicketData));
			
			ShowMessage((char *)"--- Please Insert Ticket. ---");
			iRet=1;		//nick add 20101208
		}
		
		EasyCardEnable(true);  ///悠遊卡enable
		
		// Read ticket data
		ticketType = ReadOutTicketData(&ticketData);

		// New Terminal Passage
		if(ticketType == NONE_TICKET)
		{
			memset(&thirdPartyTicketData, 0, sizeof(thirdPartyTicketData));	// Tony add 20170310 
			ticketType = IdleProcessForNewTerminel(&thirdPartyTicketData);	// Tony add 20170307
		}
		
		if(ticketType == NONE_TICKET)
		{
			IdleProcess();
			LCM_ShowTime();
			
			if(counter >10000 || GetLoop1() == false)
			{
				DispenserEnable(false);
				ShowLCMFile(0,0,(char *)"logo.lcm");
				//Frank add s 20120824
				if(ReaderCFG.HourlyReaderType == 6)
					ReaderReset();
				alarm = true;
				bSentVISTrigger = false; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
				//Frank add  e 20120824
				return;
			}
			
			if(alarm == true)					//Frank add s 20111020
			{
				now = time((time_t *)0);
				QuietTime = now - in_lastTime;
				
				//Frank mark 20120903 if((unsigned int)QuietTime == G_ParkingConfig.WaitTime.WaitCardIn_Timeout)
				if((unsigned int)QuietTime >= G_ParkingConfig.WaitTime.WaitCardIn_Timeout)					//Frank add 20120903
				{
					sprintf(buf,"Wait card insert Too long :%d",(unsigned int)QuietTime);
					ShowMessage(buf);
					SendAlarm(4,buf);
					alarm = false;
				}
			}					//Frank add e 20111020
			
			//nick mark 20120223 iRunTime =0;
			continue;

		}
		else if(ticketType == HOURLY_S)
		{	// Hourly read on season Reader
			ShowLCMFile(0,0,(char *)"TicketReadError.lcm"); ///票卡讀取錯誤
			sleep(1);
			counter = 0;
			now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			DispenserEnable(false);
			ShowLCMFile(0,0,(char *)"logo.lcm");
			return;
		}

		SendCarEnterStart(ticketType);	// 車輛放行開始

		if(ticketType == ThirdParty_Ticket)
		{
			ShowMessage((char *)"Is ThirdParty_Ticket.");
			MakeThirdPartyTicketData(&ticketData, thirdPartyTicketData);
			bOpenBr = true;
		}
		else if(ticketType == HOURLY_TICKET)
		{
			sprintf(buf,"Process Hourly Ticket. ID:%ld",ticketData.ticketID);
			ShowMessage(buf);
			bOpenBr = ProcessHourlyOut(ticketData);
			
			// ========================================================== //
			// nick mark s 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			//// =================== //
			//// nick add s 20130226 //
			//// 前次管理員不放行
			//if (iPlateRet == -1)
			//{
			//	ShowLCMFile(0,0,(char *)"PlateError.lcm");
			//	bOpenBr = false;
			//}
			//// nick add e 20130226 //
			//// =================== //
			// nick mark e 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			// ========================================================== //
		}
//Frank mark 20120514		else if(ticketType == SEASON_TICKET || ticketType == SEASON_TICKET_H)
		else if(ticketType == SEASON_TICKET || ticketType == SEASON_TICKET_H || ticketType == VALUE_TICKET)					//Frank add 20120514
		{
			ShowMessage((char *)"Process Season card.");
			//bOpenBr = ProcessSeasonCardOut(ticketData);
			bOpenBr = ProcessSeasonCardOut(&ticketData);
		}
		//Frank mark s 20120514
/*		else if(ticketType == VALUE_TICKET)
		{
			ShowMessage((char *)"Process Value card.");
			bOpenBr = ProcessValueCardOut(&ticketData);
		}*/
		//Frank mark 20120514
		// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //else if(ticketType == EASY_CARD)
		else if(ticketType == EASY_CARD || ticketType == POS_IN) // nick add 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		{
			bOpenBr = ProcessEasyCardOut(ticketData);
			// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //ShowMessage((char *)"Process RF in.");
			
			// ========================================================= //
			// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			if (ticketType == EASY_CARD)
				ShowMessage((char *)"Process RF in.");
			else if (ticketType == POS_IN)
				ShowMessage((char *)"Process POS_IN.");
			// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			// ========================================================= //
		}
		else if(ticketType == UNKNOW_TICKET || ticketType == TICKET_READ_ERROR)
		{
			AudioOn(AUDIO_INVTICKET);
			VoiceOn(VOICE_INVTICKET);            //票卡無效,請重試
			
			if(ticketType == TICKET_READ_ERROR)
			{
				ShowLCMFile(0,0,(char *)"TicketReadError.lcm");
				ShowMessage((char *)"Read Ticket Error!!");
			}
			else
			{
				ShowMessage((char *)"Unknow Ticket !!");
				ShowLCMFile(0,0,(char *)"TicketDataError.lcm");
			}
			
			RejectTicket();
			// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //usleep(100000L);
			sleep(1); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			WaitTakeTicket(ticketType);
			DrawChipCoin(false);
			counter = 0;
			now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			DispenserEnable(true);
			SendCarEnterStatus(6);
			continue;
		}
		//Frank add s 20120725
		else if(ticketType == TICKET_SEASON_READ_ERROR)
		{
			AudioOn(AUDIO_INVTICKET);
			VoiceOn(VOICE_INVTICKET);            //票卡無效,請重試
			counter = 0;
			SendCarEnterStatus(6);
			continue;
		}
		//Frank add e 20120725
		else if(ticketType == PARKTRON_CARD)
		{
			bOpenBr = true;
		}		
		else
		{
			sprintf(buf,"Ticket Type :%d\n",ticketType);
			ShowMessage(buf);
		}
		
		EasyCardEnable(false);
		
		if(bOpenBr == false)
		{	// 票卡不能出場,等待取卡
			RejectTicket();					//Frank add 20120508
			sleep(1);					//Frank add 20120515
			AudioOn(AUDIO_TAKETICKET);
			VoiceOn(VOICE_TAKETICKET);    //請取票
//Frank mark	20120508		RejectTicket();
			ShowMessage((char *)"Can't out !! Wait take ticket.");
			sleep(1);
			WaitTakeTicket(ticketType);
			DrawChipCoin(false);
			counter = 0;
			now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			DispenserEnable(true);
			SendCarEnterStatus(6);
			continue;
		}
				
		//check ticket can out
		if(ticketType == PARKTRON_CARD)
		{
			// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //RejectTicket(); //退出票卡
			// ========================================================= //
			// nick add s 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			RejectTicket();
			sleep(1);
			WaitTakeTicket(ticketType);
			DrawChipCoin(false);
			// nick add e 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
			// ========================================================= //
		}
		else if(ticketType != HOURLY_TICKET && ticketType != NONE_TICKET)
		{	// 不是計時票,要等取回票卡
			// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(ticketType == EASY_CARD)
			if(ticketType == EASY_CARD || ticketType == POS_IN || ticketType == ThirdParty_Ticket) // nick add 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
			{
				AudioOn(AUDIO_PLSEXIT);
				VoiceOn(VOICE_PLSEXIT);  //語音:請離場
				ShowLCMFile(0,0,(char *)"Leave.lcm"); //請離場
				G_bolWaitLoop2 = true;		// 20111215 Tony add
				OpenBarrier(true);
				// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //ShowMessage((char *)"RF_IN out Open Barrier.");
				
				// ========================================================= //
				// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				if (ticketType == EASY_CARD)
					ShowMessage((char *)"RF_IN out Open Barrier.");
				else if(ticketType == POS_IN)
					ShowMessage((char *)"POS_IN out Open Barrier.");
				else if(ticketType == ThirdParty_Ticket)
					ShowMessage((char *)"ThirdParty_Ticket out Open Barrier.");
				// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// ========================================================= //
			}
			else  //月票或儲值卡
			{		//寫 出場狀態
				ShowMessage((char *)"Write Season Ticket.");
				
				// ========================================================= //
				// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				// 取得票卡登入的車牌號碼 //
				char qSQL[SQLLength + 1] , *errMsg = NULL , **result;					//Frank add 20120813
				sqlite3 *db = NULL;
				int rows , cols , len ;
				
				memset(qSQL , '\0' , SQLLength + 1);
				memset(ticketData.plate , '\0' , sizeof(ticketData.plate));
				pthread_mutex_lock(&Data_mutex);
				
				if (sqlite3_open_v2(".//data//parking.s3db" , &db , SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
				{
					sprintf(qSQL , "SELECT TicketID,PlateNumber FROM Season WHERE TicketID=%ld;" , ticketData.ticketID);
					
					if(sqlite3_get_table(db , qSQL , &result , &rows , &cols , &errMsg) == SQLITE_OK)
					{
						if(rows > 0)
						{
							len = strlen(result[cols + 1]);
							strncpy(ticketData.plate , result[cols + 1] , len);
						}
						else
							sqlite3_free_table(result);
						
						sqlite3_exec(db , qSQL , 0 , 0 , &errMsg);
						sqlite3_close(db);
					}
				}
				else
				{
					printf("Can't open //data//parking.s3db\n");
				}

				pthread_mutex_unlock(&Data_mutex);
				// nick add e 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				// ========================================================= //
			
//Frank mark 20120515				if( ticketType == SEASON_TICKET_H)
//Frank mark 20120709				if( ticketType == SEASON_TICKET_H || ticketType == VALUE_TICKET)					//Frank add 20120515
				if(ticketType == SEASON_TICKET_H || ((ReaderCFG.HourlyReaderType == 6)
							&& (ReaderCFG.SeasonReaderType==0)))					//Frank add 20120709
				{
					// ========================================================= //
					// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
					// 吸入式讀卡機: 要求車牌 //
					iPlateRet = GetPlateNumber(Plate, ticketData, true);
					
					if( iPlateRet == -1)
					{
						AudioOn(AUDIO_INVTICKET);
						VoiceOn(VOICE_INVTICKET);
						ShowLCMFile(0, 0, (char *)"PlateError.lcm");
						sprintf(buf,"TicketID:%ld  VIS Plate:[%s] ; Ticket Plate:[%s]", ticketData.ticketID, Plate, ticketData.plate);
						ShowMessage(buf);
						SendAlarm(26,buf);
						
						//wait 等待管理員處理
						if(WaitPlateAllow(&ticketData, 5) == true)
						{
							RejectTicket();
							// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //WaitTakeTicket(ticketType);
							sleep(1);
							WaitTakeTicket(ticketType); // nick add s 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							counter = 0;
							now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							continue;
						}
					}
					else if(iPlateRet == 0)
					{
						ShowMessage((char *)"Plate no Response!");
						sprintf(Plate, "********");
					}
					else if (iPlateRet == 2)
					{ // 不須車牌辨識
						ShowMessage((char *)"Don't need plate ocr.");
						sprintf(Plate, "********");
						ClearVIS();
					}
					else
					{
						sprintf(buf, "Pass : Plate: [%s]", Plate);
						ShowMessage(buf);
					}
					
					sprintf(ticketData.plate, "%s", Plate);
					// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
					// ========================================================= //
					
					iRet = WriteCarOutData(&ticketData,true,true);
					//Frank add s 20120509
					if(ReaderCFG.HourlyReaderType == 6)
					{
						TicketToOutlet(ticketData);
					}
					else
					{
						RejectTicket();
						sleep(1); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
					}
					//Frank add e 20120509
					AudioOn(AUDIO_TAKETICKET);
					VoiceOn(VOICE_TAKETICKET);  //語音:請取票
//Frank mark 20120509	RejectTicket(); //退出票卡
					WaitTakeTicket(ticketType);
				}
				else
				{
					// nick mark 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //iRet = WriteCarOutData(&ticketData,false,true);
					
					// ========================================================= //
					// nick add s 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					if (G_ParkingConfig.SeasonReadTag == 0)
						iRet = WriteCarOutData(&ticketData,false,true);
					else if (G_ParkingConfig.SeasonReadTag == 1)
						iRet = 1;
					// nick add e 20150506 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					// ========================================================= //
				}
					
				if(iRet != 1)
				{
					// nick mark 20130222 //ShowLCMFile(0,0,(char *)"TicketReadError.lcm");
					sprintf(buf,"Write Out Data to Season/Value Error %d. TicketID:%ld",iRet,ticketData.ticketID);
					// nick mark 20130222 //SendAlarm(61,buf);
					ShowMessage(buf);
					bOpenBr = false;
					sprintf(LastTagId,"%s",ticketData.TagID);
					SendCarEnterStatus(3);
					// nick mark 20130222 //AudioOn(AUDIO_BEEP2);
					// nick mark 20130222 //sleep(1);
					continue;
				}
				
				memset(LastTagId, '\0', sizeof(LastTagId));		//nick add 20101208 //
				//memset(&LastTicketData, 0, sizeof(TicketData)); // nick add 20130222 //
				
				// ========================================================= //
				// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				if (strlen(Plate) <= 0)
				{ // 一般月票讀卡機用: 要求車牌 //
					iPlateRet = GetPlateNumber(Plate, ticketData, true);
					
					if( iPlateRet == -1)
					{
						AudioOn(AUDIO_INVTICKET);
						VoiceOn(VOICE_INVTICKET);
						ShowLCMFile(0, 0, (char *)"PlateError.lcm");
						sprintf(buf,"TicketID:%ld  VIS Plate:[%s] ; Ticket Plate:[%s]", ticketData.ticketID, Plate, ticketData.plate);
						ShowMessage(buf);
						SendAlarm(26,buf);
						
						//wait 等待管理員處理
						if(WaitPlateAllow(&ticketData, 5) == true)
						{
							RejectTicket();
							// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //WaitTakeTicket(ticketType);
							sleep(1);
							WaitTakeTicket(ticketType); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							counter = 0;
							now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
							continue;
						}
					}
					else if(iPlateRet == 0)
					{
						ShowMessage((char *)"Plate no Response!");
						sprintf(Plate, "********");
					}
					else if (iPlateRet == 2)
					{ // 不須車牌辨識
						ShowMessage((char *)"Don't need plate ocr.");
						sprintf(Plate, "********");
						ClearVIS();
					}
					else
					{
						sprintf(buf, "Pass : Plate: [%s]", Plate);
						ShowMessage(buf);
					}
					
					sprintf(ticketData.plate, "%s", Plate);
				}
				// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				// ========================================================= //
				
				AudioOn(AUDIO_PLSEXIT);
				VoiceOn(VOICE_PLSEXIT);  //語音:請離場
				ShowLCMFile(0,0,(char *)"Leave.lcm"); //請離場
				G_bolWaitLoop2 = true;		// 20111215 Tony add
				OpenBarrier(true);
				ShowMessage((char *)"Season/Value out Open Barrier.");
			}
		}
		else
		{	//計時票
			
			// =================== //
			// nick add s 20130226 //
			// 計時票先車牌辨識 //
			// nick mark 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //if(strlen(G_ParkingConfig.PlateServerIP) > 7 && G_ParkingConfig.VISforHourly) //有設定車牌辨識Server
			{
				iPlateRet = GetPlateNumber(Plate, ticketData, true);
				
				if( iPlateRet == -1)
				{
					// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //AudioOn(AUDIO_INVTICKET);
					AudioOn(AUDIO_PLATEERROR); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
					VoiceOn(VOICE_INVTICKET);
					ShowLCMFile(0,0,(char *)"PlateError.lcm");
// nick mark 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //					sprintf(buf,"VIS Plate:[%s] ; Ticket Plate:[%s]",Plate,ticketData.plate);
					sprintf(buf,"TicketID:%ld  VIS Plate:[%s] ; Ticket Plate:[%s]", ticketData.ticketID, Plate, ticketData.plate); // nick add 20130923 Ver:000-000-GIO_V2-133181-0100-135101 //
					ShowMessage(buf);
					SendAlarm(26,buf);
					//wait 等待管理員處理
					
					// nick mark 20130226 //if(WaitPlateAllow(&ticketData, 5) == false)
					if(WaitPlateAllow(&ticketData, 5) == true) // nick add 20130226 //
					{
						RejectTicket();
						// nick mark 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //WaitTakeTicket(ticketType);
						sleep(1);
						WaitTakeTicket(ticketType); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
						DrawChipCoin(false);
						counter = 0;
						now = time((time_t *)0); // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
						in_lastTime = now; // nick add 20140207 Ver:000-000-GIO_V2-135101-0000-13B250 //
						DispenserEnable(true);
						continue;
					}
				}
				else if(iPlateRet == 0)
				{
					ShowMessage((char *)"Plate no Response!");
					sprintf(Plate, "********"); // nick add 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				}
				// ========================================================= //
				// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				else if (iPlateRet == 2)
				{ // 不須車牌辨識
					ShowMessage((char *)"Don't need plate ocr.");
					sprintf(Plate, "********");
					ClearVIS();
				}
				// nick add e 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
				// ========================================================= //
				else
				{
					sprintf(buf,"Pass : Plate: [%s]",Plate);
					ShowMessage(buf);
				}
				
				sprintf(ticketData.plate,"%s",Plate);
			}
			// nick add e 20130226 //
			// =================== //
			
			//if(bOpenBr == true)
			{
				AudioOn(AUDIO_PLSEXIT);
				VoiceOn(VOICE_PLSEXIT);  //語音:請離場
				
				ShowLCMFile(0,0,(char *)"Leave.lcm"); //請離場
				G_bolWaitLoop2 = true;		//nick add 20101221
 				OpenBarrier(true);

				//票卡寫入 出場狀態,清空折扣
				iRet = WriteCarOutData(&ticketData,true,true);
				ShowMessage((char *)"Write Ticket out.");
				RecycleTicket();   //回收票卡.
//nick mark 20101221 				OpenBarrier(true);
				ShowMessage((char *)"Hourly out Open Barrier.");
				
				if(iRet != 1)
				{
					sprintf(buf,"Write Out Data to Hourtly Ticket Error %d. TicketID:%ld.",iRet,ticketData.ticketID);
					SendAlarm(61,buf);
					ShowMessage(buf);
				}
				
				G_ParkingStatus.Out_Retrieved++;
				SaveReTicket(G_ParkingStatus.Out_Retrieved);
			}
		}
		
		DispenserEnable(false); //for mifare card
		//等車離場
		ShowMessage((char *)"Wait car Out....");
		
		if(G_bolOverLoop2 == false)		//nick add 20101223
		{
			if(CheckBarrierClosed() == true)
			{
	//nick mark 20101220			SendAlarm(6,"Out Barrier Can't Open.");
				ShowMessage((char *)"Out Barrier Can't Open.");
				// nick mark 20130202 //usleep(200000L);
				// nick mark 20130202 //OpenBarrier(true);
			}
		}								//nick add 20101223
		
		// nick mark 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //bCarOut = WaitCarTraverse(10);
		bCarOut = WaitCarTraverse(30); // nick add 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //
		//RecycleTicket();   //回收票卡.
		
		if(bCarOut == false)
		{
			sprintf(buf,"OUT Back off!!");
			ShowMessage(buf);
			SendAlarm(33,buf);
			G_RFin = false; // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
		}
		
		// nick mark 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //if(ticketType != PARKTRON_CARD)
		// nick mark 20160321 Ver:000-000-GIO_V2-13B251-0009-13C241 //if(ticketType != PARKTRON_CARD || ticketType != POS_IN) // nick add 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		if(ticketType != PARKTRON_CARD && ticketType != POS_IN) // nick add 20160321 Ver:000-000-GIO_V2-13B251-0009-13C241 //
		{
			MakeOUTHTicketData(&hTicketData,ticketData);

			if(SendOutCarData(hTicketData) == false)
			{
				sprintf(buf,"DB Write Out. Ticket:%ld Plate:[%s].", ticketData.ticketID, ticketData.plate);
				ShowMessage(buf);
				WriteHourlyOutData(ticketData,true);
			}
		}

		//usleep(50000L);
		//SendLoop1Status(false);
		usleep(50000L);

		if(bCarOut)
		{
			SendCarEnterStatus(1);
		}
		else
		{
			SendCarEnterStatus(0);
		}

		DrawChipCoin(false);
		LCM_ShowTime();
		G_bolWaitLoop2 = false;		//nick add 20101221
		bSentVISTrigger = false; // nick add 20150715 Ver:000-000-GIO_V2-13B251-0005-13C241 //
		break;
	}
	
	alarm = true;					//Frank add 20111020
	
	if(counter > 0 || ticketType != NONE_TICKET)
	{
		DispenserEnable(false);      
		EasyCardEnable(false);  //悠遊卡off
		ShowLCMFile(0,0,(char *)"logo.lcm");
	}
}

bool WaitPlateAllow(TicketData* ticketData,int waitTime)
{
	sleep(waitTime);
	return true;
}

enum TicketType IdleProcessForNewTerminel(ThirdPartyTicketData* thirdPartyTicketData)
{
	char buf[80];
	char log[512];
	char datasTmp[512];
	serverCommand cmd;
	enum ControlCommand remoteCmd;	
	enum TicketType TicketType = NONE_TICKET;
		
	do
	{
		LCM_ShowTime();

		if(G_NewClientCmdQue.empty() == false)
		{
			cmd = (serverCommand)(G_NewClientCmdQue.front());
			remoteCmd = cmd.Command;
			G_NewClientCmdQue.pop();

			memset(log , '\0' , sizeof(log));
			sprintf(log,"IdleProcessForNewTerminel :: remoteCmd : %d, Data : %s ",remoteCmd, cmd.datas);
			ShowMessage(log);

			switch(remoteCmd)
			{
				case CMD_ThirdParty_Ticket:	
					ShowMessage((char *)"ThirdParty_Ticket.");
					memset(thirdPartyTicketData, 0, sizeof(thirdPartyTicketData));

					if(strlen(cmd.datas) > 0)
					{
						printf("IdleProcessForNewTerminel :: cmd.datas:%s\n", cmd.datas);

						// default
						thirdPartyTicketData->AreaID = G_ParkingConfig.AreaID;
						thirdPartyTicketData->IssueTicket = false;

						// Area ID
						memset(datasTmp , '\0' , sizeof(datasTmp));
						memcpy(datasTmp,cmd.datas,sizeof(datasTmp));
						printf("IdleProcessForNewTerminel :: datasTmp:%s\n", datasTmp);
						memset(buf , '\0' , sizeof(buf));
						
						if(CommDataGetParm(buf,7,datasTmp) == true)
						{
							thirdPartyTicketData->AreaID = atoi(buf);
						}
						
						printf("IdleProcessForNewTerminel :: AreaID:%s\n", buf);

						// Issue Ticket
						memset(datasTmp , '\0' , sizeof(datasTmp));
						memcpy(datasTmp,cmd.datas,sizeof(datasTmp));
						printf("IdleProcessForNewTerminel :: datasTmp:%s\n", datasTmp);
						memset(buf , '\0' , sizeof(buf));

						if(CommDataGetParm(buf,8,datasTmp) == true)
						{
							thirdPartyTicketData->IssueTicket = atoi(buf);
						}
						
						printf("IdleProcessForNewTerminel :: IssueTicket:%s\n", buf);

						// AuthID
						memset(buf , '\0' , sizeof(buf));
						CommDataGetParm(buf,2,cmd.datas);
						sprintf(thirdPartyTicketData->AuthID,"%s",buf);
						printf("IdleProcess :: AuthID:%s\n", thirdPartyTicketData->AuthID);

						// AuthType
						memset(buf , '\0' , sizeof(buf));
						CommDataGetParm(buf,3,cmd.datas);						
						sprintf(thirdPartyTicketData->AuthType,"%s",buf);
						printf("IdleProcess :: AuthType:%s\n", thirdPartyTicketData->AuthType);

						// TicketID
						memset(buf , '\0' , sizeof(buf));
						CommDataGetParm(buf,4,cmd.datas);
						thirdPartyTicketData->TicketID = atol(buf);		
						printf("IdleProcess :: TicketID:%lu\n", thirdPartyTicketData->TicketID);

						// Plate
						memset(buf , '\0' , sizeof(buf));
						CommDataGetParm(buf,5,cmd.datas);
						sprintf(thirdPartyTicketData->Plate,"%s",buf);		
						printf("IdleProcess :: Plate:%s\n", thirdPartyTicketData->Plate);

						// ProcessDateTime
						memset(buf , '\0' , sizeof(buf));
						CommDataGetParm(buf,6,cmd.datas);
						printf("IdleProcess :: ProcessDateTime:%s\n", buf);

						struct tm TempDateTime;
				
						sscanf(buf, "%4d%2d%2d%2d%2d%2d", &TempDateTime.tm_year, &TempDateTime.tm_mon,
							&TempDateTime.tm_mday, &TempDateTime.tm_hour, &TempDateTime.tm_min,
							&TempDateTime.tm_sec);
						
						TempDateTime.tm_year -= 1900;
						TempDateTime.tm_mon -= 1;
						thirdPartyTicketData->ProcessDateTime = mktime(&TempDateTime);

						sprintf(thirdPartyTicketData->TagID,"%s%s",thirdPartyTicketData->AuthID,thirdPartyTicketData->AuthType);
						printf("IdleProcess :: TagID:%s\n", thirdPartyTicketData->TagID);
						TicketType = ThirdParty_Ticket;

						if(thirdPartyTicketData->IssueTicket == true)
						{
							TicketType = ThirdParty_Ticket_IssueHS;
						}
					
					}
					
					break;
				default:
					sprintf(buf,"Unknow Command:%d",remoteCmd);
					ShowMessage(buf);
					break;
			}
		}
		else
		{
			usleep(50000L);
		}
	}while(G_bErrHold);						//Frank add e 20111020

	return TicketType;	// Tony add 20170307 
}

void IdleProcess()
{
	static int count = 0;
	char buf[80];
	int dock = 0 , ticket = 0;
	serverCommand cmd;
	enum ControlCommand remoteCmd;
	LED888 led;
	
	if(G_ParkingConfig.LED888port > 0)
	{
		// nick mark 20140325 Ver:000-000-GIO_V2-135101-0001-13B251 //led.init(G_ParkingConfig.LED888port);
		led.init(G_ParkingConfig.LED888port, G_ParkingConfig.LED888Type); // nick add 20140325 Ver:000-000-GIO_V2-135101-0001-13B251 //
	}
	
	memset(buf , '\0' , sizeof(buf));
	
	if(b_GServerConnect == true)					//Frank add s 20111020
	{
		G_ParkingStatus.status |= STATUS_TCP_CONNECT;
	}
	else
	{
		G_ParkingStatus.status &= (~STATUS_TCP_CONNECT);
	}
	
	//Process Update Server data
//	if( (count % 50) ==0)
//	{
		UpdateDataToServer();
//	}
	if((count % 5) == 0)
	{
		StoreStatus();
	}					//Frank add e 20111020
	
	if(b_GRefreshConfig == true)
	{
		b_GRefreshConfig = false;
		ReadConfig();
	}
	
	ticket = ReaderCFG.Ticket1 + ReaderCFG.Ticket2;
	G_ParkingStatus.TicketReserve = ticket;
	
	if(ticket < 20)
	{   //票卡存量低
		if((G_ParkingStatus.status & STATUS_TICKET_LOW) == 0)
		{
			sprintf(buf , "Ticket :%d,%d." , ReaderCFG.Ticket1 , ReaderCFG.Ticket2);
			SendAlarm(3 , buf);
			G_ParkingStatus.status |= STATUS_TICKET_LOW;
		}
	}
	else if(ticket <= 0)
	{
		sprintf(buf , "Ticket :%d" , ticket);
		SendAlarm(2 , buf);
		G_ParkingStatus.status |= STATUS_TICKET_EMPTY;		//nick add 20120220
	}
	else
	{
		G_ParkingStatus.status &= (~STATUS_TICKET_LOW);
		G_ParkingStatus.status &= (~STATUS_TICKET_EMPTY);					//Frank add 20121227
	}
	
	ProcessIOState();
	ProcessManulIO();
	
	do					//Frank add s 20111020
	{
		LCM_ShowTime();
		
		//if Server Command Queue has data ,Execute it.
		if(G_SvrCmdQue.empty() == false)
		{	//Do Server Command
			cmd = (serverCommand)(G_SvrCmdQue.front());
			remoteCmd = cmd.Command;
			G_SvrCmdQue.pop();

			sprintf(buf,"remoteCmd : %d, Data : %s ",remoteCmd, cmd.datas);
			ShowMessage(buf);

			switch(remoteCmd)
			{
				case CMD_CONNECT:
					b_GServerConnect = true;
					//ShowMessage("Connect.");
					break;
					
				case CMD_NOTFULL:
					G_iFullLevel = 0;
					//SendCarFullStatus();
					dock = atoi(cmd.datas);
					
					if(dock < 0)
						dock = 0;
					
					if(G_ParkingConfig.LED888port > 0)
					{
						led.Send(G_ParkingConfig.AreaID,dock);
						sprintf(buf,"Parking not FULL -> LED port:%d ID:%d place:%d",G_ParkingConfig.LED888port, G_ParkingConfig.AreaID,dock);
						ShowMessage(buf);
					}

					ShowMessage((char *)"CMD_NOTFULL.");
					
					//if parking full turn on Full light
					if(dock == 0)
					{
						TurnOnParkingFullLight(true);
					}
					else
					{
						TurnOnParkingFullLight(false);
					}
					
					break;
					
				case CMD_HOURLYFULL:
					G_iFullLevel = 1;
					//SendCarFullStatus();
					dock = atoi(cmd.datas);
					
					if(dock <0)
						dock=0;
					
					if(G_ParkingConfig.LED888port>0)
					{
						led.Send(G_ParkingConfig.AreaID,dock);
						sprintf(buf,"Hourly FULL -> LED port:%d ID:%d place:%d",G_ParkingConfig.LED888port, G_ParkingConfig.AreaID,dock);
						ShowMessage(buf);
					}

					ShowMessage((char *)"CMD_HOURLYFULL.");
					TurnOnParkingFullLight(true);
					break;
					
				case CMD_ALLFULL:
					G_iFullLevel = 2;
					//SendCarFullStatus();
					
					if(G_ParkingConfig.LED888port>0)
					{
						led.Send(G_ParkingConfig.AreaID,0);
						sprintf(buf,"Parking FULL -> LED port:%d ID:%d place:%d",G_ParkingConfig.LED888port, G_ParkingConfig.AreaID,dock);
						ShowMessage(buf);
					}

					ShowMessage((char *)"CMD_ALLFULL.");
					TurnOnParkingFullLight(true);
					break;
					
				case CMD_CLEAR_RETICKET:
					ShowMessage((char *)"Clear Re-Ticket.");
					G_ParkingStatus.Out_Retrieved = 0;
					G_ParkingStatus.TicketReserve =0;
					SaveReTicket(0);
					G_bRetriveFull = false;
					break;
					
				case CMD_ADDTICKET:
					ticket = atoi(cmd.datas);
					ReaderCFG.Ticket1 += ticket;
					SaveTicket1();   //加入票卡
					SaveReTicket(0); //清空回收數量
					sprintf(buf,"Add Ticket: +%d",ticket);
					ShowMessage(buf);
					G_bTicketLow  = false;
					G_bTicketZero = false;
					break;
					
				case CMD_OPENBR:
					ShowMessage((char *)"Run BR Open command.");
					OpenBarrier(true);
					ClsoeBarrierChkTime = GetTickCount();	// 20120920 Tony add					
					break;
					
				case CMD_CLOSEBR:
					ShowMessage((char *)"Run BR Close command.");
					CloseBarrier(true);
					break;
					
				case CMD_STOPSERVICE:
					ShowMessage((char *)"Stop Service.");
					G_bStopService = true;
					break;
					
				case CMD_IN_SERVICE:
					ShowMessage((char *)"In Service.");
					G_bStopService = false;
					break;
					
				case CMD_RESET:
					b_GReset = true;
					break;
					
				case CMD_DISCONNECT:
					b_GServerConnect = false;
					break;				
				default:
					sprintf(buf,"Unknow Command:%d",remoteCmd);
					ShowMessage(buf);
					break;
			}
		}
		else
		{
			usleep(50000L);
		}
	}while(G_bErrHold);						//Frank add e 20111020
	
	if(count++ > 30000)
	{
		time_t now = 0;
		now = time((time_t *)0L);
		printf("Delete old data\n");
		DeleteTempHourlyOldData(now-15552000); //180天前
		count = 0;
	}

	ClearReaderBuff();	// 20130117 Tony add
}

void UpdateDataToServer()
{
	char buf[128];
	HTicketData Hdata;
	bool b_HourlyDataSendOK = false;
	
	memset(buf,'\0',sizeof(buf));
	
	if(b_GServerConnect)
	{
		//pthread_mutex_lock(&Data_mutex); // if Data is locked ,wait other thread unlock
		//回傳入車資料
		if(GetTempHourlyData(&Hdata) == true)
		{
			//Send to Server
			//usleep(100000);
			
			if(Hdata.InStatus == true)
			{	//if In Status true, send normal data,else send black list
				if(IsINMachine == true)
				{
					b_HourlyDataSendOK = SendInCarData(Hdata);
					sprintf(buf,"Send Car In TicketID:%ld",Hdata.TicketID);
					ShowMessage(buf);
				}
				else
				{
					sprintf(buf,"Send Car out TicketID:%ld ,Time:%ld",Hdata.TicketID,Hdata.in_time);
					ShowMessage(buf);
					b_HourlyDataSendOK = SendOutCarData(Hdata);
				}
			}
			else
			{
				b_HourlyDataSendOK = SendBlackListData(Hdata,!IsINMachine);
				sprintf(buf,"### Send Black TicketID: %ld ###",Hdata.TicketID);
				ShowMessage(buf);
			}
			
			if(b_HourlyDataSendOK)
			{	//Send to Server Success then delete this temp data
				DeleteTempHourlyData(Hdata);
				sprintf(buf,"Delete Hourly Temp data. TicketID: %ld",Hdata.TicketID);
				ShowMessage(buf);			
			}
			
			//usleep(100000L);
		}
		
		//回傳手動開柵欄資料
		OpenBrData openData;
		
		if(ReadOpenBarrier(&openData) == true)
		{
			sprintf(buf,"Send OpenBR Record: [%ld , %ld]",openData.sn,openData.OpenTime);
			ShowMessage(buf,0);
			
			if(SendManualOpenBarrier(openData) == true)
			{
				DelOpenBarrier(openData);
				sprintf(buf,"DEL OpenBR Record: [%ld , %ld]",openData.sn,openData.OpenTime);
				ShowMessage(buf,0);
			}
			
			//usleep(100000L);
		}
		
		//回傳折扣資料
		HDiscountData Ddata;
		HTicketData Pdata;					//Frank add 20130115
		
		if(IsINMachine == false)
		{
			if(GetTempDiscountData(&Ddata) == true)
			{
				sprintf(buf,"Send Discount Record: [%ld , %ld]",Ddata.TicketID,Ddata.DiscountTime);
				ShowMessage(buf,0);
				
				if(SendDiscountData(Ddata) == true)
				{
					DeleteTempDiscountData(Ddata);
					sprintf(buf,"DEL Discount Record: [%ld , %ld]",Ddata.TicketID,Ddata.DiscountTime);
					ShowMessage(buf,0);
				}
				
				//usleep(100000L);
			}
			//Frank add s 20130115
			if(GetTempPaymentData(&Pdata) == true)
			{
				sprintf(buf , "Send Payment Record: [%ld , %ld]" , Pdata.TicketID , Pdata.pay_time);
				ShowMessage(buf , 0);
				SendPaymentData(Pdata);
				DeleteTempPaymentData(Pdata);
				sprintf(buf , "DEL Payment Record: [%ld , %ld]" , Pdata.TicketID , Pdata.pay_time);
				ShowMessage(buf , 0);
			}
			//Frank add e 20130115
		}
		
		//pthread_mutex_unlock(&Data_mutex);
	}
}

void ProcessIOState()
{
	char buf[80];
	
	memset(buf,'\0',sizeof(buf));
	
	if(CheckMachineBeStrike()== true)
	{   //箱體被撞
		if((G_ParkingStatus.status & STATUS_STRIKE) == 0)
		{
			sprintf(buf,"Strike!!");
			SendAlarm(31,buf);
			G_ParkingStatus.status |= STATUS_STRIKE;
		}
	}
	else
	{
		G_ParkingStatus.status &= (~STATUS_STRIKE);
	}

	usleep(10);
	
	if(CheckMachineBeOpen()==true)
	{   // 箱門開啟
		if((G_ParkingStatus.status & STATUS_MACHINE_OPEN) ==0)
		{
			sprintf(buf,"Machine Door Is Open .");
			SendAlarm(1,buf);
			G_ParkingStatus.status |= STATUS_MACHINE_OPEN;
		}
	}
	else
	{
		G_ParkingStatus.status &= (~STATUS_MACHINE_OPEN);
	}

	usleep(10);
	
	if(CheckBarrierClosed()==false)
	{   // 柵欄狀態
		if (CheckTimeout(&ClsoeBarrierChkTime, (unsigned long)10000))	// 20120920 Tony add
		// 20120920 Tony mark if((G_ParkingStatus.status & STATUS_BARRIER_DOWN) ==0)
		{
			sprintf(buf,"Barrier open ");
			// 20120917 Tony mark (Alarm ID 使用錯誤) SendAlarm(1,buf);
			SendAlarm(16,buf);	// 20120918 Tony add
			// 20120920 Tony mark G_ParkingStatus.status |= STATUS_BARRIER_DOWN;
			ShowMessage((char *)"Barrier Be open. ");
			ClsoeBarrierChkTime = GetTickCount();	// 20120920 Tony add
		}
		
		//Frank mark 20130104 G_ParkingStatus.status &= STATUS_BARRIER_DOWN;	// 20120920 Tony add
		G_ParkingStatus.status |= STATUS_BARRIER_DOWN;					//Frank add 20130104
	}
	else
	{
		ClsoeBarrierChkTime = GetTickCount();
		G_ParkingStatus.status &= (~STATUS_BARRIER_DOWN);
	}
	
	// ========================================================= //
	// nick add s 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
	usleep(10);
	
	if (CheckBarrierstrike() == true)
	{
		if (!(G_ParkingStatus.status & STATUS_BARRIER_STRIKE))
		{
			sprintf(buf, "Barrier be struct!");
			ShowMessage(buf);
			SendAlarm(47, (char *)"");	// 20120918 Tony add
		}
		
		G_ParkingStatus.status |= STATUS_BARRIER_STRIKE;
	}
	else
	{
		G_ParkingStatus.status &= (~STATUS_BARRIER_STRIKE);
	}
	// nick add e 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
	// ========================================================= //
}

void ProcessManulIO()
{	//手動I/O 控制
	char buf[80];
	
	memset(buf,'\0',sizeof(buf));
	
	if(CheckTestTicketButton() == true)
	{   //發測試票
		if (IsINMachine)
		{		
			//Frank mark 20121114 IssueTicket(true);
			if(IssueTicket(true) == 1)					//Frank add 20121114
				ShowMessage((char *)"Issue Test Ticket.");
			//Frank add s 20121114
			else
				ShowMessage((char *)"Issue Test Ticket fail!");
			//Frank add e 20121114
		}
	}

	usleep(10);
	
	if(GetOpenBarrierButton() == true)
	{
		OpenBrData openData;
		
		time_t now;
		now = time((time_t *)0);
		openData.OpenTime = now;
		WriteOpenBarrier(openData);
		ShowMessage((char *)"IO Manual open barrier.");
	}

	usleep(10);
	
	if(GetCloseBarrierButton() == true)
	{
		ShowMessage((char *)"IO Manual Close Barrier.");
	}
}

void ReadConfig()
{
	int MachineIndex=1;
	//Frank mark 20130123 char MachineNum[8];
	char value[64];
	bool bIN = IsINMachine;
	FILE *fh = NULL;
	size_t Fsize;
	
	fh = fopen("MachineSn","r");
	memset(value,'\0',sizeof(value));
	
	if(fh != NULL)
	{
		Fsize = fread (value,1,3,fh);
		MachineIndex = atoi(value);
		fclose(fh);
	}
	
	memset(&G_ParkingConfig,'\0',sizeof(G_ParkingConfig));
	//Frank mark 20130123 memset(MachineNum,'\0',sizeof(MachineNum));
	
	if(bIN)
	{
		//Frank mark 20130123 sprintf(MachineNum,"IN%02d",MachineIndex);
		ReadINI_INMachine(MachineIndex);
	}
	else
	{
		//Frank mark 20130123 sprintf(MachineNum,"OUT%02d",MachineIndex);
		ReadINI_OutMachine(MachineIndex);
	}
	
	//Frank add s 20120907;
	G_ParkingConfig.Hourly_Use = 1;
	G_ParkingConfig.Season_Use = 1;
	//Frank add e 20120907
	
	sprintf(G_ParkingConfig.MaintainTime, "05:00"); // nick add 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	
	ReadINI_Location();
	ReadINI_Common();
	ReadINI_FixConfig(); // nick add 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	ReadINI_NewTerminalConnectSetting();
	
	//Frank add s 20121023
	if(G_ParkingConfig.MoneyRate == 0)
	{
		G_ParkingConfig.MoneyRate = 1;
	}
	//Frank add d 20121023
	
	if(G_ParkingConfig.WaitTime.Button_Timeout == 0)					//Frank add s 20111020
	{
		G_ParkingConfig.WaitTime.Button_Timeout = G_ParkingConfig.QuietTime;
	}
	
	if(G_ParkingConfig.WaitTime.TakeTicket_Timeout == 0)
	{
		G_ParkingConfig.WaitTime.TakeTicket_Timeout = G_ParkingConfig.QuietTime;
	}
	
	//Frank mark s 20120903
	/*
	if(G_ParkingConfig.WaitTime.WaitLoop2_Timeout ==0)
	{
		G_ParkingConfig.WaitTime.WaitLoop2_Timeout = G_ParkingConfig.QuietTime;
	}
	*/
	//Frank mark e 20120903
	
	//Frank add s 20120920
	if(G_ParkingConfig.Loop2Timeout == 0)
	{
		G_ParkingConfig.Loop2Timeout = G_ParkingConfig.QuietTime;
	}
	//Frank add e 20120920
	
	if(G_ParkingConfig.WaitTime.CloseBarrier_Timeout == 0)
	{
		G_ParkingConfig.WaitTime.CloseBarrier_Timeout = G_ParkingConfig.QuietTime;
	}
	
	if(G_ParkingConfig.WaitTime.WaitCardIn_Timeout == 0)
	{
		G_ParkingConfig.WaitTime.WaitCardIn_Timeout = G_ParkingConfig.QuietTime;
	}					//Frank add e 20111020

	G_NoTicketSystem = true;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{
		G_NoTicketSystem = false;	
	}
	
	//nick add s 20110705
	//讀取語言設定
	fh = fopen("Language", "r");
	memset(value, '\0', sizeof(value));
	
	if (fh != NULL)
	{
		Fsize = fread(value, 1, 4, fh);
		MachineIndex = atoi(value);
		LoadLanguage(MachineIndex, G_ParkingConfig.ParkLanguage);
		fclose(fh);
	}
	else
	{
		printf("<<  ReadConfig() : No Language file.  >>\n");
//		sprintf(G_ParkingConfig.ParkLanguage, "eng");		//預設為英文語系
	}
	
	printf("<<  ReadConfig() : %s  >>\n", G_ParkingConfig.ParkLanguage);
	//nick add e 20110705
	
	//G_ParkingStatus.ParkingOccupied =0;
}

void Initial()
{
	G_ParkingStatus.status = 0;

	UDPInitial(G_ParkingConfig.ServerIP,G_ParkingConfig.ServerPort,false); // nick add 20141015 Ver:000-000-GIO_V2-135101-0007-13B251 //
	UDPPlateServerInitial(G_ParkingConfig.PlateServerIP, G_ParkingConfig.PlateServerPort, false); // nick add 20150727 Ver:000-000-GIO_V2-Smart_Traffic-13C241-0003-155061 //
	UDPNewClientInitial(G_ParkingConfig.NewTerminalIP,G_ParkingConfig.NewTerminalPort,false); // nick add 20141015 Ver:000-000-GIO_V2-135101-0007-13B251 //
	IOInitial();
	
	CalcTmpTime = GetTickCount();
	ReaderInitial();
	
	Boot_LCM_Reset();			// 20111205 Tony add
	Boot_LCM_Initial();
	Boot_LCM_FillScreen(0x00);
	
	//GetStatus();
	ReaderCFG.Ticket1 = GetTicket1();
	ReaderCFG.Ticket2 = GetTicket2();
	G_ParkingStatus.Out_Retrieved = GetReTicket();;
	LCM_ShowTime();
}

void CloseAllDevice()
{
	G_bThreadRun = false;
}

void SystemFault()
{
	char buf[256];
	memset(buf,'\0',sizeof(buf));
	
	sprintf(buf,"Fault.lcm");
	ShowLCMFile(0,0,buf);
	DisplayOn(true);
	sprintf(buf,"System Fault!!.");
	ShowMessage(buf);
	SendAlarm(41,buf);
	//WatchDogEnable(false);  //Disable WatchDog
	
	while(G_bSystemFault == true)
	{
		IdleProcess();
		usleep(100000L);       // 100ms
		LCM_ShowTime();
	}
	
	DisplayOn(false);
	sprintf(buf,"Exit fault.");
	printf("%s\n",buf);
	SendAlarm(41+4096,buf);
	//WatchDogEnable(true);  //Enable WatchDog
}

void StopService()
{
	char buf[256];

	memset(buf,'\0',sizeof(buf));
	ShowLCMFile(0,0,(char *)"StopService.lcm");
	DisplayOn(true);
	sprintf(buf,"Stop Service.");
	//printf("%s\n",buf);
	ShowMessage(buf);
	SendAlarm(62,buf);
	//WatchDogEnable(false);  //Disable WatchDog
	
	while(G_bStopService==true)
	{
		IdleProcess();
		usleep(100000L);       // 100ms
		LCM_ShowTime();
	}
	
	DisplayOn(false);
	sprintf(buf,"Start Service.");
	printf("%s\n",buf);
	SendAlarm(62+4096,buf);
	//WatchDogEnable(true);  //Enable WatchDog
}

int main(int argc, char **argv)
{
	int res;
	int iSysRtn;                // 20120314 Tony add
	char buf[256];
	pthread_t UdpThread;
	pthread_t ChkUpdateThread;		//nick add 20110215
	pthread_t ChkInputThread; // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	pthread_t ChkDIStatusThread; // nick add 20160702 Ver:000-000-GIO_V2-13C241-0001-166241 //
	pthread_t SyncTriggerToNewTerminalThread; // Tony add 20170606
	//pthread_t SendButtonTriggerToNewTerminalThread; 	// Tony add 20170705
	pthread_t PollingTriggerFromNewTerminalThread; 	// Tony add 20170705	
	void *thread_result = NULL;
	long MainLoopCnt;   // 20120221 Tony add
	
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long TimeTick;   // 20120221 Tony add
	unsigned long TimeTick = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	memset(buf,'\0',sizeof(buf));
	
	if (argc >1)
	{
		if (strcmp(argv[1],"IN") == 0)
		{
			IsINMachine = true;
		}
		//Frank add s 20120511
		else if(strcmp(argv[1],"D1800") == 0)
		{
			TestD1800();
			return 0;
		}
		else if(strcmp(argv[1],"MCP210") == 0)
		{
			TestMCP210();
			return 0;
		}
		//Frank add e 20120511
		else if (strcmp(argv[1],"OUT") == 0)
		{
			IsINMachine = false;
		}
		//nick add s 20110429
		else if (strcmp(argv[1],"VER") == 0)
		{
//			sprintf(buf, "SOFTWARE VER:000-000-GIO-000000-0100-115231");
			printf("\n<< %s >>\n", Software_Ver);
			return 0;
		}
		//nick add e 20110429
		else if (strcmp(argv[1],"RS232") == 0)
		{
			Test232();
			return 0;
		}
		else if (strcmp(argv[1],"MF700") == 0)
		{
			TestMF700();
			return 0;
		}
		else if (strcmp(argv[1],"CRT350") == 0)
		{
			TestCRT350();
			return 0;
		}
		else if (strcmp(argv[1],"LCM") == 0)
		{
			TestLCM();
			return 0;
		}
		else if (strcmp(argv[1],"SQLITE") == 0)
		{
			SqliteTest();
			return 0;
		}
		else if (strcmp(argv[1],"8255") == 0)
		{
			Test8255();
			return 0;
		}
		else if (strcmp(argv[1],"UDP") == 0)
		{
			UdpTest();
			return 0;
		}
		else if(strcmp(argv[1],"D1000") == 0)
		{
			TestD1000();
			return 0;
		}
		else if(strcmp(argv[1],"D3000") == 0)
		{
			TestD3000();
			return 0;
		}
		else if(strcmp(argv[1],"VOICE") == 0)
		{
			TestVoice();
			return 0;
		}
		else if(strcmp(argv[1],"PLATE") == 0)
		{
			PlateTest();
			return 0;
		}
		else if(strcmp(argv[1],"BARCODE") == 0)
		{
			BarcodeTest();
			return 0;
		}	
		else if(strcmp(argv[1],"RESET") == 0)
		{
			WatchDogSetTimer(1000L); //Set WatchDog Timer 1 sec.
			WatchDogEnable(true);
			return 0;
		}
		else if(strcmp(argv[1],"COMMAND") == 0)
		{
			CommandTest();
			return 0;
		}
		else if(strcmp(argv[1],"LED888") == 0)
		{
			TestLED888();
			return 0;
		}
		else if(strcmp(argv[1],"CLEARHP") == 0)
		{
			IOInitial();
			LCM_Initial();
			LCM_FillScreen(0x00);
			ClearChipCoin();
			return 0;
		}
		else if(strcmp(argv[1],"SDL") == 0)
		{
			SDLTest();
			return 0;
		}
		else if(strcmp(argv[1],"TUP500") == 0)
		{
			TUP500Test();
			return 0;
		}
		else if(strcmp(argv[1],"BMP") == 0)
		{
			TestBMP();
			return 0;
		}
		// ========================================================= //
		// nick add s 20140409 Ver:000-000-GIO_V2-135101-0002-13B251 //
		else if(strcmp(argv[1], "INITIAL") == 0)
		{
			ReadConfig();
			Initial();
			
			if (argc == 1)
				InitialChipCoin();
			else
				InitialChipCoin(atoi(argv[2]));
			
			return 0;
		}
		// nick add e 20140409 Ver:000-000-GIO_V2-135101-0002-13B251 //
		// ========================================================= //
		// ========================================================= //
		// nick add s 20150114 Ver:000-000-GIO_V2-135101-0003-13B251 //
		else if(strcmp(argv[1],"HF320") == 0)
		{
			printf("Testing HF320\n");
			HF320Test();
			printf("End Testing HF320\n");
			return 0;
		}
		// nick add e 20150114 Ver:000-000-GIO_V2-135101-0003-13B251 //
		// ========================================================= //
		// ========================================================= //
		// nick add s 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
		else if (strcmp(argv[1], "HF320_APDU") == 0)
		{
			printf("Testing HF320_Visual_Card\n");
			HF320_Visual_Card_Test();
			printf("End Testing HF320_Visual_Card\n");
			return 0;
		}
		// nick add e 20160701 Ver:000-000-GIO_V2-13C241-0001-166241 //
		// ========================================================= //
		else if(strcmp(argv[1],"ADDTBL") == 0)
		{
			SQLiteAddColumn((char *)"Season",(char *)"AccessControl",(char *)"CHAR(1)");
			SQLiteAddColumn((char *)"Season",(char *)"area_code",(char *)"CHAR(200)");
			//nick add s 20110210
			RunSQL((char *)"CREATE TABLE [SeasonConfig] ([Type] INTEGER PRIMARY KEY  NOT NULL  UNIQUE , "
							"[Mon_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Mon_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Tue_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Tue_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Wed_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Wed_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Thu_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Thu_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Fri_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Fri_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Sat_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Sat_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[Sun_StartTime] TEXT NOT NULL  DEFAULT \"00:00\", [Sun_EndTime] TEXT NOT NULL  DEFAULT \"00:00\", "
							"[ChargeDate] TEXT)");
			RunSQL((char *)"CREATE TABLE TempDiscount(AID INTEGER,DiscountTime INTEGER,Type  INTEGER,DiscountMinute INTEGER,TicketID INTEGER,SID INTEGER,VID INTEGER,RealDiscountMinute INTEGER)");
			RunSQL((char *)"CREATE UNIQUE INDEX [TickTime] ON [TempHourly] ([TicketID] ASC, [InDateTime] ASC)");
			//nick add e 20110210
			RunSQL((char *)"ALTER TABLE TempDiscount ADD Point INTEGER");					//Frank add 20130128
			RunSQL((char *)"ALTER TABLE [TempHourly] ADD [AreaID] smallint"); // nick add 20160720 Ver:000-000-GIO_V2-13B251-0014-13C241 //
			SQLiteAddColumn((char *)"TempHourly",(char *)"TagID",(char *)"varchar(50)");	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			SQLiteAddColumn((char *)"TempPayment",(char *)"TagID",(char *)"varchar(50)");	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			return 0;
		}
		else
			return 1;
	}
	else
	{
		return 0;
	}
	
	//Frank add s 20120504
	if (AlreadyRun()==false)
	{
		ShowMessage((char *)"Program already running!");
		return 0;
	}
	//Frank add e 20120504

	// =================== //
	// nick add s 20130202 //
	res = pthread_mutex_init(&Data_mutex,NULL);
	
	if(res != 0)
	{
		perror("Mutex creation failed");
		// nick mark 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //exit(EXIT_FAILURE);
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	// nick add e 20130202 //
	// =================== //
	
	// =================== //
	// nick add s 20130319 //
	res = pthread_mutex_init(&Log_mutex,NULL);
	
	if(res != 0)
	{
		perror("Mutex creation failed");
		// nick mark 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //exit(EXIT_FAILURE);
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	// nick add e 20130319 //
	// =================== //
	
	// =================== //
	// tony add s 20171101 //
	res = pthread_mutex_init(&NewTerminal_mutex,NULL);
	
	if(res != 0)
	{
		perror("Mutex creation failed");
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	// tony add e 20171101 //
	// =================== //
	
	//GetPrimaryIp(buf);
	//printf("IP:%s \n",buf);
	//Initial
	ReadConfig();
	Initial();
	
	// Main Programe
	if(GetLoop1() == false && GetLoop2() == false)
	{
		if(CheckBarrierClosed() == false)
		{
			// nick mark 20130311 //CloseBarrier(true);
			sprintf(buf,"Barrier is Open!");
			ShowMessage(buf);
		}
	}
	
	//WatchDogSetTimer(30000L); //Set WatchDog Timer 10 sec.
	//WatchDogEnable(true);

	// ================ //
	// nick mark s 20130202 //
	//res = pthread_mutex_init(&Data_mutex,NULL);
	//
	//if(res != 0)
	//{
	//	perror("Mutex creation failed");
	//	exit(EXIT_FAILURE);
	//}
	// nick mark e 20130202 //
	// ================ //
	
	//printf("Create Thread.\n");
	res = pthread_create(&UdpThread,NULL,UDPThread,(void*)NULL);
	
	if(res != 0)
	{
		perror("Thread creation failed !");
		// nick mark 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //exit(EXIT_FAILURE);
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	
	//nick add s 20110215
	res = pthread_create(&ChkUpdateThread, NULL, UpdateThread, (void *)NULL);
	
	if (res != 0)
	{
		perror("Update Thread creation failed !");
		// nick mark 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //exit(EXIT_FAILURE);
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	//nick add e 20110215
	
	// ========================================================= //
	// nick add s 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	res = pthread_create(&ChkInputThread, NULL, GetDI, (void *)NULL);
	
	if (res != 0)
	{
		perror("Check Digital input thread creation failed !");
		// nick mark 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //exit(EXIT_FAILURE);
		iSysRtn=system((char *)"reboot"); // nick add 20160413 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	}
	// nick add e 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	// ========================================================= //
	
	// ========================================================= //
	// nick add s 20160623 Ver:000-000-GIO_V2-13C241-0001-166241 //
	res = pthread_create(&ChkDIStatusThread, NULL, CheckDIThread, (void *)NULL);
	
	if (res != 0)
	{
		perror("Check Digital input status thread creation failed !");
		iSysRtn=system((char *)"reboot");
	}
	// nick add e 20160623 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //

	// ========================================================= //
	// Tony add 20170616 //

	res = pthread_create(&PollingTriggerFromNewTerminalThread, NULL, ThreadPollingTriggerFromNewTerimal, (void *)NULL);
	
	if (res != 0)
	{
		perror("Polling Trigger From New Terimal thread creation failed !");
		iSysRtn=system((char *)"reboot");
	}

	// Tony add 20170616 //
	// ========================================================= //

	// ========================================================= //
	// Tony add 20170616 //
	res = pthread_create(&SyncTriggerToNewTerminalThread, NULL, ThreadSyncTriggerToNewTerimal, (void *)NULL);
	
	if (res != 0)
	{
		perror("Sync IO status thread creation failed !");
		iSysRtn=system((char *)"reboot");
	}
	// Tony add 20170616 //
	// ========================================================= //
	
	
	Boot_ShowLCMFile(0,0,(char *)"logo.lcm");
	Boot_DisplayOn(true);
	
	//ReceiveChipCoin(); // nick add 20141111 Ver:000-000-GIO_V2-135101-0007-13B251 //
	//sleep(1); // nick add 20141111 Ver:000-000-GIO_V2-135101-0007-13B251 //
	
	ClearChipCoinModule(); // nick add 20141119 Ver:000-000-GIO_V2-135101-0007-13B251 //
	
	sprintf(buf, "<<  System Version:%s  >>", Software_Ver);
	ShowMessage(buf);
	// 20111205 Tony mark sprintf(buf,"==== Program Start . Parking ID:%d ,Area ID:%d ,Machine ID:%d ====",G_ParkingConfig.ParkingID,G_ParkingConfig.AreaID,G_ParkingConfig.MachineID);
	sprintf(buf,"==== Program Start. Parking ID:%d, Area ID:%d, Machine ID:%d, UpLayer ID:%d, FullLevel:%d ====",G_ParkingConfig.ParkingID,G_ParkingConfig.AreaID,G_ParkingConfig.MachineID,G_ParkingConfig.UpLayerID,G_iFullLevel);	// 20111205 Tony add
	ShowMessage(buf);
	
	UPSGetsetting();					//Frank add 20110817
	
	TimeTick = GetTickCount();   // 20120221 Tony add
	MainLoopCnt=0;              // 20120221 Tony add
		
	while(b_GReset == false)
	{
		// =============== //
		// nick add s 20130121 //
		if (G_bMaintain)
		{
			usleep(100000);
			continue;
		}
		// nick add e 20130121 //
		// =============== //
		
		if(G_bStopService == false)
		{		
			if(IsINMachine == true)
			{
				if(MainProcessIn() == -1)
				{
					G_bSystemFault = true;
				}
			}
			else
			{
				MainProcessOut();
			}
		}
		
		// 20120314 Tony add s
		if (GetLoop1()==false && GetLoop2()==false) 
		{
			if(SystemReboot() == true)
			{
				ShowMessage((char *)"System auto reboot.");
				iSysRtn=system((char *)"reboot");
			}
		}
		// 20120314 Tony add e
		
//		usleep(50000L);       // 50ms // 不能拿掉，拿掉會對IO出問題
		IdleProcess();
				
		if(G_bSystemFault==true)
		{
			SystemFault();
		}
		
//		usleep(50000L);       // 50ms // 不能拿掉，拿掉會對IO出問題
		
		if(G_bStopService == true)
		{
			StopService();
		}
		
		//nick mark 20120223 if(iRunTime == 200)
		if(CheckTimeout(&iRunTime, (unsigned long)120000))     //nick add 20120223
		{	//省電
			DisplayOn(false);
			//nick mark 20120223 iRunTime =0;
			iRunTime = GetTickCount();      //nick add 20120223
		}
		
		UPSControl();					//Frank add 20110817
		
		// 20120221 Tony add s
		if(CheckTimeout(&TimeTick, (unsigned long)60000L))
		{
			sprintf(buf, "LCNT = %ld, Server connection = %d, StopService = %d, SystemFault = %d, Tickets = %d, FullLevel = %d ",MainLoopCnt,b_GServerConnect,G_bSystemFault,G_bSystemFault,G_ParkingStatus.TicketReserve,G_iFullLevel);
			ShowMessage(buf);
			TimeTick = GetTickCount();
			MainLoopCnt=0;
		}
		
		MainLoopCnt++;
		// 20120221 Tony add e
		
		// nick mark 20130318 //usleep(10000);      //nick add 20120223
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	//WatchDogEnable(false);  //Disable WatchDog
	sprintf(buf,"StopService.lcm");
	ShowLCMFile( 0, 0, buf);
	DisConnectServer();
	
	CloseAllDevice();
	sprintf(buf,"=====  Program End . =====");
	ShowMessage(buf);
	res = pthread_mutex_destroy(&Data_mutex);
	res = pthread_mutex_destroy(&Log_mutex);
	res = pthread_mutex_destroy(&NewTerminal_mutex);	
	res = pthread_join(UdpThread,&thread_result);
	
	if(b_GReset == true)
	{
		WatchDogSetTimer(100L); //Set WatchDog Timer 100m sec.
		WatchDogEnable(true);
	}
	
	return 0;
}
//END_OF_MAIN();

// ============= Thread Function ===============
void *UDPThread(void *arg)
{
	char  buf[256];
	// nick mark 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //char  state[5];
	//static bool b_SaveRFIn_Signal = false; // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
	//static bool b_SavePOSIN_Signal = false; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	//static int index = 0;
	//static int iPOSIN_Index = 0; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	//static int iSaveRFIn_TimerCnt = 0; // nick add 20150206 Ver:000-000-GIO_V2-135101-0008-13B251 //
	//static int iSavePOSIN_TimerCnt = 0; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	//int i;
	unsigned long count = 0L;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Loop2Tick = 0L;
	time_t  startTime;
	
	memset(buf,'\0',sizeof(buf));
	// nick mark 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //memset(state,'\0',sizeof(state));
	
	// nick mark 20141015 Ver:000-000-GIO_V2-135101-0007-13B251 //UDPInitial(G_ParkingConfig.ServerIP,G_ParkingConfig.ServerPort,false);
	
	startTime = time((time_t *)0);

	pthread_detach(pthread_self()); // nick add 20160503 Ver:000-000-GIO_V2-13B251-0011-13C241 //
	
	while(G_bThreadRun)
	{
		UDPProcessCommand();
		//
		if((count % 50) == 0)
		{   //每 5秒 清WatchDog
			//WatchDogClearTimer(); //Clear WatchDog Timer
		}
		
		if((count % 100) == 0)
		{   //每10秒檢查票卡存量,回收箱存量
			if( G_ParkingStatus.Out_Retrieved > G_ParkingConfig.BinsSize)
			{   //回收箱
				sprintf(buf,"Bins Full:%d > %d",G_ParkingStatus.Out_Retrieved,G_ParkingConfig.BinsSize);
				
				if(G_bRetriveFull == false)
				{
					SendAlarm(8,buf);
					G_bRetriveFull = true;
				}
			}
			
			if(ReaderCFG.Ticket1 < 20)
			{
				sprintf(buf,"Ticket Low:%d ,%d",ReaderCFG.Ticket1,ReaderCFG.Ticket2);
				
				if(ReaderCFG.Ticket1 < 1)
				{
					if(G_bTicketZero == false)
					{
						SendAlarm(2,buf);
						G_bTicketZero = true;
					}
				}
				else
				{
					if(G_bTicketLow == false)
					{
						SendAlarm(3,buf);
						G_bTicketLow = true;
					}
				}
			}
		}
		
		if(UDP_Serial > 999999L)
			UDP_Serial =0L;
		
		usleep(100000L);
				
		count++;
		
		if(count >=860000L)
		{
			count=0;
			KillLog(15); //刪除n天前log
		}
		
		if(count ==430000L)
		{
			KillLog(15); //刪除n天前log
		}
	}
	
	//Thread end
	pthread_exit(NULL);
}

void *CheckDIThread(void *arg) // nick add 20160623 Ver:000-000-GIO_V2_ALTOB-13B251-0008-148061 //
{
	int index =0;
	//int OCR_Hourly_index = 0;
	int iPOSIN_Index = 0;
	int iSaveRFIn_TimerCnt = 0;
	//int iSaveOcrHourly_TimerCnt = 0;
	int iSavePOSIN_TimerCnt = 0;
	bool b_SaveRFIn_Signal = false;
	//bool b_SaveHourlyIn_Signal = false;
	bool b_SavePOSIN_Signal = false;
	double OpenSignalTick = 0.0;
	//double Loop2Tick = GetTickCount();
	double dl_RFSignalTicks = 0.0;
	//double dl_HSignalTicks = 0.0;
	double dl_POSIN_SignalTicks = 0.0;
	char  buf[256];

	pthread_detach(pthread_self());
	
	while (G_bThreadRun)
	{
		// Start of process RF_in
		if(GetEasyCardIn() == true)
		{
			if (dl_RFSignalTicks == 0)
				dl_RFSignalTicks = GetTickCount();
			
			if(G_RFin == false)
			{
				if(index == 2)
				{
					if (G_bSystemFault == false && b_SaveRFIn_Signal == false)
					{
						OpenSignalTick = GetTickCount();
						//printf("RF_IN Signal On Ticks %lf\n", OpenSignalTick - dl_RFSignalTicks);
						G_RFin = true;
						b_SaveRFIn_Signal = true;
						index = 0;
						sprintf(buf, "T: G_RFin = true, Ticks=%lf", OpenSignalTick - dl_RFSignalTicks);
						//ShowMessage((char *)"T: G_RFin = true");
						ShowMessage(buf);
						dl_RFSignalTicks = 0.0;
					}
				}
				else if (index < 3)
				{
					index++;
				}
			}
		}
		else
		{
			if (dl_RFSignalTicks != 0)
			{
				sprintf(buf, "Debug: RFIn Signal off, sTicks=%lf", GetTickCount() - dl_RFSignalTicks);
				ShowMessage(buf);
				dl_RFSignalTicks = 0.0;
			}
			
			if (b_SaveRFIn_Signal || index > 0)
			{
				if (iSaveRFIn_TimerCnt >= 3)
				{
					index = 0;
					iSaveRFIn_TimerCnt = -1;
					b_SaveRFIn_Signal = false;
					ShowMessage((char *)"T: G_RFin Off");
				}
				
				iSaveRFIn_TimerCnt++;
			}
		}
		// End of process RF_in
		
		// ========================================================= //
		// nick add s 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		// Start of process POS_IN
		if (GetPosOpenBar() == true)
		{
			if (dl_POSIN_SignalTicks == 0)
				dl_POSIN_SignalTicks = GetTickCount();
			
			if (G_POSIN == false)
			{
				if(iPOSIN_Index == 2)
				{
					if (G_bSystemFault == false && b_SavePOSIN_Signal == false)
					{
						OpenSignalTick = GetTickCount();
						G_POSIN = true;
						b_SavePOSIN_Signal = true;
						iPOSIN_Index = -1;
						ShowMessage((char *)"T: G_POS_IN = true");
					}
					
					iPOSIN_Index++;
				}
				else if (iPOSIN_Index < 3)
					iPOSIN_Index++;
			}
			else
				iSavePOSIN_TimerCnt = 0;
		}
		else
		{
			if (dl_POSIN_SignalTicks != 0)
			{
				sprintf(buf, "Debug: POS_IN Signal off, sTicks=%lf", GetTickCount() - dl_POSIN_SignalTicks);
				ShowMessage(buf);
				dl_POSIN_SignalTicks = 0.0;
			}
			
			if (b_SavePOSIN_Signal || iPOSIN_Index > 0)
			{
				if (iSavePOSIN_TimerCnt >= 3)
				{
					iPOSIN_Index = 0;
					iSavePOSIN_TimerCnt = -1;
					b_SavePOSIN_Signal = false;
					ShowMessage((char *)"T: G_POS_IN Off");
				}
				
				iSavePOSIN_TimerCnt++;
			}
		}
		// End of process POS_IN
		// nick add e 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
		// ========================================================= //
		
		/*
		// Start of process HOURLY_IN
		if (PlateHourlyOpenBarIN() == true)
		{
			if (dl_HSignalTicks == 0)
				dl_HSignalTicks = GetTickCount();
			
			if (G_OCR_Hourly_IN == false)
			{
				if(OCR_Hourly_index == 2)
				{
					if (G_bSystemFault == false && b_SaveHourlyIn_Signal == false)
					{
						OpenSignalTick = GetTickCount();
						G_OCR_Hourly_IN = true;
						b_SaveHourlyIn_Signal = true;
						OCR_Hourly_index = 0;
						sprintf(buf, "T: G_OCR_Hourly_IN = true, Ticks = %lf", OpenSignalTick - dl_HSignalTicks);
						//ShowMessage((char *)"T: G_OCR_Hourly_IN = true");
						ShowMessage(buf);
						dl_HSignalTicks = 0.0;
					}
				}
				else if (OCR_Hourly_index < 3)
					OCR_Hourly_index++;
			}
		}
		else
		{
			if (dl_HSignalTicks != 0)
			{
				sprintf(buf, "Debug: Hourly In Signal off, Tick = %lf", GetTickCount() - dl_HSignalTicks);
				ShowMessage(buf);
				dl_HSignalTicks = 0.0;
			}
			
			if (b_SaveHourlyIn_Signal || OCR_Hourly_index > 0)
			{
				if (iSaveOcrHourly_TimerCnt >= 3)
				{
					OCR_Hourly_index = 0;
					iSaveOcrHourly_TimerCnt = -1;
					ShowMessage((char *)"T: G_OCR_Hourly_IN Off");
				}
				
				iSaveOcrHourly_TimerCnt++;
			}
		}
		// End of process HOURLY_IN
		*/
		
		usleep(100000L);
	}
	
	pthread_exit(NULL);
}

void *GetDI(void *arg) // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
{
	long WaitTime = 20000;
	unsigned long Loop2Tick = 0; // nick add 20170112 Ver:000-000-GIO_V2-13C241-0002-16B231 //

	pthread_detach(pthread_self()); // nick add 20160503 Ver:000-000-GIO_V2-13B251-0011-13C241 //
	
	while(G_bThreadRun)
	{
		GetIC8255_All_Input(WaitTime);
		
		if(G_ButtonPress == false)
		{
			if (G_iFullLevel != 1 && G_iFullLevel != 2)
				G_ButtonPress = PressTicketButton(); // 檢查按鈕訊號
		}
		
		if(G_bolWaitLoop2 == true)
		{	//檢查Loop2
			if (GetLoop2() == true)
			{
				if(Loop2Tick == 0L)
					Loop2Tick = GetTickCount();
			}
			else
			{
				if(Loop2Tick > 0L)
				{
					if(CheckTimeout(&Loop2Tick, 300L))
					{
						if (G_bolOverLoop2 == false)
						{
							ShowMessage((char *)"Over loop 2.");
							G_bolOverLoop2 = true;
						}
					}
				}
			}
		}
		else
		{
			if (G_bolOverLoop2)
			{
				Loop2Tick = 0L;
				G_bolOverLoop2 = false;
			}
		}
 	}
	
	pthread_exit(NULL);
}


// Tony add 20170616
void *ThreadPollingTriggerFromNewTerimal(void *arg)
{
	UDPSocket udpNewTerimalPolling;
	//int myport=0;
	int len = 0;
	int parms;
	char recvData[UDPRecvBuffSize];
	char buf[512];
	serverCommand cmd;
	txParktron tx;
	unsigned long sn1001;
	unsigned long sn1002;
	unsigned long sn1003;
	unsigned long sn1004;
	unsigned long sn1005;
	char msg[1024];
	char log[1024];

	memset(buf,'\0',sizeof(buf));
	memset(recvData,'\0',sizeof(recvData));
	memset(msg,'\0',sizeof(msg));
	memset(log,'\0',sizeof(log));
	
	pthread_detach(pthread_self());

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{
		printf("ThreadPollingTriggerFromNewTerimal:: New Terminal Setting Error, SettingValue:[%s:%d]\n", G_ParkingConfig.NewTerminalIP, G_ParkingConfig.NewTerminalPort);
		pthread_exit(NULL);	
		return(0);
	}

	// Initial Upd socket
	udpNewTerimalPolling.Initial(G_ParkingConfig.NewTerminalIP,G_ParkingConfig.NewTerminalPort, G_ParkingConfig.LocalPort,false);

	sn1001 = 0;
	sn1002 = 0;
	sn1003 = 0;
	sn1004 = 0;
	sn1005 = 0;
	
	// Polling 
	while(G_bThreadRun)
	{		
		memset(buf,'\0',sizeof(buf));
		memset(recvData,'\0',sizeof(recvData));
		memset(msg,'\0',sizeof(msg));
		memset(log,'\0',sizeof(log));

		if((len = udpNewTerimalPolling.udpRecv(recvData)) > 0)
		{
			ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal :: Got data.");

			switch(recvData[0])
			{
				case Tx_RTData:
				case Tx_Data:
					parms = udpNewTerimalPolling.parseCommand(&tx, recvData);
					
					if (recvData[0] == Tx_RTData)
					{
						ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal :: Return Ack.");
						memset(msg,'\0',sizeof(msg));
						sprintf(msg,"\x06""%ld\x1c""%d",tx.sn,tx.commandID);
						udpNewTerimalPolling.udpSend(msg,strlen(msg));
					}

					memset(log,'\0',sizeof(log));
					sprintf(log,"tx.sn : %ld ,tx.commandID : %d",tx.sn,tx.commandID);
					ShowMessage(log);

					// Plate & Ticket Data
					if(tx.commandID == 1001)
					{	
						if(sn1001 == tx.sn) 
						{
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Same Passage command.");
							continue;
						}
						
						ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Run Passage command.");
						sn1001 = tx.sn;						
					
						if(udpNewTerimalPolling.GetParm(buf,1,tx))
						{
							memset(&cmd,'\0',sizeof(cmd));
							cmd.Command = (enum ControlCommand)atoi(buf);
	                  				
							if(udpNewTerimalPolling.GetParm(buf,2,tx))
							{
								if(cmd.Command == CMD_ThirdParty_Ticket)
								{
									memcpy(cmd.datas,tx.parms,sizeof(cmd.datas)-1);
									G_NewClientCmdQue.push(cmd);
								}								
							}							
						}

						if(G_bSystemFault == true)
						{						
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : System Fault.");
							SyncSystemFaultToNewTerminal();
							continue;
						}
					}

					// Play Audio
					if(tx.commandID == 1002)
					{	
						memset(log,'\0',sizeof(log));
						sprintf(log,"sn1002 : %ld ,tx.sn : %ld",sn1002,tx.sn);
						ShowMessage(log);
					
						if(sn1002 == tx.sn)
						{
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Replay Run Play Audio command.");
							continue;
						}

						sn1002 = tx.sn;
						
						if(udpNewTerimalPolling.GetParm(buf,1,tx))
						{
							memset(log,'\0',sizeof(log));
							sprintf(log,"ThreadPollingTriggerFromNewTerimal : Run Play Audio command, File Name : %s",buf);
							ShowMessage(log);
							// ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Run Play Audio command.");
							AudioOut(buf);	
						}
					}

					// Open Barrier
					if(tx.commandID == 1003)
					{	
						if(sn1003 == tx.sn) 
						{
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Replay Run BR Open command.");
							continue;
						}
						
						ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Run BR Open command.");
						sn1003 = tx.sn;
						OpenBarrier(true);
						ClsoeBarrierChkTime = GetTickCount();
					}

					// Close Barrier
					if(tx.commandID == 1004)
					{	
						if(sn1004 == tx.sn)
						{
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Replay Run BR Close command.");
							continue;
						}
						
						ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Run BR Close command.");
						sn1004 = tx.sn;
						CloseBarrier(true);
					}

					// Display Lcm
					if(tx.commandID == 1005)
					{	
						memset(log,'\0',sizeof(log));
						sprintf(log,"sn1005 : %ld ,tx.sn : %ld",sn1005,tx.sn);
						ShowMessage(log);

					
						if(sn1005 == tx.sn)
						{
							ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Replay Run Display Lcm command.");
							continue;
						}
						
						sn1005 = tx.sn;

						if(udpNewTerimalPolling.GetParm(buf,1,tx))
						{
							memset(log,'\0',sizeof(log));
							sprintf(log,"ThreadPollingTriggerFromNewTerimal : Run Display Lcm command, File Name : %s",buf);
							ShowMessage(log);
						
							//ShowMessage((char *)"ThreadPollingTriggerFromNewTerimal : Run Display Lcm command.");
							//ShowLCMFile(0,0,buf);
						}
					}
					
					break;								
			}
		}		

		usleep(100000L);
	}
	
	pthread_exit(NULL);	
}


void *ThreadSyncTriggerToNewTerimal(void *arg)
{
	st_IOSycn loop1IOSycn;
	st_IOSycn loop2IOSycn;
	st_IOSycn buttonIOSycn;
	st_IOSycn carFullStatusSycn;
	st_IOSycn rFInSycn;
	unsigned long Ticks = GetTickCount();
	bool connectionStatus = false;
	
	pthread_detach(pthread_self());

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{
		printf("ThreadSyncTriggerToNewTerimal :: New Terminal Setting Error, SettingValue:[%s:%d]\n", G_ParkingConfig.NewTerminalIP, G_ParkingConfig.NewTerminalPort);
		pthread_exit(NULL);	
		return(0);
	}

	// Default status : 未同步
	loop1IOSycn.SyncValue = -1;
	loop2IOSycn.SyncValue = -1;
	buttonIOSycn.SyncValue = -1;
	carFullStatusSycn.SyncValue = -1;
	rFInSycn.SyncValue = -1;
	G_Loop1IOSycn.SyncValue = -1;
	G_CarFullStatusSycn.SyncValue = -1;
	connectionStatus = false;

	SendCarEnterStatus(0);	
	SendLoop1Status(false);	
	
	// Polling & Sync Status
	while(G_bThreadRun)
	{	
		/*
		if(G_Passage == true)
		{
			usleep(100000L);
			continue;
		}
		*/

		if(CheckTimeout(&Ticks, 60000L))
		{
			if(SendHealthCheckToNewTerimal() == false)
			{
				if(connectionStatus == true)
				{
					ShowMessage((char *)"New Terimal disconnected.");
					loop1IOSycn.SyncValue = -1;
					loop2IOSycn.SyncValue = -1;
					buttonIOSycn.SyncValue = -1;
					carFullStatusSycn.SyncValue = -1;
					rFInSycn.SyncValue = -1;
					G_Loop1IOSycn.SyncValue = -1;
					G_CarFullStatusSycn.SyncValue = -1;
					connectionStatus = false;
				}

				continue;
			}
			else
			{
				if(connectionStatus == false)
				{
					ShowMessage((char *)"New Terimal connected.");
				}

				connectionStatus = true;
			}
			
			Ticks = GetTickCount();
		}

		SyncCarFullStatus(&carFullStatusSycn);
		SyncRFInStatus(&rFInSycn);
		SyncLoop1Status(&loop1IOSycn);
		//SyncButtonStatus(&buttonIOSycn);
		if(IsINMachine == true) SyncButtonStatus(&buttonIOSycn);		
		SyncLoop2Status(&loop2IOSycn);
		usleep(100000L);
	}
	
	pthread_exit(NULL);	
}

int ThreadEntryPoint(void  *data)
{
	return 0;
}

void *SendDataToPlateSVR(void *HourlyticketData)
//bool SendDataToPlateSVR(TicketData HourlyticketData)	//20110627 Tony add
{
	TicketData *pThreadDataInst;
	char SendCmd = 0x01;
	char DataSpace = 0x09;
	char DataBuf[128];
	char datetimeString[20];
	bool Rtn;
	VideoServerProtocol *PlateConn =new VideoServerProtocol();

	pthread_detach(pthread_self()); // nick add 20130219 //
	
	// nick mark 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge form [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //if(strlen(G_ParkingConfig.PlateServerIP) > 7) 	// 20111117 Tony add
	if(strlen(G_ParkingConfig.ImageServerIP) > 7) 	// nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge form [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
	{											// 20111117 Tony add
		pThreadDataInst = (TicketData *)HourlyticketData;
		
		// Make send data start
		memset(DataBuf,0,sizeof(DataBuf));
		memset(datetimeString,0,sizeof(datetimeString));
		
		DataBuf[0]=SendCmd;	
		sprintf(DataBuf + strlen(DataBuf),"%c",DataSpace);
		
		sprintf(DataBuf + strlen(DataBuf),"%9ld",pThreadDataInst->ticketID);
		// nick mark 20130219 //printf("Tickid = %9ld\n",pThreadDataInst->ticketID);
		printf("SendDataToPlateSVR() :: Tickid = %9ld\n",pThreadDataInst->ticketID); // nick add 20130219 //
		//sprintf(DataBuf + strlen(DataBuf),"%9ld",HourlyticketData.ticketID);
		
		sprintf(DataBuf + strlen(DataBuf),"%c",DataSpace);
		
		sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
			pThreadDataInst->in_year, pThreadDataInst->in_month, pThreadDataInst->in_day,
			pThreadDataInst->in_hour,pThreadDataInst->in_min,pThreadDataInst->in_sec);
		/*
		sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
			HourlyticketData.in_year, HourlyticketData.in_month, HourlyticketData.in_day,
			HourlyticketData.in_hour,HourlyticketData.in_min,HourlyticketData.in_sec);
		*/
		
		strcat(DataBuf,datetimeString);		
		sprintf(DataBuf + strlen(DataBuf),"%c",DataSpace);
		
		// nick mark 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //sprintf(DataBuf + strlen(DataBuf),"I%02d",G_ParkingConfig.MachineID);
		
		// ========================================================= //
		// nick add s 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
		if (IsINMachine)
			sprintf(DataBuf + strlen(DataBuf), "I%02d", G_ParkingConfig.MachineID);
		else
			sprintf(DataBuf + strlen(DataBuf), "O%02d", G_ParkingConfig.MachineID);
		// nick add e 20150527 Ver:000-000-GIO_V2-13B251-0005-13C241 //
		// ========================================================= //
		
		
		printf("SendDataToPlateSVR()\n");
		printf("Data buf : \n");
		
		for(unsigned int i=0;i<strlen(DataBuf);i++)
		{
			printf(" %2x",DataBuf[i]);		
		}
		printf("\n");
		
		// Make send data end	
		
		// nick mark 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge form [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //PlateConn->InitialNetwork(G_ParkingConfig.PlateServerIP,(unsigned short)G_ParkingConfig.PlateServerPort);
		PlateConn->InitialNetwork(G_ParkingConfig.ImageServerIP,(unsigned short)G_ParkingConfig.ImageServerPort);	// nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge form [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
		
		if(PlateConn->ConnectToServer() == false )
		{
			printf("SendDataToPlateSVR():Connect to plate server error!\n");
			//return false;
			goto byebye;
		}
		
		Rtn=PlateConn->SendCommandPackageToServer(DataBuf);
		PlateConn->CloseConnect();
	}	// 20111117 Tony add
	
byebye:
	
	delete PlateConn;
	PlateConn = NULL;		//20111117 Tony add
	
	pthread_exit(NULL);
}

void *UpdateThread(void *arg)		//nick add 20110215
{
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Ticks = 0L;
	unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	char UpdateFiles[128];
	bool bFileCheck = false; // nick add 20130121 //

	pthread_detach(pthread_self()); // nick add 20160503 Ver:000-000-GIO_V2-13B251-0011-13C241 //
	
	while(G_bThreadRun)
	{
		if (CheckTimeout(&Ticks, 3000))
		{
			if (GetLoop1()==false && GetLoop2()==false) //20120120 Tony add
        	// 20120120 Tony mark if (GetLoop1()==false)
			{
				bFileCheck = true; // nick add 20130121 //
				sprintf(UpdateFiles, "/tmp/SeasonConfig.tbl");
				
				if (access(UpdateFiles, R_OK) == 0)
				{
					G_bMaintain = true;
					// nick mark 20130121 //CheckUpdateSeasonConfig();
					bFileCheck = CheckUpdateSeasonConfig(); // nick add 20130121 //
				}
				
				sprintf(UpdateFiles, "/tmp/Parking.ini");
				
				// nick mark 20130121 //if (access(UpdateFiles, R_OK)==0)
				if (bFileCheck && access(UpdateFiles, R_OK)==0) // nick add 20130121 //
				{
					G_bMaintain = true;
					CheckParkingINI();
				}
				
				G_bMaintain = false;
			}

//			printf("Check Update!\n");
			Ticks = GetTickCount();
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		
		// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //usleep(500000L);
		// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //GetIC8255_All_Input(); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
	}
	
	pthread_exit(NULL);
}

void CheckParkingINI()		//nick add 20110216
{
	char filename[128];
	char SeasonFile[128];		//nick add 20120816
	int iSysRtn;
	
	sprintf(filename, "/tmp/Parking.ini");
	
	//nick mark 20120816 if (access(filename, R_OK)==0)
	if (access(filename, R_OK) == 0 && access(SeasonFile, R_OK) != 0)		//nick add 20120816
	{
		iSysRtn = system((char *)"mount -o,remount -rw /");		// 20111209 Tony add
		usleep(500000L);										// 20111209 Tony add
		
		//nick add s 20110325
		iSysRtn = system((char *)"rm ./Data/LocalSeting/Parking.ini");	// 20111209 Tony add
		iSysRtn = system((char *)"rm ./Parking.ini");
		usleep(500000L);
		
		iSysRtn = system((char *)"cp -f /tmp/Parking.ini /Data/LocalSeting/");	// 20111209 Tony add
		iSysRtn = system((char *)"mv -f /tmp/Parking.ini .");
		usleep(1000000L);
		iSysRtn = system((char *)"/reset.sh");
		//nick add e 20110325
	}
}

// nick mark 20130121 //void CheckUpdateSeasonConfig()		//nick add 20110215
bool CheckUpdateSeasonConfig()		//nick add 20130121
{
	char UpdateFiles[128];
	char buff[256];
	char qSQL[2048];
	int i_sysRtn = 0;
	FILE *fp = NULL;
	bool bUnlink = false;		//nick add 20120816
	
	sprintf(UpdateFiles, "/tmp/SeasonConfig.tbl");
	
	if (access(UpdateFiles, R_OK) == 0)
	{
		// nick mark 20130121 //if (!RunSQL((char *)"delete from [SeasonConfig];")) return;		//nick add 20120816
		if (!RunSQL((char *)"delete from [SeasonConfig];")) return false ;		//nick add 20130121 //
		
		fp=fopen(UpdateFiles, "r");
		
		//nick mark 20120816 RunSQL((char *)"delete from [SeasonConfig];");
		
		while(!feof(fp))
		{
			bUnlink = true;		//nick add 20120816
			struct stSeasonConfig sc;
			int iGet;
			
			memset(&sc, 0, sizeof(sc));
			memset(qSQL, '\0', sizeof(qSQL));
			memset(buff, '\0', sizeof(buff));
			
			iGet = fscanf(fp, "%2d%5s%5s%5s%5s"
								"%5s%5s%5s%5s"
								"%5s%5s%5s%5s"
								"%5s%5s%s", &sc.Type, sc.Mon_Start, sc.Mon_End, sc.Tue_Start, sc.Tue_End, 
								sc.Wed_Start, sc.Wed_End, sc.Thu_Start, sc.Thu_End, 
								sc.Fri_Start, sc.Fri_End, sc.Sat_Start, sc.Sat_End,
								sc.Sun_Start, sc.Sun_End, sc.ChargeDate);
								
			if (iGet==-1) break;
			
			sc.ChargeDate[sizeof(sc.ChargeDate)-1] = '\0';
			
			sprintf(qSQL, "insert into [SeasonConfig] ([Type], [Mon_StartTime], [Mon_EndTime], [Tue_StartTime], [Tue_EndTime], "
							"[Wed_StartTime], [Wed_EndTime], [Thu_StartTime], [Thu_EndTime], "
							"[Fri_StartTime], [Fri_EndTime], [Sat_StartTime], [Sat_EndTime], "
							"[Sun_StartTime], [Sun_EndTime], [ChargeDate]) values "
							"(%d, '%s', '%s', '%s', '%s', "
							"'%s', '%s', '%s', '%s', "
							"'%s', '%s', '%s', '%s', "
							"'%s', '%s'", 
							sc.Type, sc.Mon_Start, sc.Mon_End, sc.Tue_Start, sc.Tue_End, 
							sc.Wed_Start, sc.Wed_End, sc.Thu_Start, sc.Thu_End, 
							sc.Fri_Start, sc.Fri_End, sc.Sat_Start, sc.Sat_End, 
							sc.Sun_Start, sc.Sun_End);
						
			if (strcmp(sc.ChargeDate, "NULL") == 0)
			{
   				// 20120120 Tony mark RunSQL(strcat(qSQL, ", NULL);"));
   				//nick mark 20120816 strcat(qSQL, ", NULL);");  	// 20120120 Tony add
				
				//nick add s 20120816
				if (!RunSQL(strcat(qSQL, ", NULL);")))
				{	//寫入失敗直接跳出讓程式重新執行
					bUnlink = false;
					break;
				}
				//nick add e 20120816
			}
			else
			{
				char tmpbuff[128];
				
				sprintf(tmpbuff, ", '%s');", sc.ChargeDate);
				// 20120120 Tony mark RunSQL(strcat(qSQL, tmpbuff));
				//nick mark 20120816 strcat(qSQL, tmpbuff);      // 20120120 Tony add
				
				//nick add s 20120816
				if (!RunSQL(strcat(qSQL, tmpbuff)))
				{	//寫入失敗直接跳出讓程式重新執行
					bUnlink = false;
					break;
				}
				//nick add e 20120816
			}
			
			// 20120120 TOny add s
			//while(RunSQL(qSQL)==false)
			//	usleep(1000);
			// 20120120 Tony add e
		
			usleep(10000L); // nick add 20130121 //
		}

		fclose(fp);
		
		if (bUnlink)		//nick add 20120816
		{
			if (unlink(UpdateFiles)!=0)
			{
				sprintf(UpdateFiles, "rm %s", UpdateFiles);
				i_sysRtn = system(UpdateFiles);
			}
		}

		printf("Sync SeasonConfig End!\n"); // nick add 20130121 //
	}

	return(bUnlink); // nick add 20130121 //
}

void UPSControl()					//Frank add s 20110817
{
	char  buf[256];
	int i_sysRtn = 0; // nick add 20141119 Ver:000-000-GIO_V2-135101-0007-13B251 //
	
	
	if(UPSstartstate != PowerFailSignalIn)
	{
		if(UPSSignal() == PowerFailSignalIn)
		{
			if(PowerFailState == true )
			{
				if (GetTickCount() >= PowerFailStartTime)
				{
					UPSShutdown(UPSShutdownPulseTime);
					ShowMessage(( char *)"System shutdown now!");
					i_sysRtn = system((char *)"poweroff");
					
					while(1)
					{
						usleep(10); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
					}
				}
			}
			else
			{
				sprintf(buf, "UPS off, system will shutdown after %d seconds!",ShutdownUPSDefDelayTime);		//Frank add 20111020
				ShowMessage(buf);
				PowerFailStartTime = GetTickCount() + ShutdownUPSDelayTime;
				PowerFailState = true;
				UPSChange = true;
			}
		}
		else
		{
	 		if(UPSChange == true)
			{
				ShowMessage((char *)"UPS on!");
				PowerFailState = false;
				UPSChange = false;
			}
		}
	}
}

void UPSGetsetting(void)
{
	char temp[100];
	
	PowerFailState = false;
	
	GetParameterByNameFromFile( "UPS.ini", "Shutdown_UPS_Def_Delay_Time" , temp , 99);
	ShutdownUPSDefDelayTime = atoi(temp);
	if(ShutdownUPSDefDelayTime > UPS_DELAY_MAXTIME)
		ShutdownUPSDefDelayTime = UPS_DELAY_MAXTIME;
	else if (ShutdownUPSDefDelayTime < UPS_DELAY_MINTIME)
		ShutdownUPSDefDelayTime = UPS_DELAY_MINTIME;
		
	ShutdownUPSDelayTime = ShutdownUPSDefDelayTime * UPS_MS_TO_SEC;					//Frank add 20111020
	
	GetParameterByNameFromFile( "UPS.ini", "UPS_Shutdown_Pulse_Time" , temp , 99);
	UPSShutdownPulseTime = atoi(temp);
	if(UPSShutdownPulseTime > UPS_SHUTDOWN_PULSE_TIME_MAX)
		UPSShutdownPulseTime = UPS_SHUTDOWN_PULSE_TIME_MAX;
	else if(UPSShutdownPulseTime < UPS_SHUTDOWN_PULSE_TIME_MIN)
		UPSShutdownPulseTime = UPS_SHUTDOWN_PULSE_TIME_MIN;
		
	GetParameterByNameFromFile( "UPS.ini" , "Power_Fail_SignalIn" , temp , 99);
	PowerFailSignalIn = atoi(temp);
	
	UPSstartstate = UPSSignal();
	if(UPSstartstate == PowerFailSignalIn)
		printf("UPS setting is wrong!\n");
}					//Frank add e 20110817

void LoadLanguage(int LangID, char* Langauge)		//nick add s 20110705
{
	switch(LangID)
	{
	case 1:
		sprintf(Langauge, "ara");
		break;
	case 2:
		sprintf(Langauge, "bgr");
		break;
	case 3:
		sprintf(Langauge, "cat");
		break;
	case 4:
		sprintf(Langauge, "cht");
		break;
	case 5:
		sprintf(Langauge, "chs");
		break;
	case 6:
		sprintf(Langauge, "csy");
		break;
	case 7:
		sprintf(Langauge, "dan");
		break;
	case 8:
		sprintf(Langauge, "deu");
		break;
	case 9:
		sprintf(Langauge, "ell");
		break;
	case 10:
		sprintf(Langauge, "esp");
		break;
	case 11:
		sprintf(Langauge, "esm");
		break;
	case 12:
		sprintf(Langauge, "esn");
		break;
	case 13:
		sprintf(Langauge, "eti");
		break;
	case 14:
		sprintf(Langauge, "fin");
		break;
	case 15:
		sprintf(Langauge, "fra");
		break;
	case 16:
		sprintf(Langauge, "heb");
		break;
	case 17:
		sprintf(Langauge, "hin");
		break;
	case 18:
		sprintf(Langauge, "hrv");
		break;
	case 19:
		sprintf(Langauge, "hun");
		break;
	case 20:
		sprintf(Langauge, "isl");
		break;
	case 21:
		sprintf(Langauge, "ita");
		break;
	case 22:
		sprintf(Langauge, "jpn");
		break;
	case 23:
		sprintf(Langauge, "kor");
		break;
	case 24:
		sprintf(Langauge, "lth");
		break;
	case 25:
		sprintf(Langauge, "lvi");
		break;
	case 26:
		sprintf(Langauge, "nld");
		break;
	case 27:
		sprintf(Langauge, "nor");
		break;
	case 28:
		sprintf(Langauge, "plk");
		break;
	case 29:
		sprintf(Langauge, "ptb");
		break;
	case 30:
		sprintf(Langauge, "ptg");
		break;
	case 31:
		sprintf(Langauge, "rom");
		break;
	case 32:
		sprintf(Langauge, "rus");
		break;
	case 33:
		sprintf(Langauge, "sve");
		break;
	case 34:
		sprintf(Langauge, "sky");
		break;
	case 35:
		sprintf(Langauge, "srl");
		break;
	case 36:
		sprintf(Langauge, "tha");
		break;
	case 37:
		sprintf(Langauge, "trk");
		break;
	case 38:
		sprintf(Langauge, "ukr");
		break;
	default:		//預設為英文語系
		sprintf(Langauge, "eng");
		break;
	}
}			//nick add e 20110705

//Frank add s 20120206
bool AlreadyRun(void)
{
	int iSysRtn , RunTime = 0;
	char Value[2];
	FILE* fh = NULL;
	size_t Fsize;

	memset(Value,'/0',sizeof(Value));
	
	iSysRtn = system("ps|grep main|grep -vc grep>/tmp/run");
	
	fh = fopen("/tmp/run", "r");
	
	if(fh != NULL)
	{
		Fsize = fread (Value,1,1,fh);
		RunTime = atoi(Value);
		fclose(fh);
	}
	
	if(RunTime>1)
	{
		iSysRtn = system("rm /tmp/run");
		return false;
	}
	
	iSysRtn = system("rm /tmp/run");
	return true;
}
//Frank add e 20120206

bool SystemReboot()   // 20120314 Tony add
{
	bool Rtn = false;
	char RebootTimeStr[128];
	FILE *fh = NULL;
	time_t save_time = 0;
	time_t now = time((time_t *)0);
	struct tm *tm_ptr = localtime(&now);
	
	// ========================================================= //
	// nick add s 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	int iReboot_Hour = -1;
	int iReboot_Min = -1;
	
	sscanf(G_ParkingConfig.MaintainTime, "%d:%d", &iReboot_Hour, &iReboot_Min);
	// nick add e 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	// ========================================================= //
	
// nick mark 20131206 Ver:000-000-GIO_V2-133181-0106-135101 //	if(!(tm_ptr->tm_hour == 3 && tm_ptr->tm_min == 0))
	// nick mark 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //if(!(tm_ptr->tm_hour == 11 && tm_ptr->tm_min == 0)) // nick add 20131206 Ver:000-000-GIO_V2-133181-0106-135101 //
	if(!(tm_ptr->tm_hour == iReboot_Hour && tm_ptr->tm_min == iReboot_Min)) // nick add 20131206 Ver:000-000-GIO_V2-13B251-0010-13C241 //
	{
		return(Rtn);
	}
	
	fh = fopen("RebootTime","r+");
	                
	if(fh == NULL)
	{
		fh = fopen("RebootTime","w");
		
		if(fh != NULL)
		{   
			fprintf(fh, "%ld", now);  
			fclose(fh);
			fh = NULL;
		}
		
		Rtn = true;
			    
	}
	else
	{
		memset(RebootTimeStr,'\0',sizeof(RebootTimeStr));
		
		if(fread(RebootTimeStr, sizeof(char), sizeof(RebootTimeStr), fh)>0)
		{
			save_time = (time_t)atol(RebootTimeStr);
		}
		
		fclose(fh);
		
		if(now - save_time > 86400L)
		{
			fh = fopen("RebootTime","w");
			
			if(fh != NULL)
			{  
				fprintf(fh, "%ld", now);
				fclose(fh);
				fh = NULL;
			}
			
			Rtn = true;
		}
	}
	
	//    if (fh != NULL) fclose(fh);
	return(Rtn);
}

//Frank add s 20120907
void Waitmsec(unsigned int msec)
{
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long TimeTick;
	unsigned long TimeTick; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	TimeTick = GetTickCount();
	
	while(CheckTimeout(&TimeTick, (unsigned long)msec) == false)
	{
		IdleProcess();
		UPSControl();
		LCM_ShowTime();
		
		if(GetLoop1() == false && GetLoop2() == false)
			break;

		usleep(1000);
	}
}
//Frank add e 20120907

void MakeThirdPartyTicketData(TicketData* ticketData,ThirdPartyTicketData thirdPartyTicketData)
{	
	struct tm *tm_ptr = NULL;	

	//ShowMessage((char *)"MakeThirdPartyTicketData Start.");

	// Parking ID
	if(G_ParkingConfig.ParkingID > 999)
	{
		ticketData->parkingId =999;
	}
	else
	{
		ticketData->parkingId = G_ParkingConfig.ParkingID;
	}

	// Area ID
	ticketData->areaId = thirdPartyTicketData.AreaID;	
	printf("MakeThirdPartyTicketData :: thirdPartyEntryData.AreaID:%d \n",thirdPartyTicketData.AreaID);
	printf("MakeThirdPartyTicketData :: ticketData->areaId:%d \n",ticketData->areaId);
	
	// TicketID
	ticketData->ticketID = thirdPartyTicketData.TicketID;
	printf("MakeThirdPartyTicketData :: thirdPartyEntryData.TicketID:%lu \n",thirdPartyTicketData.TicketID);
	printf("MakeThirdPartyTicketData :: ticketData->TicketID:%lu \n",ticketData->ticketID);
	
	// Tag ID
	sprintf(ticketData->TagID,"%s", thirdPartyTicketData.TagID);
	printf("MakeThirdPartyTicketData :: thirdPartyEntryData.TagID:%s \n",thirdPartyTicketData.TagID);
	printf("MakeThirdPartyTicketData :: ticketData->TagID:%s \n",ticketData->TagID);

	// Plate Number
	sprintf(ticketData->plate,"%s", thirdPartyTicketData.Plate);	
	printf("MakeThirdPartyTicketData :: thirdPartyEntryData.Plate:%s \n",thirdPartyTicketData.Plate);
	printf("MakeThirdPartyTicketData :: ticketData->plate:%s \n",ticketData->plate);

	tm_ptr = localtime(&thirdPartyTicketData.ProcessDateTime);

	ticketData->in_year  = tm_ptr->tm_year + 1900;
	ticketData->in_month = tm_ptr->tm_mon + 1;
	ticketData->in_day	 = tm_ptr->tm_mday;
	ticketData->in_hour  = tm_ptr->tm_hour;
	ticketData->in_min	 = tm_ptr->tm_min;
	ticketData->in_sec	 = tm_ptr->tm_sec;

	if(IsINMachine == true)
	{
		ticketData->out_year  = 2000;
		ticketData->out_month = 1;
		ticketData->out_day   = 1;
		ticketData->out_hour  = 0;
		ticketData->out_min   = 0;
		ticketData->out_sec   = 0; 
	}

	//ShowMessage((char *)"MakeThirdPartyTicketData End.");
		ShowMessage((char *)"MakeThirdPartyTicketData Success.");
}

void MakeINHTicketData(HTicketData* hTicketData, TicketData ticketData)
{	
	time_t entry;
	struct tm EntryTime;
	char buf[128];	//nick add 20110124

	//ShowMessage((char *)"MakeINHTicketData Start.");
	
	memset(buf, '\0', sizeof(buf));
	
	EntryTime.tm_year = ticketData.in_year - 1900;
	EntryTime.tm_mon = ticketData.in_month - 1;
	EntryTime.tm_mday = ticketData.in_day;
	EntryTime.tm_hour = ticketData.in_hour;
	EntryTime.tm_min = ticketData.in_min;	
   	EntryTime.tm_sec = ticketData.in_sec;
	
	entry = mktime(&EntryTime);

	hTicketData->TicketID = ticketData.ticketID;
	sprintf(hTicketData->Plate,"%s",ticketData.plate);
	hTicketData->in_time = (unsigned long)entry;
	hTicketData->InStatus = true;
	hTicketData->AreaID = ticketData.areaId;
	sprintf(hTicketData->TagID,"%s",ticketData.TagID);
	
	//ShowMessage((char *)"MakeINHTicketData End.");
	ShowMessage((char *)"MakeINHTicketData Success.");

}

void MakeOUTHTicketData(HTicketData* hTicketData, TicketData ticketData)
{	
	time_t leave;
	char buf[128];	//nick add 20110124

	//ShowMessage((char *)"MakeOUTHTicketData Start.");
	
	memset(buf, '\0', sizeof(buf));
	
	leave = time((time_t *)0);

	hTicketData->TicketID = ticketData.ticketID;
	sprintf(hTicketData->Plate,"%s",ticketData.plate);
	hTicketData->in_time = (unsigned long)leave;
	hTicketData->InStatus = true;
	hTicketData->AreaID = ticketData.areaId;
	sprintf(hTicketData->TagID,"%s",ticketData.TagID);
	
	//ShowMessage((char *)"MakeOUTHTicketData End.");
	ShowMessage((char *)"MakeOUTHTicketData Success.");
}



bool CommDataGetParm(char* str ,short index,char parmsData[])
{
	char buf[64];
//	char *s = NULL;
//	char *s1 = NULL;
	short i,len,p=0;
	short point[25];
	short fix=0;
	
	point[p++] = 0;
	memset(buf,'\0',sizeof(buf));
	len = strlen(parmsData);
	parmsData[len-1]=0;
	
	printf("parmsData:%s len:%d\n",parmsData,len);
	
	for(i=0; i<len; i++)
	{
		if(parmsData[i] == 0x1c && index == 1)
		{
			fix = i+1;
		}
		if(parmsData[i] == 0x1F)
		{
			point[p] = i + 1;
			parmsData[i] = 0;
			parmsData[i-1] = 0;
			p++;
		}
	}
	
	sprintf(str,"%s",parmsData+point[index-1]+fix);
	printf("%d parm:%s\n", p, str);
	
	if(p >= index)
		return true;
	else
		return false;
}

void SendLoop1Status(bool isloop1ON)
{
	unsigned long udpSerial;
	//int loopON;
	bool status = false;
	int oldSyncValue = G_Loop1IOSycn.SyncValue;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return;
	}

	udpSerial = (GetTickCount() % 1000000L);

	if (isloop1ON == true)
	{	
		if(G_Loop1IOSycn.SwitchOn == false) G_Loop1IOSycn.BeSync = false;
	
		if((G_Loop1IOSycn.SyncValue != 1) && (G_Loop1IOSycn.BeSync == false))
		{
			G_Loop1IOSycn.BeSync = true;
			G_Loop1IOSycn.SyncValue = 1; 							
		}

		SendCarFullStatus();
		G_Loop1IOSycn.SwitchOn = true;
	}
	else
	{	
		if(G_Loop1IOSycn.SwitchOn == true) G_Loop1IOSycn.BeSync = false;
	
		if((G_Loop1IOSycn.SyncValue != 0) && (G_Loop1IOSycn.BeSync == false))
		{
			G_Loop1IOSycn.BeSync = true;
			G_Loop1IOSycn.SyncValue = 0; 							
		}
		
		G_Loop1IOSycn.SwitchOn = false;
	}

	if(G_Loop1IOSycn.BeSync == true)
	{
		status = SendLoop1TriggerToNewTerimal(udpSerial,G_Loop1IOSycn.SyncValue);
		
		if( status == false)
		{
			ShowMessage((char *)"Sync Loop1 Trigger To New Terimal failed.");
			G_Loop1IOSycn.SyncValue = oldSyncValue;	
		}
		else
		{
			ShowMessage((char *)"Sync Loop1 Trigger To New Terimal success.");
		}
		
		G_Loop1IOSycn.BeSync = false;
	}
}

void SendCarFullStatus()
{	
	int oldSyncValue = 0;
	unsigned long udpSerial;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return;
	}

	udpSerial = (GetTickCount() % 1000000L);
	oldSyncValue = G_CarFullStatusSycn.SyncValue;

	if(G_CarFullStatusSycn.SyncValue != G_iFullLevel)
	{
		G_CarFullStatusSycn.BeSync = true;
		G_CarFullStatusSycn.SyncValue = G_iFullLevel;
	}

	if(G_CarFullStatusSycn.BeSync == true)
	{
		status = SendCarFullTriggerToNewTerimal(udpSerial,G_CarFullStatusSycn.SyncValue);
		
		if(status == false)
		{
			G_CarFullStatusSycn.SyncValue = oldSyncValue;
			ShowMessage((char *)"Sync Car Full Status To New Terimal failed.");
		}else
		{
			ShowMessage((char *)"Sync Car Full Status To New Terimal success.");
		}
		
		G_CarFullStatusSycn.BeSync = false;
	}
}

void SendCarEnterStatus(int carInStatus)
{
	unsigned long udpSerial;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return;
	}
	
	udpSerial = (GetTickCount() % 1000000L);

	if(SendCarEnterStatusToNewTerimal(udpSerial,carInStatus) == true)
	{
		ShowMessage((char *)"Send Car Enter Status To New Terimal success.");
	}
	else
	{
		ShowMessage((char *)"Send Car Enter Status To New Terimal failed."); 	
	}

	G_Passage = false;
}

void SendCarEnterStart(TicketType ticketType)
{
	unsigned long udpSerial;
	TicketType _ticketType;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return;
	}

	G_Passage = true;
	_ticketType = ticketType;

	if(_ticketType == ThirdParty_Ticket_IssueHS)
		_ticketType = ThirdParty_Ticket;
	
	udpSerial = (GetTickCount() % 1000000L);

	if(SendCarEnterStartToNewTerimal(udpSerial,((int)ticketType)) == true)
	{
		ShowMessage((char *)"Send Car Enter Start To New Terimal success.");
	}
	else
	{
		ShowMessage((char *)"Send Car Enter Status To New Terimal failed."); 	
	}

}

bool SyncRFInStatus(st_IOSycn* rFInIOSycn)
{
	unsigned long udpSerial;
	int oldSyncValue = rFInIOSycn->SyncValue;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}

	udpSerial = (GetTickCount() % 1000000L);

	if (G_RFin == true)
	{	
		if(rFInIOSycn->SwitchOn == false) rFInIOSycn->BeSync = false;
	
		if((rFInIOSycn->SyncValue != 1) && (rFInIOSycn->BeSync == false))
		{
			rFInIOSycn->BeSync = true;
			rFInIOSycn->SyncValue = 1;								
		}

		rFInIOSycn->SwitchOn = true;
	}
	else
	{	
		if(rFInIOSycn->SwitchOn == true) rFInIOSycn->BeSync = false;
	
		if((rFInIOSycn->SyncValue != 0) && (rFInIOSycn->BeSync == false))
		{
			rFInIOSycn->BeSync = true;
			rFInIOSycn->SyncValue = 0;								
		}
		
		rFInIOSycn->SwitchOn = false;
	}

	if(rFInIOSycn->BeSync == true)
	{
		status = SendRFInTriggerToNewTerimal(udpSerial,rFInIOSycn->SyncValue);
		
		if( status == false)
		{
			rFInIOSycn->SyncValue = oldSyncValue;
			ShowMessage((char *)"Sync RF-In Trigger To New Terimal failed.");
		}
		else
		{
			ShowMessage((char *)"Sync RF-In Trigger To New Terimal success.");
		}
		
		rFInIOSycn->BeSync = false;
	}

	return status;
}


bool SyncLoop1Status(st_IOSycn* loop1IOSycn)
{
	unsigned long udpSerial;
	int oldSyncValue = loop1IOSycn->SyncValue;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}

	udpSerial = (GetTickCount() % 1000000L);

	if (GetLoop1() == true)
	{	
		if(loop1IOSycn->SwitchOn == false) loop1IOSycn->BeSync = false;
	
		if((loop1IOSycn->SyncValue != 1) && (loop1IOSycn->BeSync == false))
		{
			loop1IOSycn->BeSync = true;
			loop1IOSycn->SyncValue = 1;								
		}

		SendCarFullStatus();
		loop1IOSycn->SwitchOn = true;	
	}
	else
	{	
		if(loop1IOSycn->SwitchOn == true) loop1IOSycn->BeSync = false;
	
		if((loop1IOSycn->SyncValue != 0) && (loop1IOSycn->BeSync == false))
		{
			loop1IOSycn->BeSync = true;
			loop1IOSycn->SyncValue = 0;								
		}
		
		loop1IOSycn->SwitchOn = false;
	}

	if(loop1IOSycn->BeSync == true)
	{
		status = SendLoop1TriggerToNewTerimal(udpSerial,loop1IOSycn->SyncValue);
		
		if( status == false)
		{
			loop1IOSycn->SyncValue = oldSyncValue;
			ShowMessage((char *)"Sync Loop1 Trigger To New Terimal failed.");
		}
		else
		{
			ShowMessage((char *)"Sync Loop1 Trigger To New Terimal success.");
		}
		
		loop1IOSycn->BeSync = false;
	}

	return status;
}

bool SyncLoop2Status(st_IOSycn* loop2IOSycn)
{
	unsigned long udpSerial;
	int oldSyncValue = loop2IOSycn->SyncValue;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}

	udpSerial = (GetTickCount() % 1000000L);

	if (GetLoop2() == true)
	{	
		if(loop2IOSycn->SwitchOn == false) loop2IOSycn->BeSync = false;
	
		if((loop2IOSycn->SyncValue != 1) && (loop2IOSycn->BeSync == false))
		{
			loop2IOSycn->BeSync = true;
			loop2IOSycn->SyncValue = 1;								
		}
		
		loop2IOSycn->SwitchOn = true;
	}
	else
	{	
		if(loop2IOSycn->SwitchOn == true) loop2IOSycn->BeSync = false;
	
		if((loop2IOSycn->SyncValue != 0) && (loop2IOSycn->BeSync == false))
		{
			loop2IOSycn->BeSync = true;
			loop2IOSycn->SyncValue = 0;								
		}
		
		loop2IOSycn->SwitchOn = false;
	}

	if(loop2IOSycn->BeSync == true)
	{
		status = SendLoop2TriggerToNewTerimal(udpSerial,loop2IOSycn->SyncValue);
		
		if(status == false)
		{
			loop2IOSycn->SyncValue = oldSyncValue;
			ShowMessage((char *)"Sync Loop2 Trigger To New Terimal failed.");
		}
		else
		{
			ShowMessage((char *)"Sync Loop2 Trigger To New Terimal success.");
		}

		loop2IOSycn->BeSync = false;
	}

	return status;
}

bool SyncButtonStatus(st_IOSycn* buttonIOSycn)
{
	unsigned long udpSerial;
	int oldSyncValue = buttonIOSycn->SyncValue;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}
	
	udpSerial = (GetTickCount() % 1000000L);

	if (PressTicketButton() == true)
	{	
		if(buttonIOSycn->SwitchOn == false) buttonIOSycn->BeSync = false;
	
		if((buttonIOSycn->SyncValue != 1) && (buttonIOSycn->BeSync == false))
		{
			buttonIOSycn->BeSync = true;
			buttonIOSycn->SyncValue = 1;								
		}
		
		buttonIOSycn->SwitchOn = true;
	}
	else
	{	
		if(buttonIOSycn->SwitchOn == true) buttonIOSycn->BeSync = false;
	
		if((buttonIOSycn->SyncValue != 0) && (buttonIOSycn->BeSync == false))
		{
			buttonIOSycn->BeSync = true;
			buttonIOSycn->SyncValue = 0;								
		}
		
		buttonIOSycn->SwitchOn = false;
	}

	if(buttonIOSycn->BeSync == true)
	{
		status = SendButtonTriggerToNewTerimal(udpSerial,buttonIOSycn->SyncValue);
		
		if(status == false)
		{
			buttonIOSycn->SyncValue = oldSyncValue;
			ShowMessage((char *)"Sync Button Trigger To New Terimal failed.");
		}
		else
		{
			ShowMessage((char *)"Sync Button Trigger To New Terimal success.");
		}

		buttonIOSycn->BeSync = false;
	}

	return status;
}

bool SyncCarFullStatus(st_IOSycn* carFullStatusSycn)
{
	int oldFullLevel = 0;
	unsigned long udpSerial;
	bool status = false;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}

	udpSerial = (GetTickCount() % 1000000L);
	oldFullLevel = carFullStatusSycn->SyncValue;

	if(carFullStatusSycn->SyncValue != G_iFullLevel)
	{
		carFullStatusSycn->BeSync = true;
		carFullStatusSycn->SyncValue = G_iFullLevel;
	}

	if(carFullStatusSycn->BeSync == true)
	{
		status = SendCarFullTriggerToNewTerimal(udpSerial,carFullStatusSycn->SyncValue);
		
		if(status == false)
		{
			carFullStatusSycn->SyncValue = oldFullLevel;
			ShowMessage((char *)"Sync Car Full Status To New Terimal failed.");
		}else
		{
			ShowMessage((char *)"Sync Car Full Status To New Terimal success.");
		}
		
		carFullStatusSycn->BeSync = false;
	}

	return status;
}

void SyncSystemFaultToNewTerminal(void)
{
	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return;
	}

	enum TicketType ticketType = NONE_TICKET;
	ThirdPartyTicketData thirdPartyTicketData;
	
	ticketType = IdleProcessForNewTerminel(&thirdPartyTicketData);
	SendCarEnterStart(ticketType);
	usleep(50000L);
	SendCarEnterStatus(5);

	ShowMessage((char *)"Sync System Fault To New Terminal success.");
}

