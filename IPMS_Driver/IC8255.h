/* I/O Extend IC 8255 Control Class */

#ifndef ____IPMS_DRIVER_IC8255____
#define ____IPMS_DRIVER_IC8255____

#include <string.h>
const unsigned char Bits[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

enum PORT8255
{
	IC8255_PORTA=0,
	IC8255_PORTB=1,
	IC8255_PORTC=2,
	IC8255_CONTROL=3
};

enum BIT8255
{
	BIT0 =0,
	BIT1 =1,
	BIT2 =2,
	BIT3 =3,
	BIT4 =4,
	BIT5 =5,
	BIT6 =6,
	BIT7 =7
};

class IC8255
{
public:
	IC8255(void)
	{
		memset(m_OuputSignal,0xFF,sizeof(m_OuputSignal));
	};
	~IC8255(void){};
	void Init8255(unsigned short addr,bool AInput,bool BInput,bool CHInput,bool CLInput);
	void Out8255Bit(enum PORT8255 port,enum BIT8255 bit,bool On,bool bSavelog=false);
	void Out8255Byte(enum PORT8255 port,char data);
	void Out8255HByte(enum PORT8255 port,char data);
	void Out8255LByte(enum PORT8255 port,char data);
	bool Get8255Bit(enum PORT8255 port,enum BIT8255 bit ,bool bSavelog=false);
	void SetPortCBit(short bit,bool On);
	unsigned char Get8255Byte(enum PORT8255 port);
private:
	void SaveLog( char msg[]);
	unsigned char m_OuputSignal[3];
	unsigned short m_PortAddr;
};

#endif