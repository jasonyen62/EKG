/* Reader Control Functions */

#include <stdio.h>	/* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sqlite3.h>					//Frank add 20120509

#include "CommonDef.h"
#include "IPMS_Driver/rs232.h"
#include "IPMS_Driver/MF700.h"
#include "IPMS_Driver/CRT350.h"
#include "IPMS_Driver/Eltra1000.h"
#include "IPMS_Driver/D1000.h"
#include "IPMS_Driver/D3000.h"
#include "IPMS_Driver/BarCode.h"
#include "IPMS_Driver/Tup500.h"
#include "IPMS_Driver/MCP210.h"		//nick add 20120505
#include "IPMS_Driver/HF320.h" // nick add 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
#include "Dio.h"
#include "Datafile.h"					//Frank add 20120509
#include "Network.h"					//Frank add 20120813
#include "ServerTalk.h"					//Frank add 20120813

#include "Reader.h"
#include "test.h"
#include "traceLog.h"
#include "Voice.h"		//nick add 20130202 //

MF700 *mf700h			 = NULL;
MF700 *mf700h2 		 = NULL;
CRT350 *crt350h		 = NULL;
Eltra1000 *eltra1000h = NULL;
BarCode *barcode		 = NULL;
MF700 *mf700s			 = NULL;
CRT350 *crt350s		 = NULL;
Eltra1000 *eltra1000s = NULL;
MCP210 *r_mcp210		 = NULL; 	//nick add 20120505
TUP500 *printer		 = NULL;	// 20130117 Tony add
HF320 *hf320h			 = NULL; // nick add 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //
HF320 *hf320h2			 = NULL; // nick add 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //
HF320 *hf320s			 = NULL; // nick add 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //

ReaderConfig ReaderCFG;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long CalcTmpTime;  // 20120222 Tony add
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long ulCheckTicketTick = 0; // nick add 20130510 marge 20130624 //
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long ulCheckSeasonTicketTick = 0; // nick add 20130510 marge 20130624 //
unsigned long CalcTmpTime = 0; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
unsigned long ulCheckTicketTick = 0; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
unsigned long ulCheckSeasonTicketTick = 0; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //

bool IsINMachine = true;
int IssueDispense = 1;
char G_ReadType = 0;	//20130904 KARATE add
// ========================================================= //
// nick add s 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //
int HourlyReaderModule1 = 0;
int HourlyReaderModule2 = 0;
int HourlyDispenser1 = 0;
int HourlyDispenser2 = 0;
int SeasonReaderModule = 0;
// nick add e 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //
// ========================================================= //

