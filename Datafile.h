/* filename : Datafile.h    Data File Function */

#ifndef ____DATAFILE____
#define ____DATAFILE____
/*


typedef struct HourlyTicketData
{
	unsigned long TicketID;
	unsigned long in_time;
	char          Plate[10];
	bool          InStatus;
}HTicketData;
*/

#include <sqlite3.h> // nick add 20130121 //

enum LOG_LEVEL
{
	LOG_LEVEL0=0,
	LOG_LEVEL1=1,
	LOG_LEVEL2=2,
	LOG_LEVEL3=3
};

// ini Functions
bool INIReadLine(char* lineData,FILE* fh);					//Frank add 20120509
bool ReadIni(char* data,char* filename,char* Section,char* key);
bool ReadINI_Common(void);
bool ReadINI_ApsMachine(short sn);
bool ReadINI_OutMachine(short sn);
bool ReadINI_INMachine(short sn);
bool ReadINI_Location(void);
bool ReadINI_FixConfig(void); // nick add 20150814 Ver:000-000-GIO_V2-13B251-0005-13C241 //
bool ReadINI_NewTerminalConnectSetting(void);


//Temp Hourly Data
bool GetTempHourlyData(HTicketData* data);
bool DeleteTempHourlyData(HTicketData data);
bool WriteTempHourlyData(HTicketData data,bool bIsOut = false);
void DeleteTempHourlyOldData(time_t in_time);

//Temp Discount Data
bool GetTempDiscountData(HDiscountData* data);
bool WriteTempDiscountData(HDiscountData data);
bool DeleteTempDiscountData(HDiscountData data);

//nick add s 20111219
//Temp Payment Data
//Frank mark 20130115 bool DeleteTempPaymentData(HTicketData* t_data);
bool DeleteTempPaymentData(HTicketData t_data);					//Frank add 20130115
bool GetTempPaymentData(HTicketData* t_data);
bool WriteTempPaymentData(HTicketData t_data);
bool WriteTempValueData(TicketData *ticketData , int ValuePay);					//Frank add 20120524
//nick add e 20111219

void GetKeyName(char *key,char LineString[]);
unsigned long GetTicketSerial(void);
void GetStatus(void);
void StoreStatus(void);
void WriteLog(enum LOG_LEVEL llv,char logMessage[]);
//nick mark 20110209 char CheckSeasonIsValid(unsigned long ticketID,char* areaCode,char* TagID = NULL);
// nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //char CheckSeasonIsValid(TicketData Ticket,char* areaCode);
char CheckSeasonIsValid(TicketData* Ticket,char* areaCode); // nick mark 20150505 Ver:000-000-GIO_V2-13B251-0004-13C241 //
bool CheckValueIsValid(TicketData *ticketData);
bool CheckBlackList(unsigned long ticketID);
bool CheckSeasonTicketPaid(TicketData ticketData);     //nick add 20120110

//Keep Count file
void SaveTicketNumber(void);
unsigned short GetTicket2(void);
unsigned short GetTicket1(void);
unsigned short GetReTicket(void);
void SaveReTicket(unsigned short ReTickets);
void SaveTicket1(void);
void SaveTicket2(void);

//open barrier data
void DelOpenBarrier  (OpenBrData openData);
void WriteOpenBarrier(OpenBrData openData);
bool ReadOpenBarrier (OpenBrData* openData);

int sqlite3OpenV2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs = NULL);
bool RunSQL(char SQL[]);
void SQLiteAddColumn(char* tblName,char* colName,char* Type);

void PopCommandDataQue(CMD_msg* CMD);
void PushCommandDataQue(CMD_msg CMD);
// Global variable
//extern struct ParkingConfig G_ParkingStatus;

#endif
