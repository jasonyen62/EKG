/* rs232.cpp
	
*/
#include <stdio.h>	/* Standard input/output definitions */
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <sys/ioctl.h>
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include "../CommonDef.h"
#include "rs232.h"
#include "../traceLog.h" // nick add 20150113 Ver:000-000-GIO_V2-13B251-0001-13C241 //

enum COMPORT SerialPort[9]={NONE,COM1,COM2,COM3,COM4,COM5,COM6,COM7,COM8};

/* Table of CRC values for high-order byte */ 
static BYTE auchCRCHi[] = { 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 
}; 
 
/* Table of CRC values for low-order byte */ 
static BYTE auchCRCLo[] = { 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
}; 

RS232::~RS232(void)
{
	PortClose();
}

void RS232::out(BYTE c)
{
 	//if(m_portHandle == NULL)
	if(m_portHandle < 0) // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
		return;
	
	int n;
	
	n = write(m_portHandle, &c, 1);
}

bool RS232::PortRead( int *bytes, char *data, int wait)
{	 // wait : msec
	int timeout;
	int nbyte=0;
	
	if(m_portHandle < 0) return false; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	usleep(10000);
	
	for (timeout = 0; timeout < wait/10; timeout++) 
	{	
		nbyte = read(m_portHandle, data,1024);
		
		if (nbyte > 1)
			break;
		usleep(10000);
	}
	
	*bytes = nbyte;
	
	if(nbyte <1)
		return false;
	return true;
}

bool RS232::PortRead2( int *bytes, char *data, int wait)
{	 // wait : msec  // For performance
	int timeout,retry=0;	
	char c;
	bool bAlreadyRecv = false;
	int TempLen;	 // 20120221 Tony add
	
	if(m_portHandle < 0) return false; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	if (*bytes > 1)					//Frank add s 20120511
		TempLen = (*bytes)-1; // 20120221 Tony add
	else
		TempLen = (*bytes);					//Frank add e 20120511
	
	*bytes = 0;
	//printf("Comport Handle Number:%d Recv:", m_portHandle);
	
	for (timeout = 0; timeout < wait; timeout++) 
	{
		while(read(m_portHandle, &c, 1) == 1)
		{
			data[(*bytes)] = c;
			//printf("%02X ", data[(*bytes)]);
			(*bytes)++;
			bAlreadyRecv = true;
			retry=0;

			// 20120221 Tony add s
			if (TempLen == (*bytes))
				goto FUNCEXIT;
			// 20120221 Tony add e

			usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
		}
		
		usleep(1000);
		
		if(bAlreadyRecv == true)
		{
			retry++;
			
			if(retry > 5)
				break;
		}
	}
	
	//printf("\n");
	
FUNCEXIT:
	
	if((*bytes) < 1)
		return false;
	//tcflush(m_portHandle, TCIOFLUSH);
	//usleep(3000);
	return true;
}

//bool RS232::PortRead3( int *bytes, char *data, int Wait1Ms, int ReadMaxLen)		//Frank add s 20111116
int RS232::PortRead3( char *data, int Wait1Ms, int ReadMaxLen)		//Frank add s 20111122
{
	// wait : msec
	int timeout;
	int nbyte=0;
//	char temp[256];

//	usleep(1000);

//	memset( temp, 0, 256);
	
	if(m_portHandle < 0) return false; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	for( timeout = 0; timeout < Wait1Ms; timeout++)
	{
		nbyte = read( m_portHandle, data, ReadMaxLen);
		//nbyte = read( m_portHandle, data, 255);

//Frank mark 20120508		if ( nbyte > 1 )
		if ( nbyte > 0 )					//Frank add 20120508
			break;
		usleep(1000);
	}
//	printf("\ntemp length=(%d)(%s)\n", nbyte, temp);
	//*bytes = 0;
	//*bytes = nbyte;
	//if(nbyte <1)
	//	return false;
	//return true;
	return nbyte;
}					//Frank add e 20111122

void RS232::Clear232Bufer(void)
{
	char buffer[1024];
	unsigned char aa;
	
	if(m_portHandle < 0) return; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	//
	for( aa = 0; aa < 10; aa++)
	{
		memset( buffer, 0, 1024);
		if( read(m_portHandle, buffer,1024) < 1 )
			break;
	}
}			//Frank add e 20111116

