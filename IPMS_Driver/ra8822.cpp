/* RA8822 Controler LCM */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "../Dio.h"
#include "../CommonDef.h"
#include "ra8822.h"


// LCM Control : RA8802 Controler
/* Register Define : LCD CONTROL */
#define REG_LCD_CONTROL		0x00
//===============================================
// 反白整個營幕 0:反白 1:正常
#define LCM_CONTROL_NOREVERSE     0x01
// 閃爍: 0:正常 1: 閃爍 
#define LCM_CONTROL_BLINK       0x02
// LCM On,OFF
#define LCM_CONTROL_ON          0x04
// 文字模式
#define LCM_CONTROL_TEXTMODE    0x08
// 粗體
#define LCM_CONTROL_BOLD        0x10
// 重置
#define LCM_CONTROL_RESET       0x20
// Power Mode
#define LCM_CONTROL_POWER       0xC0

/* Register Define : LCD Contrast Control */
//=================================================
#define REG_CONTRAST_CONTROL	0xD0
//------------------------------------------------
// Enable顏色深淺控制 
#define LCM_CONTRAST_LIGHT_CTRL     0x80
#define LCM_CONTRAST_DAC_ENABLE     0x40
#define LCM_CONTRAST_RESETLIGHT     0x20
// bit 0~4 is DAC value

/* Register Define : Cursor Control */
//==================================================
#define REG_CURSOR				0x10
//----------------------------------------------------
// Cursor width
#define LCM_CURSOR_WIDTH            0x01
// Cursor blink
#define LCM_CURSOR_BLINK            0x02
// CURSOR On
#define LCM_CURSOR_ON               0x04
// cursor auto write shift
#define LCM_CURSOR_AUTO_WRITE_SHIFT 0x08
// 粗體字
#define LCM_CURSOR_BOLD             0x10
// 將資料存於DDRAM
#define LCM_CURSOR_DDRAM_NOREVERS   0x20
//中英文字對齊
#define CURSOR_ALIGH            	0x40
// CURSOR auto read shift
#define CURSOR_AUTO_READ_SHIFT  	0x80


/* Register Point x and Point y*/
//======================================
#define REG_POINT_X			0x60
//======================================
#define REG_POINT_Y			0x70

/* Register Blink Time Register */
//========================================
#define REG_BLINK_TIME  	0x80

/* Register Cursor size */
//========================================
#define REG_CURSOR_SIZE     0x18

/* Register LCM Status */
//========================================
#define REG_STATUS          0xA0

/* Register : Font Control */
#define REG_FONT_CONTROL 	0xF0
//=========================================
// Enable
#define LCM_FONT_ROM_ENABLE 0x80
// Auto Fill DDRAM
#define LCM_FONT_AUTO_FILL  0x08
// 外部ROM 選擇 1:外部ROM 0: 內部
#define LCM_EXTROM_SELECT   0x40
// bi5,bit4 語系選擇 
#define LCM_FONT_BIG5       0x10
#define LCM_FONT_GB         0x20
// 選擇字碼 ASCII/中文字
#define LCM_FONT_ACSII		0x02


//void LCM_OutputControl(unsigned char ctrl);
//bool LCM_IsBusy(void);
//void LCM_RESET(bool bOn);
//void LCD_CmdWrite(unsigned char c);


void LCM_Initial()
{
	unsigned char ControlSig = 0x00;
//	ControlSig = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
	ControlSig = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);

	LCM_WriteCommand(REG_LCD_CONTROL,ControlSig);  // LCM mode initial
	LCM_WriteCommand(0xC0,0x10);  	// disable Touch Screen Function
	LCM_WriteCommand(0x01,0xF3);    // 12M Hz
	LCM_WriteCommand(0x21,0x1D);	//240*64
	LCM_WriteCommand(0x31,0x3F);
	LCM_WriteCommand(0x41,0x00);    //X from 0
	LCM_WriteCommand(0x51,0x00);    //Y from 0
	LCM_WriteCommand(REG_CURSOR,(LCM_CURSOR_AUTO_WRITE_SHIFT | LCM_CURSOR_DDRAM_NOREVERS));	
	LCM_WriteCommand(REG_FONT_CONTROL,(LCM_FONT_ROM_ENABLE | LCM_FONT_BIG5)); // 選擇內部字型ROM繁體字型	
	LCM_WriteCommand(REG_CONTRAST_CONTROL,0x0F); // 設亮度value from 00000b ~ 11111b
}

