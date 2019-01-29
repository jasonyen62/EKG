/* Display function */

#ifndef ____IPMS_DISPLAY____
#define ____IPMS_DISPLAY____

void LCM_ShowTime(void);
void LCM_ShowImage(short pX,short pY,unsigned long xByte,unsigned long yByte,char* image);
void LCM_LoadBitmap(char* bitmapData,unsigned long* xByte,unsigned long* yByte,char filename[]);
void LCM_PrintString(unsigned char x,unsigned char y,char* showString);
void LCM_PrintStringRightAlign(unsigned char y,char* showString);
void ShowLCMFile(short pX,short pY,char imageFile[]);
void DisplayOn(bool bOn);
void ClearScreen(void);

void Boot_ShowLCMFile(short pX,short pY,char imageFile[]);
void Boot_DisplayOn(bool bOn);
void Boot_ClearScreen(void);
#endif