
#ifndef ____ELTRA1000____
#define ____ELTRA1000____

#include "rs232.h"


class Eltra1000
{
public :
	Eltra1000(void);
	~Eltra1000(void);
	
	void init(enum COMPORT port);
	void ClearBuff(void);					//Frank add 20111116
	
private:
	RS232 comport;
	unsigned char ReaderID;
	//short  SendCommand(enum COMMAND cmd,short len,char* data);
	//bool  GetResponse(char* rData);
};

#endif