void RS232::PortWrite( char *chars ,int nByte)
{
	size_t s_write;
	
 	//if(m_portHandle == NULL)
	if(m_portHandle < 0)
		return;

//	tcflush(m_portHandle, TCIOFLUSH);
	tcflush(m_portHandle, TCOFLUSH);
	usleep(3000);
	
	//printf("Send m_portHandle(%d): ", m_portHandle);
	//
	//for(int i = 0; i < nByte; i++)
	//{
	//	printf("%02x ",chars[i]);	
	//}
	//
	//printf("\n");
	
	s_write = write(m_portHandle, chars, nByte);
	usleep(100000); // nick add 20131016 Ver:000-000-GIO_V2-133181-0101-135101 //
}

void RS232::PortWriteWithCRC16( char *chars ,int nByte) 
{
	int i;
	BYTE uchCRCHi = 0xFF ;						  /* high CRC byte initialized */ 
	BYTE uchCRCLo = 0xFF ;						  /* low CRC byte initialized */ 
	unsigned uIndex ; 										 /* will index into CRC lookup*/ 

	if(m_portHandle < 0) return; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	for(i=0; i<nByte; i++)
	{
		out(chars[i]);		
		uIndex = uchCRCHi ^ chars[i]; /* calculate the CRC */ 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex]; 
		uchCRCLo = auchCRCLo[uIndex]; 
	}
	
	out(uchCRCLo);	
	out(uchCRCHi);	
	usleep(100);
}

void RS232::PortWriteWithBCC( char *chars ,int nByte) 
{	//There bcc is xor STX to ETX
	int i;
	unsigned char bcc=0x00;
	
	if(m_portHandle < 0) return; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	//printf("Send:");
	for(i=0; i<nByte; i++)
	{
		out(chars[i]);
		bcc ^= chars[i];
		//printf("%02x ",(unsigned char)chars[i]);
	}	
	out(bcc);
	//printf("%02x \n",(unsigned char)bcc);
	usleep(100);
}

bool RS232::PortInit(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits)
{
	char device[40];
	struct termios options;
	
	if(port > 8) return false;
	
	memset(device,'\0',sizeof(device));
	sprintf(device,"/dev/ttyS%d",port);
	
	m_portHandle = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);//
	
	if (m_portHandle == -1) 
	{
		printf("open_port: Unable to open %s - ",device);
		return false;
	}
	else 
	{
		fcntl(m_portHandle, F_SETFL, 0);
	}
	
	// Get the current options for the port...	
	tcgetattr(m_portHandle, &options);
	memcpy(&old_Options, &options, sizeof(options)); // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	//bzero(&options, sizeof(options));
	//memset(&options, 0, sizeof(options));
	
	// Set the baud rates 
	switch(baudrate)
	{
		default:
		case 9600:
			cfsetospeed(&options, B9600);
			cfsetispeed(&options, B9600);
			break;
		case 19200:
			cfsetospeed(&options, B19200);
			cfsetispeed(&options, B19200);
			break;
		case 38400:
			cfsetospeed(&options, B38400);
			cfsetispeed(&options, B38400);
			break;
		case 115200:
			cfsetospeed(&options, B115200);
			cfsetispeed(&options, B115200);
			break;
		case 2400:
			cfsetospeed(&options, B2400);
			cfsetispeed(&options, B2400);
			break;
		case 1200:
			cfsetospeed(&options, B1200);
			cfsetispeed(&options, B1200);
			break;	
	}
	
	switch (parity)
	{
	default:
	case 'N':
		  options.c_cflag &= ~PARENB; 
		  break; 
	case 'O':
		  options.c_cflag |= PARENB;			
		  options.c_iflag |= (INPCK | ISTRIP); 
		options.c_cflag |= PARODD; 
		  break; 
	 case 'E':
		options.c_cflag |= PARENB; 
		options.c_iflag |= (INPCK | ISTRIP);		  
		  options.c_cflag &= ~PARODD; 
		  break; 	 
	}
	
	switch(databits)
	{
	default:
	case 8:
		options.c_cflag |= CS8;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	}
	
	options.c_cc[VTIME]= 0;// inter-char timer unused //read timeout
	options.c_cc[VMIN]= 0;// blocking read //wait read n char to return
	
	if( stopbits == 1 )
		options.c_cflag &=  ~CSTOPB;
	else
		options.c_cflag |=  CSTOPB;
	// Enable the receiver and set local mode...
	//options.c_cflag |= (CLOCAL | CREAD |CRTSCTS );
	//options.c_cflag &= ~CSIZE;
	
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);	/*Input*/
	options.c_oflag  &= ~OPOST;	/*Output*/
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	
	// Set the new options for the port...	
	usleep(20000);
	tcflush(m_portHandle, TCIOFLUSH);
	tcsetattr(m_portHandle, TCSANOW, &options);
	
	return true;
}

