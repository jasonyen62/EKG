/* Digital I/O control Function */
#include <stdio.h>
#include <unistd.h>

#include "CommonDef.h"
#include "Dio.h"
#include "IC8255.h"
#include "traceLog.h"

#define ADDR8255_1 0x0300
#define ADDR8255_2 0x0304

IC8255 io8255_1;
IC8255 io8255_2;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long ClsoeBarrierChkTime;	// 20120920 Tony add 
// nick mark 20140304 Ver:000-000-GIO_V2-135101-0002-13B251 //double ClsoeBarrierChkTime = 1; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
unsigned long ClsoeBarrierChkTime = GetTickCount(); // nick add 20140304 Ver:000-000-GIO_V2-135101-0002-13B251 //


// ========================================================= //
// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
bool bIC8255A_A_DI[8];
bool bIC8255A_B_DI[8];
// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// ========================================================= //

void Delay10ns(int n)
{   // 800MHz: 1 clock= 1.25ns
//	int i,j;
	int j;

	for(j=0; j<n; j++)
	{
		{	//10ns
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
			asm volatile("nop");
		}
	}
}

void IOInitial()
{
	// ========================================================= //
	// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
	for (int i = 0; i < 8; i++)
	{
		bIC8255A_A_DI[i] = false;
		bIC8255A_B_DI[i] = false;
	}
	// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
	// ========================================================= //
	
	io8255_1.Init8255(ADDR8255_1,true,true,false,false);
	io8255_2.Init8255(ADDR8255_2,false,false,false,false);
}

// ========================================================= //
// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //void GetIC8255_All_Input()
void GetIC8255_All_Input(long l_WaitTime) // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
{
	unsigned char tmp_inp = 0x00;
	unsigned char cinp = 0x00;
	int i = 0;
	
	cinp = io8255_1.Get8255Byte(IC8255_PORTA);
	
	for (i = 0; i < 8; i++)
	{
		tmp_inp = cinp & (0x01 << i);
		
		if (tmp_inp > 0)
			bIC8255A_A_DI[i] = false;
		else
			bIC8255A_A_DI[i] = true;
	}
	
	// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //usleep(15000L);
	usleep(l_WaitTime); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
	cinp = io8255_1.Get8255Byte(IC8255_PORTB);
	
	for (i = 0; i < 8; i++)
	{
		tmp_inp = cinp & (0x01 << i);
		
		if (tmp_inp > 0)
			bIC8255A_B_DI[i] = false;
		else
			bIC8255A_B_DI[i] = true;
	}
	
	// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //usleep(15000L);
	usleep(l_WaitTime); // nick add 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //
}
// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// ========================================================= //

