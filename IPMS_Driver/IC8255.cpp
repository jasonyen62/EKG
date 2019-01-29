#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h> 
#include <time.h>
#include <string.h>	/* String function definitions */
#include <unistd.h>
#include <sys/ioctl.h>

//#include <ioport.h>
#include "IC8255.h"

#define outp(a, b)  outb(b, a) 
#define inp(a) 	  inb(a) 

void IC8255::Init8255(unsigned short addr,bool AInput,bool BInput,bool CHInput,bool CLInput)
{	//Only 8255 Mode 0
	unsigned char Control8255 = 0x80;
	
	m_PortAddr = addr;
	
	if(AInput)
	{
		Control8255 |= Bits[4];
	}
	if(BInput)
	{
		Control8255 |= Bits[1];
	}
	if(CHInput)
	{	
		Control8255 |= Bits[3];
	}
	if(CLInput)
	{
		Control8255 |= Bits[0];
	}
	
	ioperm(addr,4,1);
	//printf("8255Init:%02x\n",Control8255);

	outp(m_PortAddr + IC8255_CONTROL,Control8255);
	
	//usleep(10000L);
	//Set default output signal to OFF
	if(AInput == false)
	{
		outp(m_PortAddr + IC8255_PORTA,0xFF);
	}
	if(BInput == false)
	{
		outp(m_PortAddr + IC8255_PORTB,0xFF);
	}
	if(CLInput == false)
	{
		outp(m_PortAddr + IC8255_PORTC,0xFF);
	}
	
	//outp(ADDR8255_1+3,0x92);  // Mode 0, PortA,PortB => input  PortC => output
	//outp(ADDR8255_2+3,0x80);  // Mode 0, PortA,PortB,PortC => output
}

void IC8255::Out8255Bit(enum PORT8255 port,enum BIT8255 bit,bool On,bool bSavelog)
{
	char buf[256];
	
	memset(buf,'\0',sizeof(buf));
	if(On == false)
	{
		m_OuputSignal[port] |= Bits[bit];
	}
	else
	{
		m_OuputSignal[port] &= (~Bits[bit]);
	}

	outp(m_PortAddr+port,m_OuputSignal[port]);
	
	if(bSavelog == true)
	{
		if(On)
		{
			sprintf(buf,"Out Port%d Bit:%d on  [%02X]",port,bit,m_OuputSignal[port]);
		}
		else
		{
			sprintf(buf,"Out Port%d Bit:%d off [%02X]",port,bit,m_OuputSignal[port]);
		}
		SaveLog(buf);
	}	
}

void IC8255::Out8255HByte(enum PORT8255 port,char data)
{
	m_OuputSignal[port] &= 0x0F;
	data &= 0xF0;
	m_OuputSignal[port] |= data;
	outp(m_PortAddr + port,m_OuputSignal[port]);
}

void IC8255::Out8255LByte(enum PORT8255 port,char data)
{
	m_OuputSignal[port] &= 0xF0;
	data &= 0x0F;
	m_OuputSignal[port] |= data;
	outp(m_PortAddr + port,m_OuputSignal[port]);
}

void IC8255::Out8255Byte(enum PORT8255 port,char data)
{
	if(port<(int)sizeof(m_OuputSignal))
	{
		m_OuputSignal[port] = data;
		outp(m_PortAddr + port,m_OuputSignal[port]);
	}
}

bool IC8255::Get8255Bit(enum PORT8255 port,enum BIT8255 bit ,bool bSavelog)
{	
	char buf[256];
	unsigned char c = 0x00;
	static unsigned char cc = 0xFF;
	memset(buf,'\0',sizeof(buf));

	c = inp(m_PortAddr+port);

	if(bSavelog==true && cc != c)
	{
		sprintf(buf,"Get Port%d: Bit:%d [%02X]",port,bit,(unsigned char)m_OuputSignal[port]);	
		SaveLog(buf);
		cc = c;
	}
	
	if((c & Bits[bit]) >0)
	{	 // if signal = Hi voltage,it is Off
		return false;		
	}
	return true;
}

unsigned char IC8255::Get8255Byte(enum PORT8255 port)
{
	unsigned char c = 0x00;

	c = inp(m_PortAddr+port);
	return c;
}

void IC8255::SetPortCBit(short bit,bool bOn)
{
	if(bOn)
		Out8255Byte(IC8255_CONTROL,bit <<1 | 1);
	else
		Out8255Byte(IC8255_CONTROL,bit <<1 );	
}

void IC8255::SaveLog( char msg[])
{
	return;
	
	char buf[256];
	char logFilename[80];
	time_t now;
	char *logLevel = NULL;
	FILE *fh = NULL;
	short logLV=0;
	 
	logLevel = getenv("LOG_LEVEL");
	
	if(logLevel)
	{
		logLV=atoi(logLevel);
	}
	
	if(logLV<3) return;
	
	now = time((time_t *)0);
	struct tm *tm_ptr;

	memset(buf,'\0',sizeof(buf));
	memset(logFilename,'\0',sizeof(logFilename));

	tm_ptr = localtime(&now);
  
	sprintf(buf,"%02d:%02d:%02d  %s",tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec,msg);
	printf("%s\n",buf);
	sprintf(logFilename,"./log/%04d%02d%02d.log",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
	
	 fh = fopen(logFilename,"a");
	 {
		  fprintf(fh,"%s\n",buf);
	 }
	 fclose(fh);	  
}