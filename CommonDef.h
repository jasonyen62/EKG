/* FileName : CommonDef.h 

*/

#ifndef ____COMMONDATA____
#define ____COMMONDATA____

#include <queue>
#include <pthread.h> // nick add 20130202 //

using namespace std;

//Alarm Message ID
/*
   AM_CaseOpen = 1              '機體被打開
   AM_NoTicket = 2              '票卡用盡
   AM_LowTicket = 3             '票卡數量不足
   AM_NoAction = 4              '超過時間未動作
   AM_TicketNotTake = 5         '逾時未取回票卡
   AM_BarrierNoAction = 6       '柵欄無作用
   AM_NotTake = 7               '有車未取回票卡
   AM_FullReTicket = 8          '票卡回收箱已滿
   AM_CoinNotEnought = 9        '找錢箱硬幣存量不足
   AM_FullReCoin = 10           '硬幣回收箱已滿
   AM_FullReBanknote = 11       '紙鈔回收箱已滿
   AM_LessReceipt = 12          '收據紙不足
   AM_LessInvoice = 13          '發票不足
   AM_NoReadCard = 14           '讀卡機無作用
   AM_Cashopen = 15             '錢櫃打開
   AM_Hinder = 16               '有障礙物,柵欄無法關閉
   AM_Monunsuited = 17          '月票車車牌號碼不符
   AM_NotDifferentiate = 18     '車牌無法辨識
   AM_Repeatin = 19             '月票車企圖重覆進場
   AM_Null = 20                 '月票卡無效
   AM_NotLeave = 21             '票卡已回收,車輛未離場
   AM_UseBlack = 22             '有使用黑名單票卡車輛
   AM_RepeatOut = 23            '月票車企圖重覆出場
   AM_TicketOver = 24           '月票卡過期
   AM_Decied = 25               '票卡無法判辨
   AM_Unsuited = 26             '車牌號碼不符
   AM_Steal = 27                '發現贓車
   AM_OverBuffer = 28           '車輛超過出場緩衝時間
   AM_Move = 29                 '機體遭受移動
   AM_NotGiveChange = 30        '無法找錢
   AM_Strike = 31               '機體遭受撞擊
   AM_NoAvail = 32              '票卡無效
   AM_Back = 33                 '車輛倒車
   AM_UseError = 34             '使用者登入錯誤
   AM_NotUser = 35              '無使用者登入
   AM_BanknoteBoxMove = 36      '紙鈔回收箱異常取出
   AM_Dispenserfail = 37        '紙鈔機故障
   AM_TicketJam = 38            '讀卡機卡票
   AM_InvoicePrinterError = 39  '統一發票機故障
   AM_BCDPrinterError = 40      '收據印表機故障
   AM_DeviceError = 41          '入口驗票機故障
   AM_ExitDeviceError = 42      '出口驗票機故障
   AM_ExitDoorOpen = 43         '出口機體被打開
   AM_FeeConvert = 44           '費率轉換
   AM_DataBaseLock = 45         '資料庫被使用者開啟
   AM_BarStrike = 47            '振臂遭受撞擊     '2.09.16
   AM_BlackDisctTick = 48       '使用非法折扣卷 'Nick add 3.0
   AM_EDisctTickJam = 49        '折扣券卡票 'Nick add 3.0f
   AM_TurnOverError = 50        '週轉金數量錯誤 'Nick add 3.1.1
   AM_DisctTicketOutDate = 51   '折扣券過期 'Nick add 3.1.2c
   AM_DisctTicketInvalid = 52   '折扣券無效 'Nick add 3.1.2c
   AM_DeviceNotConnect = 53     '裝置不連線 'Nick add 3.1.7
   AM_InvoiceLowmun = 54        '統一發票用盡 'Nick add 3.1.8
   AM_NoPay = 55                '車主尚未繳費 'Nick add 3.2h
   AM_OutOfPayTime = 56         '超過出場緩衝時間 'Nick add 3.2h
   AM_LaneClose = 57            '車道封閉 'Nick add 3.2h
   AM_ValueNotEnough = 58       '儲值餘額不足 'Nick add 3.2h
   AM_OverMaxPay = 59           '繳費金額過大 'Nick add 3.2h
   AM_ParkingIDErr = 60         '使用他場票卡 'Nick add 3.2h
   AM_WriteErr = 61             '票卡寫入失敗 'Nick add 3.2h
   AM_ObjectPause = 62          '設備暫停使用 'Nick add 3.2h
   AM_NoChange = 63             '零錢不足未找錢 'Nick add 3.2h
   AM_NotEnoughDisctHour = 64   '預購小時數即將用盡 'Nick add 3.1.15
   AM_FindAlarmPlate = 65       '發現警告車牌號碼 'Nick add 3.1.20
   AM_HopperAbout2Full = 66     '硬幣數量即將達到最大量 'Nick add 4.0
 */
