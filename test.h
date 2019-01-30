/*  FileName : test.h 
	All Test function here
*/

#ifndef ____IPMS_TEST____
#define ____IPMS_TEST____

void Test232(void);
void TestMF700(void);

void Beep(int freq,int msec);
void Test232(void);
void WriteData(sectorData sdata);					//Frank add 20121113
void ReadData(sectorData rsdata);					//Frank add 20121113
void TestMF700(void);
void TestCRT350(void);
void SqliteTest(void);

void WatchDogEnable(bool yes);
void WatchDogSetTimer(unsigned long msec);
void WatchDogClearTimer(void);
void Test8255(void);
void UdpTest(void);
unsigned int IsVortex86SX(void);

void split(char **arr, char *str, const char *del);
void timetest(void);
void TestLCM(void);
void TestD1000(void);
void TestD3000(void);
void TestVoice(void);
void TestLED888(void);
void PlateTest(void);
void CommandTest(void);
int  SDLTest(void);
void TestBMP(void);
void BarcodeTest(void);
void TUP500Test(void);
void TestMCP210(void);					//Frank add 20120511
void TestD1800(void);					//Frank add 20120511
void HF320Test(void); // nick add 20150114 Ver:000-000-GIO_V2-135101-0003-13B251 //
void HF320_Visual_Card_Test(void); // nick add 20160629 Ver:000-000-GIO_V2-13C241-0001-166241 //

#endif