void ReaderInitial()
{	// Initial reader port
	D3000 Retrive;
	D1000 Dispenser;

	ShowMessage((char *)"ReaderInitial()");	// 20130117 Tony add

	// Initial Hourly Ticket Reader //
	if (ReaderCFG.HourlyReaderType == 3)
	{	// ChipCoin
		printf("Hourly COM:[%d]\n", ReaderCFG.HourlyReaderComPort + 1);
		
		if (ReaderCFG.HourlyReaderComPort == NONE)
			HourlyReaderModule1 = -1;
		else
		{
			hf320h = new HF320();
			
			if (hf320h->init(ReaderCFG.HourlyReaderComPort) == false)
			{
				hf320h->Close();
				hf320h = NULL;
				
				mf700h = new MF700();
				mf700h->init(ReaderCFG.HourlyReaderComPort);

				/*
				if (mf700h->init(ReaderCFG.HourlyReaderComPort))
				{
					printf("Initial ChipCoin MF700 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort + 1);
					HourlyReaderModule1 = 0;
				}
				else
				{
					mf700h->Close();
					mf700h = NULL;
					HourlyReaderModule1 = -1;
				}
				*/
			}
			else
			{
				printf("Initial ChipCoin HF320 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort + 1);
				HourlyReaderModule1 = 1;
			}
		}
	}
	else if (ReaderCFG.HourlyReaderType == 4)
	{	// Mifare Card
		printf("Hourly COM:[%d]\n", ReaderCFG.HourlyReaderComPort + 1);
		
		if (ReaderCFG.HourlyReaderComPort == NONE)
			HourlyReaderModule1 = -1;
		else
		{
			hf320h = new HF320();
			
			if (hf320h->init(ReaderCFG.HourlyReaderComPort) == false)
			{
				hf320h->Close();
				hf320h = NULL;
				
				mf700h = new MF700();
				mf700h->init(ReaderCFG.HourlyReaderComPort);

				/*
				if (mf700h->init(ReaderCFG.HourlyReaderComPort))
				{
					printf("Initial Mifare MF700 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort + 1);
					HourlyReaderModule1 = 0;
				}
				else
				{
					mf700h->Close();
					mf700h = NULL;
					HourlyReaderModule1 = -1;
				}
				*/
			}
			else
			{
				printf("Initial Mifare HF320 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort + 1);
				HourlyReaderModule1 = 1;
			}
		}
		
		if (ReaderCFG.DispenserQuantity == 2)
		{
			if (ReaderCFG.HourlyReaderComPort2 == NONE)
				HourlyReaderModule2 = -1;
			else
			{
				hf320h2 = new HF320();
				
				if (hf320h2->init(ReaderCFG.HourlyReaderComPort2) == false)
				{
					hf320h2->Close();
					hf320h2 = NULL;
					
					mf700h2 = new MF700();
					mf700h2->init(ReaderCFG.HourlyReaderComPort2);

					/*
					if (mf700h2->init(ReaderCFG.HourlyReaderComPort2))
					{
						printf("Initial Mifare MF700 Hourly2 COM%d \n",ReaderCFG.HourlyReaderComPort2 + 1);
						HourlyReaderModule2 = 0;
					}
					else
					{
						mf700h2->Close();
						mf700h2 = NULL;
						HourlyReaderModule2 = -1;
					}
					*/
				}
				else
				{
					printf("Initial Mifare HF320 Hourly2 COM%d \n",ReaderCFG.HourlyReaderComPort2 + 1);
					HourlyReaderModule2 = 1;
				}
			}
		}
		
		if (IsINMachine)
		{
			printf("Initial D1000 COM%d \n",ReaderCFG.MifareDispenserComPort+1);
			
			Dispenser.init(ReaderCFG.MifareDispenserComPort);

			for (int i = 0; i < ReaderCFG.DispenserQuantity; i++)
			{
				if (Dispenser.Reset(IssueDispense))
					break;
				
				if (IssueDispense == 1)
					IssueDispense = 2;
				else
					IssueDispense = 1;
			}
		}
		else
		{
			printf("Initial D3000 COM%d \n",ReaderCFG.MifareDispenserComPort+1);
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.Reset(0);
		}
	}
	else if (ReaderCFG.HourlyReaderType == 5)
	{	// TUP592 (BarCode)
		if(IsINMachine)
		{
			Tup500Initial();
		}
		else
		{
			printf("Initial BarCode Reader COM%d \n",ReaderCFG.HourlyReaderComPort+1);
			barcode = new BarCode();
			barcode->init(ReaderCFG.HourlyReaderComPort);
			
			printf("Initial D3000  COM%d \n",ReaderCFG.MifareDispenserComPort+1);
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.Reset(0);
		}
	}
	else if (ReaderCFG.HourlyReaderType == 6)
	{	// MCP210 + Mifare + D1000
		if (ReaderCFG.HourlyReaderComPort == NONE)
			return;
		else
		{
			printf("Initial MCP210 COM%d \n", ReaderCFG.HourlyReaderComPort + 1);
			r_mcp210 = new MCP210();
			
			if(r_mcp210 ->Reset() == false)
			{
				ShowMessage((char *)"MCP210 Offline!");
				G_ParkingStatus.status &= (~STATUS_READER_CONNECT);					//Frank add 20121227
				r_mcp210->Close();
				r_mcp210 = NULL;
				return;
			}
		}
		
		if (ReaderCFG.HourlyReaderComPort2 == NONE)
			HourlyReaderModule1 = -1;
		else
		{
			hf320h = new HF320();
			
			if (hf320h->init(ReaderCFG.HourlyReaderComPort2) == false)
			{
				hf320h->Close();
				hf320h = NULL;
				
				mf700h = new MF700();
				mf700h->init(ReaderCFG.HourlyReaderComPort2);

				/*
				if (mf700h->init(ReaderCFG.HourlyReaderComPort2))
				{
					printf("Initial MF700 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort2 + 1);
					HourlyReaderModule1 = 0;
				}
				else
				{
					mf700h->Close();
					mf700h = NULL;
					HourlyReaderModule1 = -1;
				}
				*/
			}
			else
			{
				printf("Initial HF320 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort2 + 1);
				HourlyReaderModule1 = 1;
			}
		}
		
		if (IsINMachine)
		{
			printf("Initial D1800 COM%d \n", ReaderCFG.MifareDispenserComPort + 1);
			Dispenser.init(ReaderCFG.MifareDispenserComPort);
			
			if(Dispenser.Reset(IssueDispense) == false)
			{
				ShowMessage((char *)"Dispenser Offline!");
				G_ParkingStatus.status &= (~STATUS_READER_CONNECT);
				return;
			}
		}
	}
	// ========================================================= //
	// nick add s 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	else if (ReaderCFG.HourlyReaderType == 7)
	{  // Visual token use mp90
		printf("Hourly COM:[%d]\n", ReaderCFG.HourlyReaderComPort + 1);
		
		if (ReaderCFG.HourlyReaderComPort == NONE)
			HourlyReaderModule1 = -1;
		else
		{
			hf320h = new HF320();
			
			if (hf320h->init(ReaderCFG.HourlyReaderComPort) == false)
			{
				printf("Initial ChipCoin HF320 Hourly COM%d fail.\n",ReaderCFG.HourlyReaderComPort + 1);
				HourlyReaderModule1 = -1;
			}
			else
			{
				hf320h->SetAPDUMode(true);
				printf("Initial ChipCoin HF320 Hourly COM%d \n",ReaderCFG.HourlyReaderComPort + 1);
				HourlyReaderModule1 = 1;
			}
		}
	}
	// nick add e 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// ========================================================= //
	//
	// Season Ticket Reader //
	if (ReaderCFG.SeasonReaderType == 3)
	{
		printf("Initial Season COM:[%d]\n", ReaderCFG.SeasonReaderComPort + 1);
		
		if (ReaderCFG.SeasonReaderComPort == NONE)
			SeasonReaderModule = -1;
		else
		{
			hf320s = new HF320();
			
			if (hf320s->init(ReaderCFG.SeasonReaderComPort) == false)
			{
				hf320s->Close();
				hf320s = NULL;
				
				mf700s = new MF700();
				mf700s->init(ReaderCFG.SeasonReaderComPort);

				/*
				if (mf700s->init(ReaderCFG.SeasonReaderComPort))
				{
					printf("Initial MF700 Season COM%d \n",ReaderCFG.SeasonReaderComPort + 1);
					SeasonReaderModule = 0;
				}
				else
				{
					mf700s->Close();
					mf700s = NULL;
					SeasonReaderModule = -1;
				}
				*/
			}
			else
			{
				printf("Initial HF320 Season COM%d \n",ReaderCFG.SeasonReaderComPort + 1);
				SeasonReaderModule = 1;
			}
		}
	}
	//

	printf("HourlyReaderModule1:%d, HourlyReaderModule2:%d, SeasonReaderModule:%d\n", HourlyReaderModule1, HourlyReaderModule2, SeasonReaderModule);
	
	G_ParkingStatus.status |= STATUS_READER_CONNECT;
}

bool CheckTicketEmpty()
{
	switch(ReaderCFG.HourlyReaderType)
	{
		default:
		case 0: //none
			break;
		case 1: //Eltra
			break;
		case 2: //CRT350
			break;
		case 3: // ChipCoin
		case 7: // Visual token // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
			if(ReaderCFG.Ticket1 == 0)
			{
				return true;
			}
			
			break;
		case 4:
			// 只有入口
			if(IsINMachine == true)
			{
				D1000 Dispense;
				
				// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
				Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				
				if(Dispense.IsEmpty(IssueDispense) == true)
				{
					if(ReaderCFG.DispenserQuantity > 1)
					{
						if(IssueDispense == 1)
						{
							ReaderCFG.Ticket1 = 0;
							IssueDispense = 2;
							
							if(Dispense.IsEmpty(IssueDispense) == true)
							{
								ReaderCFG.Ticket2 = 0;
								IssueDispense = 1;
								return true;
							}
						}
						else if(IssueDispense == 2)
						{
							ReaderCFG.Ticket2 = 0;
							IssueDispense = 1;
							
							if(Dispense.IsEmpty(IssueDispense) == true)
							{
								ReaderCFG.Ticket1 = 0;
								IssueDispense = 2;
								return true;
							}
						}
					}
					else
					{
						ReaderCFG.Ticket1 = 0;
						return true;
					}
				}
			}
			
			break;
		case 5:
		{
			// 20130117 Tony mark TUP500 printer;
			// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);
			
			// 20130117 Tony mark if(printer.CheckPaperEnd()==true)
			if(printer->CheckPaperEnd()==true)	// 20130117 Tony add
			{
				ShowMessage((char *)"CheckTicketEmpty :: TUP500 : PaperEnd");	// 20120427 Tony add
				return 1;
			}
			
			break;
		}
		//Frank add s 20120508
		case 6:
			if(IsINMachine == true)
			{
				char status[16];					//Frank add 20120821
				D1000 Dispense;
				
				// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
				Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				
				//Frank add s 20120821
				memset (status , '\0' , sizeof(status));
				
				if(Dispense.GetStatusD2(IssueDispense,status) == 1)
				{
					if(status[7] == '2')
					//Frank add e 20120821
					{
						ReaderCFG.Ticket1 = 0;
						return true;
					}
				}					//Frank add 20120821
			}
			break;
		//Frank add e 20120508
	} //End switch
	
	return false;
}

char CheckTicketIssue()
{	//return: -1:error; 0:no Ticket 1:detected Ticket
	//int i;
	int iRet = 0; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	char Tag[16]; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	
	switch(ReaderCFG.HourlyReaderType)
	{
		default:
		case 0: //none
			break;
		case 1: //Eltra
			break;
		case 2: //CRT350
			break;
		case 3: // ChipCoin
		case 7: // Visual token
			//for (i = 0; i < 3; i++)
			{
				// ================================== //
				// nick add s 20130510 marge 20130624 //
				if(CheckCoinOnInlet() == true)
					return 1;
				
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				memset(Tag, 0, sizeof(Tag));
				
				if (HourlyReaderModule1 == 0)
				{
					if (mf700h != NULL)
						iRet = mf700h->CacheCard();
				}
				else if (HourlyReaderModule1 == 1)
				{
					memset(Tag, 0, sizeof(Tag));
					
					if (hf320h != NULL)
						iRet = hf320h->CacheCard(Tag);
				}
				// ========================================================= //
				// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				else if (HourlyReaderModule1 < 0)
					return(0);
				// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// ========================================================= //
				
				if (iRet == 1)
					return (iRet);
				
				usleep(100000L);
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
				
				ulCheckTicketTick = GetTickCount();
			}
			
			break;
		case 4:
			// 只有入口有
			if(IsINMachine == true)
			{
				//D1000 Dispense;
				//Dispense.init(ReaderCFG.MifareDispenserComPort);
				
				//if(Dispense.IsCardInMachine(IssueDispense) == true)
				//{
				//	printf("card in machine\n");
				//	return 1;
				//}
				
				//usleep(50000L);
				
				if(IssueDispense == 1)
				{
					// ========================================================== //
					// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					//if(mf700h != NULL)
					//{
					//	//Frank mark 20120713 usleep(100000L);
					//	return mf700h ->CacheCard();
					//}
					// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================== //
					// ========================================================= //
					// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					if (HourlyReaderModule1 == 0)
					{
						if(mf700h != NULL)
							iRet = mf700h->CacheCard();
					}
					else if (HourlyReaderModule1 == 1)
					{
						memset(Tag, 0, sizeof(Tag));
						
						if (hf320h != NULL)
							iRet = hf320h->CacheCard(Tag);
					}
					// ========================================================= //
					// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					else if (HourlyReaderModule1 < 0)
						return(0);
					// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
					// ========================================================= //
					
					if (iRet == 1)
						return 1;
					else
						return 0;
					// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================= //
				}
				else
				{
					// ========================================================== //
					// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					//if(mf700h2 != NULL)
					//{
					//	//Frank mark 20120713 usleep(100000L);
					//	return mf700h2 ->CacheCard();
					//}
					// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================== //
					// ========================================================= //
					// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					if (HourlyReaderModule2 == 0)
					{
						if(mf700h2 != NULL)
							iRet = mf700h2->CacheCard();
					}
					else if (HourlyReaderModule2 == 1)
					{
						memset(Tag, 0, sizeof(Tag));
						
						if (hf320h2 != NULL)
							iRet = hf320h2->CacheCard(Tag);
					}
					
					if (iRet == 1)
						return 1;
					else
						return 0;
					// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================= //
				}
			}
			
			break;
		case 5:
		{
			// 20130117 Tony mark TUP500 printer;
			// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);
			
			//if(printer.CheckPaperOnOutlet()==true)					//Frank mark 20111122
			// 20120426 Tony mark if( (printer.CheckPaperOnOutlet()==true) && (CheckTicketEmpty()==0))					//Frank add 20111122
			// 20130117 Tony mark if( (printer.CheckPaperOnOutlet()==true) && (printer.CheckPaperEnd()==false))	// 201020426 Tony add
			if( (printer->CheckPaperOnOutlet()==true) && (printer->CheckPaperEnd()==false))	// 20130117 Tony add
			{//Frank add 20121226
				ShowMessage((char *)"CheckTicketIssue :: TUP500 : PaperOnOutlet and PaperEnd");	// 20120427 Tony add
				return 1;
			}//Frank add 20121226
			break;
		}
		//nick add s 20120505
		case 6:
			if (r_mcp210 == NULL) return 0;
			
			if (r_mcp210 ->GetStatus() == 3)
				return 1;
				
			break;
		//nick add e 20120505
	}
	
	return 0;
}

char GetSeasonTicketInsert(char* tag)
{
	char buf[128];
	char iRet = 0;
	
	memset(buf,'\0',sizeof(buf));
	
	//nick add s 20120505
	switch(ReaderCFG.HourlyReaderType)
	{
		default:
		case 0:
			break;
		case 6:
			if(IsINMachine==true)
			{
				if (r_mcp210 == NULL) return 0;
				
				if (r_mcp210->GetStatus() == 1)
					return (r_mcp210->InsertTicket());
			}
			
			break;
	}
	//nick add e 20120505
	
	switch(ReaderCFG.SeasonReaderType )
	{
		default:
		case 0:			
			break;
		case 1: //Eltra
			break;
		case 2: //CRT350
			break;
		case 3:	// only MF700
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //			if (CheckTimeout(ulCheckSeasonTicketTick, 300))
			{
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				//if(mf700s != NULL)
				//{
				//	iRet = mf700s->CacheCard(tag);
	         //
				//	if (iRet == 1)
				//	{
				//		sprintf(buf,"Season Tag: [%s].", tag);
				//		ShowMessage(buf);
				//	}
				//	
				//	ulCheckSeasonTicketTick = GetTickCount();
				//	return iRet;
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				if (SeasonReaderModule == 0)
				{
					if(mf700s != NULL)
						iRet = mf700s->CacheCard(tag);
				}
				else if (SeasonReaderModule == 1)
				{
					if(hf320s != NULL)
						iRet = hf320s->CacheCard(tag);
				}
				
				if (iRet == 1)
				{
					sprintf(buf,"Season Tag: [%s].", tag);
					ShowMessage(buf);
				}
				
				ulCheckSeasonTicketTick = GetTickCount();
				return iRet;
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
			}
			
			break;
		case 4: //MF700 with 收卡機
			if(IsINMachine == false)
			{
				D3000 Retrive ;
				
				Retrive.init(ReaderCFG.MifareDispenserComPort);
				
				if(Retrive.IsCardInMachine(0)==true)
				{
					return 1;
				}
			}
			
			break;
	}
	
	return 0;
}

void ReaderReset()
{
	switch(ReaderCFG.HourlyReaderType)
	{
		case 4: // MF700 + 收卡機
		case 5:	// D3000 + Barcode Reader
		{
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.Reset(0);
			//printf("Reset D3000\n");
			break;
		}
		//Frank add s 20120508
		case 6:
			//Frank add s 20120824
			if(IsINMachine == true)
			{
				D1000 Dispenser;
			
				// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispenser.init(ReaderCFG.MifareDispenserComPort);
				Dispenser.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				Dispenser.Reset(IssueDispense);
			}
			//Frank add e 20120824
			r_mcp210 ->Reset();
			break;
		//Frank add e 20120508
	}
	
	switch(ReaderCFG.SeasonReaderType)
	{
		case 3:	//MF700
		case 7: // Visual token // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
			//printf("Reset MF700 season\n");
			//mf700s->mfHalt();
			break;
		case 4: // MF700 + 收卡機
		case 5:	// D3000 + Barcode Reader
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.Reset(0);
			break;
	}
}

char GetHourlyTicketInsert()
{
	char buf[128];	//nick add 20110124
	char tag[80];	//nick add 20110124
	char iRet = 0;		//nick add 20110124
	
	memset(buf, '\0', sizeof(buf));	//nick add 20110124
	memset(tag, '\0', sizeof(tag));	//nick add 20110124
	
	switch(ReaderCFG.HourlyReaderType)
	{
		default:
		case 0: //none or 月票,計時票共用讀卡機
//nick mark s 20110124
			/*
			if(mf700h != NULL)
			{
				if(mf700h->mfHalt()==-1)
					return -1;
				return mf700h->CacheCard();
			}
			
			break;
			*/
//nick mark e 20110124

			//nick add s 20110124
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //			if (CheckTimeout(ulCheckTicketTick, 300))
			{
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				//if(mf700h != NULL)
				//{
				//	iRet = mf700h->CacheCard(tag);
				//	
				//	if (iRet == 1)
				//	{
				//		sprintf(buf,"CPCO Read Tag Hourly Insert: [%s].",tag);
				//		ShowMessage(buf);
				//	}
				//	
				//	ulCheckTicketTick = GetTickCount();
				//	return iRet;
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				if (HourlyReaderModule1 == 0)
				{
					if(mf700h != NULL)
						iRet = mf700h->CacheCard(tag);
				}
				else if (HourlyReaderModule1 == 1)
				{
					if (hf320h != NULL)
						iRet = hf320h->CacheCard(tag);
				}
				// ========================================================= //
				// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				else if (HourlyReaderModule1 < 0)
					return(0);
				// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// ========================================================= //
				
				if (iRet == 1)
				{
					sprintf(buf,"CPCO Read Tag Hourly Insert: [%s].",tag);
					ShowMessage(buf);
				}
				
				ulCheckTicketTick = GetTickCount();
				return iRet;
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
			}
			//nick add e 20110124
			
			break;
		case 1: //Eltra
			break;
		case 2: //CRT350
			break;
		case 3:	// MF700 (chip coin 機構)
		case 7: // Visual token // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
			if(CheckCoinOnInlet()== true)
			{
				ShowMessage((char *)"Senser Get CC.");
				return 1;
			}
				
			//nick add s 20110124
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //			if (CheckTimeout(ulCheckTicketTick, 300))
			{
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				//if(mf700h != NULL)
				//{
				//	iRet = mf700h->CacheCard(tag);
				//	
				//	if (iRet == 1)
				//	{
				//		sprintf(buf,"CPCO Read Tag Hourly Insert: [%s].",tag);
				//		ShowMessage(buf);
				//	}
				//	
				//	ulCheckTicketTick = GetTickCount();
				//	return iRet;
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				if (HourlyReaderModule1 == 0)
				{
					if(mf700h != NULL)
						iRet = mf700h->CacheCard(tag);
				}
				else if (HourlyReaderModule1 == 1)
				{
					if (hf320h != NULL)
						iRet = hf320h->CacheCard(tag);
				}
				// ========================================================= //
				// nick add s 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				else if (HourlyReaderModule1 < 0)
					return(0);
				// nick add e 20150515 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// ========================================================= //
				
				if (iRet == 1)
				{
					sprintf(buf,"CPCO Read Tag Hourly Insert: [%s].",tag);
					ShowMessage(buf);
				}
				
				ulCheckTicketTick = GetTickCount();
				return iRet;
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
			}
			//nick add e 20110124
			
			break;
		case 4: // MF700 + 收卡機
			//if(mf700h != NULL)
			//	return mf700h->CacheCard();
			
			if(IsINMachine == false)
			{
				D3000 Retrive;
				
				Retrive.init(ReaderCFG.MifareDispenserComPort);
				
				if( Retrive.IsCardInMachine(0) == true)
				{
					return 1;
				}
			}
			
			break;
		case 5: // Barcode + 收卡機
			if(IsINMachine == false)
			{
				D3000 Retrive;
				
				Retrive.init(ReaderCFG.MifareDispenserComPort);
				
				if(Retrive.IsCardInMachine(0) == true)
				{
					return 1;
				}
			}
			
			break;
		//Frank add s 20120508
		case 6:
			if(r_mcp210 == NULL) return 0;
			
			if(r_mcp210 ->GetStatus() == 1)
				//Frank mark 20120824 return (r_mcp210->InsertTicket());
			//Frank add s 20120827
			{
				r_mcp210 ->InsertTicket();
			}
			if(r_mcp210 ->GetStatus() == 3)
			{
				return 1;
			}
			//Frank add e 20120827
			
			break;
		 //Frank add e 20120508
	}
	
	return 0;
}

bool CheckTakeTicket(enum TicketType ticketType)
{
	int i;
	bool	bRet = false;
	
	switch(ticketType)
	{
		case ThirdParty_Ticket_IssueHS:
		case UNKNOW_TICKET:
		case TICKET_READ_ERROR:
		case HOURLY_TICKET:
		case SEASON_TICKET_H:
		case HOURLY_S:					//Frank add 20120516
			// nick mark 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
			if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
			{	 //chip coin
				if(CheckCoinOnOutlet() == false)
				{
					bRet = true;
					DrawChipCoin(false); //關shutter
				}
			}
			else if(ReaderCFG.HourlyReaderType == 2)
			{
			}
			else if(ReaderCFG.HourlyReaderType == 1)
			{
			}
			else if(ReaderCFG.HourlyReaderType == 4)
			{
				if(IsINMachine == true)
				{	//入口
					D1000 Dispense;				
					// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
					Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					
					if(Dispense.IsCardIn(IssueDispense) == false)
					{	//卡票不在(被拿走)時 return true;
						bRet = true;
					}
				}
				else
				{	//出口
					D3000 Retrive;
					Retrive.init(ReaderCFG.MifareDispenserComPort);
					Retrive.EnableWork(0,D3000_ENABLE);
					
					if(Retrive.IsCardIn(0)==false)
					{
						Retrive.EnableWork(0,D3000_DISABLE);
						return true;
					}
					
					Retrive.EnableWork(0,D3000_DISABLE);
				}
			}
			else if(ReaderCFG.HourlyReaderType == 5)
			{
				if(IsINMachine == true)
				{
					// 20130117 Tony mark TUP500 printer;
					
					// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);
					
					// 20130117 Tony mark if(printer.CheckPaperOnOutlet()==false)
					if(printer->CheckPaperOnOutlet()==false)	// 20130117 Tony add
						return true;
				}
				else
				{
					D3000 Retrive;
					Retrive.init(ReaderCFG.MifareDispenserComPort);
					Retrive.EnableWork(0,D3000_ENABLE);
					
					if(Retrive.IsCardIn(0)==false)
					{
						Retrive.EnableWork(0,D3000_DISABLE);
						return true;
					}
					
					Retrive.EnableWork(0,D3000_DISABLE);
				}
			}
			//Frank add s 20120508
			else if(ReaderCFG.HourlyReaderType == 6)
			{
				if(r_mcp210 ->GetStatus() != 2)
					bRet = true;
			}
			//Frank add e 20120508
			break;
		case VALUE_TICKET:
		case SEASON_TICKET:
		case TICKET_SEASON_READ_ERROR:
			if(ReaderCFG.SeasonReaderType == 3)
			{
				return true;
			}
			else if(ReaderCFG.SeasonReaderType == 2)
			{
			}
			else if(ReaderCFG.SeasonReaderType == 1)
			{
			}
			else if(ReaderCFG.HourlyReaderType == 4)
			{
				if(IsINMachine == true)
				{	//入口
					D1000 Dispense;
					// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
					Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					
					for(i=0;i<ReaderCFG.DispenserQuantity;i++)
					{
						// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //if(Dispense.IsCardIn(i+1) == false)
						if(Dispense.IsCardIn(IssueDispense) == false) // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						{
							return true;
						}
						
						// ========================================================= //
						// nick add s 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						if (IssueDispense == 1)
							IssueDispense = 2;
						else
							IssueDispense = 1;
						// nick add e 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================= //
					}
				}
				else
				{	//出口
					D3000 Retrive;
					Retrive.init(ReaderCFG.MifareDispenserComPort);
					Retrive.EnableWork(0,D3000_ENABLE);
					
					if(Retrive.IsCardIn(0)==false)
					{
						Retrive.EnableWork(0,D3000_DISABLE);
						return true;
					}
					
					Retrive.EnableWork(0,D3000_DISABLE);
				}
			}
			//Frank add s 20120508
			else if(ReaderCFG.HourlyReaderType == 6)
			{
				if(r_mcp210 ->GetStatus() != 2)
					bRet = true;
			}
			//Frank add e 20120508
			break;
		case EASY_CARD:
			return true;
		default:
			break;
	}
	
	return bRet;
}

char IssueTicket(bool bTest)
{
	char buf[80];
	int retry = 0;
	int iGetRet = 0; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	char IssueOK = 1; // 1:OK 0:no Ticket Issue	-1:Equipment fault
	
	printf("IssueTicket :: Start : %ld\n",GetTickCount()-CalcTmpTime);
	ShowMessage((char *)"Issue Ticket.",1);
	//if(CheckTicketEmpty() == true)
	//	return 0;
	
	// nick mark 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
	if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		//for(retry=0;retry<3;retry++)
		{
			IssueDispense = 1;
			IssueChipCoin1();
			
			if(ReaderCFG.Ticket1 < 0)
				ReaderCFG.Ticket1 = 0;
			
			if(bTest) // Todo: Visual token 測試票功能尚未開發
			{//有問題回收,沒問題退出來
				//Frank add s 20121114
				int retry;
				char Tag[24];
				unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
				sectorData sdata , rsdata;
				
				time_t now;
				struct tm *tm_ptr = NULL;
				
				usleep(500000L);
				
				for(retry = 0 ; retry < 3 ; retry++)
				{
					if (CheckTicketIssue() == 1) // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					{
						memset(&sdata , 0xFF , sizeof(sdata));
						memset(&rsdata , 0xFF , sizeof(rsdata));
						
						IssueOK = -1;
						
						// ========================================================== //
						// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						//if(mf700h->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata) == 1)
						//{
						//	ReadData(rsdata);
						//	ShowMessage((char*)"ReadSector OK!");
						//}
						//else
						//{
						//	ShowMessage((char*)"ReadSector NG!");
						//	continue;
						//}
						// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================== //
						
						now = time((time_t *)0);
						tm_ptr = localtime(&now);
						Int2BCD(tm_ptr ->tm_year + 1900 , 4 , &sdata.block1[0]);
						Int2BCD(tm_ptr ->tm_mon + 1 , 2 , &sdata.block1[2]);
						Int2BCD(tm_ptr ->tm_mday , 2 , &sdata.block1[3]);
						Int2BCD(tm_ptr ->tm_hour , 2 , &sdata.block1[4]);
						Int2BCD(tm_ptr ->tm_min , 2 , &sdata.block1[5]);
						Int2BCD(tm_ptr ->tm_sec , 2 , &sdata.block1[6]);
						
						memcpy(&sdata.block2 , &sdata.block1 , sizeof(sdata.block1));
						memcpy(&sdata.block3 , &sdata.block1 , sizeof(sdata.block1));
						
						// ========================================================== //
						// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						//if(mf700h->WriteSector(Tag , PARKING_DATA_SECTOR , sdata) == 1)
						//{
						//	WriteData(sdata);
						//	ShowMessage((char*)"WriteSector OK!");
						//}
						//else
						//{
						//	ShowMessage((char*)"WriteSector NG!");
						//	continue;
						//}
						//
						//if(mf700h ->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata) == 1)
						//{
						//	ReadData(rsdata);
						//	ShowMessage((char*)"ReadSector OK!");
						//}
						//else
						//{
						//	ShowMessage((char*)"ReadSector NG!");
						//	continue;
						//}
						// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================== //
						
						// ========================================================= //
						// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						iGetRet = 0;
						
						if (HourlyReaderModule1 == 0)
						{
							if (mf700h != NULL)
								iGetRet = mf700h->WriteSector(Tag , PARKING_DATA_SECTOR , sdata);
						}
						else if (HourlyReaderModule1 == 1)
						{
							if (hf320h != NULL)
								iGetRet = hf320h->WriteSector(Tag , PARKING_DATA_SECTOR , sdata);
						}
						
						if (iGetRet == 1)
						{
							WriteData(sdata);
							ShowMessage((char*)"WriteSector OK!");
						}
						else
						{
							ShowMessage((char*)"WriteSector NG!");
							continue;
						}

						iGetRet = 0;
						
						if (HourlyReaderModule1 == 0)
						{
							if (mf700h != NULL)
								iGetRet = mf700h->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata);
						}
						else if (HourlyReaderModule1 == 1)
						{
							if (hf320h != NULL)
								iGetRet = hf320h->ReadSector(Tag , PARKING_DATA_SECTOR , &rsdata);
						}
						
						if(iGetRet == 1)
						{
							ReadData(rsdata);
							ShowMessage((char*)"ReadSector OK!");
						}
						else
						{
							ShowMessage((char*)"ReadSector NG!");
							continue;
						}
						// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================= //
						
						if(memcmp(&sdata , &rsdata , sizeof(rsdata)) == 0)
						{
							ShowMessage((char*)"The Same!");
							IssueOK = 1;
							break;
						}
						else
						{
							ShowMessage((char*)"Different!");
						}
					}
				}
				
				if(IssueOK == -1)
				{
					HopperReset();
					RecycleTicket();
				}
				else if (retry >= 3)
				{
					HopperReset();
					IssueOK = -1;
				}
				else
				{
					DrawChipCoin(true);
					usleep(500000L);
					Ticks = GetTickCount();
					
					while(CheckTimeout(&Ticks, (unsigned long)5000L) == false)
					{
						if(CheckCoinOnOutlet() == false)
							break;

						usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
					}
					
					usleep(500000L);
					DrawChipCoin(false);
				}
				//Frank add e 20121114
			}
		}
	}
	else if(ReaderCFG.HourlyReaderType == 4)
	{
		D1000 Dispense;
		
		// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
		Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		
		if(ReaderCFG.DispenserQuantity > 1)
		{	 //有2台發卡機
			// ========================================================= //
			// nick add s 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //
			for (retry = 0; retry < 2; retry++)
			{
				// ========================================================= //
				// nick add s 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				if (retry > 0)
				{
					if (CheckTicketIssue() == 1)
						break;
				}
				// nick add e 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
				
				if (Dispense.IsEmpty(IssueDispense) == true)
				{
					if (IssueDispense == 1)
						IssueDispense = 2;
					else
						IssueDispense = 1;
					
					continue;
				}
				
				if(G_ReadType == 1)
				{
					printf("Read Type = D1000\n");
					
					if (Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER) == 1)
						break;
				}
				else
				{
					printf("Read Type = ACT_F1\n");
					
					if(Dispense.DispenseCard(IssueDispense,ACT_F1_DISPENSE_TO_READER) == 1)
						break;
				}
			}
			
			if (retry >= 2)
				IssueOK = 0;
			// nick add e 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //
			// ========================================================= //
			
			// ========================================================== //
			// nick mark s 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //
			//if(G_ReadType == 1)		//20130904 KARATE add
			//{
			//	printf("Read Type = D1000\n");
			//	if(Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER)==-1)
			//		IssueOK=-1;
			//}
			//else //20130904 KARATE add s
			//{
			//	printf("Read Type = ACT_F1\n");
			//	if(Dispense.DispenseCard(IssueDispense,ACT_F1_DISPENSE_TO_READER)==-1)
			//		IssueOK=-1;
			//}	//20130904 KARATE add e
			// nick mark s 20140905 Ver:000-000-GIO_V2-135101-0006-13B251 //
			// ========================================================== //
			
			//nick mark 20120220 if(IssueDispense==2)					//Frank modify 20120117
				//ReaderCFG.Ticket1--;					//Frank mark 20120117
			//else					//Frank mark 20120117
				//nick mark 20120220 ReaderCFG.Ticket2--;
				
			//nick mark 20120220 sprintf(buf,"Issue Dispenser %d ",IssueDispense);
			//nick mark 20120220 ShowMessage(buf);
				
			if(bTest==true)
				Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_OUT);
		}
		else
		{
			IssueDispense = 1;
			
			//nick mark 20120220 sprintf(buf,"Issue Dispenser.");
			//nick mark 20120220 ShowMessage(buf);
			// 20120221 Tony mark Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER);
			//ReaderCFG.Ticket1--;					//Frank mark 20120117
			
			// 20120221 Tony add s
			if(G_ReadType == 1)		//20130904 KARATE add
			{
				printf("Read Type = D1000\n");
				if(Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER)==-1)
					IssueOK=-1;
				}
			else //20130904 KARATE add s
			{
				printf("Read Type = ACT_F1\n");
				if(Dispense.DispenseCard(IssueDispense,ACT_F1_DISPENSE_TO_READER)==-1)
					IssueOK=-1;
			}	//20130904 KARATE add e
			// 20120221 Tony add e
			
			if(bTest==true)
			{
				Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_OUT);
			}
		}
		
		// ========================================================== //
		// nick mark s 20151230 Ver:000-000-GIO_V2-13B251-0007-13C241 //
		//// 20120221 Tony add s
		//if(IssueOK==1)
		//{
		//	if(IssueDispense==1)
		//	{
		//		ReaderCFG.Ticket1--;
		//		sprintf(buf,"Issue Dispenser[%d] : %d",IssueDispense,ReaderCFG.Ticket1);
		//		ShowMessage(buf);
		//	}
		//	else if(IssueDispense==2)
		//	{
		//		ReaderCFG.Ticket2--;
		//		sprintf(buf,"Issue Dispenser[%d] : %d ",IssueDispense,ReaderCFG.Ticket2);
		//		ShowMessage(buf);
		//	}
		//}
		//// 20120221 Tony add e
		// nick mark e 20151230 Ver:000-000-GIO_V2-13B251-0007-13C241 //
		// ========================================================== //
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		// 20130117 Tony mark TUP500 printer;
//		char status;
//		char datas[128];
		
		// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);

		if(printer->CheckOffline()==true)	// 20130117 Tony add
		// 20130117 Tony mark if(printer.CheckOffline()==true)
		{					//Frank add 20111122 
			// nick mark 20120222 G_ParkingStatus.status |= STATUS_READER_CONNECT;
			G_ParkingStatus.status &= (~STATUS_READER_CONNECT); // nick add 20120222
			// 20120427 Tony mark ShowMessage((char *)"TUP500 Offline!");				//Frank add s 20111122
			ShowMessage((char *)"IssueTicket :: TUP500 : Offline!");	// 20120427 Tony add
			IssueOK = 0;
		}						//Frank add e 20111122
		else
		{
			if ((G_ParkingStatus.status & STATUS_READER_CONNECT) == 0)					//Frank modify 20120223
				// 20130117 Tony mark ReaderInitial();					//Frank add 20111122
				Tup500Initial();	// 20130117 Tony add
			G_ParkingStatus.status |= STATUS_READER_CONNECT;					//Frank add 20120223
			//Frank mark 20120806 IssueOK = 0;		//nick add 20111024
		}

		if(printer->CheckNearEnd()==true)	// 20130117 Tony add			
		// 20130117 Tony mark if(printer.CheckNearEnd()==true)
		{
			G_ParkingStatus.status |= STATUS_TICKET_LOW;
			ShowMessage((char *)"IssueTicket :: TUP500 : NearEnd");	// 20120427 Tony add
		}
		
		// 20120426 Tony mark if(CheckTicketEmpty() == true)					//Frank add s 20111116
		// 20130117 Tony mark if(printer.CheckPaperEnd()==true)	// 20120426 Tony add
		if(printer->CheckPaperEnd()==true)	// 20130117 Tony add
		{
			ShowMessage((char *)"IssueTicket :: TUP500 : PaperEnd");	// 20120427 Tony add
			IssueOK = 0;
			G_ParkingStatus.status |= STATUS_TICKET_EMPTY;		//nick add 20120220
		}					//Frank add e 20111116
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
		//ReaderCFG.Ticket1--;					//Frank mark 20120117
		IssueOK = 1;
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
		//ReaderCFG.Ticket1--;					//Frank mark 20120117
		IssueOK = 1;
	}
	//Frank add s 20120508
	else if(ReaderCFG.HourlyReaderType == 6)
	{
		D1000 Dispense;
		char status [16];					//Frank add 20120821
		
		memset(status , '\0' , sizeof(status));					//Frank add 20120821
		
		// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
		Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		
		//Frank add s 20120821
		if(bTest == true)
		{
			IssueOK = -1;
		}
		//Frank add e 20120821
		
		if(Dispense.DispenseCardOut(IssueDispense) == -1)
		//Frank add s 20120813
		{
			sprintf(buf , "D1800 dispense card fail!");
			ShowMessage(buf);
			SendAlarm(38 , buf);
		//Frank add e 20120813
			IssueOK = -1;
		}					//Frank add 20120813
		else
		{
			usleep(50000L);
			//Frank add s 20120821
			Dispense.GetStatusD2(IssueDispense , status);
			
			if(status[6] == '1')
			//Frank add e 20120821
				r_mcp210 ->InsertTicket();
		}
	}
	//Frank add e 20120508
	