bool RS232::PortInit2(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits)
{
	char device[40];
	struct termios options;
	
    if(port>8) return false;
	memset(device,'\0',sizeof(device));
	sprintf(device,"/dev/ttyS%d",port);
	
	//m_portHandle = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);//
	m_portHandle = open(device, O_RDWR | O_NOCTTY | O_NDELAY);//
	
	if (m_portHandle == -1) 
	{
		printf("open_port: Unable to open %s - ",device);
		return false;
	}
	else 
	{
		fcntl(m_portHandle, F_SETFL, 0);
	}
	
	// Get the current options for the port...	
	if (tcgetattr(m_portHandle, &old_Options) != 0)
	{
		perror("SetupSerial 1"); 
		return false; 
	}
	
	bzero(&options, sizeof(options));
	
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE;
	
	// Set the baud rates 
	switch(baudrate)
	{
		default:
		case 9600:
			cfsetospeed(&options, B9600);
			cfsetispeed(&options, B9600);
			break;
		case 19200:
			cfsetospeed(&options, B19200);
			cfsetispeed(&options, B19200);
			break;
		case 38400:
			cfsetospeed(&options, B38400);
			cfsetispeed(&options, B38400);
			break;
		case 115200:
			cfsetospeed(&options, B115200);
			cfsetispeed(&options, B115200);
			break;
		case 2400:
			cfsetospeed(&options, B2400);
			cfsetispeed(&options, B2400);
			break;
		case 1200:
			cfsetospeed(&options, B1200);
			cfsetispeed(&options, B1200);
			break;	
	}
	
	switch (parity)
	{
		default:
		case 'N':
			options.c_cflag &= ~PARENB;
			break; 
		case 'O':
			options.c_cflag |= PARENB;        
			options.c_iflag |= (INPCK | ISTRIP); 
			options.c_cflag |= PARODD; 
			break; 
		case 'E':
			options.c_cflag |= PARENB; 
			options.c_iflag |= (INPCK | ISTRIP);        
			options.c_cflag &= ~PARODD; 
			break;     
	}
	
	switch(databits)
	{
	default:
	case 8:
		options.c_cflag |= CS8;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	}
	
	options.c_cc[VTIME]= 0;// inter-char timer unused //read timeout 
	options.c_cc[VMIN]= 0;// blocking read //wait read n char to return

	if( stopbits == 1 )
		options.c_cflag &= ~CSTOPB;
	else 
		options.c_cflag |=  CSTOPB;
	
	// Enable the receiver and set local mode...
	//options.c_cflag |= (CLOCAL | CREAD |CRTSCTS );
	//options.c_cflag &= ~CSIZE;
	
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/
	
	// Set the new options for the port...	
	//usleep(20000);
	tcflush(m_portHandle, TCIOFLUSH);
	
	if (tcsetattr(m_portHandle, TCSANOW, &options) != 0)
	{
		perror("com set error");
		return false;
	}
	
	usleep(20000);
	return true;
}

void RS232::PortClose()
{
	// ========================================================== //
	// nick mark s 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	//if (m_portHandle > 0)
	//{
	//	close(m_portHandle);
	//}
	// nick mark e 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================== //
	
	// ========================================================= //
	// nick add s 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	if (m_portHandle < 0)
		return;
	else
	{
		//printf("RS232::PortClose, m_portHandle = %d\n", m_portHandle);
		close(m_portHandle);
		//tcsetattr(m_portHandle, TCSANOW, &old_Options);
	}
	// nick add e 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	// ========================================================= //
}