bool CheckMachineBeStrike()
{	// 箱體被撞
/*
	//nick mark s 20110325
	if(	io8255_1.Get8255Bit(IC8255_PORTA,BIT0) == true )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTA,BIT0) == true )
		{
			return true;
		}
	}
	
	return false;
//nick mark e 20110325
*/

// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return (io8255_1.Get8255Bit(IC8255_PORTA,BIT0));		//nick add 20110325
	return (bIC8255A_A_DI[0]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool CheckMachineBeOpen()
{	// 檢查箱門開啟
/*	if(	io8255_1.Get8255Bit(IC8255_PORTA,BIT1) == true )
	{
		usleep(100000L);
		if(	io8255_1.Get8255Bit(IC8255_PORTA,BIT1) == true )
		{
			return true;
		}
	}
	
	return false;	*/

// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTA,BIT1);	//nick add 20110201
	return (bIC8255A_A_DI[1]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool PressTicketButton()
{   // 檢查取票按鈕
/*	if(io8255_1.Get8255Bit(IC8255_PORTA,BIT2) == true )
	{
		//usleep(10000L);
		//if(	io8255_1.Get8255Bit(IC8255_PORTA,BIT2) == true )
		{
			return true;
		}
	}
	
	return false;	*/

// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTA,BIT2);	//nick add 20110201
	return (bIC8255A_A_DI[2]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool CheckTestTicketButton()
{   //檢查測試票按鈕
// ========================================================== //
// nick mark s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	if(io8255_1.Get8255Bit(IC8255_PORTA,BIT3) == true )
//	{
//		usleep(20000L);
//		
//		if(io8255_1.Get8255Bit(IC8255_PORTA,BIT3) == true )
//		{
//			return true;
//		}
//	}
//	
//	return false;
// nick mark e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// ========================================================== //
	
	// ========================================================= //
	// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
	if (bIC8255A_A_DI[3] == true)
	{
		usleep(20000L);
		
		return (bIC8255A_A_DI[3]);
	}
	
	return (false);
	// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
	// ========================================================= //
}

bool GetOpenBarrierButton()
{   // 檢查手動開柵欄按鈕
/*
	//nick mark s 20110325
	if(io8255_1.Get8255Bit(IC8255_PORTA,BIT4) == true )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTA,BIT4) == true )
		{
			return true;
		}
	}
	
	return false;
	//nick mark e 20110325
*/

// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return (io8255_1.Get8255Bit(IC8255_PORTA,BIT4));		//nick add 20110325
	return (bIC8255A_A_DI[4]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool GetCloseBarrierButton()
{   // 檢查手動關柵欄按鈕
/*
	//nick mark s 20110325
	if(io8255_1.Get8255Bit(IC8255_PORTA,BIT5) == true )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTA,BIT5) == true )
		{
			return true;
		}
	}
	
	return false;
	//nick mark e 20110325
*/
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return (io8255_1.Get8255Bit(IC8255_PORTA,BIT5));		//nick add 20110325
	return (bIC8255A_A_DI[5]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool UPSSignal(void)									//Frank add s 20110817
{													
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTA,BIT7);
	return (bIC8255A_A_DI[7]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}														//Frank add e 20110817

bool GetLoop1()
{   //檢查 Loop1
/*	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT0) == true )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT0) == true )
		{
			return true;
		}
	}
	
	return false;	*/
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTB,BIT0);	//nick add 20110201
	return (bIC8255A_B_DI[0]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool GetLoop2()
{
/*	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT1) == false )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT1) == false )
		{
			return false;
		}
	}
	
	return true;	*/
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTB,BIT1);	//nick add 20110201
	return (bIC8255A_B_DI[1]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool CheckBarrierClosed()
{
/*
//nick mark s 20110302
	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT3) == false )
	{
//nick mark 20101220		usleep(100000L);
		usleep(350000L);	//nick add 20101220
		
		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT3) == false )
		{
			return true;
		}
	}
	
	return false;
//nick mark e 20110302
*/

// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return (!io8255_1.Get8255Bit(IC8255_PORTB,BIT3));		//nick add 20110302
	return (!bIC8255A_B_DI[3]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool IsTicketLow()
{
/*	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT4) == true )
	{
		usleep(100000L);
		
		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT4) == true )
		{
			return true;
		}
	}
	
	return false;	*/
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTB,BIT4);
	return (bIC8255A_B_DI[4]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool GetEasyCardIn()
{
/*	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT2,true) == true )
	{
		//usleep(100000L);//200ms
		//if(	io8255_1.Get8255Bit(IC8255_PORTB,BIT2) == true )
		{
			return true;
		}
	}
	
	return false;	*/
// nick mark 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //	return io8255_1.Get8255Bit(IC8255_PORTB,BIT2,true);
	return (bIC8255A_B_DI[2]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

// ========================================================= //
// nick add s 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
bool GetPosOpenBar(void)
{
	return (bIC8255A_A_DI[6]);
}
// nick add e 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
// ========================================================= //

void OpenBarrier(bool bOpen,bool bAutoOff)
{
	usleep(100L);
	io8255_2.Out8255Bit(IC8255_PORTA,BIT0,bOpen,true);
	
	// ========================================================= //
	// nick add s 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
	usleep(100L);
	io8255_1.Out8255Bit(IC8255_PORTC, BIT6, bOpen, true);
	// nick add e 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
	// ========================================================= //
	
	if(bAutoOff == true)
	{
		//nick add s 20110302
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //		long sTick=GetTickCount();
		unsigned long sTick = GetTickCount();
		
		while(1)
		{
			// nick mark 20140526 Ver:000-000-GIO_V2-135101-0004-13B251 //usleep(100000L);
			usleep(500000L); // nick add 20140526 Ver:000-000-GIO_V2-135101-0004-13B251 //
			
			if ((CheckBarrierClosed() == false) || (CheckTimeout(&sTick, (unsigned long)2000)))
			//if(CheckBarrierClosed()==false || (GetTickCount()-sTick>500))	// 20111215 Tony add
				break;
		}
		//nick add 20110302

//nick mark 20110302		usleep(300000L);
		io8255_2.Out8255Bit(IC8255_PORTA,BIT0,false,true);
		
		// ========================================================= //
		// nick add s 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
		usleep(100L);
		io8255_1.Out8255Bit(IC8255_PORTC, BIT6, false, true);
		// nick add e 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
		// ========================================================= //
	}

	G_RFin = false; // nick add 20141218 Ver:000-000-GIO_V2-135101-0008-13B251 //
	G_POSIN = false; // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void CloseBarrier(bool bClose,bool bAutoOff)
{
	if (IsINMachine == false)
		ReceiveChipCoin(); // nick add 20131122 Ver:000-000-GIO_V2-133181-0103-135101 //

	usleep(100L);
	io8255_2.Out8255Bit(IC8255_PORTA,BIT1,bClose,true);
	
	// ========================================================== //
	// nick mark s 20131225 Ver:000-000-GIO_V2-135101-0000-13B250 //
	//// ========================================================= //
	//// nick add s 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
	//usleep(100L);
	//io8255_1.Out8255Bit(IC8255_PORTC, BIT7, bClose, true);
	//// nick add e 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
	//// ========================================================= //
	// nick mark e 20131225 Ver:000-000-GIO_V2-135101-0000-13B250 //
	// ========================================================== //
	
	//if(bAutoOff == true)
	//{
		usleep(100000L);
		io8255_2.Out8255Bit(IC8255_PORTA,BIT2, true); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 // Counter On		
		usleep(500000L);
	
		io8255_2.Out8255Bit(IC8255_PORTA,BIT1,false,true);
		ShowMessage((char *)"CloseBarrier :: Close Barrier Success.");
		usleep(100000L);
		io8255_2.Out8255Bit(IC8255_PORTA,BIT2, false); // nick add 20140415 Ver:000-000-GIO_V2-135101-0003-13B251 // Counter Off
		ShowMessage((char *)"CloseBarrier :: Send Counter Success.");
		
		
		// ========================================================== //
		// nick mark s 20131225 Ver:000-000-GIO_V2-135101-0000-13B250 //
		//// ========================================================= //
		//// nick add s 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
		//usleep(100L);
		//io8255_1.Out8255Bit(IC8255_PORTC, BIT7, false, true);
		//// nick add e 20130909 Ver:000-000-GIO_V2-133181-0100-135101 //
		//// ========================================================= //
		// nick mark e 20131225 Ver:000-000-GIO_V2-135101-0000-13B250 //
		// ========================================================== //
	//}

	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
	ClsoeBarrierChkTime = GetTickCount();	// 20120920 Tony add
}

void OutCounter()
{
	usleep(10); // nick add 20130409 //
	io8255_2.Out8255Bit(IC8255_PORTA,BIT2,true);
	usleep(3000000L);
	io8255_2.Out8255Bit(IC8255_PORTA,BIT2,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void EasyCardEnable(bool On)
{
	static bool bEnabled = false; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //

	if (On == bEnabled) return; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	usleep(10); // nick add 20130409 //
// nick mark 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //	io8255_2.Out8255Bit(IC8255_PORTA,BIT3,On);
	io8255_2.Out8255Bit(IC8255_PORTA,BIT3, !On); // nick add 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
	bEnabled = On; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void TurnOnParkingFullLight(bool bOn)
{
	static bool bEnabled = false; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //

	if (bOn == bEnabled) return; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	usleep(10); // nick add 20130409 //
	io8255_2.Out8255Bit(IC8255_PORTA,BIT4,bOn);
	bEnabled = bOn; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void TurnOnTransferGateLight(bool bOn)
{
	static bool bEnabled = false; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //

	if (bOn == bEnabled) return; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	usleep(10); // nick add 20130409 //
	io8255_2.Out8255Bit(IC8255_PORTA,BIT5,bOn);
	bEnabled = bOn; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void UPSShutdown(int msec)					//Frank add s 20110817
{
	usleep(10); // nick add 20130409 //
	io8255_2.Out8255Bit(IC8255_PORTA,BIT7,true);
	usleep(msec*1000L);
	io8255_2.Out8255Bit(IC8255_PORTA,BIT7,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}												//Frank add e 20110817


/* ====================================
	Chip Coin Control Moudle
   ==================================== */

void IssueChipCoin1()
{   // 發卡 Send a pulse 60ms
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT4,true);
// nick mark 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //	usleep(60000L);
	usleep(100000L); // nick add 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT4,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void IssueChipCoin2()
{   // 發卡 Send a pulse 60ms
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT5,true);
// nick mark 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //	usleep(60000L);
	usleep(100000L); // nick add 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT5,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void DrawChipCoin(bool bOn)
{   // 出票 ,開Shutter
	static bool bEnabled = false; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //

	if (bOn == bEnabled) return; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT1,bOn);
	bEnabled = bOn; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	//usleep(50000L);
	//io8255_1.Out8255Bit(IC8255_PORTC,BIT4,false);
	//直到取走才關

	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void TurnOnCoinShutter(bool bOn)
{	// 投入口(出口) / 取票口回收(入口) shutter開關.
	static bool bEnabled = false; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //

	if (bOn == bEnabled) return; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT0,bOn);
	bEnabled = bOn; // nick add 20131112 Ver:000-000-GIO_V2-133181-0103-135101 //
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void ClearChipCoin()
{   //清空Hopper
	int i;
	
	for( i=0; i<50; i++)
	{
		printf("%d\n",i);
		IssueChipCoin1();
		//IssueChipCoin2();
		usleep(250000L);
	}
	
	sleep(6);
	HopperReset();
}

void ReceiveChipCoin()
{   //回收票卡 (讀卡機位置,電路會自己保持1秒) 
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT2,true);
	usleep(200000L);
	io8255_1.Out8255Bit(IC8255_PORTC,BIT2,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void HopperReset()
{   //Hopper 重置
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT3,true);
// nick mark 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //	usleep(200000L);
	usleep(500000L); // nick add 20131219 Ver:000-000-GIO_V2-133181-0107-135101 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT3,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

void RecycleChipCoin()
{   //On Ticket outlet
	usleep(10); // nick add 20130409 //
	io8255_1.Out8255Bit(IC8255_PORTC,BIT0,true);
	usleep(600000L);
	io8255_1.Out8255Bit(IC8255_PORTC,BIT0,false);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}

// ========================================================= //
// nick add s 20141119 Ver:000-000-GIO_V2-135101-0007-13B251 //
void ClearChipCoinModule()
{
	// Turn On
	TurnOnCoinShutter(true);
	DrawChipCoin(true);
	ReceiveChipCoin();
	//
	sleep(2);
	// Turn Off
	TurnOnCoinShutter(false);
	DrawChipCoin(false);
	//
}
// nick add e 20141119 Ver:000-000-GIO_V2-135101-0007-13B251 //
// ========================================================= //

// ========================================================== //
// nick mark s 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
//bool CheckHopperTicketLow1()
//{
//// ========================================================== //
//// nick mark s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
////	bool bRet = false;
////	
////	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT6) == true )
////	{
////		usleep(50000L);
////		
////		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT6) == true )
////		{
////			bRet = true;
////		}
////	}
////
////	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
////	return(bRet);
//// nick mark e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//// ========================================================== //
//
//	// ========================================================= //
//	// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	if (bIC8255A_B_DI[6] == true)
//	{
//		usleep(50000L);
//		return (bIC8255A_B_DI[6]);
//	}
//	
//	return (false);
//	// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	// ========================================================= //
//}
//
//bool CheckHopperTicketLow2()
//{
//// ========================================================== //
//// nick mark s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
////	bool bRet = false;
////	
////	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT7) == true )
////	{
////		usleep(50000L);
////		
////		if(io8255_1.Get8255Bit(IC8255_PORTB,BIT7) == true )
////		{
////			bRet = true;
////		}
////	}
////
////	return(bRet);
//// nick mark e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//// ========================================================== //
//
//	// ========================================================= //
//	// nick add s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	if (bIC8255A_B_DI[7] == true)
//	{
//		usleep(50000L);
//		return(bIC8255A_B_DI[7]);
//	}
//	
//	return(false);
//	// nick add e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	// ========================================================= //
//}
// nick mark e 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
// ========================================================== //

// ========================================================= //
// nick add s 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
bool CheckBarrierstrike()
{ // 振臂遭受撞擊 (硬體預設為High電位，所以回傳值取反向)
	return(bIC8255A_B_DI[6]);
}
// nick add e 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
// ========================================================= //

bool CheckCoinOnOutlet()
{   // 檢查取票口 有無票卡
// ========================================================== //
// nick mark s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT5) == true )
//	{
//		return true;
//	}
//	
//	return false;
// nick mark e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// ========================================================== //
	
	return (bIC8255A_B_DI[5]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

bool CheckCoinOnInlet()
{   // 出口:檢查進票口 有無票卡經過 , 入口: 讀卡機位置
// ========================================================== //
// nick mark s 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
//	if(io8255_1.Get8255Bit(IC8255_PORTB,BIT4) == true )
//	{
//		return true;
//	}
//	
//	return false;
// nick mark e 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
// ========================================================== //

	return (bIC8255A_B_DI[4]); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
}

/* LCM I/O Control */
#define LCM_WR_ON	0x01
#define LCM_RD_ON   0x02
#define LCM_RS_ON   0x04
#define LCM_CS_ON   0x08
#define LCM_RESET_ON	0x10

void LCM_WR(bool bOn)
{	// Write
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Bit(IC8255_PORTC,BIT0,bOn);
	}
	//io8255_2.SetPortCBit(0,bOn);
}

void LCM_RD(bool bOn)
{   // Read
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Bit(IC8255_PORTC,BIT1,bOn);
	}
	//io8255_2.SetPortCBit(1,bOn);
}

void LCM_RS(bool bOn)
{   // Register/Memory Select:  L(bOn = true): command write H(bOn= false): data   write
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Bit(IC8255_PORTC,BIT3,bOn);
	}
	//io8255_2.SetPortCBit(2,bOn);
}

void LCM_CS(bool bOn)
{   // Chip Select;Chip Enable

	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Bit(IC8255_PORTC,BIT2,bOn);
	}
	
	Delay10ns(4);
}

void LCM_RESET(bool bOn)
{
	if (G_NoTicketSystem == false)
	{
		// Chip Select;Chip Enable
		io8255_2.Out8255Bit(IC8255_PORTC,BIT4,bOn);
		//io8255_2.SetPortCBit(3,bOn);
	}		
}

void LCM_OutputData(unsigned char data)
{
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Byte(IC8255_PORTB,data);
	}
}

unsigned char LCM_InputData()
{
	if (G_NoTicketSystem == false)
	{
		return io8255_2.Get8255Byte(IC8255_PORTB);
	}
	
	return 0x00;
}

void LCM_OutputControl(unsigned char ctrl)
{
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Byte(IC8255_PORTC,ctrl);
	}
}

bool LCM_IsBusy()
{
	if (G_NoTicketSystem == false)
	{
		if(io8255_2.Get8255Bit(IC8255_PORTC,BIT7) == false )
			return true;
	}	
	
	return false;
}
void LCM_Light(bool bOn)
{
	if (G_NoTicketSystem == false)
	{
		io8255_2.Out8255Bit(IC8255_PORTA,BIT6,bOn);
	}
}

void Boot_LCM_WR(bool bOn)
{	// Write
	io8255_2.Out8255Bit(IC8255_PORTC,BIT0,bOn);
	//io8255_2.SetPortCBit(0,bOn);
}

void Boot_LCM_RD(bool bOn)
{   // Read
	io8255_2.Out8255Bit(IC8255_PORTC,BIT1,bOn);
	//io8255_2.SetPortCBit(1,bOn);
}

void Boot_LCM_RS(bool bOn)
{   // Register/Memory Select:  L(bOn = true): command write H(bOn= false): data   write
		io8255_2.Out8255Bit(IC8255_PORTC,BIT3,bOn);
	//io8255_2.SetPortCBit(2,bOn);
}

void Boot_LCM_CS(bool bOn)
{   // Chip Select;Chip Enable
	io8255_2.Out8255Bit(IC8255_PORTC,BIT2,bOn);
	Delay10ns(4);
}

void Boot_LCM_RESET(bool bOn)
{
	// Chip Select;Chip Enable
	io8255_2.Out8255Bit(IC8255_PORTC,BIT4,bOn);
	//io8255_2.SetPortCBit(3,bOn);
}

void Boot_LCM_OutputData(unsigned char data)
{
	io8255_2.Out8255Byte(IC8255_PORTB,data);
}

unsigned char Boot_LCM_InputData()
{
	return io8255_2.Get8255Byte(IC8255_PORTB);
}

void Boot_LCM_OutputControl(unsigned char ctrl)
{
	io8255_2.Out8255Byte(IC8255_PORTC,ctrl);
}

bool Boot_LCM_IsBusy()
{
	if(io8255_2.Get8255Bit(IC8255_PORTC,BIT7) == false )
		return true;
	
	return false;
}
void Boot_LCM_Light(bool bOn)
{
	io8255_2.Out8255Bit(IC8255_PORTA,BIT6,bOn);
}


// VOICE
void IOVoiceOut(int index)
{
	io8255_2.Out8255HByte(IC8255_PORTC,~((unsigned char)((index)<<4)));
	usleep(50000L);
	io8255_2.Out8255HByte(IC8255_PORTC,0xFF);
	usleep(100L); // nick add 20130510 ver:000-000-GIO_V2-133181-0100-135101 //
}