//Frank mark 20120511	if ((IssueOK == 1 )||(IssueDispense == 1))					//Frank add s 20120117
	if (IssueOK == 1)		//nick add 20120220
	{
		//nick add s 20120220
		if (IssueDispense == 1)
		{
			ReaderCFG.Ticket1--;
			sprintf(buf,"Issue Dispenser %d : %d", IssueDispense, ReaderCFG.Ticket1);
		}
		else if (IssueDispense == 2)
		{
			ReaderCFG.Ticket2--;
			sprintf(buf,"Issue Dispenser %d : %d", IssueDispense, ReaderCFG.Ticket2);
		}
		//nick add e 20120220
		
		ShowMessage(buf);
	}					//Frank add e 20120117
	
	printf("IssueTicket :: End : %ld\n",GetTickCount()-CalcTmpTime);
	return IssueOK;
}

//Frank mark 20120508 bool TicketToOutlet()
bool TicketToOutlet(TicketData ticketData)					//Frank add 20120508
{
	ShowMessage((char *)"Dispense Ticket out.",1);
	
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
	if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		//Frank mark s 20130201
		/*DrawChipCoin(true);
		sleep(1);
		
		if(CheckCoinOnInlet()==true)
		{	 //卡票: 票卡沒下去 再開關一次(震下去)
			DrawChipCoin(false);
			usleep(200000L);
			DrawChipCoin(true);
			usleep(500000L);
			DrawChipCoin(false);
			usleep(200000L);
			DrawChipCoin(true);
			sleep(1);
			
			if(CheckCoinOnInlet()==true)
			{
				ReceiveChipCoin();
				return false;
			}
		}*/
		//Frank mark e 20130201
		//Frank add s 20130201
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //		long Ticks;
		unsigned long Ticks = GetTickCount();
		int i;
		
		for(i = 0 ; i < 3 ; i++)
		{
			DrawChipCoin(true);
			Ticks = GetTickCount();
			
			while(CheckTimeout(&Ticks, (unsigned long)2000L) == false)
			{
				if(CheckCoinOnOutlet() == true)
				{
					//Frank mark 20130201 usleep(300000L);
					//Frank mark 20130201 DrawChipCoin(false);
					return true;
				}

				usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
			}
			
			DrawChipCoin(false);
			usleep(500000L);
		}
		
		// ========================================================== //
		// nick mark s 20140526 Ver:000-000-GIO_V2-135101-0004-13B251 //
		//if(i >= 3)
		//{
		//	if(CheckCoinOnOutlet() == false)
		//	{
		//		ReceiveChipCoin();
		//		return false;
		//	}
		//}
		// nick mark e 20140526 Ver:000-000-GIO_V2-135101-0004-13B251 //
		// ========================================================== //
		//Frank add e 20130201
	}
	else if(ReaderCFG.HourlyReaderType == 4)
	{
		D1000 Dispense;
		// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
		Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		
		Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_TAKE);
		//Frank mark s 20120222
		/*
		if(IssueDispense == 1)
		{
			ReaderCFG.Ticket1--;
		}
		else
		{
			ReaderCFG.Ticket2--;
		}
		//Frank mark e 20120222
		*/
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
	}
	
	//Frank add s 20120508
	else if(ReaderCFG.HourlyReaderType == 6)
	{
		char PrintData[128];
		
		memset(PrintData , '\0' , sizeof(PrintData));
		
		GetPrintData(PrintData , &ticketData);
		if(r_mcp210 ->EjectCardOut(PrintData) != 1)
		{
			ShowMessage((char *)"Eject again!");
			r_mcp210 ->Reset();					//Frank add 20120821
			if(r_mcp210 ->EjectCardOut(PrintData) != 1)
				return false;
		}
	}
	//Frank add e 20120508
	
	return true;
}

void RecycleTicket(bool bCanReUse)
{	//回收票(卡)
	int i = 0;
	
	ShowMessage((char *)"Recycle Ticket.");
	
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
	if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		if(bCanReUse) return;

		ReceiveChipCoin(); //讀票位置 // nick add 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //
		usleep(200000); // nick add 20131218 Ver:000-000-GIO_V2-133181-0107-135101 //
		
		if(IsINMachine == true)
		{
			DrawChipCoin(true);
			TurnOnCoinShutter(true);
			sleep(1);
			TurnOnCoinShutter(false);
			DrawChipCoin(false);
			//RecycleChipCoin(); //取票口位置
		}
		else
		{
			ReceiveChipCoin(); //讀票位置
		}
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 4)
	{
		if(bCanReUse) return;

		usleep(200000L); // nick add 20140530 Ver:000-000-GIO_V2-135101-0004-13B251 //
		
		if(IsINMachine == true)
		{	//入口
			D1000 Dispense;
			
			// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
			Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			
			for(i=0;i<ReaderCFG.DispenserQuantity;i++)
			{	 // 有卡在機器裡就回收
				if(Dispense.IsCardInMachine(i+1)==true)
				{
					Dispense.RetrieveCard(i+1);
				}
			}
		}
		else
		{	//出口
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.EnableWork(0,D3000_ENABLE);
			
			for(i=0; i<3; i++)
			{
				if(Retrive.IsCardInMachine(0) == true)
				{
					// nick mark 20160311 Ver:000-000-GIO_V2-13B251-0008-13C241 //Retrive.RetrieveCard(0);
					if (Retrive.RetrieveCard(0) == true) // nick add 20160311 Ver:000-000-GIO_V2-13B251-0008-13C241 //
						break; // nick add 20160311 Ver:000-000-GIO_V2-13B251-0008-13C241 //
				}
				else
					break;
			}
			
			Retrive.EnableWork(0,D3000_DISABLE);
		}
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		if(IsINMachine == true)
		{
			// 20130117 Tony mark TUP500 printer;
//			char datas[128];
			
			// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);
			// 20130117 Tony mark printer.Receive();
			printer->Receive();	// 20130117 Tony add
		}
		else
		{
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.EnableWork(0,D3000_ENABLE);
			
			if(Retrive.IsCardInMachine(0)==true)
				Retrive.RetrieveCard(0);
			
			Retrive.EnableWork(0,D3000_DISABLE);
		}
	}	
	//Frank add s 20120508
	else if(ReaderCFG.HourlyReaderType == 6)
	{
		//Frank add s 20120821
		char ret;
		
		ret = r_mcp210 ->GetStatus();		
		//Frank mark 20120821 if (r_mcp210 ->GetStatus() == 3)
		if(ret == 3)
		//Frank add e 20120821
		{
			if(r_mcp210 ->RetrieveCard() != 1)
			{
				ShowMessage((char *)"Recycle Ticket again.");
				r_mcp210 ->RetrieveCard();
			}
		}
		//Frank add s 20120821
		else if(ret == -2)
		{
			r_mcp210 ->Reset();
			r_mcp210 ->RetrieveCard();
		}
		//Frank add e 20120821
	}
	//Frank add e 20120508
}