enum COMPORT
{
	COM1=0,
	COM2,
	COM3,
	COM4,
	COM5,
	COM6,
	COM7,
	COM8,
	NONE = 99
};

enum ControlCommand
{
	CMD_NONE = 0,
	CMD_IN_SERVICE = 1,
	CMD_STOPSERVICE = 2,
	CMD_OPENBR = 3,
	CMD_CLOSEBR = 4,
	CMD_RESET = 5,
	CMD_FULL = 6,
	CMD_NOFULL = 7,
	CMD_FREE = 8,
	CMD_ThirdParty_Ticket = 9,
	CMD_Play_Voice = 10,
	CMD_NOTFULL = 200,
	CMD_HOURLYFULL = 201,
	CMD_ALLFULL = 202,
	//自訂(Server沒有定義)
	CMD_CLEAR_RETICKET = 900,
	CMD_ADDTICKET = 901,
	CMD_CONNECT = 999 ,
	CMD_DISCONNECT = 998    
};

#pragma pack(1) /// force alignment to 1 byte

typedef struct HourlyTicketData
{
	unsigned long			TicketID;
	unsigned long			in_time;
	char					Plate[10];
	bool					InStatus;
	unsigned long			out_time;
	unsigned long			pay_time;
//nick mark 20111219	unsigned int			Amount;
	double				Amount;					//nick add 20111219
	int					Disct_Type;				//第一筆折扣 id   //nick add 20111219
	int                  			payment_subject;     //繳費型態   : 1.STOP_PAYMENT 2.SELL 3.LOST 4.RENEWAL 5.RELOAD 6.REFUND 7.DAMAGED 8.OTHER
	int					Pay_Type;				//付費型式	 : 1.CASH 2.CHECK 3.CREDIT_CARD 4.DEBIT_CARD 5.BONUS 6.VALUE_CARD 7.CUSTOMER_VALUE_CARD 8.NOT_PAYED 9.CREDIT_RECEIPT 10.VALUE_CASH
	unsigned long			Deduct_Value;			//扣除金額   //nick add 20111219
	unsigned long			Remaining_Value;		//剩餘金額   //nick add 20111221
	int						AreaID; // 區域ID // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
	char					TagID[24];			// tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
}HTicketData;

typedef struct HourlyDisountData_st
{
	unsigned long			TicketID;
	unsigned long			DiscountTime;
	int					AID;
	int					VID;
	int					SID;
	int					DiscountMinute;
	int					Type;  //limit:45, hour:46, %:47
	int					RealDiscountMinute;
	bool					bolIsDisct;		//此折扣是否有效 //nick add 20111219
	unsigned long			Point;					//Frank add 20130114
}HDiscountData;

typedef struct strDiscounts
{
	unsigned char			AID;
	unsigned char			SID;
	unsigned char			VID;
	unsigned char			DiscountHour; // half hour or 1hour
}Discounts;

