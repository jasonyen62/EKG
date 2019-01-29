/* ra8822.h  RA8822 Control LCM Control Program Header File */

#ifndef ____IPMS_DRIVER_RA8822____
#define ____IPMS_DRIVER_RA8822____

void LCM_Initial(void);
void LCM_FillScreen(unsigned char c);
void LCM_BusyWait(void);
void LCM_AdjustBKLight(unsigned char value);
unsigned char LCM_ReadCommand(unsigned char reg);
void LCM_WriteCommand(unsigned char reg,unsigned char data);
void LCM_WriteData(unsigned char data);
void LCM_GotoXY(unsigned char x,unsigned char y);
unsigned char LCM_GetStatus(void);
void LCM_Reset(void);
void LCM_SetGraphMode(bool bGraph);


void Boot_LCM_Initial(void);
void Boot_LCM_FillScreen(unsigned char c);
void Boot_LCM_BusyWait(void);
void Boot_LCM_AdjustBKLight(unsigned char value);
unsigned char Boot_LCM_ReadCommand(unsigned char reg);
void Boot_LCM_WriteCommand(unsigned char reg,unsigned char data);
void Boot_LCM_WriteData(unsigned char data);
void Boot_LCM_GotoXY(unsigned char x,unsigned char y);
unsigned char Boot_LCM_GetStatus(void);
void Boot_LCM_Reset(void);
void Boot_LCM_SetGraphMode(bool bGraph);


/*
void LCM_PrintString(unsigned char x,unsigned char y, char* showString);

void LCM_LoadBitmap(char* bitmapData,unsigned long* xByte,unsigned long* yByte,char filename[]);
void ShowImage(short pX,short pY,unsigned long xByte,unsigned long yByte,char* image);
void LCMShowTime(void);
*/

#endif