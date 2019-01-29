/* Tup500 Termal printer header file */

#ifndef ____TUP500____
#define ____TUP500____

#include "rs232.h"

enum STAR_FONT
{
	STAR_FONTA =0,
	STAR_FONTB = 1,
	STAR_FONTOCRB = 16
};

enum START_BCODETYPE
{
	STAR_BCODE_UPCE = 0,
	STAR_BCODE_UPCA = 1,
	STAR_BCODE_JPNEAN8 = 2,
	STAR_BCODE_JPNEAN13 = 3,
	STAR_BCODE_CODE39 = 4,
	STAR_BCODE_ITF = 5,
	STAR_BCODE_CODE128 = 6,
	STAR_BCODE_CODE93 = 7,
	STAR_BCODE_NW7 = 8	
};



class TUP500
{
public :
	TUP500(void);
	~TUP500(void);
	
	void  init(enum COMPORT port,char type=1);
	void  Close(void);
	void  Reset(void);
	void  ClearSetting(void);	
	void  FormFeed(void);
	void  SetHLine(int x1,int x2,int y,int width);
	void  SetString(void);
	void  ClearBitmap(void);
	void  SetPrintArea(int length = 700);
	void  SetBarCodePosition(int x, int y, int mode, int type, int rotation,int height);
	void  SetBarCodeData(char datas[]);
	void  SetCharactPosition(int x, int y, int width, int height, int type,int rotation);
	void  SetCharactData(int ID,char datas[]);
	void  SelectFont(enum STAR_FONT font);	
	void  SetArrowImage(int x,int y);
	void  IssuePaper(void);
	void  IssuePaper(int start,int len);
	void  SetPrintDensity(int n);
	void  SetCutter(void);
	void  Receive(void);
	bool  CheckOffline(void);
	bool  CheckPaperOnOutlet(void);
	bool  CheckPaperJam(void);
	bool  CheckPaperEnd(void);
	bool  CheckNearEnd(void);
	void  SetPrintImage(int x,int y, char FileName[]);	// 20121029 Tony add
	bool  CheckFE(void);	// 20130117 Tony add
	void  ClearBuff(void);	// 20130117 Tony add
	
private:
	
	RS232	comport;
	enum COMPORT my_port;
	int		BufferSize; // 20120221 Tony add
	char		m_Type;
	int		lineSn;
	int		barcodeSn;
	int		characterSn;
	// 20130117 Tony mark void 	ClearBuff(void);					//Frank add 20120223
	void 	TUP500WaitTime(unsigned int DataSize);	// 20121102 Tony add
};

#endif