void LCM_SetGraphMode(bool bGraph)
{
	unsigned char readReg = 0x00;

	//readReg = LCM_ReadCommand(REG_LCD_CONTROL);
	//printf("reg:%02x\n",readReg);
	if(bGraph == true)
	{
//		readReg = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_NOREVERSE);
		readReg = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_NOREVERSE);
	}
	else
	{
//		readReg = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
		readReg = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
	}

	usleep(10);
	LCM_WriteCommand(REG_LCD_CONTROL,readReg);
	usleep(10);
}

void LCM_AdjustBKLight(unsigned char value)
{
	LCM_WriteCommand(REG_CONTRAST_CONTROL,value & 0x1F);
}

void LCM_Reset()
{
	LCM_RESET(true);
	usleep(250000);
	LCM_RESET(false);
	usleep(30000);
}

unsigned char LCM_GetStatus()
{	
	unsigned char status=0x00;
	status = LCM_ReadCommand(REG_STATUS);
	return status;	
}

void LCM_BusyWait()
{
	if (G_NoTicketSystem == true)
	{
		Delay10ns(16);
		return;
	}

	if(LCM_IsBusy() == true)
	{
		Delay10ns(16);
	}
}

void LCM_FillScreen(unsigned char c)
{
	unsigned char readReg = 0x00;

	LCM_WriteCommand(0xE0,c);
//	readReg = LCM_ReadCommand(REG_FONT_CONTROL);
//	readReg &= 0xF7;

	readReg = (LCM_FONT_ROM_ENABLE | LCM_FONT_BIG5 | LCM_FONT_AUTO_FILL) ;
	LCM_WriteCommand(REG_FONT_CONTROL,readReg);
	usleep(1);
}

void LCD_CmdWrite(unsigned char c)
{
	LCM_BusyWait();

	LCM_CS(true);
	LCM_RS(true);
	LCM_RD(false);
	LCM_OutputData(c);
	LCM_WR(true);
	Delay10ns(3);
	LCM_WR(false);
	LCM_CS(false);
}

unsigned char LCM_ReadCommand(unsigned char reg)
{
	unsigned char readReg =0x00;
	
	LCD_CmdWrite(reg);

	LCM_BusyWait();

	LCM_CS(true);
	LCM_RS(true);
	LCM_WR(false);
	LCM_RD(true);
	Delay10ns(30);
	readReg = LCM_InputData();
	LCM_RD(false);
	LCM_CS(false);
	
	return readReg;
}

void LCM_WriteCommand(unsigned char reg,unsigned char data)
{
	LCD_CmdWrite(reg);
	LCD_CmdWrite(data); 
}

void LCM_WriteData(unsigned char data)
{	// Write display data
	LCM_BusyWait();
	LCM_CS(true);
	LCM_RS(false);
	LCM_RD(false);

	LCM_OutputData(data);	
	LCM_WR(true);
	Delay10ns(3);
	LCM_WR(false);
	LCM_CS(false);	
}

void LCM_GotoXY(unsigned char x,unsigned char y)
{
	LCM_WriteCommand(0x60,x);
	LCM_WriteCommand(0x70,y*0x10);
}


void Boot_LCM_Initial()
{
	unsigned char ControlSig = 0x00;
//	ControlSig = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
	ControlSig = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);

	Boot_LCM_WriteCommand(REG_LCD_CONTROL,ControlSig);  // LCM mode initial
	Boot_LCM_WriteCommand(0xC0,0x10);  	// disable Touch Screen Function
	Boot_LCM_WriteCommand(0x01,0xF3);    // 12M Hz
	Boot_LCM_WriteCommand(0x21,0x1D);	//240*64
	Boot_LCM_WriteCommand(0x31,0x3F);
	Boot_LCM_WriteCommand(0x41,0x00);    //X from 0
	Boot_LCM_WriteCommand(0x51,0x00);    //Y from 0
	Boot_LCM_WriteCommand(REG_CURSOR,(LCM_CURSOR_AUTO_WRITE_SHIFT | LCM_CURSOR_DDRAM_NOREVERS));	
	Boot_LCM_WriteCommand(REG_FONT_CONTROL,(LCM_FONT_ROM_ENABLE | LCM_FONT_BIG5)); // 選擇內部字型ROM繁體字型	
	Boot_LCM_WriteCommand(REG_CONTRAST_CONTROL,0x0F); // 設亮度value from 00000b ~ 11111b
}