int RS232::GetCRC16(BYTE* puchMsg, int usDataLen)
{
	BYTE uchCRCHi = 0xFF ;						  /* high CRC byte initialized */ 
	BYTE uchCRCLo = 0xFF ;						  /* low CRC byte initialized */ 
	unsigned uIndex ; 										 /* will index into CRC lookup*/ 
	/* table */ 
	while (usDataLen--)										  /* pass through message buffer */ 
	{ 
		uIndex = uchCRCHi ^ *puchMsg++; /* calculate the CRC */ 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex]; 
		uchCRCLo = auchCRCLo[uIndex];
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	} 
	return (uchCRCHi << 8 | uchCRCLo); 
}

void RS232::SetRTS(bool On)
{
	int status;
	
	if(m_portHandle < 0) return; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //	
	ioctl(m_portHandle, TIOCMGET, &status); /* get the serial port status */
	
	if ( On )		/* set the RTS line */
		status &= ~TIOCM_RTS;
	else
		status |= TIOCM_RTS;
		
	ioctl(m_portHandle, TIOCMSET, &status); /* set the serial port status */
}

bool RS232::GetCTS(void)
{
	int status;
	bool CTS_ON;
	
	if(m_portHandle < 0) return false; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	/* get the serial port's status */
	ioctl(m_portHandle, TIOCMGET, &status);
	CTS_ON = (status&TIOCM_CTS ? false:true);
	
	// Debug 用
	//if(CTS_ON)
	//	printf("CTS ON\n");
	//else
	//	printf("CTS  Off\n");
	//
	
	return (CTS_ON);
}

bool RS232::GetDTR(void)
{
	int status;
	bool DTR_ON;
	
	if(m_portHandle < 0) return false; // nick add 20140609 Ver:000-000-GIO_V2-135101-0004-13B251 //
	
	/* get the serial port's status */
	ioctl(m_portHandle, TIOCMGET, &status);
	DTR_ON = (status&TIOCM_DTR ? false:true);
	
	// Debug 用
	//if(DTR_ON)
	//	printf("DTR ON\n");
	//else
	//	printf("DTR Off\n");
	//
	
	return (DTR_ON);
}

void RS232::PortWriteForMF700( char *chars ,int nByte)
{
	size_t s_write;
	
	if(m_portHandle < 0)
		return;

	tcflush(m_portHandle, TCOFLUSH);
	usleep(3000);
	
	//printf("Send m_portHandle(%d): ", m_portHandle);
	//
	//for(int i = 0; i < nByte; i++)
	//{
	//	printf("%02x ",chars[i]);	
	//}
	//
	//printf("\n");
	
	s_write = write(m_portHandle, chars, nByte);
}

// ========================================================= //
// nick add s 20141218 Ver:000-000-GIO_V2-13B251-0001-13C241 //
bool RS232::PortInitForHF320(enum COMPORT port, int baudrate, unsigned char parity, int databits, int stopbits)
{
	char device[40];
	struct termios options;
	
	if(port > 8) return false;
	
	memset(device,'\0',sizeof(device));
	sprintf(device,"/dev/ttyS%d",port);
	
	m_portHandle = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);//
	
	if (m_portHandle == -1) 
	{
		printf("open_port: Unable to open %s - ",device);
		return false;
	}
	else 
	{
		fcntl(m_portHandle, F_SETFL, 0);
	}
	
	// Get the current options for the port...	
	tcgetattr(m_portHandle, &options);
	memcpy(&old_Options, &options, sizeof(options));
	
	// Set the baud rates 
	switch(baudrate)
	{
		default:
		case 9600:
			cfsetospeed(&options, B9600);
			cfsetispeed(&options, B9600);
			break;
		case 19200:
			cfsetospeed(&options, B19200);
			cfsetispeed(&options, B19200);
			break;
		case 38400:
			cfsetospeed(&options, B38400);
			cfsetispeed(&options, B38400);
			break;
		case 115200:
			cfsetospeed(&options, B115200);
			cfsetispeed(&options, B115200);
			break;
		case 2400:
			cfsetospeed(&options, B2400);
			cfsetispeed(&options, B2400);
			break;
		case 1200:
			cfsetospeed(&options, B1200);
			cfsetispeed(&options, B1200);
			break;	
	}
	
	switch (parity)
	{
		default:
		case 'N':
			  options.c_cflag &= ~PARENB; 
			  break; 
		case 'O':
			  options.c_cflag |= PARENB;			
			  options.c_iflag |= (INPCK | ISTRIP); 
			options.c_cflag |= PARODD; 
			  break; 
		 case 'E':
			options.c_cflag |= PARENB; 
			options.c_iflag |= (INPCK | ISTRIP);		  
			  options.c_cflag &= ~PARODD; 
			  break; 	 
	}
	
	switch(databits)
	{
		default:
		case 8:
			options.c_cflag |= CS8;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
	}
	
	options.c_cc[VTIME]= 0;// inter-char timer unused //read timeout 
	options.c_cc[VMIN]= 0;// blocking read //wait read n char to return
	
	if( stopbits == 1 )
		options.c_cflag &=  ~CSTOPB;
	else
		options.c_cflag |=  CSTOPB;
	// Enable the receiver and set local mode...
	//options.c_cflag |= (CLOCAL | CREAD |CRTSCTS );
	//options.c_cflag &= ~CSIZE;
	
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);	/*Input*/
	options.c_oflag  &= ~OPOST;	/*Output*/
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	
	// Set the new options for the port...	
	usleep(20000);
	tcflush(m_portHandle, TCIOFLUSH);
	tcsetattr(m_portHandle, TCSANOW, &options);
	
	return true;
}