//Frank add s 20120912
typedef struct strDiscountsv2
{
	unsigned char 		AID;
	unsigned char 		SID;
	unsigned char 		VID;
	unsigned char 		DiscountHour[4];
}Discountsv2;
//Frank add e 20120912

// Tony add 20170307
typedef struct ThirdPartyTicketData
{
	char			AuthID[19];
	char			AuthType[3];
	unsigned long 	TicketID;
	char			Plate[40];
	char			TagID[25];
	time_t       ProcessDateTime;
	unsigned int	AreaID;
	bool			IssueTicket;
}ThirdPartyTicketData;


typedef struct TicketData
{    //票卡資料    	
	unsigned char			TicketVer;	// 20110510 Tony add
	unsigned char			NextSector;	// 20110510 Tony add 
	unsigned int			parkingId;
	unsigned char			areaId;
	unsigned char			langId;
	unsigned long			ticketID; 
	//Frank mark 20120814 char					plate[10];
	char					plate[40];	//預留3組車牌					//Frank add 20120814
	unsigned char			status;
	unsigned int			in_year;
	unsigned char			in_month;
	unsigned char			in_day;
	unsigned char			in_hour;
	unsigned char			in_min;
	unsigned char			in_sec;	// 20110506 Tony add
	unsigned long			value;         //儲值金額
	unsigned long			seasonVersion; //月票版本 
	unsigned int			out_year;		//儲值卡的最後儲值時間 年
	unsigned char			out_month;		//儲值卡的最後儲值時間 月
	unsigned char			out_day;		//儲值卡的最後儲值時間 日
	unsigned char			out_hour;		//儲值卡的最後儲值時間 時 
	unsigned char			out_min;		//儲值卡的最後儲值時間 分
	unsigned char			out_sec;		//儲值卡的最後儲值時間 秒
	char					optime[3];
	unsigned long			staytime;	// 20110506 Tony add
	unsigned char			staytimetype;	// 20110516 Tony add
	char					DisctSector;	// 20110510 Tony add
	//Discount data
	unsigned int			discount_year;
	unsigned char			discount_month;
	unsigned char			discount_day;
	unsigned char			discount_hour;
	unsigned char			discount_min;
	unsigned char			discount_sec;					//Frank add 20120914
	bool					LimitDiscount;
	unsigned char			LHDisctCnt;					//Frank add 20120910
	unsigned char			LimitAID;
	unsigned char			LimitSID;
	unsigned char			LimitVID;
	unsigned int			LimitYear;
	unsigned char			LimitMon;
	unsigned char			LimitDay;
	unsigned char			LimitHour; // limit to HH:00  free out
	unsigned char			DiscountCount;
	Discounts				discounts[8];
	Discountsv2			Discounts2[16];					//Frank add 20120912
	char					TagID[24];
}TicketData;

struct stTimeOut					//Frank add s 20111020
{
	unsigned int			Button_Timeout;
	unsigned int			TakeTicket_Timeout;
//Frank mark 20120903	unsigned int			WaitLoop2_Timeout;
	unsigned int			CloseBarrier_Timeout;
	unsigned int			WaitCardIn_Timeout;
};						//Frank add e 20111020


typedef struct st_IOSycn
{
	bool SwitchOn;
	bool BeSync;
	int SyncValue;
}st_IOSycn;

