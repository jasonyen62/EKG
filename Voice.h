/* Voice Function */

#ifndef ____IPMS_VOICE____
#define ____IPMS_VOICE____

enum VOICEDEF
{
	VOICE_NONE =0,
	VOICE_WELCOME=1,    // 歡迎光臨,請按鈕取票
	VOICE_TAKETICKET=2, // 請取回票卡
	VOICE_PLSENTER=3,   // 請入場
	VOICE_PFULL =4,     // 車位己滿
	VOICE_FAULT=5,      // 設備故障,
	VOICE_INVTICKET=6,  // 票卡無效
	VOICE_PROCESSING=7, // 票卡處理中
	VOICE_PLSEXIT=8,    // 請出場
	VOICE_BRCLOSE=9,    // 柵欄即將關閉
	VOICE_INSERT =10,   // 請插入票卡
	VOICE_READERROR=11, // 票卡無法讀取,請確認方向
	VOICE_PLSWAIT=12    //12.請稍候    
};

void VoiceOn(enum VOICEDEF index);

enum AUDIODEF
{
	AUDIO_BEEP2       = 0,
	AUDIO_INSERT      = 5,
	AUDIO_TAKETICKET  = 6,
	AUDIO_SEASONEXPIRE=	8,
	AUDIO_REENTER     =	9,
	AUDIO_REEXIT	  =	9,
	AUDIO_INVTICKET   = 10,
	AUDIO_PLSENTER    =	12,
	AUDIO_FAULT       =	14,
	AUDIO_PFULL       = 15,
	AUDIO_VALUE_LACK  = 16,
	AUDIO_NOPAY       = 18,
	AUDIO_READERROR   = 20,
	AUDIO_PLSEXIT	  = 22,
	AUDIO_AREAERR     = 29,
	AUDIO_STOPSERVICE = 35,
	AUDIO_WELCOME     = 38,
	AUDIO_PROCESSING  = 40,
	AUDIO_PLATEERROR  =	42,
	AUDIO_BRCLOSE	= 44,					//Frank add 20120903
	AUDIO_HOURLYFULL = 77, // nick add 20140122 Ver:000-000-GIO_V2-135101-0000-13B250 //
	AUDIO_PLATEPROCESS= 79,
	AUDIO_SEASON_W_SUC = 9999		//nick add 20120420
};
/*
#define AUDIO_INSERT        5
#define AUDIO_TAKETICKET  	6
#define AUDIO_SEASONEXPIRE 	8
#define AUDIO_REENTER   	9
#define AUDIO_REEXIT	  	9
#define AUDIO_PLSENTER  	12
#define AUDIO_FAULT     	14
#define AUDIO_PFULL         15
#define AUDIO_VALUE_LACK    16
#define AUDIO_READERROR     20
#define AUDIO_PLSEXIT	  	22
#define AUDIO_AREAERR       29
#define AUDIO_STOPSERVICE  	35
#define AUDIO_WELCOME       38
#define AUDIO_PROCESSING    40
#define AUDIO_PLATEERROR  	42
#define AUDIO_PLATEPROCESS  79
*/
void AudioOn(enum AUDIODEF index);
void AudioOut(char filename[]);
#endif