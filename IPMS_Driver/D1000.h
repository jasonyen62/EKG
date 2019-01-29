/* 天騰 D1000發卡機 */
#ifndef ____D1000____
#define ____D1000____

#include "rs232.h"

enum D1000_DISPENSE
{
	D1000_DISPENSE_TO_OUT  =  0, // 不會咬住的位置 再送發卡指令會再出一張
	D1000_DISPENSE_TO_TAKE =  4, // 取卡 會咬住
	ACT_F1_DISPENSE_TO_READER=6, // ACT_F1讀卡機
	D1000_DISPENSE_TO_READER= 7  // D1000讀卡機
};

class D1000
{
public :
	D1000(void);
	~D1000(void);
	
	// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //void	init(enum COMPORT port);
	void	init(enum COMPORT port, int Dispense = 1); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	//Frank mark 20121102 void	Reset(int ID);
	bool Reset(int ID);					//Frank add 20121102
	char	GetVer(int iID);		//20130904 KARATE add
	char    GetVerResponse(void);		//20130904 KARATE add
	char	GetStatus(int ID,char *status);
	char	DispenseCard(int ID,enum D1000_DISPENSE place);
	char	DispenseCardOut(int ID);
	char	RetrieveCard(int ID);
	bool	IsCardInMachine(int ID);
	bool	IsCardIn(int ID);
	bool	IsEmpty(int ID);
	void	Close(void);
	void	ClearBuff(void);   // 20120221 Tony add
	char GetStatusD2(int ID,char *status);					//Frank add 20120821
	
private:
	
	RS232	comport;
	enum COMPORT my_port;
	int		BufferSize;   // 20120221 Tony add	
	int		SendCommand(int ID,char command[],char* parm);
	bool		GetResponse(char* rData);
	void		DoCommand(int ID);
	char		GetACK(int ID);
};

#endif