typedef struct st_ParkingConfig
{    //停車場設定
	int					UseMQ;
	char					ParkingName[40];
	unsigned int			ParkingID;
	char					ServerIP[16];
	int					ServerPort;
	char					PlateServerIP[16];
	int					PlateServerPort;
	char					ImageServerIP[16];	// nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
	int					ImageServerPort;		// nick add 20150525 Ver:000-000-GIO_V2-13B251-0005-13C241 merge from [000-000-GIO_V2-Smart_Traffic-13C241-0001-155061] //
	int					PlateTimeout;        // 車牌辨識timeout時間	
	char					NewTerminalIP[16];	// Tony add 20170616 
	int						NewTerminalPort;		// Tony add 20170616
	int					LocalPort;		// Tony add 20170616
	//Frank add s 20120907
	struct stTimeOut		WaitTime;		//未動作時間			//Frank add 20111020
	bool					VISOpenBarrier;
	bool					VISforHourly;
	bool					VISforSeason;
	//Frank add e 20120907
	unsigned long			MaxDiscountHours;					//最大折扣時數					//Frank add 20130123
	
	//機器設定
	char					MachineID;
	bool					AutoIssueTicket;         //自動發卡
	bool					PreTicket;               //預發卡
	enum  COMPORT		MifareDispenserPort;
	enum  COMPORT		LED888port;
	char					LED888Type;                   //0:none ,1:盈德888 (3,4位數),2:盈德888(6位數)
	bool					bReadTag;
	int					Loop2Timeout;
	char					SeasonReadTag;                // 月票讀Tag  1:Yes  0:No
	unsigned int			BinsSize;            // 回收箱容量
	unsigned long			ClientID;            //communication id
	unsigned long			PlateClientID;       //communication id
	unsigned int			RecvPort;            // =>if communication ID = 11223333H  port = 11H * 100 + 22H + 3333H = 14841
	
	//場區相關設定
	unsigned int			AreaID;
	unsigned int			UpLayerID;		//設備上設定的上層ID	//20110505 Tony add
	unsigned int			UpLayerFreeTime;		//設備上設定的上層入場免費時間 	//20110520 Tony add 
	unsigned int			ParkingSpace;  //Area space
	unsigned int			FreeTime;      //min
	unsigned int			FeeType;       //0: normal
	unsigned int			Fee;
	unsigned int			FeeTime; 		//0:計次 1: 1小時計費一次 2:半小時計費一次 
	unsigned int			PaidTime;
	unsigned int			deviation;       // 幾分鐘以上以一小時計
	unsigned char			ParkingFullCanEntry;
	unsigned int			QuietTime;
	unsigned int			RFCheckPlate;     //RFin 要不要檢查車牌 0:不檢查 1:要檢查
	//Frank mark 20120907 struct stTimeOut		WaitTime;		//未動作時間			//Frank add 20111020
	//Frank add s 20120907
	unsigned int			Hourly_Use;
	unsigned int			Season_Use;
	//Frank add e 20120907
	unsigned int			MoneyRate;					//Frank add 20120919
	
	//Barcode
	int					BarCodeType;
	char					BarCodeDetail[16];  //註1:palsdtn
	char					DateFormat[8];
	char					DateTimeFormat[25]; // nick add 20130909 Ver:000-000-GIO_V2-133181-0100-135101
	char					Note[80];
	//Language
	char					ParkLanguage[10];	//設定圖檔和語音存放位置	//nick add 20110705
	// Time
	char					MaintainTime[6]; // nick add 20160412 Ver:000-000-GIO_V2-13B251-0010-13C241 //
}ParkingConfig;

typedef struct strParkingStatus
{
	unsigned int			Out_Retrieved;    // 出口,回收箱數量
	unsigned int			TicketNumber;     // 票號(下一張要發的序號);  
	unsigned int			TicketReserve;    //票卡存量
	unsigned int			status;    // 狀態 Status define 
}parkingStatus;

typedef struct ServerCommand
{
	enum ControlCommand		Command;
	char					datas[512];
}serverCommand;

typedef struct ServerACK
{
	unsigned long			num;
}serverAck;

typedef struct stOpenBarrierData
{
	unsigned long			sn;
	unsigned long			OpenTime;
}OpenBrData;

typedef struct Madia_msg_st
{
	long					msg_type; //0:Show bmp  1:sound  2:mpeg
	char					fileName[BUFSIZ];   // index of array 0~29
}CMD_msg;

