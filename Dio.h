/* I/O Control Function */

#ifndef ____IPMS_DIO____
#define ____IPMS_DIO____

/* 
8255-1 PortA : *Input*
0: 箱體受撞
1: 箱門開關
2: 取票按鈕
3: 出票測試
4: 手動開柵欄
5: Chip Coin 位置1 (Reader)
6: Chip Coin 位置2 (Take Ticket)
7: Chip Coin 出票完成

PortB : *Input*
0: Loop1
1: Loop2
2: 悠遊卡(月票)開柵欄
3: 柵欄開關狀態
4: Ticket low

PortC : *Output*
0: Chip Coin Shutter
1: Chip Coin 出票 (讀票位置)
2: Chip Coin 回收1 (讀票位置)
3: Chip Coin 回收2 (取票位置)
4: Chip Coin 發票
5: 出票指示LED

8255-2 PortA : *Output*
0: 開柵欄
1: 關柵欄
2: 計數
3: 悠遊卡(月票)入場
4: 車位己滿
5: 調撥車道

PortB : *Output ->LCM* DB0~7
PortC : *Output ->LCM*
0: WR
1: RD
2: C/D
*/

void IOInitial(void);
void Delay10ns(int n);
//Input
// nick mark 20150206 Ver:000-000-GIO_V2-13B251-0001-13C241 //void GetIC8255_All_Input(void); // nick add 20131203 Ver:000-000-GIO_V2-133181-0106-135101 //
void GetIC8255_All_Input(long l_WaitTime = 15000); // nick add 20131203 Ver:000-000-GIO_V2-13B251-0001-13C241 //

bool GetLoop1(void);
bool GetLoop2(void);
bool CheckMachineBeStrike(void);
bool CheckMachineBeOpen(void);
bool PressTicketButton(void);
bool CheckTestTicketButton(void);
bool GetOpenBarrierButton(void);
bool GetCloseBarrierButton(void);

bool CheckCoinOnInlet(void);
bool CheckCoinOnOutlet(void);
bool GetEasyCardIn(void);
bool GetPosOpenBar(void); // nick add 20150407 Ver:000-000-GIO_V2-13B251-0004-13C241 //
bool CheckBarrierClosed(void);
// nick mark 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //bool CheckHopperTicketLow1(void);
// nick mark 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //bool CheckHopperTicketLow2(void);
bool CheckBarrierstrike(void); // nick add 20160531 Ver:000-000-GIO_V2-13B251-0013-13C241 //
bool UPSSignal(void);					//Frank add 20110817

//Output
void TurnOnCoinShutter(bool bOn);
void DrawChipCoin(bool bOn);
void ReceiveChipCoin(void);
void RecycleChipCoin(void);
void IssueChipCoin1(void);
void IssueChipCoin2(void);
void TurnOnLED(bool bOn);
void HopperReset(void);
void OpenBarrier (bool bOpen, bool bAutoOff=true);
void CloseBarrier(bool bClose,bool bAutoOff=true);
void OutCounter(void);
void EasyCardEnable(bool On);
void TurnOnParkingFullLight(bool bOn);
void TurnOnTransferGateLight(bool bOn);
void UPSShutdown(int msec);			//Frank add 20110817
void ClearChipCoinModule(void); // nick add 20141119 Ver:000-000-GIO_V2-135101-0007-13B251 //

//Hopper
void ClearChipCoin(void);
// LCM I/O Function
void LCM_WR(bool bOn);
void LCM_RD(bool bOn);
void LCM_RS(bool bOn);
void LCM_CS(bool bOn);
void LCM_OutputData(unsigned char data);
unsigned char LCM_InputData(void);
bool LCM_IsBusy(void);
void LCM_RESET(bool bOn);
void LCM_Light(bool bOn);


void Boot_LCM_WR(bool bOn);
void Boot_LCM_RD(bool bOn);
void Boot_LCM_RS(bool bOn);
void Boot_LCM_CS(bool bOn);
void Boot_LCM_OutputData(unsigned char data);
unsigned char Boot_LCM_InputData(void);
bool Boot_LCM_IsBusy(void);
void Boot_LCM_RESET(bool bOn);
void Boot_LCM_Light(bool bOn);

// Voice
void IOVoiceOut(int index);

#endif
