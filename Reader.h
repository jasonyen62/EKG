/* Reader Object Base Class */

#ifndef ____IPMS_READER____
#define ____IPMS_READER____

#include "IPMS_Driver/rs232.h"

enum TicketType
{
	NONE_TICKET=0,
	HOURLY_TICKET,
	SEASON_TICKET,
	VALUE_TICKET,
	EASY_CARD,
	PARKTRON_CARD,
	SEASON_TICKET_H,    // Season Ticket in Hourly Reader
	HOURLY_S,           // Hourly Ticket in Season Reader
	POS_IN, // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	ThirdParty_Ticket,	// Tony add 20170307 
	ThirdParty_Ticket_IssueHS,	// Tony add 20170307 
	UNKNOW_TICKET=999,
	TICKET_READ_ERROR=1000,
	DESPENSE_ERROR=1001,
	TICKET_SEASON_READ_ERROR=1002		//nick add 20101207
};

struct ReaderConfig
{
	enum 	COMPORT HourlyReaderComPort;
	enum 	COMPORT HourlyReaderComPort2;
	enum 	COMPORT SeasonReaderComPort;
	enum    COMPORT MifareDispenserComPort;
	// readerType => 0:none 1:Eltra 2:CRT350 3:MF700 4:Dispenser+MF700
	int 	HourlyReaderType;
	int 	SeasonReaderType;
	bool    TagSeason;
	int   DispenserQuantity;     // Dispenser Quantity , on Hourly reader Type = 4	
	int   Ticket1;
	int   Ticket2;
};
/*
COM port 規化
COM1  月票讀卡機
COM2  語音
COM3  計時票讀卡機
COM4  計時票讀卡機 當MifareType 設成3時,HourlyReaderComPort2 = HourlyReaderComPort +1
COM5  D1000 D3000 發卡機 MifareType = 2,3時才有效
*/


//Globe variable define
extern ReaderConfig ReaderCFG;
extern int IssueDispense; // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //

//Function define 
void ReaderReset(void);
void DispenserEnable(bool bEnable);
bool CheckTakeTicket(enum TicketType ticketType);
char GetHourlyTicketInsert(void);
char GetSeasonTicketInsert(char* tag);
char CheckTicketIssue(void);
void ReaderInitial(void);
char IssueTicket(bool bTest = false);
bool CheckTicketEmpty(void);
void RejectTicket(void);
//Frank mark 20120508 bool TicketToOutlet(void);
bool TicketToOutlet(TicketData ticketData);					//Frank add 20120508
void RecycleTicket(bool bCanReUse = false);
// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) int WriteIssueData(TicketData ticketData);
int WriteIssueData(TicketData* ticketData);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
int WriteCarOutData(TicketData* ticketData,bool bIsHourly,bool IsOut);
int ReadTicketData(TicketData* ticketData,enum TicketType ticketType);
int Int2BCD(int Value, int ValueDigit,unsigned char* BCD);
int BCD2Int(unsigned char bcd[],int byte=1);
unsigned long BCD2Ulong(unsigned char bcd[]);					//Frank add 20120917
void ClearReaderBuff(void);					//Frank add 20111116
bool GetPrintData(char *data , TicketData *ticketData);					//Frank add 20120508
void MakePrintString(char *data , TicketData *ticketData);					//Frank add 20120508
int GetPntLogoAddress(char FileName[], int* x , int* y);			// 20121101 Tony add
int GetPntNoteAddress(char FileName[], int* y);			// 20121101 Tony add
void Tup500Initial(void);	// 20130117 Tony add
void InitialChipCoin(int DisctSectorQty = 2); // nick add 20140409 Ver:000-000-GIO_V2-135101-0002-13B251 //
void *PreIssueTicket(void *arg); // nick add 20150210 Ver:000-000-GIO_V2-13B251-0001-13C241 //
void PreIssue(void); // nick add 20150326 Ver:000-000-GIO_V2-13B251-0004-13C241 //

#endif