void Boot_LCM_SetGraphMode(bool bGraph)
{
	unsigned char readReg = 0x00;

	//readReg = LCM_ReadCommand(REG_LCD_CONTROL);
	//printf("reg:%02x\n",readReg);
	if(bGraph == true)
	{
//		readReg = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_NOREVERSE);
		readReg = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_NOREVERSE);
	}
	else
	{
//		readReg = (REG_LCD_CONTROL,LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
		readReg = (LCM_CONTROL_POWER | LCM_CONTROL_ON | LCM_CONTROL_TEXTMODE | LCM_CONTROL_NOREVERSE);
	}

	usleep(10);
	Boot_LCM_WriteCommand(REG_LCD_CONTROL,readReg);
	usleep(10);
}

void Boot_LCM_AdjustBKLight(unsigned char value)
{
	Boot_LCM_WriteCommand(REG_CONTRAST_CONTROL,value & 0x1F);
}

void Boot_LCM_Reset()
{
	Boot_LCM_RESET(true);
	usleep(250000);
	Boot_LCM_RESET(false);
	usleep(30000);
}

unsigned char Boot_LCM_GetStatus()
{	
	unsigned char status=0x00;
	status = Boot_LCM_ReadCommand(REG_STATUS);
	return status;	
}

void Boot_LCM_BusyWait()
{
	if(Boot_LCM_IsBusy() == true)
	{
		Delay10ns(16);
	}
}

void Boot_LCM_FillScreen(unsigned char c)
{
	unsigned char readReg = 0x00;

	Boot_LCM_WriteCommand(0xE0,c);
//	readReg = LCM_ReadCommand(REG_FONT_CONTROL);
//	readReg &= 0xF7;

	readReg = (LCM_FONT_ROM_ENABLE | LCM_FONT_BIG5 | LCM_FONT_AUTO_FILL) ;
	Boot_LCM_WriteCommand(REG_FONT_CONTROL,readReg);
	usleep(1);
}

void Boot_LCD_CmdWrite(unsigned char c)
{
	Boot_LCM_BusyWait();

	Boot_LCM_CS(true);
	Boot_LCM_RS(true);
	Boot_LCM_RD(false);
	Boot_LCM_OutputData(c);
	Boot_LCM_WR(true);
	Delay10ns(3);
	Boot_LCM_WR(false);
	Boot_LCM_CS(false);
}

unsigned char Boot_LCM_ReadCommand(unsigned char reg)
{
	unsigned char readReg =0x00;
	
	Boot_LCD_CmdWrite(reg);

	Boot_LCM_BusyWait();

	Boot_LCM_CS(true);
	Boot_LCM_RS(true);
	Boot_LCM_WR(false);
	Boot_LCM_RD(true);
	Delay10ns(30);
	readReg = Boot_LCM_InputData();
	Boot_LCM_RD(false);
	Boot_LCM_CS(false);
	
	return readReg;
}

void Boot_LCM_WriteCommand(unsigned char reg,unsigned char data)
{
	Boot_LCD_CmdWrite(reg);
	Boot_LCD_CmdWrite(data); 
}

void Boot_LCM_WriteData(unsigned char data)
{	// Write display data
	Boot_LCM_BusyWait();
	Boot_LCM_CS(true);
	Boot_LCM_RS(false);
	Boot_LCM_RD(false);

	Boot_LCM_OutputData(data);	
	Boot_LCM_WR(true);
	Delay10ns(3);
	Boot_LCM_WR(false);
	Boot_LCM_CS(false);	
}

void Boot_LCM_GotoXY(unsigned char x,unsigned char y)
{
	Boot_LCM_WriteCommand(0x60,x);
	Boot_LCM_WriteCommand(0x70,y*0x10);
}



