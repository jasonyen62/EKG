/* LED888 車位數顥示器
	┌──────┬──────┬──────┬────────┬──────┐
	│ SOH  │  XX  │  STX │ YYYYY  │ ETX  │
	└──────┴──────┴──────┴────────┴──────┘
Ex:	 01 30 31 02 31 32 33 03
*/

#ifndef ____LED888____
#define ____LED888____

#include "rs232.h"

class LED888
{
public :
	LED888(void);
	~LED888(void);
	
	void  init(enum COMPORT port,char type=1);
	void  Close(void);
	void  Send(char ID,int Number);
	
private:
	
	RS232 comport;
	enum COMPORT my_port;
	char m_Type;
};

#endif