bool RS232::PortReadForHF320(unsigned char* Data, int iDataLength, int* iRecvDataLen, unsigned long msTimeout)
{
	int nbyte = 0;
	int iRecvLoc = 0;
	int iHeaderLoc = -1;
	int iPackLen = 0;
	
	unsigned char tmpRecv[iDataLength];
	
	memset(Data, 0, iDataLength);
	memset(tmpRecv, 0, iDataLength);
	
	if (m_portHandle < 0) return (false);
	
	unsigned long sTick = GetTickCount();
	
	while (CheckTimeout(&sTick, msTimeout) == false)
	{
		nbyte = read(m_portHandle, tmpRecv + iRecvLoc, 1);
		
		if (nbyte > 0)
		{
			iRecvLoc += nbyte;
			
			if (iHeaderLoc == -1)
			{
				// 檢查是否收到Packet header
				for (int i = 0; i < nbyte; i++)
				{
					if (tmpRecv[i] == 0xAE)
					{
						iHeaderLoc = i;
						break;
					}
				}
				//
			}
			else
			{
				// 擷取Packet length
				if (iPackLen == 0)
				{
					if (iRecvLoc > iHeaderLoc + 2)
					{
						iPackLen = (tmpRecv[iHeaderLoc + 1] * 0x100) + tmpRecv[iHeaderLoc + 2];
						*iRecvDataLen = iPackLen + 4;
					}
				}
				//
			}
			
			if ((iPackLen > 0 && iRecvLoc >= *iRecvDataLen) || iRecvLoc >= iDataLength)
				break;
		}
		
		usleep(50);
	}
	
	// Debug 用
	char msg[1024];
	char msg2[512];
	
	memset(msg, 0, sizeof(msg));
	memset(msg2, 0, sizeof(msg2));
	
	for (nbyte = 0; nbyte < iRecvLoc; nbyte++)
		sprintf(msg2 + (3 * nbyte), "%02X ", tmpRecv[nbyte]);
	
	sprintf(msg, "RS232::PortReadForHF320() -> RecvLen:[%d], RecvData:[%s]", iRecvLoc, msg2);
	ShowMessage(msg, 5);
	//
	
	memcpy(Data, tmpRecv + iHeaderLoc, *iRecvDataLen);
	return (iRecvLoc > 0);
}

void RS232::PortWriteForHF320(unsigned char* datas, int iLength)
{
	size_t s_write;
	
	if(m_portHandle < 0)
		return;
	
	tcflush(m_portHandle, TCOFLUSH);
	
	// Debug 用
	char msg[1024];
	char msg2[512];
	
	memset(msg, 0, sizeof(msg));
	memset(msg2, 0, sizeof(msg2));
	
	for (int nbyte = 0; nbyte < iLength; nbyte++)
		sprintf(msg2 + (3 * nbyte), "%02X ", *(datas + nbyte));
	
	sprintf(msg, "RS232::PortWriteForHF320() -> SendLen:[%d], SendData:[%s]", iLength, msg2);
	ShowMessage(msg, 5);
	//
	
	s_write = write(m_portHandle, datas, iLength);
}
// nick add e 20150112 Ver:000-000-GIO_V2-13B251-0001-13C241 //
// ========================================================= //