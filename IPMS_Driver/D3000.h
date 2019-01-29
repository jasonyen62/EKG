/* 天騰 D3000發卡機 */
#ifndef ____D3000____
#define ____D3000____

#include "rs232.h"

enum D3000_ENABLE
{
	D3000_ENABLE = 0,
	D3000_DISABLE = 1
};

class D3000
{
public :
	D3000(void);
	~D3000(void);
	
	void 	init(enum COMPORT port);
	void 	Reset(int ID);
	bool 	SetAddress(int ID,int newID);
	int		GetMachineID(int ID);
	bool 	GetStatus(int ID,char *status);
	bool 	RejectCard(int ID);
	void 	EnableWork(int ID,enum D3000_ENABLE bEnable);
	bool 	RetrieveCard(int ID);
	bool 	IsCardInMachine(int ID);
	bool 	IsCardIn(int ID);
	void 	Close(void);
	
private:

	RS232	comport;
	enum COMPORT my_port;
	int		BufferSize;   // 20120221 Tony add
	bool		bMalfunction;
	int		SendCommand(int ID,char command[],char* parm);
	bool		GetResponse(char* rData);
	void		DoCommand(int ID);
	bool		GetACK(int ID);
	void		ClearBuff(void);    // 20120221 Tony add
};

#endif
