/* rs232.h */

#include <termios.h> /* POSIX terminal control definitions */

#ifndef ____RS232H____
#define ____RS232H____

typedef unsigned char BYTE;
#define SOH    0x01
#define STX    0x02
#define ETX    0x03
#define EOT    0x04
#define ENQ    0x05
#define ACK    0x06
#define BS     0x08
#define HT     0x09
#define LF     0x0A
#define FF     0x0C
#define CR     0x0D
#define SO     0x0E
#define SI     0x0F
#define DLE    0x10
#define DC2    0x12
#define DC3    0x13
#define DC4    0x14
#define NAK    0x15
#define CAN    0x18
#define ESC    0x1B
#define US     0x1F
#define SPACE  0x20

/*
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
*/
extern enum COMPORT SerialPort[9];

class RS232
{
public:	
//	RS232(void){m_portHandle = NULL;};
	RS232(void){m_portHandle = -1;};
	~RS232(void);
	
	void PortClose(void);
	void out(BYTE c);
	void PortWrite(char *datas, int nbyte);
	void PortWriteForMF700(char *datas, int nbyte); // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
	void PortWriteForHF320(unsigned char* datas, int iLength); // nick add 20150112 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	void PortWriteWithCRC16(char *datas, int nbyte);
	void PortWriteWithBCC(char *datas, int nbyte);
	bool PortRead(int *nByte, char *data, int wait);
	bool PortRead2(int *nByte, char *data, int wait);
	int PortRead3(char *data, int Wait1Ms, int ReadMaxLen);					//Frank add 20111116
	bool PortReadForHF320(unsigned char* data, int iLength, int* iRecvDataLen, unsigned long timeout); // nick add 20150113 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	void Clear232Bufer(void);					//Frank add 20111116
	bool PortInit(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits);
	bool PortInit2(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits);
	bool PortInitForHF320(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits); // nick add 20150119 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	void SetRTS(bool On);
	bool GetCTS(void);
	bool GetDTR(void);
	
private:
	int GetCRC16(BYTE* puchMsg,  int usDataLen);	
	char Port[20];
	int m_portHandle;
	struct termios old_Options; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
};

#endif