// ========================================================= //
// nick add s 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
typedef struct ID_Card_Status_st
{
	unsigned long TicketID;
	time_t Last_Entry_Exit_Time;
	int Status;
}IDCardStatus;
// nick add e 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
// ========================================================= //

typedef struct Fix_Config_st
{
	char	ch_Allow_Area[10];
}FixConfig;

struct cmd_msg_st
{
	long int				msg_type;
	char					fileName[80];
};

struct stSeasonConfig	//20110212 Nick add
{
	int					Type;
	char					Mon_Start[6];
	char					Mon_End[6];
	char					Tue_Start[6];
	char					Tue_End[6];
	char					Wed_Start[6];
	char					Wed_End[6];
	char					Thu_Start[6];
	char					Thu_End[6];
	char					Fri_Start[6];
	char					Fri_End[6];
	char					Sat_Start[6];
	char					Sat_End[6];
	char					Sun_Start[6];
	char					Sun_End[6];
	char					ChargeDate[8];
};

//註1: 
/*
p(2) : 'Parking ID
a(1) : 'Area ID
l(0) : 'Language ID
s(1) : 'status
xd(4) : 'Date  ->yyyymmdd
xt(4) : 'Time  ->hhMM
n(8) : 'TicketID
*/
#pragma pack()  /// set alignment back to default

/* ==== Status define ==== */
#define STATUS_TCP_CONNECT      0x0001
#define STATUS_PARKING_FREE     0x0002 /* Parking Free is need take ticket , no Fee */
#define STATUS_GATE_OPEN        0x0004 /* Gate Open is no need take ticket */
#define STATUS_GATE_ON_SERVICE  0x0008
#define STATUS_BARRIER_DOWN     0x0010
#define STATUS_MACHINE_OPEN     0x0020
#define STATUS_TICKET_LOW       0x0040
#define STATUS_STRIKE           0x0080
#define STATUS_READER_CONNECT   0x0100
#define STATUS_HOPPER_SELECT    0x0200
#define STATUS_TICKET_EMPTY     0x0400
#define STATUS_BARRIER_STRIKE	0x0800 //振臂受撞擊 // nick add 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //

// Version Define //
#define Software_Ver			"000-000-GIO_V2-13C241-0006-182141 (2-Output)"
#define UDPRecvBuffSize		1024					//Frank add 20111020
#define SQLLength			1024		//nick add 20111219

//Global variable
extern parkingStatus G_ParkingStatus;
extern bool G_bErrHold;
extern char G_ErrorMessage[128];
extern queue <serverCommand> G_SvrCmdQue;
extern queue <serverAck> G_SvrAckQue;
extern queue <serverAck> G_NewClientAckQue;
extern queue <IDCardStatus> G_SvrIDCardStatusQue; // nick add 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
extern ParkingConfig G_ParkingConfig;
extern FixConfig G_FixConfigSetting; // nick add 20150813 Ver:000-000-GIO_V2-13B251-0005-13C241 //
extern bool volatile b_GServerConnect;
extern bool volatile G_bMaintain; //nick add 20130121 //
extern bool volatile G_RFin; // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
extern bool volatile G_POSIN; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
extern bool IsINMachine;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //extern long CalcTmpTime;  // 20120222 Tony add
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //extern long ClsoeBarrierChkTime;	// 20120920 Tony add
extern unsigned long CalcTmpTime; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
extern unsigned long ClsoeBarrierChkTime; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
extern pthread_mutex_t Data_mutex; // nick add 20130202 //
extern pthread_mutex_t Log_mutex; // nick add 20130318 //
extern pthread_mutex_t NewTerminal_mutex;
extern TicketData LastTicketData; // nick add 20130222 //
extern char G_ReadType;	//20130904 KARATE add
extern unsigned long G_MemBlocksWriteSuc[16]; // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
extern bool G_NoTicketSystem;
#endif
