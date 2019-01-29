// LED888.cpp
// type:
// 1: 紈计
// 2: 紈せ计
// 3: 痲竧

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include "../CommonDef.h"
#include "LED888.h"
#include "../traceLog.h"

/*
CntStr = Format(CarCnt, "@@@@")
OutB(1) = &HED
OutB(2) = &HED
OutB(3) = IIf(Mid(CntStr, 1, 1) = " ", &HF, Val(Mid(CntStr, 1, 1)))
OutB(4) = IIf(Mid(CntStr, 2, 1) = " ", &HF, Val(Mid(CntStr, 2, 1)))
OutB(5) = IIf(Mid(CntStr, 3, 1) = " ", &HF, Val(Mid(CntStr, 3, 1)))
OutB( 6 ) = IIf(Mid(CntStr, 4, 1) = " ", &HF, Val(Mid(CntStr, 4, 1)))
OutB(7) = &H1
OutB( 8 ) = &H0
OutB(9) = &H0
comCarCnt.Output = OutB
*/
/*
Dim STX(1) As Byte
   Dim ETX(2) As Byte
   Dim DATA(3) As Byte
   'Nick mark 3.1.13 Dim OutByte( 8 ) As Byte
   ReDim OutByte( 8 ) As Byte 'Nick add 3.1.13

   STX( 0 ) = &HED 'Start Code
   STX(1) = &HED 'Start Code
   
   For i = 1 To 4
      DATA(i - 1) = mId(Format(CStr(TotalCarNumber), "0000"), i, 1)
   Next i
   
   'erosan mark 3.2.4 ETX( 0 ) = &H1 'ID Code
   ETX( 0 ) = CarCountID 'ID Code   erosan add 3.2.4
   ETX(1) = &H0 'Function Code
   ETX(2) = &H0 'End Code
       
   OutByte( 0 ) = STX( 0 )
   OutByte(1) = STX(1)
   OutByte(2) = DATA( 0 )
   OutByte(3) = DATA(1)
   OutByte(4) = DATA(2)
   OutByte(5) = DATA(3)
   OutByte(6) = ETX( 0 )
   OutByte(7) = ETX(1)
   OutByte(8) = ETX(2)
   MDIServer.MSComm1.Output = OutByte   '癳戈

*/
LED888::LED888(void)
{
}

LED888::~LED888(void)
{
	Close();
}

void LED888::init(enum COMPORT port,char type)
{	
	m_Type	= type;
	my_port = port;
	
	if(type ==0)
		return;
	
	if(type == 1 || type == 2)
	{
		//comport.PortInit(port,9600,'N',8,1);
		comport.PortInit2(port,9600,'N',8,1);
	}
}

void LED888::Close()
{
	//char buffMsg[200];
	
	comport.PortClose();
	//sprintf(buffMsg, "LED888 Port Close Port:[ COM%d ].", my_port);
	//ShowMessage(buffMsg);

}

void LED888::Send(char ID,int Number)
{
	char data[64];
	int sLen=0;//,i;
	
	memset(data,'\0',sizeof(data));
	if (Number < 0) Number = 0; // nick add 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	
	if(m_Type == 1)
	{ // 紈 4 计 LED //
		//sprintf(data,"%c%02d%c%04d\x03",SOH,ID,STX,Number);
		sprintf(data,"\xED\xED%04d%c%c%c",Number,ID,0,0);
		/*
		printf("Send 888:");
		for(i=0;i<9;i++)
		{
			printf("%02X ",data[i]);
		}
		printf("\n");
		*/
		sLen=9;
	}
	// ========================================================== //
	// nick mark s 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	// 既ノぃMARK
	//else if(m_Type == 2)
	//{ // 紈 6 计 LED //
	//	sprintf(data,"%c%02d%c%06d\x03",SOH,ID,STX,Number);
	//	sLen = strlen(data);
	//}
	// nick mark e 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	// ========================================================== //
	// ========================================================= //
	// nick add s 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	else if (m_Type == 2)
	{ // 痲竧 LED //
		sprintf(data, "[%03dC+%04d]", ID, Number);
		sLen = strlen(data);
	}
	// nick add e 20140219 Ver:000-000-GIO_V2-135101-0001-13B251 //
	// ========================================================= //
	else if(m_Type ==0)
	{
		return;
	}
	
	//sLen = strlen(data);
	comport.PortWrite(data,sLen);
}
