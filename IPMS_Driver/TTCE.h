/* * 天騰 發卡,收卡機控制程式  */
#ifndef ____TTCE____
#define ____TTCE____

#include "rs232.h"

enum D1000_DISPENSE
{
	D1000_DISPENSE_TO_OUT  =0,  // 不會咬住的位置 再送發卡指令會再出一張
	D1000_DISPENSE_TO_TAKE = 4, // 取卡 會咬住
	D1000_DISPENSE_TO_READER=7  // 讀卡機
};

enum D3000_ENABLE
{
	D3000_ENABLE = 0,
	D3000_DISABLE = 1
};

class TTCE
{
public :
	TTCE(void);
	TTCE(bool isInMachine)
	{
		m_bIsInMachine = isInMachine;
	};
	~TTCE(void);
	
	void init(enum COMPORT port);
	void  Close(void);
	
	void Reset(short ID);
	bool GetStatus(short ID,char *status);	
	bool RetrieveCard(short ID);
	bool IsCardInMachine(short ID);	
		
	// in 
	bool DispenseCard(short ID,enum D1000_DISPENSE place);
	bool DispenseCardOut(short ID);
	// out
	bool RejectCard(short ID);
	void EnableWork(short ID,enum D3000_ENABLE bEnable);
	
private:
	
	RS232 comport;
	enum COMPORT my_port;
	bool m_bIsInMachine;
	short SendCommand(short ID,char command[],char* parm);
	bool  GetResponse(char* rData);
	void  DoCommand(short ID);
	bool  GetACK(short ID);
};

#endif