/*
return 值:
1:寫入成功 0:無法寫入(磁條), MF700 Block write error
-1:MF700 Request error
-2:MF700 Anticollision Error
-3:MF700 Select Card Error
-4:MF700 Authenticate error
-5:MF700 halt error
*/
// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) int WriteIssueData(TicketData ticketData)
int WriteIssueData(TicketData* ticketData)	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
{
	int iRet = 1;
	int i,j;
	int len,len2,sum;
	int index=0,indexN=0;
//	unsigned long lTicketid = 0;
	char ticketID[12],buf[20];
	unsigned char *SectorData = NULL;
	char logbuf[200];   // 20120131 Tony add
	
	ShowMessage((char *)"Write IssueData");
	
	memset(ticketID,0,sizeof(ticketID));
	
//Frank mark 20120508 if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4)
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || ReaderCFG.HourlyReaderType ==6)					//Frank add 20120508
	if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || ReaderCFG.HourlyReaderType == 6
		|| ReaderCFG.HourlyReaderType == 7) // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		sectorData sdata;
		
		Block1V2 *block1 = NULL;
		Block2V2 *block2 = NULL;
		Block3V2 *block3 = NULL;
		
		/* 20110506  Tony mark
		Block1 *block1 = NULL;
		Block2 *block2 = NULL;
		Block3 *block3 = NULL;
		*/
		
		SectorData = new unsigned char[50];
		
		memset(SectorData , '\0' , 50);
		
		/* 20110506 Tony mark 
		block1 = (Block1*)(SectorData + 0);
		block2 = (Block2*)(SectorData + 16);
		block3 = (Block3*)(SectorData + 32);	
		
		block1->format = 1;
		block1->parkingId = (unsigned char)G_ParkingConfig.ParkingID & 0xFF;
		
		block1->areaId = G_ParkingConfig.AreaID;
		block1->langId = 1;
		
		block1->ticketID = ticketData.ticketID;
		memcpy(block1->plate,ticketData.plate,8);
		
		block2->status = 0x01;
		Int2BCD(ticketData.in_year,4,block2->in_year);
		Int2BCD(ticketData.in_month,2,&block2->in_month);
		Int2BCD(ticketData.in_day,2,&block2->in_day);
		Int2BCD(ticketData.in_hour,2,&block2->in_hour);
		Int2BCD(ticketData.in_min,2,&block2->in_min);
		
		block2->Value=0L;
		block2->seasonVersion=0L;
		block2->empty = (unsigned char)0xFF;
		
		Int2BCD(ticketData.out_year,4,block3->out_year);
		Int2BCD(ticketData.out_month,2,&block3->out_month);
		Int2BCD(ticketData.out_day,2,&block3->out_day);
		Int2BCD(ticketData.out_hour,2,&block3->out_hour);
		Int2BCD(ticketData.out_min,2,&block3->out_min);
		Int2BCD(ticketData.out_sec,2,&block3->out_sec);
		block3->optime[0] = 0;
		block3->optime[1] = 0;
		block3->optime[2] = 0;
		memset(block3->empty,0xFF,sizeof(block3->empty));
		block3->randon = (unsigned char)(rand() % 256);
		*/
		
		// 20110506 Tony add s
		block1 = (Block1V2*)(SectorData + 0);
		block2 = (Block2V2*)(SectorData + 16);
		block3 = (Block3V2*)(SectorData + 32);
		
		block1->TicketVer = 2;
		block1->NextSector = (unsigned char)(G_ParkingConfig.ParkingID & 0x300)>>2;
		block1->parkingId = (unsigned char)G_ParkingConfig.ParkingID & 0xFF;
		block1->areaId = (unsigned char)G_ParkingConfig.AreaID & 0xFF;
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) block1->ticketID = ticketData.ticketID;
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) memcpy(block1->plate,ticketData.plate,8);
		block1->ticketID = ticketData->ticketID;	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		memcpy(block1->plate,ticketData->plate,8);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
		block2->status = 0x01;
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		//Int2BCD(ticketData.in_year,4,block2->in_year);
		//Int2BCD(ticketData.in_month,2,&block2->in_month);
		//Int2BCD(ticketData.in_day,2,&block2->in_day);
		//Int2BCD(ticketData.in_hour,2,&block2->in_hour);
		//Int2BCD(ticketData.in_min,2,&block2->in_min);
		//Int2BCD(ticketData.in_sec,2,&block2->in_sec);
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		Int2BCD(ticketData->in_year,4,block2->in_year);
		Int2BCD(ticketData->in_month,2,&block2->in_month);
		Int2BCD(ticketData->in_day,2,&block2->in_day);
		Int2BCD(ticketData->in_hour,2,&block2->in_hour);
		Int2BCD(ticketData->in_min,2,&block2->in_min);
		Int2BCD(ticketData->in_sec,2,&block2->in_sec);
		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
		block2->Value=0L;
		block2->seasonVersion=0L;
		
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		//Int2BCD(ticketData.out_year,4,block3->out_year);
		//Int2BCD(ticketData.out_month,2,&block3->out_month);
		//Int2BCD(ticketData.out_day,2,&block3->out_day);
		//Int2BCD(ticketData.out_hour,2,&block3->out_hour);
		//Int2BCD(ticketData.out_min,2,&block3->out_min);
		//Int2BCD(ticketData.out_sec,2,&block3->out_sec);
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		Int2BCD(ticketData->out_year,4,block3->out_year);
		Int2BCD(ticketData->out_month,2,&block3->out_month);
		Int2BCD(ticketData->out_day,2,&block3->out_day);
		Int2BCD(ticketData->out_hour,2,&block3->out_hour);
		Int2BCD(ticketData->out_min,2,&block3->out_min);
		Int2BCD(ticketData->out_sec,2,&block3->out_sec);
		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		
		block3->optime[0] = 0;
		block3->optime[1] = 0;
		block3->optime[2] = 0;
		/*
		block3->staytime[0] = 0;
		block3->staytime[1] = 0;
		block3->staytime[2] = 0;
		block3->staytime[3] = 0;
		*/
		block3->staytime = 0;
		block3->DisctSector = 0;
		// 20110506 Tony add e
		
		block3->bcc = INIT_DATA_BCC;
		
		for(i=0;i<47;i++)
		{	// check sum
			block3->bcc ^= (unsigned char)SectorData[i];
		}
		
		/* 20110506 Tony mark 
		memcpy(sdata.block1,block1,sizeof(Block1));
		memcpy(sdata.block2,block2,sizeof(Block2));
		memcpy(sdata.block3,block3,sizeof(Block3));
		*/
		
		// 20110506 Tony add s
		memcpy(sdata.block1,block1,sizeof(Block1V2));
		memcpy(sdata.block2,block2,sizeof(Block2V2));
		memcpy(sdata.block3,block3,sizeof(Block3V2));
		
		/*
		sprintf(buf,"Block1 : %s ",sdata.block1);
		ShowMessage(buf);
		
		sprintf(buf,"Block2 : %s ",sdata.block2);
		ShowMessage(buf);
		
		sprintf(buf,"Block3 : %s ",sdata.block3);
		ShowMessage(buf);
		*/
		
		//Frank mark s 20120206
		/* 
		printf("Block1:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block1[i]);
		}
		
		printf("\n");
		printf("Block2:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block2[i]);
		}
		
		printf("\n");
		printf("Block3:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block3[i]);
		}
		
		printf("\n");
		*/
		//Frank mark e 20120206
		// 20110506 Tony add e
		
		//Frank add s 20120206
		char TmpBuf[128];
		
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		memset(logbuf , '\0' , sizeof(logbuf));
		
		// Block 1
		sprintf(logbuf , "WriteIssueData:: Block1:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block1[i]);
			strcat(logbuf,TmpBuf);
		}
		
		ShowMessage(logbuf);
		
		memset(logbuf , '\0' , sizeof(logbuf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		// Block 2
		sprintf(logbuf,"WriteIssueData:: Block2:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block2[i]);
			strcat(logbuf,TmpBuf);
		}
		
		ShowMessage(logbuf);
		
		memset(logbuf , '\0' , sizeof(logbuf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		// Block 3
		sprintf(logbuf,"WriteIssueData:: Block3:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block3[i]);
			strcat(logbuf,TmpBuf);
		}
		
		ShowMessage(logbuf);
		
		memset(logbuf , '\0' , sizeof(logbuf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		//Frank add e 20120206
		
		iRet = 0; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		
		if(IssueDispense==1)
		{
			// ========================================================== //
			// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			//if(mf700h != NULL)
			//{
			//	ShowMessage((char *)"write MF700-1" , 1); 													 // 20120131 Tony add
			//	iRet = mf700h->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true);	  // 20120131 Tony add
			//	// 20120131 Tony mark if((iRet = mf700h->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true)) == 1)
			//	// 20120131 Tony mark				{
			//		//mf700h->WriteSector(ticketData.TagID,PARKING_DATA_BAK_SECTOR,sdata);
			//	// 20120131 Tony mark				}
			//}
			// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================== //
			// ========================================================= //
			// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			if (HourlyReaderModule1 == 0)
			{
				if (mf700h != NULL)
				{
					ShowMessage((char *)"write MF700-1" , 1);
					// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) iRet = mf700h->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true);
					iRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				}
			}
			else if (HourlyReaderModule1 == 1)
			{
				if (hf320h != NULL)
				{
					// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //ShowMessage((char *)"write hf320-1" , 1);
					// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //iRet = hf320h->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata);
					
					// ========================================================= //
					// nick add s 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
					// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)if (strlen(ticketData.TagID) <= 0)
						// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)iRet = hf320h->CacheCard(ticketData.TagID);
					for (i = 0; i < 3; i++)
					{
						if (strlen(ticketData->TagID) <= 0)	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
							iRet = hf320h->CacheCard(ticketData->TagID);// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
						else
							iRet = 1; // 表示有TAG

						if (iRet == 1)
							break;
					}
					
					if (iRet == 1)
					{
						if (ReaderCFG.HourlyReaderType == 7)
						{
							// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf, "%02d%02d%02d%02d", ticketData.in_month, ticketData.in_day, ticketData.in_hour, ticketData.in_min);
							sprintf(buf, "%02d%02d%02d%02d", ticketData->in_month, ticketData->in_day, ticketData->in_hour, ticketData->in_min);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
							
							for(i = 0; i < 3; i++)
							{
								if (hf320h->ShowDisplay(buf, strlen(buf)))
									break;
							}
							
							if (i >= 3)
								iRet = 0;
						}
						
						if (iRet == 1)
						{
							ShowMessage((char *)"write hf320-1" , 1);
							// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) iRet = hf320h->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata);
							iRet = hf320h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
						}
					}
					// nick add e 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
					// ========================================================= //
				}
			}
			// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
		}
		else
		{
			// ========================================================== //
			// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			//if(mf700h2 != NULL)
			//{
			//	ShowMessage((char *)"write MF700-2" , 1);
			//	iRet = mf700h2->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true);   // 20120131 Tony add
			//	
			//	// 20120131 Tony mark if((iRet = mf700h2->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true)) == 1)
			//	// 20120131 Tony mark				{
			//		//mf700h2->WriteSector(ticketData.TagID,PARKING_DATA_BAK_SECTOR,sdata);
			//	// 20120131 Tony mark				}
			//}
			// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================== //
			// ========================================================= //
			// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			if (HourlyReaderModule2 == 0)
			{
				if (mf700h2 != NULL)
				{
					ShowMessage((char *)"write MF700-2" , 1);
					// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) iRet = mf700h2->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata,true);
					iRet = mf700h2->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				}
			}
			else if (HourlyReaderModule2 == 1)
			{
				if (hf320h2 != NULL)
				{
					// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //ShowMessage((char *)"write hf320-2" , 1);
					// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //iRet = hf320h2->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata);
					
					// ========================================================= //
					// nick add s 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
					// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) if (strlen(ticketData.TagID) <= 0)
						// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) iRet = hf320h2->CacheCard(ticketData.TagID);
					if (strlen(ticketData->TagID) <= 0)	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
						iRet = hf320h2->CacheCard(ticketData->TagID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)					
					else
						iRet = 1; // 表示有TAG

					if (iRet == 1)
					{
						if (ReaderCFG.HourlyReaderType == 7)
						{
							// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf, "%02d%02d%02d%02d", ticketData.in_month, ticketData.in_day, ticketData.in_hour, ticketData.in_min);
							sprintf(buf, "%02d%02d%02d%02d", ticketData->in_month, ticketData->in_day, ticketData->in_hour, ticketData->in_min);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
							
							for(i = 0; i < 3; i++)
							{
								if (hf320h2->ShowDisplay(buf, strlen(buf)))
									break;
							}
							
							if (i >= 3)
								iRet = 0;
						}

						if (iRet == 1)
						{
							ShowMessage((char *)"write hf320-2" , 1);
							// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) iRet = hf320h2->WriteSector(ticketData.TagID,PARKING_DATA_SECTOR,sdata);
							iRet = hf320h2->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
						}
					}
					// nick add e 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
					// ========================================================= //
				}
			}
			// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
		}
		
		if(iRet != 1)					// 20120131 Tony add
		{	 //logbuf size : 200					// 20120131 Tony add
			// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(logbuf,"WriteIssueData :: Ticket Tag:[%s] ,Ticket ID:[%ld] ,iRet:[%d]",ticketData.TagID,ticketData.ticketID,iRet); 						 // 20120131 Tony add
			sprintf(logbuf,"WriteIssueData :: Ticket Tag:[%s] ,Ticket ID:[%ld] ,iRet:[%d]",ticketData->TagID,ticketData->ticketID,iRet);// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			ShowMessage(logbuf); 													 // 20120131 Tony add
		}// 20120131 Tony add
		
		if(SectorData != NULL)
			delete SectorData;
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		// 20130117 Tony mark TUP500 printer;
//		char status;
		char datas[256],buf1[40],buf2[40];
		char timeString[256];
		struct tm entryData;
		
		memset(timeString,'\0',sizeof(timeString));
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		//entryData.tm_year = ticketData.in_year-1900;
		//entryData.tm_mon	= ticketData.in_month-1;
		//entryData.tm_mday = ticketData.in_day;
		//entryData.tm_hour = ticketData.in_hour;
		//entryData.tm_min	= ticketData.in_min;
		// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		entryData.tm_year = ticketData->in_year-1900;
		entryData.tm_mon = ticketData->in_month-1;
		entryData.tm_mday = ticketData->in_day;
		entryData.tm_hour = ticketData->in_hour;
		entryData.tm_min = ticketData->in_min;
		// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
		entryData.tm_sec	= 59;
		
		// 20130117 Tony mark printer.init(ReaderCFG.HourlyReaderComPort);
		if(printer->CheckOffline()==true)	// 20130117 Tony add
		// 20130117 Tony mark if(printer.CheckOffline()==true)
		{
			// 20120223 Tony mark G_ParkingStatus.status |= STATUS_READER_CONNECT;
			G_ParkingStatus.status &= (~STATUS_READER_CONNECT); // 20120223 Tony add
			ShowMessage((char *)"TUP500 Offline!");					//Frank add 20111122
			iRet = -1;					//Frank add 20111122
			goto FUNCEXIT; // nick add 20140604 Ver:000-000-GIO_V2-135101-0004-13B251 //
		}
		else
		{
			// 20120223 Tony mark G_ParkingStatus.status &= STATUS_READER_CONNECT;
			G_ParkingStatus.status |= STATUS_READER_CONNECT;	 // 201202223 Tony add
		}
		
		memset(datas,'\0',sizeof(datas));
		
		//Frank mark 20120806 if(G_ParkingConfig.BarCodeType == 1)
		if(G_ParkingConfig.BarCodeType != 0)					//Frank add 20120806
		{	//IPMS Format
			// Ticket information //
			// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			//sprintf(datas,"%02d%1d1%02d%02d%02d%02d%08ld",ticketData.parkingId,ticketData.areaId,
			//	ticketData.in_month, ticketData.in_day, ticketData.in_hour ,ticketData.in_min, ticketData.ticketID/10L);
			// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

			// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			sprintf(datas,"%02d%1d1%02d%02d%02d%02d%08ld",ticketData->parkingId,ticketData->areaId,
				ticketData->in_month, ticketData->in_day, ticketData->in_hour ,ticketData->in_min, ticketData->ticketID/10L);
			// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			// 20130117 Tony mark printer.SetBarCodeData(datas);
			printer->SetBarCodeData(datas);	// 20130117 Tony add 
			//
			
			//Frank add s 20120806
			memset(buf1 , '\0' , sizeof(buf1));
			
			len = strlen(G_ParkingConfig.ParkingName);
			
			// 20120914 Tony mark if(len > 0)
			if(len > 23)	// 20120914 Tony add
			{
				// 20120914 Tony mark memcpy(buf1 , G_ParkingConfig.ParkingName , 20);
				memcpy(buf1 , G_ParkingConfig.ParkingName , 23);	// 20120914 Tony add
			}
			else
			{
				memcpy(buf1 ,G_ParkingConfig.ParkingName , len);
			}
			
			// 20130117 Tony mark printer.SetCharactData(0 , buf1);
			printer->SetCharactData(0 , buf1);	// 20130117 Tony add
			//Frank add e 20120806

// ======================================================= //
// nick mark s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
//			sprintf(datas,"%04d.%02d.%02d %02d:%02d SN:%08ld Plate:%s",ticketData.in_year,ticketData.in_month, ticketData.in_day,
//				ticketData.in_hour ,ticketData.in_min, ticketData.ticketID,ticketData.plate);	// 20120913 Tony add
// nick mark s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
// ======================================================= // 
			
			// ====================================================== //
			// nick add s 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
			if (G_ParkingConfig.DateTimeFormat[0] == 'd')
			{
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				//sprintf(datas,"%02d.%02d.%04d %02d:%02d SN:%08ld Plate:%s", ticketData.in_day, ticketData.in_month, ticketData.in_year,
				//	ticketData.in_hour, ticketData.in_min, ticketData.ticketID, ticketData.plate);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				
				sprintf(datas,"%02d.%02d.%04d %02d:%02d SN:%08ld Plate:%s", ticketData->in_day, ticketData->in_month, ticketData->in_year,
					ticketData->in_hour, ticketData->in_min, ticketData->ticketID, ticketData->plate);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
			else if (G_ParkingConfig.DateTimeFormat[0] == 'y')
			{
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				//sprintf(datas,"%04d.%02d.%02d %02d:%02d SN:%08ld Plate:%s", ticketData.in_year, ticketData.in_month, ticketData.in_day,
				//	ticketData.in_hour, ticketData.in_min, ticketData.ticketID,ticketData.plate);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				
				sprintf(datas,"%04d.%02d.%02d %02d:%02d SN:%08ld Plate:%s", ticketData->in_year, ticketData->in_month, ticketData->in_day,
					ticketData->in_hour, ticketData->in_min, ticketData->ticketID,ticketData->plate);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}
			else
			{
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				//sprintf(datas,"%02d.%02d.%04d %02d:%02d SN:%08ld Plate:%s", ticketData.in_month, ticketData.in_day, ticketData.in_year,
				//	ticketData.in_hour, ticketData.in_min, ticketData.ticketID, ticketData.plate);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				
				sprintf(datas,"%02d.%02d.%04d %02d:%02d SN:%08ld Plate:%s", ticketData->in_month, ticketData->in_day, ticketData->in_year,
					ticketData->in_hour, ticketData->in_min, ticketData->ticketID, ticketData->plate);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
			}			
			// nick add e 20130828 Ver:000-000-GIO_V2-133181-0100-135101 //
			// ====================================================== //
			
			// 20120913 Tony mark
			//sprintf(datas,"%04d.%02d.%02d %02d:%02d SN:%08ld",ticketData.in_year,ticketData.in_month, ticketData.in_day,
			//	ticketData.in_hour ,ticketData.in_min, ticketData.ticketID);

			// 20130117 Tony mark printer.SetCharactData(1 , datas);
			printer->SetCharactData(1 , datas);	// 20130117 Tony add
			
			usleep(15000L);	// 20120913 Tony add
		}
		//Frank mark 20120806 else if(G_ParkingConfig.BarCodeType == 0)
		else					//Frank add 20120806
		{	//Old EASY TYPE	
			memset(buf1,'\0',sizeof(buf1));
			memset(buf2,'\0',sizeof(buf2));
			
			len = strlen(G_ParkingConfig.ParkingName);
			
			//Title
			
			// 20121101 Tony add s
			int Logo_Print_x = 0;
			int Logo_Print_y = 0;
			
			if(GetPntLogoAddress((char *)"Tick_Logo.bmp" ,&Logo_Print_x,&Logo_Print_y) < 0)
			{
			// 20121101 Tony add e
				if(len >15)
				{	//splite parking name from space
					// 20120313 Tony mark s
					/*for(i=0;i<15;i++)
					{
						if(G_ParkingConfig.ParkingName[i] == ' ')
						{
							index = i;
						}
					}
				
					strncpy(buf1,G_ParkingConfig.ParkingName,index+1);
					strncpy(buf2,G_ParkingConfig.ParkingName+index+1,len-index);*/
					// 20120313 Tony mark e
				
					// 20120313 Tony add s
					index=15;
					memcpy(buf1,G_ParkingConfig.ParkingName,index);
					memcpy(buf2,G_ParkingConfig.ParkingName+index,15);
					// 20120313 Tony add e

					printer->SetCharactData(0,buf1);	// 20130117 Tony add
					printer->SetCharactData(1,buf2);	// 20130117 Tony add
					 
					// 20130117 Tony mark printer.SetCharactData(0,buf1);
					// 20130117 Tony mark printer.SetCharactData(1,buf2);
				}
				else
				{
					printer->SetCharactData(0,G_ParkingConfig.ParkingName);	//20130117 Tony add
					// 20130117 Tony mark printer.SetCharactData(0,G_ParkingConfig.ParkingName);					
				}
			}
			// barcode格式
			index = 0;
			len = strlen(G_ParkingConfig.BarCodeDetail);
			
			for(i=0;i<len;i++)
			{
				memset(buf,'\0',sizeof(buf));
				
				switch(G_ParkingConfig.BarCodeDetail[i])
				{
					case 'd':
						len2 = strlen(G_ParkingConfig.DateFormat);
						
						for(j=0;j<len2;j++)
						{
							memset(buf,'\0',sizeof(buf));
							
							switch(G_ParkingConfig.DateFormat[j])
							{
								case 'd':
									// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf,"%02d",ticketData.in_day);
									sprintf(buf,"%02d",ticketData->in_day);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
									datas[index++]=buf[0];
									datas[index++]=buf[1];
									break;
								case 'm':
									// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf,"%02d",ticketData.in_month);
									sprintf(buf,"%02d",ticketData->in_month);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
									datas[index++]=buf[0];
									datas[index++]=buf[1];
									break;
								case 'y':
									// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf,"%02d",ticketData.in_year%100);
									sprintf(buf,"%02d",ticketData->in_year%100);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
									datas[index++]=buf[0];
									datas[index++]=buf[1];
									break;
							}
						}
						
						break;
					case 'g':
						sprintf(buf,"%02d",G_ParkingConfig.MachineID);
						datas[index++]=buf[0];
						datas[index++]=buf[1];
						break;
					case 't':
						// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf,"%02d%02d",ticketData.in_hour,ticketData.in_min);
						sprintf(buf,"%02d%02d",ticketData->in_hour,ticketData->in_min);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)			
						datas[index++]=buf[0];
						datas[index++]=buf[1];
						datas[index++]=buf[2];
						datas[index++]=buf[3];
						break;
					case 'p':
						// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(buf,"%08ld",ticketData.ticketID);
						sprintf(buf,"%08ld",ticketData->ticketID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
						datas[index++]=buf[0];
						datas[index++]=buf[1];
						datas[index++]=buf[2];
						datas[index++]=buf[3];
						datas[index++]=buf[4];
						datas[index++]=buf[5];
						datas[index++]=buf[6];
						datas[index++]=buf[7];
						break;
				}
			}
			
			len = strlen(datas);
			sum = 0;
			
			for(i=0;i<len;i++)
			{
				sum += datas[i];
			}
			
			memset(buf,'\0',sizeof(buf));
			
			sprintf(buf,"%02d",sum%100);
			datas[index++]=buf[0];
			datas[index++]=buf[1];
			
			//印barcode
			// 20130117 Tony mark printer.SetBarCodeData(datas);
			printer->SetBarCodeData(datas);	// 20130117 Tony add
			
			if(GetPntLogoAddress((char *)"Tick_Logo.bmp" ,&Logo_Print_x,&Logo_Print_y) < 0)
			{
				// 印barcode 字串
				// 20130117 Tony mark printer.SetCharactData(5,datas);
				printer->SetCharactData(5,datas);	// 20130117 Tony add
				
				//印 日期
				strftime(timeString,256,"%d-%b-%Y %H:%M",&entryData);
				sprintf(datas,"Date/Time : %s",timeString);
				// 20130117 Tony mark printer.SetCharactData(2,datas);
				printer->SetCharactData(2,datas);	// 20130117 Tony add
				
				//印 票號
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(datas,"Ticket  No: %08ld",ticketData.ticketID);
				sprintf(datas,"Ticket  No: %08ld",ticketData->ticketID);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)				
				//printf("TicketID:%ld\n",ticketData.ticketID);
				// 20130117 Tony mark printer.SetCharactData(3,datas);
				printer->SetCharactData(3,datas);	// 20130117 Tony add
				
				//印 站號
				// 20120914 Tony mark sprintf(datas,"Station No: %d",G_ParkingConfig.MachineID);
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(datas,"Station No: %d,Plate: %s",G_ParkingConfig.MachineID,ticketData.plate);	// 20120914 Tony add
				sprintf(datas,"Station No: %d,Plate: %s",G_ParkingConfig.MachineID,ticketData->plate);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				// 20130117 Tony mark printer.SetCharactData(4,datas);
				printer->SetCharactData(4,datas);	// 20130117 Tony add
				
				//usleep(15000L);	// 20120914 Tony add
			}
			else	
			{
				// 印barcode 字串
				// 20130117 Tony mark printer.SetCharactData(3,datas);
				printer->SetCharactData(3,datas);	// 20130117 Tony add
				
				//印 日期	
				strftime(timeString,256,"%d-%b-%Y %H:%M",&entryData);
				sprintf(datas,"Date/Time : %s",timeString);
				// 20130117 Tony mark printer.SetCharactData(0,datas);
				printer->SetCharactData(0,datas);	// 20130117 Tony add
				
				//印 票號
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(datas,"Ticket  No: %08ld",ticketData.ticketID);
				sprintf(datas,"Ticket  No: %08ld",ticketData->ticketID);	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				// 20130117 Tony mark printer.SetCharactData(1,datas);
				printer->SetCharactData(1,datas);	// 20130117 Tony add
				
				//印 站號
				// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) sprintf(datas,"Station No: %d,Plate: %s",G_ParkingConfig.MachineID,ticketData.plate);	// 20120914 Tony add
				sprintf(datas,"Station No: %d,Plate: %s",G_ParkingConfig.MachineID,ticketData->plate);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
				// 20130117 Tony mark printer.SetCharactData(2,datas);
				printer->SetCharactData(2,datas);	// 20130117 Tony add
				//usleep(15000L); // 20120914 Tony add
			}
			
			//印 Note
			if(GetPntNoteAddress((char *)"Tick_Note.bmp",&Logo_Print_y ) < 0)
			{
				len = strlen(G_ParkingConfig.Note);
			
				if(len > 30)
				{	//splite parking name from space
					
					 // 20120313 Tony mark s
					/*for(i=0;i<30;i++)
					{
						if(G_ParkingConfig.Note[i] == ' ')
						{
							indexN = i;
						}
					}
					strncpy(buf1,G_ParkingConfig.Note,indexN+1);
					strncpy(buf2,G_ParkingConfig.Note+indexN+1,len-indexN);*/
					// 20120313 Tony mark e
					
					// 20120313 Tony add s
					memset(buf1,'\0',sizeof(buf1));
					memset(buf2,'\0',sizeof(buf2));
					indexN = 30;
					memcpy(buf1,G_ParkingConfig.Note,indexN);
					memcpy(buf2,G_ParkingConfig.Note+indexN,30); 
					// 20120313 Tony add e
					
					//printf("b1:%s b2:%s\n",buf1,buf2);
					// 20130117 Tony mark printer.SetCharactData(6,buf1);
					// 20130117 Tony mark printer.SetCharactData(7,buf2);
					printer->SetCharactData(6,buf1);	// 20130117 Tony add
					printer->SetCharactData(7,buf2);	// 20130117 Tony add
				}
				else
				{
					//printer.SetCharactData(6,G_ParkingConfig.Note);
					// 20130117 Tony mark printer.SetCharactData(4,G_ParkingConfig.Note);
					printer->SetCharactData(6,G_ParkingConfig.Note);	// 20130117 Tony add
					//printf("note:%s\n",G_ParkingConfig.Note);
				}
			}
		}
		
		// 20120426 Tony mark if(CheckTicketEmpty()==false)					//Frank add s 20111122
		if(printer->CheckPaperEnd()==false)	// 20130117 Tony add
		// 20130117 Tony mark if(printer.CheckPaperEnd()==false)	// 20120426 Tony add
		{
			// 20130117 Tony mark printer.IssuePaper();
			printer->IssuePaper();	// 20130117 Tony add
		}
		else
		{
			iRet=-1;
			goto FUNCEXIT; // nick add 20140604 Ver:000-000-GIO_V2-135101-0004-13B251 //
		}					//Frank add e 20111122
		//printf("barcode TYpe:%d\n",G_ParkingConfig.BarCodeType);
		
		if(printer->CheckNearEnd()==true)	// 20130117 Tony add
		// 20130117 Tony mark if(printer.CheckNearEnd()==true)
		{
			G_ParkingStatus.status |= STATUS_TICKET_LOW;
		}
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
	}

FUNCEXIT: // nick add 20140604 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	return iRet;
}

int WriteCarOutData(TicketData* ticketData,bool bIsHourly,bool bIsOut)
{ // bIsHourly (True: 計時票讀卡機寫卡, false: 月票讀卡機寫卡)
  // bIsOut (True: 出口終端機寫卡, false: 入口終端機寫卡)
	int bRet = 1;
	int i;
	unsigned char *SectorData = NULL;
	char logbuf[200];   // 20120131 Tony add
	
	//20110512 Tony mark struct tm* nowTm_ptr = NULL;
	//20110512 Tony mark time_t now;

	// =============== //
	// nick add s 20130222 //
	if (memcmp(ticketData, &LastTicketData, sizeof(TicketData)) != 0)
		memcpy(&LastTicketData, ticketData, sizeof(TicketData));
	// nick add e 20130222 //
	// =============== //
	
	if(ReaderCFG.HourlyReaderType ==5 && bIsHourly == true)
	{	//bar code
		//printf("Bar code do notthing.\n");
	}
	//20120117 Tony mark	else if(ReaderCFG.HourlyReaderType == 4 || (ReaderCFG.SeasonReaderType ==3 && bIsHourly == false))
//Frank mark 20120507	else if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || (ReaderCFG.SeasonReaderType ==3 && bIsHourly == false))	 // 20120117 Tony add
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //else if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //		|| (ReaderCFG.SeasonReaderType ==3 && bIsHourly == false)|| ReaderCFG.HourlyReaderType == 6)					//Frank add 20120507
	else if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4
		|| (ReaderCFG.SeasonReaderType ==3 && bIsHourly == false)|| ReaderCFG.HourlyReaderType == 6
		|| ReaderCFG.HourlyReaderType == 7)
	{
		if (ticketData->status == 6 && bIsHourly == true)	//限時折扣票券，不更改調任何資料(換卡機制)
			return bRet;
		
		sectorData sdata;
		
		/* 20110510 Tony mark 
		Block1 *block1 = NULL;
		Block2 *block2 = NULL;
		Block3 *block3 = NULL;
		*/
		
		// 20110510 Tony add s
		Block1V2 *block1 = NULL;
		Block2V2 *block2 = NULL;
		Block3V2 *block3 = NULL;
		// 20110510 Tony add e
		
		SectorData = new unsigned char[50];
		
		memset(SectorData,'\0',50);
		
		/* 201105010 Tony mark 
		block1 = (Block1V2*)(SectorData+0);
		block2 = (Block2V2*)(SectorData+16);
		block3 = (Block3V2*)(SectorData+32);
		
		block1->format 	= 1;
		block1->TicketVer = 2;
		block1->parkingId = (unsigned char)G_ParkingConfig.ParkingID & 0xFF;
		block1->areaId 	= G_ParkingConfig.AreaID ;
		block1->langId 	= 1;
		
		block1->ticketID = ticketData->ticketID;
		memcpy(block1->plate,ticketData->plate,8);
		
		if(bIsOut)
			block2->status = 3; // status is out car
		else
			block2->status = 1; // status is in car
			
		Int2BCD(ticketData->in_year, 4,block2->in_year);
		Int2BCD(ticketData->in_month,2,&block2->in_month);
		Int2BCD(ticketData->in_day,  2,&block2->in_day);	
		Int2BCD(ticketData->in_hour, 2,&block2->in_hour);
		Int2BCD(ticketData->in_min,  2,&block2->in_min);
		
		block2->Value=ticketData->value;	
		block2->seasonVersion=ticketData->seasonVersion;
		block2->empty = (unsigned char)0x00;
		
		now = time((time_t *)0);
		nowTm_ptr = localtime(&now);
		//change out time to now
		ticketData->out_year = nowTm_ptr->tm_year+1900;
		ticketData->out_month = nowTm_ptr->tm_mon+1;
		ticketData->out_day = nowTm_ptr->tm_mday;
		ticketData->out_hour = nowTm_ptr->tm_hour;
		ticketData->out_min = nowTm_ptr->tm_min;
		ticketData->out_sec = nowTm_ptr->tm_sec;
		
		Int2BCD(nowTm_ptr->tm_year + 1900, 4,block3->out_year);
		Int2BCD(nowTm_ptr->tm_mon+1,	  2,&block3->out_month);
		Int2BCD(nowTm_ptr->tm_mday,	  2,&block3->out_day);
		Int2BCD(nowTm_ptr->tm_hour,	  2,&block3->out_hour);
		Int2BCD(nowTm_ptr->tm_min, 	  2,&block3->out_min);
		Int2BCD(nowTm_ptr->tm_sec, 	  2,&block3->out_sec);
		
		block3->optime[0] = 0;
		block3->optime[1] = 0;
		block3->optime[2] = 0;
		memset(block3->empty,0x00,sizeof(block3->empty));
		block3->randon = (unsigned char)(rand() % 256);
		*/
		
		// 20110510 Tony add s
		block1 = (Block1V2*)(SectorData+0);
		block2 = (Block2V2*)(SectorData+16);
		block3 = (Block3V2*)(SectorData+32);
		
		//Frank mark 20121024 block1->TicketVer = ticketData->TicketVer ;
		block1->TicketVer = 2;					//Frank add 20121024
		block1->NextSector = (ticketData->parkingId & 0x300) >> 2; 
		block1->parkingId = ticketData->parkingId & 0xFF;
		// nick mark 20160601 Ver:000-000-GIO_V2-13B251-0013-13C241 //block1->areaId = ticketData->areaId & 0xFF;
		block1->ticketID = ticketData->ticketID;
		
		memcpy(block1->plate,ticketData->plate,8);
		
		// nick mark 20160601 Ver:000-000-GIO_V2-13B251-0013-13C241 //if(G_ParkingConfig.UpLayerID== 0)
		if(G_ParkingConfig.UpLayerID <= 0) // nick add 20160601 Ver:000-000-GIO_V2-13B251-0013-13C241 //
		{
			if(bIsOut)
			{					//Frank add 20111116
				block2->status = 3; // status is out car
			}					//Frank add 20111116
			else
			{					//Frank add 20111116
				block2->status = 1; // status is in car
			}					//Frank add 20111116

			block1->areaId = ticketData->areaId & 0xFF;
		}
		else
		{
			block2->status = ticketData->status; // status is in car

			if (bIsOut == false)
				block1->areaId = ticketData->areaId & 0xFF;
			else
				block1->areaId = G_ParkingConfig.UpLayerID & 0xFF;
		}
		
		//printf("\n\n<< Status:%d >>\n\n", block2->status);
		
		Int2BCD(ticketData->in_year,4,block2->in_year);
		Int2BCD(ticketData->in_month,2,&block2->in_month);
		Int2BCD(ticketData->in_day,2,&block2->in_day);
		Int2BCD(ticketData->in_hour,2,&block2->in_hour);
		Int2BCD(ticketData->in_min,2,&block2->in_min);
		Int2BCD(ticketData->in_sec,2,&block2->in_sec);	
		block2->Value = ticketData->value;
		block2->seasonVersion = ticketData->seasonVersion;
		
		Int2BCD(ticketData->out_year,4,block3->out_year);
		Int2BCD(ticketData->out_month,2,&block3->out_month);
		Int2BCD(ticketData->out_day,2,&block3->out_day);
		Int2BCD(ticketData->out_hour,2,&block3->out_hour);
		Int2BCD(ticketData->out_min,2,&block3->out_min);
		Int2BCD(ticketData->out_sec,2,&block3->out_sec);
		
		memcpy(block3->optime,ticketData->optime,3);
		
		if (ticketData->staytime > 99999999L) 
		{
			ticketData->staytime=99999999L;
		}
		
		ticketData->staytime |= 0x10000000;
		block3->staytime = ticketData->staytime;
//		memcpy(block3->staytime,ticketData->staytime,4);
		
		block3->DisctSector = ticketData->DisctSector;
		// 20110510 Tony add e
		
		block3->bcc = INIT_DATA_BCC;
		
		for(i=0;i<47;i++)
		{	// BCC
			block3->bcc ^= (unsigned char)SectorData[i];
		}
		
		/*
		memcpy(sdata.block1,block1,sizeof(Block1));
		memcpy(sdata.block2,block2,sizeof(Block2));
		memcpy(sdata.block3,block3,sizeof(Block3));
		*/
		
		// 201105010 Tony add s
		memcpy(sdata.block1,block1,sizeof(Block1V2));
		memcpy(sdata.block2,block2,sizeof(Block2V2));
		memcpy(sdata.block3,block3,sizeof(Block3V2));
		
		/*
		sprintf(buf,"Block1 : %s ",sdata.block1);
		ShowMessage(buf);
		
		sprintf(buf,"Block2 : %s ",sdata.block2);
		ShowMessage(buf);
		
		sprintf(buf,"Block3 : %s ",sdata.block3);
		ShowMessage(buf);
		*/
		
		//Frank mark s 20120206
		/*
		printf("Block1:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block1[i]);
		}
		
		printf("\n");
		printf("Block2:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block2[i]);
		}
		
		printf("\n");
		printf("Block3:");
		
		for(i=0;i<16;i++)
		{
			printf("%02X ",(unsigned char)sdata.block3[i]);
		}
		
		printf("\n");
		*/
		//Frank mark e 20120206
		// 201105010 Tony add e
		
		//Frank add s 20120206
		char TmpBuf[128];
		
		memset(TmpBuf,'\0',sizeof(TmpBuf));
		
		// Block 1
		sprintf(logbuf,"WriteCarOutData:: Block1:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block1[i]); 
			strcat(logbuf,TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(logbuf);
		ShowMessage(logbuf, 3); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		
		memset(logbuf,'\0',sizeof(logbuf));
		memset(TmpBuf,'\0',sizeof(TmpBuf));
		
		// Block 2		
		sprintf(logbuf,"WriteCarOutData:: Block2:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block2[i]); 
			strcat(logbuf,TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(logbuf);
		ShowMessage(logbuf, 3); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		
		memset(logbuf,'\0',sizeof(logbuf));
		memset(TmpBuf,'\0',sizeof(TmpBuf));
		
		// Block 3
		sprintf(logbuf,"WriteCarOutData:: Block3:");
		
		for(i=0;i<16;i++)
		{
			sprintf(TmpBuf,"%02X ",(unsigned char)sdata.block3[i]); 
			strcat(logbuf,TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(logbuf);
		ShowMessage(logbuf, 3); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //
		
		memset(logbuf,'\0',sizeof(logbuf));
		memset(TmpBuf,'\0',sizeof(TmpBuf));
		//Frank add e 20120206
		
		bRet = 0; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		
		if(bIsHourly == true)
		{
			// nick mark 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);   // 20120131 Tony add
			// ========================================================= //
			// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			if (HourlyReaderModule1 == 0)
			{
				if (mf700h != NULL)
					bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);
			}
			else if (HourlyReaderModule1 == 1)
			{
				if (hf320h != NULL)
					bRet = hf320h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata);
			}
			// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
			
			// 20120131 Tony mark if((bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true)) == 1)
			// 20120131 Tony mark			{
				//mf700h->WriteSector(PARKING_DATA_BAK_SECTOR,sdata);
			// 20120131 Tony mark			}
			//clear Discount
			// 20120131 Tony mark memset(&sdata,'\0',48);
			// 20120131 Tony mark			if((bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DISCOUNT_SECTOR,sdata,true)) == 1)
			// 20120131 Tony mark			{
				//mf700h->WriteSector(PARKING_DISCOUNT_BAK_SECTOR,sdata);
			// 20120131 Tony mark			}
		}
		else
		{
			// ========================================================== //
			// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			//if(mf700s != NULL)
			//{
			//	bRet = mf700s->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);   // 20120131 Tony add
			//	// 20120131 Tony mark if((bRet = mf700s->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true)) == 1)
			//	// 20120131 Tony mark				{
			//		//mf700s->WriteSector(PARKING_DATA_BAK_SECTOR,sdata);
			//	// 20120131 Tony mark				}
			//}
			//else
			//{
			//	bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);   // 20120131 Tony add
			//	// 20120131 Tony mark if((bRet = mf700h->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true)) == 1)
			//	// 20120131 Tony mark			{
			//		//mf700h->WriteSector(PARKING_DATA_BAK_SECTOR,sdata);
			//	// 20120131 Tony mark			}
			//}
			// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================== //
			// ========================================================= //
			// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			if (SeasonReaderModule == 0)
			{
				if (mf700s != NULL)
					bRet = mf700s->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata,true);
				else
				{
					
				}
			}
			else if (SeasonReaderModule == 1)
			{
				if (hf320s != NULL)
					bRet = hf320s->WriteSector(ticketData->TagID,PARKING_DATA_SECTOR,sdata);
			}
			// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
		}
		
		// 20120131 Tony add s
		if (bRet != 1)
		{	 // logbuf size : 200
			sprintf(logbuf,"WriteCarOutData :: Ticket Tag:[%s], Ticket ID:[%ld], bRet :[%d]",ticketData->TagID,ticketData->ticketID,bRet);
			ShowMessage(logbuf);
		}
		// 20120131 Tony add e
		//nick add s 20120502
		else
		{	//寫卡成功 //
			memset(&LastTicketData, 0, sizeof(TicketData));
			
			if (ReaderCFG.SeasonReaderType == 3 && bIsHourly == false)
			{
				AudioOn(AUDIO_SEASON_W_SUC);
				usleep(300000L);
			}
		}
		//nick add e 20120502
		
		if(SectorData != NULL)
			delete SectorData;
	}
	else
	{
	}
	
	return bRet;
}

void DispenserEnable(bool bEnable)
{
	if(ReaderCFG.HourlyReaderType == 4)
	{
		if(IsINMachine == false)
		{
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			
			if(bEnable==true)
			{
				Retrive.EnableWork(0,D3000_ENABLE);
			}
			else
			{
				Retrive.EnableWork(0,D3000_DISABLE);
			}
		}
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		barcode->ReadStart();
		
		if(IsINMachine == false)
		{
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			
			if(bEnable==true)
			{
				Retrive.EnableWork(0,D3000_ENABLE);
			}
			else
			{
				Retrive.EnableWork(0,D3000_DISABLE);
			}
		}
	}
	// nick mark 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //else if(ReaderCFG.HourlyReaderType == 3)
	else if (ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160706 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		TurnOnCoinShutter(bEnable);
	}
	//Frank mark s 20120824
	//Frank add s 20120508
	/*else if(ReaderCFG.HourlyReaderType == 6)
	{
		if(IsINMachine == false)
		{
			if (r_mcp210 ->GetStatus() == 3)
				if(r_mcp210 ->RetrieveCard() != 1)
				{
					r_mcp210 ->Reset();					//Frank add 20120821
					ShowMessage((char *)"RetrieveCard Ticket again.",1);
					r_mcp210 ->RetrieveCard();
				}
		}
	}*/
	//Frank add e 20120508
	//Frank mark e 20120824
}

void RejectTicket()
{
	int i;
	
	ShowMessage((char *)"Reject Ticket.");
	
	if(ReaderCFG.HourlyReaderType == 4)
	{
		if(IsINMachine == true)
		{	//入口
			D1000 Dispense;
			
			// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //Dispense.init(ReaderCFG.MifareDispenserComPort);
			Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			
			for(i=0;i<ReaderCFG.DispenserQuantity;i++)
			{
				if(Dispense.IsCardInMachine(i+1) == true)
				{
					Dispense.DispenseCard(i+1,D1000_DISPENSE_TO_OUT);
				}
			}
		}
		else
		{
			D3000 Retrive;
			
			Retrive.init(ReaderCFG.MifareDispenserComPort);
			Retrive.EnableWork(0,D3000_ENABLE);
			
			for(i=0; i<3; i++)
			{
				if(Retrive.IsCardInMachine(0) == true)
					Retrive.RejectCard(0);
				else
					break;
			}
			
			Retrive.EnableWork(0,D3000_DISABLE);
		}
	}
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //else if(ReaderCFG.HourlyReaderType == 3)
	else if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		DrawChipCoin(true); // 退出
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		D3000 Retrive;
		
		Retrive.init(ReaderCFG.MifareDispenserComPort);
		Retrive.EnableWork(0,D3000_ENABLE);
		
		if(Retrive.IsCardInMachine(0) == true)
			Retrive.RejectCard(0);
		
		Retrive.EnableWork(0,D3000_DISABLE);
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
	}
	//Frank add s 20120508
	else if(ReaderCFG.HourlyReaderType == 6)
	{
		if(r_mcp210 ->EjectCardOut((char * )"") != 1)
		{
			r_mcp210 ->Reset();					//Frank add 20120821
			ShowMessage((char *)"Reject Ticket again.");
			r_mcp210 ->EjectCardOut((char * )"");
		}
	}
	//Frank add e 20120508
}

bool DetectTicketInSensor(int sec)
{
	bool bRet = false;
	int i = 0;
	
	for(i=0;i< (sec*1000);i++)
	{
		if(CheckCoinOnInlet() == true)
		{
			bRet = true;
			break;
		}
		
		usleep(1000L); //1ms
	}
	
	 return bRet;
}

int ReadTicketData(TicketData *ticketData, enum TicketType ticketType)
{	// return 0: no Ticket 1: Normal Ticket  -1:Ticket Read Error
	int i, iRet = 1;
	int iMF_Reader = -1; // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	int iLastRtn = 0; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //bool bReadTicket = false;
//	bool bHasTicket  = false;	
	//char CardID[20];
	char BCodeData[128];
	char buf[128];
	BarCodeData* BCdata=NULL;
	sectorData parkData,discountData;
	DiscountSectorData discountSector;
	//Frank add s 20120912
	DiscountSectorDatav2 DiscountSectorv2;
	sectorData NextNiscountData;
	sectorData NextNiscountData2;
	Discountstr Discountarray;
	//Frank add e 20120912
	
	// 20110506 Tony mark Block1 block1;
	// 20110506 Tony mark Block2 block2;
	// 20110506 Tony mark Block3 block3;
//	DiscountBlock1 dictBlock1;
//	DiscountBlock23 dictBlock23;
	
	//memset(CardID,'\0',sizeof(CardID));
	memset(buf,'\0',sizeof(buf));
	memset(&parkData,'\0',sizeof(sectorData));
	memset(&discountSector,'\0',sizeof(SectorDiscountData));
	//Frank add s 20120917
	memset(&discountData , '\0' , sizeof(discountData));
	memset(&NextNiscountData , '\0' , sizeof(NextNiscountData));
	memset(&DiscountSectorv2 , '\0' , sizeof(DiscountSectorv2));
	memset(&NextNiscountData2, '\0' , sizeof(NextNiscountData2));
	//Frank add e 20120917
	
//Frank mark	20120508 if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4 || (ReaderCFG.SeasonReaderType ==3 && ticketType == SEASON_TICKET))
	if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 4
		|| ((ReaderCFG.SeasonReaderType == 3 || ReaderCFG.SeasonReaderType == 7) && ticketType == SEASON_TICKET)
		|| ReaderCFG.HourlyReaderType == 6 || ReaderCFG.HourlyReaderType == 7) // nick edit 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		usleep(100000L);
		
		//Frank mark 20120507(ticketType == SEASON_TICKET && mf700s != NULL )
		if(ticketType == SEASON_TICKET)	// Frank add 20120507
		{
			if(G_ParkingConfig.bReadTag == true)
			{	// Use TAG to get Season Data from db
				// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return 1;
				iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
				goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			}
			else
			{
				// Frank mark 20120507 if((iRet=mf700s->ReadSector(ticketData->TagID,PARKING_DATA_SECTOR,&parkData)) != 1)
				
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// Frank add s 20120507
				//if (mf700s != NULL)
				//{
				//	iRet = mf700s ->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
				//}
				//else if(mf700h != NULL)
				//{
				//	iRet = mf700h ->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				iRet = -99;
				
				if (SeasonReaderModule == 0)
				{
					if (mf700s != NULL)
						iRet = mf700s->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
				}
				else if (SeasonReaderModule == 1)
				{
					if (hf320s != NULL)
						iRet = hf320s->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
				}
				
				if (iRet == -99)
				{ // 表示沒有月票讀卡機
					if (HourlyReaderModule1 == 0)
					{
						if (mf700h != NULL)
							iRet = mf700h->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
					}
					else if (HourlyReaderModule1 == 1)
					{
						if (ticketType == SEASON_TICKET) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
							hf320h->SetAPDUMode(false); // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
						
						if (hf320h != NULL)
							iRet = hf320h->ReadSector(ticketData->TagID , PARKING_DATA_SECTOR , &parkData);
					}
				}
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
				
				if (iRet != 1)
				// Frank add e 20120507
				{
					sprintf(buf,"Season Read data sector error %d",iRet);
					ShowMessage(buf);
					// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
					iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
					goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
				}
				
				// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //bReadTicket = true;
				iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			}
		}
		else if(IsINMachine == false)
		{	 // 只用在出口
			// ========================================================= //
			// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			iRet = 0;
			
			if ((HourlyReaderModule1 == 0 && mf700h != NULL))
				iMF_Reader = 0;
			else if ((HourlyReaderModule1 == 1 && hf320h != NULL))
				iMF_Reader = 1;
			// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
			
			// nick mark 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //if(mf700h != NULL )
			if (iMF_Reader >= 0) // nick add 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			{
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				//if(mf700h ->ReadSector(ticketData ->TagID , PARKING_DATA_SECTOR , &parkData) != 1)
				//{
				//	RejectTicket();
				//	//Frank mark 20120830 printf("read data sector Error.\n");
				//	ShowMessage((char *)"read data sector Error.");					//Frank add 20120830
				//	return -1;
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				iRet = 0;
				
				if (iMF_Reader == 0)
					iRet = mf700h->ReadSector(ticketData->TagID, PARKING_DATA_SECTOR, &parkData);
				else if (iMF_Reader == 1)
				{
					if (ticketType == SEASON_TICKET) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
						hf320h->SetAPDUMode(false); // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
					
					iRet = hf320h->ReadSector(ticketData->TagID, PARKING_DATA_SECTOR, &parkData);
				}
				
				if (iRet != 1)
				{
					RejectTicket();
					ShowMessage((char *)"read data sector Error.");
					// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
					iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
					goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
				}
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
				
				if(parkData.block1[0] == 1)
				{
					// ========================================================== //
					// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					//if(mf700h ->ReadSector(ticketData ->TagID , PARKING_DISCOUNT_SECTOR , &discountData) != 1)
					//{
					//	RejectTicket();
					//	//Frank mark 20120830 printf("read dicsount sector Error.\n");
					//	ShowMessage((char *)"read dicsount sector Error.");					//Frank add 20120830
					//	return -1;
					//}
					// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================== //
					// ========================================================= //
					// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					iRet = 0;
					
					if (iMF_Reader == 0)
						iRet = mf700h->ReadSector(ticketData->TagID, PARKING_DISCOUNT_SECTOR, &discountData);
					else if (iMF_Reader == 1)
						iRet = hf320h->ReadSector(ticketData->TagID, PARKING_DISCOUNT_SECTOR, &discountData);
					
					if (iRet != 1)
					{
						RejectTicket();
						ShowMessage((char *)"read dicsount sector Error.");
						// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
						iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
						goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
					}
					// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================= //
				}
				else if ((parkData.block1[0] == 2) & (parkData.block3[14] != 0))
				{
					// ========================================================== //
					// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					//if(mf700h->ReadSector(ticketData->TagID , parkData.block3[14] , &discountData) != 1)
					//{
					//	RejectTicket();
					//	//Frank mark 20120830 printf("read dicsount sector Error.\n");
					//	ShowMessage((char *)"read dicsount sector Error.");					//Frank add 20120830
					//	return -1;
					//}
					// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================== //
					// ========================================================= //
					// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					iRet = 0;
					
					if (iMF_Reader == 0)
						iRet = mf700h->ReadSector(ticketData->TagID, parkData.block3[14], &discountData);
					else if (iMF_Reader == 1)
						iRet = hf320h->ReadSector(ticketData->TagID, parkData.block3[14], &discountData);
					
					if (iRet != 1)
					{
						RejectTicket();
						ShowMessage((char *)"read dicsount sector Error.");
						// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
						iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
						goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
					}
					// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
					// ========================================================= //
					
					//Frank add s 20120912
					unsigned char BCC = 0x69;
					
					ticketData->DiscountCount = (discountData.block1[7] & 0xF0) >> 4;
					
					if(ticketData ->DiscountCount  > 0)
					{
						for(i = 0 ; i < 15 ; i++)
						{
							BCC ^= discountData.block2[i];
						}
						
						if(BCC != discountData.block2[15])
						{
							RejectTicket();
							sprintf(buf , "discount block1 BCC error! BCC:%02X block:%02X" , BCC , discountData.block2[15]);
							ShowMessage(buf);
							// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
							iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
							goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
						}
						
						if(ticketData ->DiscountCount  > 2)
						{
							BCC = 0x69;
							
							for(i = 0 ; i < 15 ; i++)
							{
								BCC ^= discountData.block3[i];
							}
							
							if(BCC != discountData.block3[15])
							{
								RejectTicket();
								sprintf(buf , "discount block2 BCC error! BCC:%02X block:%02X" , BCC , discountData.block3[15]);
								ShowMessage(buf);
								// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
								iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
								goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
							}
						}
					}
					
					memcpy(&Discountarray.limit , &discountData , sizeof(SectorData));
					
					if(discountData.block3[14] != 0)
					{
						// ========================================================== //
						// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						//if(mf700h ->ReadSector(ticketData ->TagID , discountData.block3[14] , &NextNiscountData) != 1)
						//{
						//	RejectTicket();
						//	ShowMessage((char *)"read dicsount2 sector Error.");
						//	return -1;
						//}
						// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================== //
						// ========================================================= //
						// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						iRet = 0;
						
						if (iMF_Reader == 0)
							iRet = mf700h->ReadSector(ticketData->TagID, discountData.block3[14], &NextNiscountData);
						else if (iMF_Reader == 1)
							iRet = hf320h->ReadSector(ticketData->TagID, discountData.block3[14], &NextNiscountData);
						
						if (iRet != 1)
						{
							RejectTicket();
							ShowMessage((char *)"read dicsount2 sector Error.");
							// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
							iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
							goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
						}
						// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
						// ========================================================= //
						
						if(ticketData ->DiscountCount  > 4)
						{
							BCC = 0x69 ;
							
							for(i = 0 ; i < 15 ; i++)
							{
								BCC ^= NextNiscountData.block1[i];
							}
							
							if(BCC != NextNiscountData.block1[15])
							{
								RejectTicket();
								sprintf(buf , "discount block3 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData.block1[15]);
								ShowMessage(buf);
								// nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
								iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
								goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
							}
							
							if(ticketData ->DiscountCount  > 6)
							{
								BCC = 0x69 ;
								
								for(i = 0 ; i < 15 ; i++)
								{
									BCC ^= NextNiscountData.block2[i];
								}
								
								if(BCC != NextNiscountData.block2[15])
								{
									RejectTicket();
									sprintf(buf , "discount block4 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData.block2[15]);
									ShowMessage(buf);
									// nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
									iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
									goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
								}
								
								if(ticketData ->DiscountCount  > 8)
								{
									BCC = 0x69 ;
									
									for(i = 0 ; i < 15 ; i++)
									{
										BCC ^= NextNiscountData.block3[i];
									}
									
									if(BCC != NextNiscountData.block3[15])
									{
										RejectTicket();
										sprintf(buf , "discount block5 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData.block3[15]);
										ShowMessage(buf);
										// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
										iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
										goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
									}
								}
							}
						}
						
						memcpy(&Discountarray.block[2] , &NextNiscountData , sizeof(SectorData));
						
						if(NextNiscountData.block3[14] != 0)
						{
							// ========================================================== //
							// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
							//if(mf700h ->ReadSector(ticketData ->TagID , NextNiscountData.block3[14] , &NextNiscountData2) != 1)
							//{
							//	RejectTicket();
							//	sprintf(buf , "read dicsount3 sector Error.");
							//	ShowMessage(buf);
							//	return -1;
							//}
							// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
							// ========================================================== //
							// ========================================================= //
							// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
							iRet = 0;
							
							if (iMF_Reader == 0)
								iRet = mf700h->ReadSector(ticketData->TagID, NextNiscountData.block3[14], &NextNiscountData2);
							else if (iMF_Reader == 1)
								iRet = hf320h->ReadSector(ticketData->TagID, NextNiscountData.block3[14], &NextNiscountData2);
							
							if (iRet != 1)
							{
								RejectTicket();
								sprintf(buf , "read dicsount3 sector Error.");
								ShowMessage(buf);
								// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
								iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
								goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
							}
							// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
							// ========================================================= //
							
							if(ticketData ->DiscountCount  > 10)
							{
								BCC = 0x69;
								
								for(i = 0 ; i < 15 ; i++)
								{
									BCC ^= NextNiscountData2.block1[i];
								}
								
								if(BCC != NextNiscountData2.block1[15])
								{
									RejectTicket();
									sprintf(buf , "discount block6 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData2.block1[15]);
									ShowMessage(buf);
									// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
									iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
									goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
								}
								
								if(ticketData ->DiscountCount  > 12)
								{
									BCC = 0x69;
									
									for(i = 0 ; i < 15 ; i++)
									{
										BCC ^= NextNiscountData2.block2[i];
									}
									
									if(BCC != NextNiscountData2.block2[15])
									{
										RejectTicket();
										sprintf(buf , "discount block7 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData2.block2[15]);
										ShowMessage(buf);
										// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
										iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
										goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
									}
									
									if(ticketData ->DiscountCount  > 14)
									{
										BCC = 0x69;
										
										for(i = 0 ; i < 15 ; i++)
										{
											BCC ^= NextNiscountData2.block3[i];
										}
										
										if(BCC != NextNiscountData2.block3[15])
										{
											RejectTicket();
											sprintf(buf , "discount block8 BCC error! BCC:%02X block:%02X" , BCC , NextNiscountData2.block3[15]);
											ShowMessage(buf);
											// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return -1;
											iLastRtn = -1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
											goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
										}
									}
								}
							}
							
							memcpy(&Discountarray.block[5] , &NextNiscountData2 , sizeof(SectorData));
						}
					}
					//Frank add e 20120912
				}
				
				// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //bReadTicket = true;
				iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			}
		}
		
		// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //if(bReadTicket == false)
		// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //{
		// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //	return 0;
		// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //}
		
		if (iLastRtn == 0) // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
		
		// 20110506 Tony add s
		
		// 20120206 Frank add s
		char TmpBuf[128];
		
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		// Block 1
		sprintf(buf , "ReadTicketData:: Block1:");
		
		for(i = 0 ; i < 16 ; i++)
		{
			sprintf(TmpBuf , "%02X " , (unsigned char)parkData.block1[i]); 
			strcat(buf , TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		memset(buf , '\0' , sizeof(buf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		// Block 2
		sprintf(buf , "ReadTicketData:: Block2:");
		
		for(i = 0 ; i < 16 ; i++)
		{
			sprintf(TmpBuf , "%02X " , (unsigned char)parkData.block2[i]); 
			strcat(buf , TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		memset(buf , '\0' , sizeof(buf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		// Block 3
		sprintf(buf , "ReadTicketData:: Block3:");
		
		for(i = 0 ; i < 16 ; i++)
		{
			sprintf(TmpBuf , "%02X " , (unsigned char)parkData.block3[i]); 
			strcat(buf , TmpBuf);
		}
		
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		memset(buf , '\0' , sizeof(buf));
		memset(TmpBuf , '\0' , sizeof(TmpBuf));
		
		sprintf(buf , "TicketData :: TicketVer:%2X " , parkData.block1[0]);
		// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
		
		memset(buf , '\0' , sizeof(buf));
		// 20120206 Frank add e
		
		//Frank add s 20120911
		if(ticketData->DiscountCount > 0)					//Frank add 20121123
		{					//Frank add 20121123
			// Block 1
			sprintf(buf , "ReadSector%02X:: Block1:" , parkData.block3[14]);
			
			for(i = 0 ; i < 16 ; i++)
			{
				sprintf(TmpBuf , "%02X " , (unsigned char)discountData.block1[i]); 
				strcat(buf , TmpBuf);
			}
			
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			memset(buf , '\0' , sizeof(buf));
			memset(TmpBuf , '\0' , sizeof(TmpBuf));
			
			// Block 2
			sprintf(buf , "ReadSector%02X:: Block2:" , parkData.block3[14]);
			
			for(i = 0 ; i < 16 ; i++)
			{
				sprintf(TmpBuf , "%02X " , (unsigned char)discountData.block2[i]); 
				strcat(buf , TmpBuf);
			}
			
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			memset(buf , '\0' , sizeof(buf));
			memset(TmpBuf , '\0' , sizeof(TmpBuf));
			
			// Block 3
			sprintf(buf , "ReadSector%02X:: Block3:" , parkData.block3[14]);
			
			for(i = 0 ; i < 16 ; i++)
			{
				sprintf(TmpBuf , "%02X " , (unsigned char)discountData.block3[i]); 
				strcat(buf , TmpBuf);
			}
			
			// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
			
			memset(buf , '\0' , sizeof(buf));
			memset(TmpBuf , '\0' , sizeof(TmpBuf));
			
			if(ticketData->DiscountCount > 5)					//Frank add 20121123
			{					//Frank add 20121123
				// Block 1
				sprintf(buf , "ReadSector%02X:: Block1:" , discountData.block3[14]);
				
				for(i = 0 ; i < 16 ; i++)
				{
					sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData.block1[i]); 
					strcat(buf , TmpBuf);
				}
				
				// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
				
				memset(buf , '\0' , sizeof(buf));
				memset(TmpBuf , '\0' , sizeof(TmpBuf));
				
				// Block 2
				sprintf(buf , "ReadSector%02X:: Block2:" , discountData.block3[14]);
				
				for(i = 0 ; i < 16 ; i++)
				{
					sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData.block2[i]); 
					strcat(buf , TmpBuf);
				}
				
				// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
				
				memset(buf , '\0' , sizeof(buf));
				memset(TmpBuf , '\0' , sizeof(TmpBuf));
				
				// Block 3
				sprintf(buf , "ReadSector%02X:: Block3:" , discountData.block3[14]);
				
				for(i = 0 ; i < 16 ; i++)
				{
					sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData.block3[i]); 
					strcat(buf , TmpBuf);
				}
				
				// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
				
				memset(buf , '\0' , sizeof(buf));
				memset(TmpBuf , '\0' , sizeof(TmpBuf));
				
				if(ticketData->DiscountCount > 10)					//Frank add 20121123
				{					//Frank add 20121123
					// Block 1
					sprintf(buf , "ReadSector%02X:: Block1:" , NextNiscountData.block3[14]);
					
					for(i = 0 ; i < 16 ; i++)
					{
						sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData2.block1[i]); 
						strcat(buf , TmpBuf);
					}
					
					// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
					
					memset(buf , '\0' , sizeof(buf));
					memset(TmpBuf , '\0' , sizeof(TmpBuf));
					
					// Block 2
					sprintf(buf , "ReadSector%02X:: Block2:" , NextNiscountData.block3[14]);
					
					for(i = 0 ; i < 16 ; i++)
					{
						sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData2.block2[i]); 
						strcat(buf , TmpBuf);
					}
					
					// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
					
					memset(buf , '\0' , sizeof(buf));
					memset(TmpBuf , '\0' , sizeof(TmpBuf));
					
					// Block 3
					sprintf(buf , "ReadSector%02X:: Block3:" , NextNiscountData.block3[14]);
					
					for(i = 0 ; i < 16 ; i++)
					{
						sprintf(TmpBuf , "%02X " , (unsigned char)NextNiscountData2.block3[i]); 
						strcat(buf , TmpBuf);
					}
					
					// nick mark 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 //ShowMessage(buf);
					
					memset(buf , '\0' , sizeof(buf));
					memset(TmpBuf , '\0' , sizeof(TmpBuf));
				}					//Frank add 20121123
			}					//Frank add 20121123
		}					//Frank add 20121123
		//Frank add e 20120911
		
		//Frank mark 20120206 printf("Ticket Ver :  %2X ",(unsigned char)parkData.block1[0]);
		//Frank mark 20120206 printf("\n");
		
		if (parkData.block1[0] == 1)
		{
			Block1 block1;
			Block2 block2;
			Block3 block3;
		// 20110506 Tony add e
			
			memcpy(&block1,parkData.block1,sizeof(strBlock1));
			memcpy(&block2,parkData.block2,sizeof(strBlock2));
			memcpy(&block3,parkData.block3,sizeof(strBlock3));
			memcpy(&discountSector,&discountData,sizeof(SectorDiscountData));
			
			// ==================== //
			//Frank mark s 20120206
			// 20110512 Tony add s
			//sprintf(buf,"Block1 : %s ",parkData.block1);
			//ShowMessage(buf);
			//
			//sprintf(buf,"Block2 : %s ",parkData.block2);
			//ShowMessage(buf);
			//
			//sprintf(buf,"Block3 : %s ",parkData.block3);
			//ShowMessage(buf);
			//
			//printf("Block1:");
			//
			//for(i=0;i<16;i++)
			//{
			//	printf("%02X ",(unsigned char)parkData.block1[i]); 
			//}
			//
			//printf("\n");
			//printf("Block2:");
			//
			//for(i=0;i<16;i++)
			//{
			//	printf("%02X ",(unsigned char)parkData.block2[i]);
			//}
			//
			//printf("\n");
			//printf("Block3:");
			//
			//for(i=0;i<16;i++)
			//{
			//	printf("%02X ",(unsigned char)parkData.block3[i]);
			//}
			//
			//printf("\n");
			// 20110512 Tony add e
			//Frank mark e 20120206
			// ==================== //
			
			//Frank mark 20121024 ticketData->TicketVer = 2;					//Frank add 20120206
			ticketData->TicketVer = block1.format;					//Frank add 20121024
			ticketData->parkingId = block1.parkingId | ((block1.areaId & 0xC0) << 2);
			ticketData->areaId = (block1.areaId & 0x3F);
			ticketData->ticketID = block1.ticketID;
			ticketData->status = block2.status;
			
			memcpy(ticketData->plate,block1.plate,sizeof(block1.plate));
			
			ticketData->in_year = BCD2Int(block2.in_year,2);
			ticketData->in_month = BCD2Int(&block2.in_month);
			ticketData->in_day = BCD2Int(&block2.in_day);
			ticketData->in_hour = BCD2Int(&block2.in_hour);
			ticketData->in_min = BCD2Int(&block2.in_min);
			ticketData->value = block2.Value;
			ticketData->seasonVersion = block2.seasonVersion;
			
			// 折扣
			//=======================================================================
			ticketData->discount_year	= BCD2Int(discountSector.block1.DiscountYear,2);
			ticketData->discount_month	= BCD2Int(&discountSector.block1.DiscountMon);
			ticketData->discount_day	= BCD2Int(&discountSector.block1.DiscountDay);
			//ticketData->discount_hour	= BCD2Int(&discountSector.block1.DiscountHour);
			//ticketData->discount_min	= BCD2Int(&discountSector.block1.DiscountMin);
			
			//Frank mark 20120914 ticketData->LimitDiscount	= discountSector.block1.LimitDiscount;
			ticketData->LHDisctCnt		= discountSector.block1.LimitDiscount;					//Frank add 20120914
			ticketData->LimitAID			= discountSector.block1.LimitAID;
			ticketData->LimitSID			= discountSector.block1.LimitSID;
			ticketData->LimitVID			= discountSector.block1.LimitVID;
			ticketData->LimitYear		= BCD2Int(discountSector.block1.LimitYear,2);
			ticketData->LimitMon			= BCD2Int(&discountSector.block1.LimitMon);
			ticketData->LimitDay			= BCD2Int(&discountSector.block1.LimitDay);
			ticketData->LimitHour		= BCD2Int(&discountSector.block1.LimitHour); 
			
			if(discountSector.block1.DiscountCount > 7)
				discountSector.block1.DiscountCount = 7;
			
			ticketData->DiscountCount = discountSector.block1.DiscountCount; //折扣數量
			
			for(i = 0; i < discountSector.block1.DiscountCount; i++)
			{
				ticketData->discounts[i].AID = discountSector.block23.discounts[i].AID;
				ticketData->discounts[i].SID = discountSector.block23.discounts[i].SID;
				ticketData->discounts[i].VID = discountSector.block23.discounts[i].VID;
				ticketData->discounts[i].DiscountHour = discountSector.block23.discounts[i].DiscountHour;
			}
			
			//限時折扣的AID,SID VID
			ticketData->discounts[7].AID = discountSector.block1.LimitAID;
			ticketData->discounts[7].SID = discountSector.block1.LimitSID;
			ticketData->discounts[7].VID = discountSector.block1.LimitVID;
			//=======================================================================
			
			memcpy(ticketData->optime,block3.optime,3);			
			
			ticketData->out_year  = BCD2Int(block3.out_year,2);
			ticketData->out_month = BCD2Int(&block3.out_month);
			ticketData->out_day	 = BCD2Int(&block3.out_day);
			ticketData->out_hour  = BCD2Int(&block3.out_hour);
			ticketData->out_min	 = BCD2Int(&block3.out_min);
			ticketData->out_sec	 = BCD2Int(&block3.out_sec);
			
		// 20110506 Tony add s
		}
		else if(parkData.block1[0] == 2)
		{
			Block1V2 block1;
			Block2V2 block2;
			Block3V2 block3;
			
			//Frank modify s 20120206
			memcpy(&block1,parkData.block1,sizeof(strBlock1V2));
			memcpy(&block2,parkData.block2,sizeof(strBlock2V2));
			memcpy(&block3,parkData.block3,sizeof(strBlock3V2));
			//Frank modify e 20120206
			
			/*
			sprintf(buf,"Block1 : %s ",parkData.block1);
			ShowMessage(buf);
			
			sprintf(buf,"Block2 : %s ",parkData.block2);
			ShowMessage(buf);
			
			sprintf(buf,"Block3 : %s ",parkData.block3);
			ShowMessage(buf);
			*/
			
			//Frank mark s 20120206
			/*
			printf("Block1:");
			
			for(i=0;i<16;i++)
			{
				printf("%2X ",(unsigned char)parkData.block1[i]);
			}
			
			printf("\n");
			printf("Block2:");
			
			for(i=0;i<16;i++)
			{
				printf("%02X ",(unsigned char)parkData.block2[i]);
			}
			
			printf("\n");
			printf("Block3:");
			
			for(i=0;i<16;i++)
			{
				printf("%2X ",(unsigned char)parkData.block3[i]);
			}
			
			printf("\n");
			*/
			//Frank mark e 20120206
			
			ticketData->TicketVer = block1.TicketVer;
			ticketData->parkingId = ((block1.NextSector & 0x300)>>2) * 255 + (block1.parkingId & 0xFF);
			ticketData->areaId = (block1.areaId & 0xFF);
			ticketData->ticketID = block1.ticketID;
			
			memcpy(ticketData->plate,block1.plate,sizeof(block1.plate));
			
			ticketData->status = block2.status;		
			ticketData->in_year = BCD2Int(block2.in_year,2);
			ticketData->in_month = BCD2Int(&block2.in_month);
			ticketData->in_day = BCD2Int(&block2.in_day);
			ticketData->in_hour = BCD2Int(&block2.in_hour);
			ticketData->in_min = BCD2Int(&block2.in_min);
			ticketData->in_sec = BCD2Int(&block2.in_sec);
			ticketData->value = block2.Value;
			ticketData->seasonVersion = block2.seasonVersion;
			
			ticketData->out_year  = BCD2Int(block3.out_year,2);
			ticketData->out_month = BCD2Int(&block3.out_month);
			ticketData->out_day	 = BCD2Int(&block3.out_day);
			ticketData->out_hour  = BCD2Int(&block3.out_hour);
			ticketData->out_min	 = BCD2Int(&block3.out_min);
			ticketData->out_sec	 = BCD2Int(&block3.out_sec);
			
			memcpy(ticketData->optime,block3.optime,3);
			
			ticketData->staytimetype = block3.staytime >> 28 & 0xF;
			ticketData->staytime = block3.staytime & 0xFFFFFFF;
			
			if (block3.DisctSector != 0)
			{
				//Frank mark 20120912 memcpy(&discountSector , &discountData , sizeof(SectorDiscountData));
				//Frank add s 20120912
				memset(&ticketData->discounts , '\0' , sizeof(ticketData ->discounts));
				
				ticketData->LHDisctCnt = Discountarray.limit.LHDisctCnt;
				
				ticketData->discount_year		= BCD2Int(Discountarray.limit.DiscountYear , 2);
				ticketData->discount_month		= BCD2Int(&Discountarray.limit.DiscountMon);
				ticketData->discount_day		= BCD2Int(&Discountarray.limit.DiscountDay);
				ticketData->discount_hour		= BCD2Int(&Discountarray.limit.DiscountHour);
				ticketData->discount_min		= BCD2Int(&Discountarray.limit.DiscountMin);
				ticketData->discount_sec		= BCD2Int(&Discountarray.limit.DiscountSec);
				
				ticketData->LimitAID		= Discountarray.limit.LimitAID;
				ticketData->LimitSID		= Discountarray.limit.LimitSID;
				ticketData->LimitVID		= Discountarray.limit.LimitVID;
				ticketData->LimitYear		= BCD2Int(Discountarray.limit.LimitYear , 2);
				ticketData->LimitMon		= BCD2Int(&Discountarray.limit.LimitMon);
				ticketData->LimitDay		= BCD2Int(&Discountarray.limit.LimitDay);
				ticketData->LimitHour		= BCD2Int(&Discountarray.limit.LimitHour);
				
				for(i = 0 ; i <= ticketData->DiscountCount / 2; i++)
				{
					ticketData->Discounts2[2 * i].AID = Discountarray.block[i].discount[0].AID;
					ticketData->Discounts2[2 * i].SID = Discountarray.block[i].discount[0].SID;
					ticketData->Discounts2[2 * i].VID = Discountarray.block[i].discount[0].VID;
					
					memcpy(ticketData ->Discounts2[2 * i].DiscountHour , Discountarray.block[i].discount[0].DiscountHour , 4);
					
					ticketData->Discounts2[2 * i + 1].AID = Discountarray.block[i].discount[1].AID;
					ticketData->Discounts2[2 * i + 1].SID = Discountarray.block[i].discount[1].SID;
					ticketData->Discounts2[2 * i + 1].VID = Discountarray.block[i].discount[1].VID;
					
					memcpy(ticketData ->Discounts2[2 * i + 1].DiscountHour , Discountarray.block[i].discount[1].DiscountHour , 4);
					
				}
				//Frank add e 20120912
			}
		}
		// 20110506 Tony add e
		
		//Frank add s 20120206
		if((ticketData->ticketID % 10L) == 2)
		{
			sprintf(buf , "TicketData :: TicketID:%09ld , SeasonVersion:%ld" , ticketData->ticketID , ticketData->seasonVersion);
		}
		else
		{
			sprintf(buf , "TicketData :: TicketID:%09ld" , ticketData->ticketID );
		}
		
		ShowMessage(buf);
		//Frank add e 20120206
		
		// nick mark 20140929 Ver:000-000-GIO_V2-135101-0006-13B251 //sprintf(buf,"TicketData :: Entry Time:%04d.%02d.%02d %02d:%02d:%02d" , ticketData->in_year , ticketData->in_month ,
		// nick mark 20140929 Ver:000-000-GIO_V2-135101-0006-13B251 //	ticketData->in_day , ticketData->in_hour , ticketData->in_min , ticketData->in_sec);				//Frank add 20120208
		sprintf(buf, "TicketData :: Entry Time:[%04d.%02d.%02d %02d:%02d:%02d], LimitDiscount:[%d], DiscountCnt:[%d]", ticketData->in_year, ticketData->in_month ,
			ticketData->in_day, ticketData->in_hour, ticketData->in_min, ticketData->in_sec, ticketData->LHDisctCnt, ticketData->DiscountCount); // nick add 20140929 Ver:000-000-GIO_V2-135101-0006-13B251 //
		
		ShowMessage(buf);						//Frank add 20120208
		// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //bReadTicket = true;
		iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
	}
	else if(ReaderCFG.HourlyReaderType == 5)
	{
		int parkid,area,status,year,month,day,hour,minu;
		long ticketId;
		time_t now;
		struct tm *tm_ptr = NULL;
		struct tm entryData;
		time_t t_start;
		
		now = time((time_t *)0);
		tm_ptr = localtime(&now);
		
		BCdata = (BarCodeData*)(&BCodeData);
		
		for(i=0; i<1; i++)
		{
			//memset(BCodeData,'\0',sizeof(BCodeData));					//Frank mark 20111116
			memset( BCodeData, 0, 128);					//Frank add 20111116
			
			if(barcode->GetData((char*)(&BCodeData))==true)
			{
				//bReadTicket = true;
				//printf("Barcode:%s\n",BCodeData);					//Frank mark 20111116
				sprintf(buf,"Barcode:%s\n",BCodeData);					//Frank add 20111116
				ShowMessage(buf);									//Frank add 20111116
				iLastRtn = 1;
			}
			else 
			{
				RejectTicket();
				sprintf(buf,"Hourly Read Barcode data error!");
				ShowMessage(buf);					//Frank add 20111116
				//return -1;
				iLastRtn = -1;
				goto FUNCEXIT;
			}
		}
		
		if(strlen(BCodeData) > 20)
		{
			sscanf(BCodeData,"%2d%1d%1d%2d%2d%2d%2d%8ld",&parkid,&area,&status,&month,&day,&hour,&minu,&ticketId);
			entryData.tm_year = tm_ptr->tm_year;
			entryData.tm_mon	= month-1;
			entryData.tm_mday = day;
			entryData.tm_hour = hour;
			entryData.tm_min	= minu;
			entryData.tm_sec	= 59;
			ticketData->status= status;
			t_start = mktime(&entryData);
			
			if(t_start > now)
			{
				entryData.tm_year = tm_ptr->tm_year-1;
			}
			
			ticketData->in_year	 = entryData.tm_year+1900;
			ticketData->in_month  = month;
			ticketData->in_day	 = day;
			ticketData->in_hour	 = hour;
			ticketData->in_min	 = minu;
			ticketData->ticketID  = (ticketId *10L +1L);
			ticketData->parkingId = parkid;
			ticketData->areaId	 = area;
		}
		
		if(strlen(BCodeData) > 44)
		{
			sscanf(BCodeData+21+13,"0%1d%2d%2d%2d%2d%02d",&status,&year,&month,&day,&hour,&minu);
			
			ticketData->in_year	= year + 2000; //票卡資料是存 20xx 末2碼
			ticketData->in_month = month;
			ticketData->in_day	= day;
			ticketData->in_hour	= hour;
			ticketData->in_min	= minu;
			ticketData->status	  = 2;
			//printf("PayTime2:%04d-%02d-%02d %02d:%02d \n",year + 2000,month,day,hour,minu);
		}
		else if(strlen(BCodeData) > 22)
		{
			sscanf(BCodeData+21,"0%1d%2d%2d%2d%2d%02d",&status,&year,&month,&day,&hour,&minu);
			ticketData->in_year	= year + 2000;
			ticketData->in_month = month;
			ticketData->in_day	= day;
			ticketData->in_hour	= hour;
			ticketData->in_min	= minu;
			ticketData->status	  = 2;
			//printf("PayTime1:%04d-%02d-%02d %02d:%02d \n",year + 2000,month,day,hour,minu);
		}
		
		barcode->ReadEnd();
	}
	else if(ReaderCFG.HourlyReaderType == 2)
	{
		printf("ReaderType2\n");
	}
	else if(ReaderCFG.HourlyReaderType == 1)
	{
		printf("ReaderType3\n");
	}
	else
	{
		if(ticketType == SEASON_TICKET && mf700s != NULL )
		{
			printf("Read season reader. \n");
			
			if(G_ParkingConfig.bReadTag == true)
			{	// Use TAG to get Season Data from db
				// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //return 1;
				iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
				goto FUNCEXIT; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			}
			else
			{
				// ========================================================== //
				// nick mark s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				//if((iRet=mf700s->ReadSector(ticketData->TagID,PARKING_DATA_SECTOR,&parkData)) != 1)
				//{
				//	ShowMessage((char *)"Read data sector error.");
				//	return -1;
				//}
				// nick mark e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================== //
				// ========================================================= //
				// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				iRet = 0;
				
				if (SeasonReaderModule == 0)
				{
					if (mf700s != NULL)
						iRet = mf700s->ReadSector(ticketData->TagID, PARKING_DATA_SECTOR, &parkData);
				}
				else if (SeasonReaderModule == 1)
				{
					if (hf320s != NULL)
						iRet = hf320s->ReadSector(ticketData->TagID, PARKING_DATA_SECTOR, &parkData);
				}
				
				if (iRet != 1)
				{
					ShowMessage((char *)"Read data sector error.");
					//return -1;
					iLastRtn = -1;
					goto FUNCEXIT;
				}
				// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
				// ========================================================= //
				
				// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //bReadTicket = true;
				iLastRtn = 1; // nick add 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //
			}
		}
	}
	
	FUNCEXIT:
	
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //if(bReadTicket)
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //	return 1;
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //else 
	// nick mark 20160705 Ver:000-000-GIO_V2-13C241-0001-166241 //	return 0;
	
	if (ReaderCFG.HourlyReaderType == 7) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
		hf320h->SetAPDUMode(true); // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	
	return (iLastRtn); // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
}

int Int2BCD(int Value, int ValueDigit,unsigned char* BCD)
{
	char tmp[10];
	int i,j,Result = 0;
	 
	memset(tmp,'\0',sizeof(tmp));
	
	for(i=0;ValueDigit>0;Value /= 100,i++,ValueDigit-=2)
	{
		Result = Value % 100;
		tmp[i] = (char)( ((Result/10) << 4) | (Result%10) );
	}
	
	for(j=0,i=i-1;i>=0;i--,j++)
		BCD[j] = tmp[i];
	
	return j;
}

int BCD2Int( unsigned char bcd[], int byte)
{
	int result = 0;
	
	switch(byte)
	{
		default:
		case 1:
			result = (((bcd[0] & 0xf0) >> 4) * 10 + (bcd[0] & 0x0f));
			break;
		case 2:
			result = (((bcd[1]&0xf0)>>4)*10+(bcd[1]&0x0f)+((bcd[0]&0xf0)>>4)*1000+(bcd[0]&0x0f)*100);
			break;	
		case 3 :	// 20111205 Tony add
			result = (((bcd[2]&0xf0)>>4)*10+(bcd[2]&0x0f)+((bcd[1]&0xf0)>>4)*1000+(bcd[1]&0x0f)*100+((bcd[0]&0xf0)>>4)*100000+(bcd[0]&0x0f)*10000);
			break;
	}
	
	return result;
}

//Frank add s 20120917
unsigned long BCD2Ulong(unsigned char bcd[])
{
	unsigned long result = 0;
	
	return result = (((bcd[3] & 0xf0) >> 4) * 16 + (bcd[3] & 0x0f) + ((bcd[2] & 0xf0) >> 4) * 4096 + (bcd[2] & 0x0f) * 256
			+ ((bcd[1] & 0xf0) >> 4) * 1048576 + (bcd[1] & 0x0f) * 65536 + (bcd[0] & 0x0f) * 16777216);
}
//Frank add e 20120917

//Frank add s 20111116
void ClearReaderBuff()
{
	if (mf700h != NULL)
		mf700h->ClearBuff();
	if (mf700h2 != NULL)
		mf700h2->ClearBuff();
	if (crt350h != NULL)
		crt350h->ClearBuff();
	if(eltra1000h != NULL)
		eltra1000h->ClearBuff();
	if(barcode != NULL)
		barcode->ClearBuff();
	if(mf700s != NULL)
		mf700s->ClearBuff();
	if(crt350s != NULL)
		crt350s->ClearBuff();
	if(eltra1000s != NULL)
		eltra1000s->ClearBuff();
	//Frank add s 20120509
	if(r_mcp210 != NULL)
		r_mcp210 ->ClearBuff();
	//Frank add e 20120509
	// 20130117 Tony add s
	if(printer != NULL)
	{
		if(printer->CheckFE() == true)
		{
			printf("ClearReaderBuff :: printer->CheckFE = True , Power reopen");
			Tup500Initial();
		}else{
			printer->ClearBuff();
		}
	}
	// 20130117 Tony add e
	// ========================================================= //
	// nick add s 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	if (hf320h != NULL)
		hf320h->ClearBuff();

	if (hf320h2 != NULL)
		hf320h2->ClearBuff();

	if (hf320s != NULL)
		hf320s->ClearBuff();
	// nick add e 20150120 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	// ========================================================= //
}
//Frank add e 20111116

//Frank add s 20120509
bool GetPrintData(char *data , TicketData *ticketData)
{
	FILE *fh = NULL;
	char* SectionPoint = NULL;
	char PrintString[20] , InorOut[5] , Type[10];
	
	fh = fopen("PrintData.ini" , "r");
	
	if(fh == NULL)
	{
		return false;
	}
	
	memset(InorOut , '\0' , sizeof(InorOut));
	memset(Type , '\0' , sizeof(Type));
	memset(PrintString , '\0' , sizeof(PrintString));
	
	fseek (fh , 0L , SEEK_SET);
	
	if(IsINMachine == true)
	{
		sprintf(InorOut , "in");
	}
	else
	{
		sprintf(InorOut , "out");
	}
	
	if(ticketData->ticketID % 10L == 2 && ticketData->seasonVersion > 0)
	{
		sprintf(Type , "month");
	}
	else if(ticketData->ticketID % 10L == 3)
	{
		sprintf(Type , "value");
	}
	else if(ticketData->ticketID % 10L == 1)
	{
		sprintf(Type , "ticket");
	}
	else
	{
		printf("ticketData->ticketID : %8ld\n" , ticketData->ticketID);
		ShowMessage((char *)"unkown ticket.");
		return false;
	}
	
	sprintf(PrintString , "[%s_print_%s]" , Type , InorOut);
	
	while(true)
	{
		if(INIReadLine(data , fh) == true)
			break;
		
		if( SectionPoint == NULL)
		{
			SectionPoint = strstr(data , PrintString);
		}
		else
		{
			break;
		}

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	MakePrintString(data , ticketData);
	fclose(fh);
	return true;
}

void MakePrintString(char *data , TicketData *ticketData)
{
	unsigned int i , j = 0 , k;
	char Temp[128] , StrTemp[128] , qSQL[SQLLength + 1];
	int rows , cols;
	char *errMsg = NULL;
	char **result;
	sqlite3 *db = NULL;
	time_t now;
	long endT = 0;
	struct tm *EndTime = NULL;
	
	now = time((time_t *)0);
	pthread_mutex_lock(&Data_mutex); // nick add 20130202 //
	
	if (sqlite3_open_v2("./data/parking.s3db" , &db , SQLITE_OPEN_READWRITE , NULL) == SQLITE_OK)
	{
		sprintf(qSQL , "SELECT TicketID,EndTime FROM Season WHERE TicketID=%ld;" , ticketData->ticketID);
		
		if(sqlite3_get_table(db , qSQL , &result , &rows , &cols , &errMsg) == SQLITE_OK)
		{
			if(rows > 0)
			{
				endT = atol(result[cols + 1]);
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
	}
	else
	{
		printf("Can't Open parking.s3db\n");
	}

	pthread_mutex_unlock(&Data_mutex); // nick add 20130202 //
	
	memset(Temp , '\0' , sizeof(Temp));
	memset(StrTemp , '\0' , sizeof(StrTemp));
	
	for(i = 0 ; i < strlen(data) ; i++)
	{
		if(data[i] != '%')
		{
			Temp[j] = data[i];
			j++;
		}
		else if(data[i] == '%')
		{
			i += 2;
			
			if(data[i] == 'd')
			{
				sprintf(StrTemp , "%4d%02d%02d" , ticketData->in_year , ticketData->in_month , ticketData->in_day);
			}
			else if(data[i] == 't')
			{
				sprintf(StrTemp , "%02d:%02d" , ticketData->in_hour , ticketData->in_min);
			}
			else if(data[i] == 's')
			{
				if(IsINMachine == true)
				{
					if (G_ParkingConfig.ParkLanguage != 0)
					{
						sprintf(StrTemp , "已進場");
					}
					else
					{
						sprintf(StrTemp , "IN");
					}
				}
				else
				{
					if (G_ParkingConfig.ParkLanguage != 0)
					{
						sprintf(StrTemp , "已出場");
					}
					else
					{
						sprintf(StrTemp , "OUT");
					}
				}
			}
			else if(data[i] == 'n')
			{
				sprintf(StrTemp , "%8ld" , ticketData->ticketID);
			}
			else if(data[i] == 'c')
			{
				sprintf(StrTemp , "%s" , ticketData->plate);
			}
			else if(data[i] == 'v')
			{
				sprintf(StrTemp , "%ld" , ticketData->value);
			}
			else if(data[i] == 'e')
			{
				// nick mark 20130311 //EndTime = localtime((time_t*)&endT);
				EndTime = gmtime((time_t *)&endT); // nick add 20130311 //
				sprintf(StrTemp , "%4d%02d%02d" , EndTime->tm_year + 1900 , EndTime->tm_mon + 1 , EndTime->tm_mday);
			}
			else if(data[i] == '~')
			{
				memset(StrTemp , '\0' , sizeof(StrTemp));
				StrTemp[0] = 0x1f;
			}
			else if(data[i] == 'b')
			{
				sprintf(StrTemp , "%02d" , G_ParkingConfig.MachineID);
			}
			else if(data[i] == 'u')
			{
				sprintf(StrTemp , "%d" , ticketData->DiscountCount);
			}
			else if(data[i] == 'm')
			{
				sprintf(StrTemp , "!");
			}
			else if(data[i] == 'y')
			{
				sprintf(StrTemp , "!");
			}
			else if(data[i] == 'a')
			{
				sprintf(StrTemp , "!");
			}
			else					//Frank add 20120710
				memset(StrTemp , '\0' , sizeof(StrTemp));					//Frank add 20120710
				
			strcat(Temp , StrTemp);
			k = strlen(StrTemp);
			j += k;
		}
	}
	
	memcpy(data , Temp , sizeof(Temp));
/*	
	for(i = 0 ; i < strlen(data) ; i++)
	{
		printf("%c" , data[i]);
		if(data[i] == 0x1f)
			printf("\n");
	}
	printf("len : %d\n", strlen(data));
*/
}
//Frank add e 20120509

// 20121101 Tony add s
int GetPntLogoAddress(char FileName[], int* x , int* y)
{
	// < 0 , 找不到圖片
	// >= 0 , 回傳圖片列印的X軸
	char bmp[80];
	char buffer[100];
	int len;
	size_t result;
	unsigned long Width=0;
	unsigned long Height=0;
	int RtnX =0;
	int RtnY =0;
	
	FILE* fh=NULL;
	
	memset(bmp,'\0',sizeof(bmp));
	memset(buffer,'\0',sizeof(buffer));
	
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)
		sprintf(bmp,"./jpg/%s/%s", G_ParkingConfig.ParkLanguage,FileName);
	else
		sprintf(bmp,"./jpg/%s", FileName);
	
	//printf("FilePath = %s\n",bmp);
	
	fh =fopen(bmp,"r");
	
	if(fh == NULL)
	{
		sprintf(bmp,"./jpg/%s", FileName);
		fh =fopen(bmp,"r");
		
		if(fh == NULL)
		{
			printf("no bmp file\n");
			return -1;
		}
	}
	
	fseek (fh, 0, SEEK_END);
	len = ftell (fh);
	
	fseek (fh, 0, SEEK_SET);
	result = fread(buffer,1,54,fh);
	
	memcpy(&Width, buffer + 0x12, 4);
	//printf("BmpHead.Width = %ld\n",Width);
	
	memcpy(&Height, buffer + 0x16, 4);
	//printf("BmpHead.Height = %ld\n",Height);
	
	fclose (fh);
	
	RtnX = (23 - (Width * 125 / 1000)) * 10;
	//printf("Rtn X = %d\n",RtnX);
	memcpy(x, &RtnX , sizeof(x));
	
	RtnY = (60 - (Height * 125 / 1000)) * 10;
	//printf("Rtn Y = %d\n",RtnY);
	memcpy(y, &RtnY , sizeof(y));
	
	return 0;
}

int GetPntNoteAddress(char FileName[], int* y)
{
	// < 0 , 找不到圖片
	// >= 0 , 回傳圖片列印的X軸
	char bmp[80];
	char buffer[100];
	int len;
	size_t result;
	//unsigned long Width=0;
	unsigned long Height=0;
	//int RtnX =0;
	int RtnY =0;
	
	FILE* fh=NULL;
	
	memset(bmp,'\0',sizeof(bmp));
	memset(buffer,'\0',sizeof(buffer));
	
	if (strlen(G_ParkingConfig.ParkLanguage) != 0)
		sprintf(bmp,"./jpg/%s/%s", G_ParkingConfig.ParkLanguage,FileName);
	else
		sprintf(bmp,"./jpg/%s", FileName);
	
	//printf("FilePath = %s\n",bmp);
	
	fh =fopen(bmp,"r");
	
	if(fh == NULL)
	{
		sprintf(bmp,"./jpg/%s", FileName);
		fh =fopen(bmp,"r");
		
		if(fh == NULL)
		{
			printf("no bmp file\n");
			return -1;
		}
	}
	
	fseek (fh, 0, SEEK_END);
	len = ftell (fh);
	
	fseek (fh, 0, SEEK_SET);
	result = fread(buffer,1,54,fh);
	
	//memcpy(&Width, buffer + 0x12, 4);
	//printf("BmpHead.Width = %ld\n",Width);
	
	memcpy(&Height, buffer + 0x16, 4);
	//printf("BmpHead.Height = %ld\n",Height);
	
	fclose (fh);
	
	//RtnX = (23 - (Width * 125 / 1000)) * 10;
	//printf("Rtn X = %d\n",RtnX);
	//memcpy(x, &RtnX , sizeof(x));
	
	RtnY = (60 - (Height * 125 / 1000)) * 10;
	//printf("Rtn Y = %d\n",RtnY);
	memcpy(y, &RtnY , sizeof(y));
	
	return 0;
}

// 20121101 Tony add e

// 20121101 Tony add e

void Tup500Initial()
{	
	ShowMessage((char *)"Tup500Initial()");	// 20120427 Tony add
	printf("ReaderInitial :: TUP500 : Start : %ld\n",GetTickCount()-CalcTmpTime);
	
	// ========================================================== //
	// nick mark s 20140606 Ver:000-000-GIO_V2-135101-0004-13B251 //
	if (printer != NULL)
	{
		printer->Close();
		delete printer;
		usleep(6000000L);
	}
	
	printer = new TUP500();
	printer->init(ReaderCFG.HourlyReaderComPort);
	// nick mark e 20140606 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================== //

	//printf("Initial TUP500 COM%d. \n",ReaderCFG.HourlyReaderComPort+1);
	//
	//if (printer == NULL) // nick add s 20140606 Ver:000-000-GIO_V2-135101-0004-13B251 //
	//{
	//	printer = new TUP500();
	//	printer->init(ReaderCFG.HourlyReaderComPort);
	//}
	//
	//sleep(6); // nick add s 20140606 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	// Initial Tup500
	//Frank mark 20120806 if(G_ParkingConfig.BarCodeType ==1)
	if(G_ParkingConfig.BarCodeType != 0)					//Frank add 20120806
	{
		printer->ClearSetting();
		//printer.ClearSetting();
		printf("ReaderInitial :: TUP500 : ClearSetting : %ld\n",GetTickCount()-CalcTmpTime);
		
		printer->SetPrintArea(710);
		//printer.SetPrintArea(710);
		printf("ReaderInitial :: TUP500 : SetPrintArea : %ld\n",GetTickCount()-CalcTmpTime);
		
		printer->ClearBitmap();
		//printer.ClearBitmap();
		printf("ReaderInitial :: TUP500 : ClearBitmap : %ld\n",GetTickCount()-CalcTmpTime);
		
		// 20120913 Tony mark printer.SetHLine(120,600,20,2); //畫橫線
		printer->SetHLine(120,600,40,2); //畫橫線	// 20120913 Tony add
		//printer.SetHLine(120,600,40,2); //畫橫線	// 20120913 Tony add				
		printf("ReaderInitial :: TUP500 : SetHLine : %ld\n",GetTickCount()-CalcTmpTime);
		
		//printer.SetArrowImage(120,673);  //箭頭
		//printer.SetArrowImage(570,673);  //箭頭
		printer->SetArrowImage(295,677);  //箭頭 中間
		//printer.SetArrowImage(295,677);  //箭頭 中間				
		printf("ReaderInitial :: TUP500 : SetArrowImage : %ld\n",GetTickCount()-CalcTmpTime);
		
		printer->SetBarCodePosition(180, 566, 1, 7, 0, 100); // Barcode 位置
		//printer.SetBarCodePosition(180, 566, 1, 7, 0, 100); // Barcode 位置
		printf("ReaderInitial :: TUP500 : SetBarCodePosition : %ld\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCharactPosition(599, 100, 1, 2, 2, 22); //停車場名稱
		//printer.SetCharactPosition(599, 100, 1, 2, 2, 22); //停車場名稱
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n",GetTickCount()-CalcTmpTime);
		
		// 20120913 Tony mark printer.SetCharactPosition(570,18, 1, 1, 1, 22); //小字
		printer->SetCharactPosition(599,36, 1, 2, 1, 22); //小字	// 20120913 Tony add
		//printer.SetCharactPosition(599,36, 1, 2, 1, 22); //小字	// 20120913 Tony add
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCutter();
		//printer.SetCutter();
		printf("ReaderInitial :: TUP500 : SetCutter : %ld\n",GetTickCount()-CalcTmpTime);
		
		//printer.SetPrintDensity(5);
	}
	else if(G_ParkingConfig.BarCodeType == 0)
	//else
	{
		printer->ClearSetting();
		//printer.ClearSetting();
		printf("ReaderInitial :: TUP500 : ClearSetting : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		printer->SetPrintArea(700);
		//printer.SetPrintArea(700);
		printf("ReaderInitial :: TUP500 : SetPrintArea : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		// 20121101 Tony add s
		printer->ClearBitmap();
		//printer.ClearBitmap();
		printf("ReaderInitial :: TUP500 : ClearBitmap : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		int Logo_Print_x = 0;
		int Logo_Print_y = 0;
		
		if(GetPntLogoAddress((char *)"Tick_Logo.bmp" ,&Logo_Print_x,&Logo_Print_y) < 0)
		{
		// 20121101 Tony add e
			printer->SetCharactPosition(100,600,2,2,2,33); //前2行 Title
			//printer.SetCharactPosition(100,600,2,2,2,33); //前2行 Title
			printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
			
			printer->SetCharactPosition(175,600,2,2,2,33);
			//printer.SetCharactPosition(175,600,2,2,2,33);
			printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		// 20121101 Tony add s
		}
		else	
		{
			printer->SetPrintImage(Logo_Print_x,Logo_Print_y,(char *)"Tick_Logo.bmp");
			//printer.SetPrintImage(Logo_Print_x,Logo_Print_y,(char *)"Tick_Logo.bmp");
			printf("ReaderInitial :: TUP500 : SetPrintImage : Logo : %ld\n\n",GetTickCount()-CalcTmpTime);
		}
		// 20121101 Tony add e
		
		printer->SetCharactPosition(245,595,1,1,2,33); //DataTime
		//printer.SetCharactPosition(245,595,1,1,2,33); //DataTime
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCharactPosition(282,595,1,1,2,33); //Ticket No
		//printer.SetCharactPosition(282,595,1,1,2,33); //Ticket No
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCharactPosition(319,595,1,1,2,33); //Station
		//printer.SetCharactPosition(319,595,1,1,2,33); //Station
		//Frank mark 20121119 printer.SetCharactPosition(328,595,1,1,2,33); //Station
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCharactPosition(491,520,1,1,2,33); // Barcode String
		//printer.SetCharactPosition(491,520,1,1,2,33); // Barcode String
		//printer.SetCharactPosition(500,520,1,1,2,33); // Barcode String
		printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		// 20121101 Tony add s
		Logo_Print_y = 0;
		if(GetPntNoteAddress((char *)"Tick_Note.bmp",&Logo_Print_y ) < 0)
		{
		// 20121101 Tony add e
			//printer.SetCharactPosition(560,600,1,1,2,33); // Note1
			printer->SetCharactPosition(540,600,1,1,2,33); // Note1
			//printer.SetCharactPosition(540,600,1,1,2,33); // Note1
			printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
			
			//printer.SetCharactPosition(590,600,1,1,2,33); // Note2
			printer->SetCharactPosition(570,600,1,1,2,33); // Note2
			//printer.SetCharactPosition(570,600,1,1,2,33); // Note2
			printf("ReaderInitial :: TUP500 : SetCharactPosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		// 20121101 Tony add s
		}
		else
		{
			Logo_Print_x = 530;
			printer->SetPrintImage(Logo_Print_x,Logo_Print_y,(char *)"Tick_Note.bmp");
			//printer.SetPrintImage(Logo_Print_x,Logo_Print_y,(char *)"Tick_Note.bmp");
			printf("ReaderInitial :: TUP500 : SetPrintImage : Note : %ld\n\n",GetTickCount()-CalcTmpTime);
		}
		// 20121101 Tony add e
		
		printer->SetBarCodePosition(361, 520, 1, 7, 3, 120);
		//printer.SetBarCodePosition(361, 520, 1, 7, 3, 120);
		//printer.SetBarCodePosition(370, 520, 1, 7, 3, 120);
		printf("ReaderInitial :: TUP500 : SetBarCodePosition : %ld\n\n",GetTickCount()-CalcTmpTime);
		
		printer->SetCutter();
		//printer.SetCutter();
		//printer.SetPrintDensity(5);
		printf("ReaderInitial :: TUP500 : SetCutter : %ld\n\n",GetTickCount()-CalcTmpTime);
	}

	//if(printer.CheckOffline() == true)
	if(printer->CheckOffline() == true)
	{					//Frank add s 20111122
		// nick mark 20120222 G_ParkingStatus.status |= STATUS_READER_CONNECT;
		//Frank mark 20121102 G_ParkingStatus.status &= (~STATUS_READER_CONNECT);					//Frank add 20120223
		G_ParkingStatus.status &= (~STATUS_READER_CONNECT);					//Frank add 20121227
		ShowMessage((char *)"TUP500 Offline!");
		return;					//Frank add 20121102
	}					//Frank add e 20111122
	// nick mark 20120222 else
	// nick mark 20120222 {
	// nick mark 20120222	G_ParkingStatus.status &= STATUS_READER_CONNECT;
	// nick mark 20120222 }
	
	printf("ReaderInitial :: TUP500 : End : %ld\n",GetTickCount()-CalcTmpTime);
}

// ========================================================= //
// nick add s 20140409 Ver:000-000-GIO_V2-135101-0002-13B251 //
void InitialChipCoin(int DisctSectorQty)
{
	if (ReaderCFG.HourlyReaderType != 3) return;
	if (DisctSectorQty <= 0) DisctSectorQty = 2;
	if (DisctSectorQty > 12) DisctSectorQty = 12;
	
	TicketData ticketData;
	sectorData parkData;
	bool bIssueTimeout = false;
	bool bTestRst = false;
	unsigned long Ticks = GetTickCount();
	unsigned long errBeep = GetTickCount();
	int iGetRet = 0; // nick add 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	int iDSec = 0;
	int iChangeDisctSectorSort[13] = {12, 13, 14, 15, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	char logBuff[256];
	
	while(true)
	{
		GetIC8255_All_Input(); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
		usleep(100);
		
		// 有票卡在取票位置，故不繼續發卡 //
		if (CheckTakeTicket(HOURLY_TICKET) == false)
		{
			if (CheckTimeout(&errBeep, 5000))
				AudioOn(AUDIO_SEASON_W_SUC);
			
			continue;
		}
		//
		// 檢查是否有票卡在讀卡機位置 //
		if (CheckTicketIssue() == false)
		{
			// 發卡 //
			if (IssueTicket(false) == 1)
			{
				bIssueTimeout = false;
				Ticks = GetTickCount();
				
				// 檢查是否發出票卡 //
				while(true)
				{
					if (CheckTicketIssue() == true)
						break;

					if (CheckTimeout(&Ticks, 5000))
					{
						bIssueTimeout = true;
						break;
					}
					
					usleep(100);
				}
				//
				HopperReset();
				if (bIssueTimeout) continue;
			}
			//
			// 
		}
		
		for (int i = 0; i < 16; i++)
			G_MemBlocksWriteSuc[i] = 0;
		
		memset(&parkData, 0, sizeof(sectorData));
		memset(&ticketData, 0, sizeof(TicketData));
		bTestRst = false;
		
		// 變更ParkDataSector密碼 //
		ShowMessage((char*)"Change Parking Password!",2);
		// nick mark 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //mf700h->ChangeSectorKey(PARKING_DATA_SECTOR);
		// ========================================================= //
		// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		if (HourlyReaderModule1 == 0)
			mf700h->ChangeSectorKey(PARKING_DATA_SECTOR);
		else if (HourlyReaderModule1 == 1)
			hf320h->ChangeSectorKey(PARKING_DATA_SECTOR);
		else
			return;
		// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================= //

		// 讀取ParkDataSector //
		ShowMessage((char*)"Reading ChipCoin PARKING Sector!",2);
		// ========================================================= //
		// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		iGetRet = 0;
		
		if (HourlyReaderModule1 == 0)
			iGetRet = mf700h->ReadSector(ticketData.TagID, PARKING_DATA_SECTOR, &parkData);
		else if (HourlyReaderModule1 == 1)
			iGetRet = hf320h->ReadSector(ticketData.TagID, PARKING_DATA_SECTOR, &parkData);
		
		if (iGetRet != 1)
			bTestRst = false;
		else
		{
			// 讀取成功後測試寫入 //
			ShowMessage((char*)"Writing ChipCoin PARKING Sector!",2);
			iGetRet = 0;
			
			if (HourlyReaderModule1 == 0)
				iGetRet = mf700h->WriteSector(ticketData.TagID, PARKING_DATA_SECTOR, parkData, false);
			else if (HourlyReaderModule1 == 1)
				iGetRet = hf320h->WriteSector(ticketData.TagID, PARKING_DATA_SECTOR, parkData);
			
			if (iGetRet != 1)
				bTestRst = false;
			else
				bTestRst = true;
		}
		// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================= //
		// ========================================================== //
		// nick mark s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		//if (mf700h->ReadSector(ticketData.TagID, PARKING_DATA_SECTOR, &parkData) != 1)
		//	bTestRst = false;
		//else
		//{
		//	// 讀取成功後測試寫入 //
		//	ShowMessage((char*)"Writing ChipCoin PARKING Sector!",2);
		//	if (mf700h->WriteSector(ticketData.TagID, PARKING_DATA_SECTOR, parkData, false) != 1)
		//		bTestRst = false;
		//	else
		//		bTestRst = true;
		//	//
		//}
		// nick mark e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================== //
		
		// 變更ParkDataSector密碼 //
		ShowMessage((char*)"Change Parking BAK Password!",2);
		// nick mark 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //mf700h->ChangeSectorKey(PARKING_DATA_BAK_SECTOR);
		// ========================================================= //
		// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		if (HourlyReaderModule1 == 0)
			mf700h->ChangeSectorKey(PARKING_DATA_BAK_SECTOR);
		else if (HourlyReaderModule1 == 1)
			hf320h->ChangeSectorKey(PARKING_DATA_BAK_SECTOR);
		// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================= //
		
		// 讀取ParkDataSector //
		ShowMessage((char*)"Reading ChipCoin PARKING BAK Sector!",2);
		// ========================================================= //
		// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		iGetRet = 0;
		
		if (HourlyReaderModule1 == 0)
			iGetRet = mf700h->ReadSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, &parkData);
		else if (HourlyReaderModule1 == 1)
			iGetRet = hf320h->ReadSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, &parkData);
		
		if (iGetRet != 1)
			bTestRst = false;
		else
		{
			// 讀取成功後測試寫入 //
			ShowMessage((char*)"Writing ChipCoin PARKING Sector!",2);
			iGetRet = 0;
			
			if (HourlyReaderModule1 == 0)
				iGetRet = mf700h->WriteSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, parkData, false);
			else if (HourlyReaderModule1 == 1)
				iGetRet = hf320h->WriteSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, parkData);
			
			if (iGetRet != 1)
				bTestRst = false;
			else
				bTestRst = true;
		}
		// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================= //
		// ========================================================== //
		// nick mark s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		//if (mf700h->ReadSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, &parkData) != 1)
		//	bTestRst = false;
		//else
		//{
		//	// 讀取成功後測試寫入 //
		//	ShowMessage((char*)"Writing ChipCoin PARKING BAK Sector!",2);
		//	if (mf700h->WriteSector(ticketData.TagID, PARKING_DATA_BAK_SECTOR, parkData, false) != 1)
		//		bTestRst = false;
		//	else
		//		bTestRst = true;
		//	//
		//}
		// nick mark e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
		// ========================================================== //
	
		if (bTestRst == false)
		{
			// 測試失敗，退出卡片 //
			RejectTicket();
			sleep(1);
			RejectTicket();
			//
//			sleep(10);
			continue;
		}
		
		// nick mark 20141107 Ver:000-000-GIO_V2-135101-0006-13B251 //ShowMessage((char*)"Change Parking Password Success!",2);
		
		// 變更DiscountDataSector密碼 //
		for (int i = 0; i < DisctSectorQty; i++)
		{
			iDSec = iChangeDisctSectorSort[i];
			
			// === 變更後讀寫有一次失敗就退出卡片 === //
			sprintf(logBuff, "Change Discount Sector:%d Password!", iDSec);
			ShowMessage(logBuff, 2);
			// nick mark 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //mf700h->ChangeSectorKey(iDSec);
			// ========================================================= //
			// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			if (HourlyReaderModule1 == 0)
				mf700h->ChangeSectorKey(iDSec);
			else if (HourlyReaderModule1 == 1)
				hf320h->ChangeSectorKey(iDSec);
			// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
			
			// 讀取DisctDataSector //
			sprintf(logBuff, "Reading Discount Sector:%d", iDSec);
			ShowMessage(logBuff, 2);
			// ========================================================= //
			// nick add s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			iGetRet = 0;
			
			if (HourlyReaderModule1 == 0)
				iGetRet = mf700h->ReadSector(ticketData.TagID, iDSec, &parkData);
			else if (HourlyReaderModule1 == 1)
				iGetRet = hf320h->ReadSector(ticketData.TagID, iDSec, &parkData);
			
			if (iGetRet != 1)
				bTestRst = false;
			else
			{
				// 讀取成功後測試寫入 //
				ShowMessage((char*)"Writing ChipCoin PARKING Sector!",2);
				iGetRet = 0;
				
				if (HourlyReaderModule1 == 0)
					iGetRet = mf700h->WriteSector(ticketData.TagID, iDSec, parkData, false);
				else if (HourlyReaderModule1 == 1)
					iGetRet = hf320h->WriteSector(ticketData.TagID, iDSec, parkData);
				
				if (iGetRet != 1)
					bTestRst = false;
				else
					bTestRst = true;
			}
			// nick add e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================= //
			// ========================================================== //
			// nick mark s 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			//if (mf700h->ReadSector(ticketData.TagID, iDSec, &parkData) != 1)
			//	bTestRst = false;
			//else
			//{
			//	// 讀取成功後測試寫入 //
			//	sprintf(logBuff, "Writing Discount Sector:%d", iDSec);
			//	ShowMessage(logBuff, 2);
			//	if (mf700h->WriteSector(ticketData.TagID, iDSec, parkData, false) != 1)
			//		bTestRst = false;
			//	else
			//		bTestRst = true;
			//	//
			//}
			// nick mark e 20150121 Ver:000-000-GIO_V2-13B251-0001-13C241 //
			// ========================================================== //
			
			if (bTestRst == false)
			{
				sprintf(logBuff, "Change Discount Sector:%d Password Fail!", iDSec);
				ShowMessage(logBuff, 2);
				
				// 測試失敗，退出卡片 //
				RejectTicket();
				sleep(1);
				RejectTicket();
				//
//				sleep(10);
				break;
			}
			
			sprintf(logBuff, "Change Discount Sector:%d Password Success!", iDSec);
			ShowMessage(logBuff, 2);
		}
		//
		
		if (bTestRst)
		{
			// 測試成功後回收卡片 //
			RecycleTicket();
			usleep(150000L);
			//
//			sleep(10);
		}
	}
}
// nick add e 20140409 Ver:000-000-GIO_V2-135101-0002-13B251 //
// ========================================================= //

// =================================================================== //
// nick add s 20150115 Ver:000-000-GIO_V2-VNM-VISCO-13B251-0004-149121 //
void *PreIssueTicket(void *arg)
{
	if (IsINMachine)
	{
		if(G_ParkingConfig.PreTicket == true)
		{
			printf("PreIssue Ticket:[%d]\n", G_ParkingConfig.PreTicket);
			
			if (CheckTicketIssue() == 0)
			{   //發卡機裡面沒有票
				if(CheckTicketEmpty() == true)
				{
					G_ParkingStatus.status |= STATUS_TICKET_EMPTY;
				}
				
				PreIssue();
				ShowMessage((char *)"@ Pre Issue Ticket.");
			}
			else
				printf("PreIssue Have Ticket\n");
		}
	}

	pthread_exit(NULL);
}
// nick add e 20150115 Ver:000-000-GIO_V2-VNM-VISCO-13B251-0004-149121 //
// =================================================================== //

// ========================================================= //
// nick add s 20150326 Ver:000-000-GIO_V2-13B251-0004-13C241 //
void PreIssue()
{
	char buf[80];
	int retry = 0;
	
	if (IsINMachine == false) return; // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	
	printf("IssueTicket :: Start : %ld\n",GetTickCount() - CalcTmpTime);
	ShowMessage((char *)"PreIssue Ticket.");
	
	// nick mark 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //if(ReaderCFG.HourlyReaderType == 3)
	if(ReaderCFG.HourlyReaderType == 3 || ReaderCFG.HourlyReaderType == 7) // nick add 20160704 Ver:000-000-GIO_V2-13C241-0001-166241 //
	{
		IssueDispense = 1;
		IssueChipCoin1();
	}
	else if(ReaderCFG.HourlyReaderType == 4)
	{
		D1000 Dispense;
		
		Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense);
		
		if(ReaderCFG.DispenserQuantity > 1)
		{	 //有2台發卡機
			for (retry = 0; retry < 2; retry++)
			{
				if (Dispense.IsEmpty(IssueDispense) == true)
				{
					if (IssueDispense == 1)
						IssueDispense = 2;
					else
						IssueDispense = 1;
					
					continue;
				}
				
				if(G_ReadType == 1)
				{
					printf("Read Type = D1000\n");
					Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER);
				}
				else
				{
					printf("Read Type = ACT_F1\n");
					Dispense.DispenseCard(IssueDispense,ACT_F1_DISPENSE_TO_READER);
				}
				
				break;
			}
		}
		else
		{
			IssueDispense = 1;

			if(G_ReadType == 1)
			{
				printf("Read Type = D1000\n");
				Dispense.DispenseCard(IssueDispense,D1000_DISPENSE_TO_READER);
			}
			else
			{
				printf("Read Type = ACT_F1\n");
				Dispense.DispenseCard(IssueDispense,ACT_F1_DISPENSE_TO_READER);
			}
		}
	}
	else if(ReaderCFG.HourlyReaderType == 6)
	{
		D1000 Dispense;
		char status [16];
		
		memset(status , '\0' , sizeof(status));
		Dispense.init(ReaderCFG.MifareDispenserComPort, IssueDispense);
		
		if(Dispense.DispenseCardOut(IssueDispense) == -1)
		{
			sprintf(buf , "D1800 dispense card fail!");
			ShowMessage(buf);
			SendAlarm(38 , buf);
		}
		else
		{
			usleep(50000L);
			Dispense.GetStatusD2(IssueDispense , status);
			
			if(status[6] == '1')
				r_mcp210 ->InsertTicket();
		}
	}
}
// nick add e 20150326 Ver:000-000-GIO_V2-13B251-0004-13C241 //
// ========================================================= //




