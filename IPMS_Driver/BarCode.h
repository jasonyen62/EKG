

#ifndef ____BARCODE____
#define ____BARCODE____

#include "../CommonDef.h"
#include "rs232.h"


#pragma pack(1) /// force alignment to 1 byte
typedef struct stInData
{
	char datas[21];
}InData;

typedef struct stPayData
{
	char datas[13];
}PayData;

typedef struct stBarCodeData
{
	InData  indata;
	PayData paydata1;
	PayData paydata2;
}BarCodeData;


#pragma pack()  /// set alignment back to default


class BarCode
{
public:
	BarCode(void);
	~BarCode(void);

	void  init(enum COMPORT port);
	void  Close(void);
	void  ReadStart();
	void  ReadEnd();
	bool  GetData(char* datas);
	void  ClearBuff(void);					//Frank add 20111116
	
private:
	
	RS232 comport;
	enum COMPORT my_port;
};

#endif
