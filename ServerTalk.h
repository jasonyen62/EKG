/*
FileName : ServerTalk.h 
ServerTalk functions 
*/
#ifndef ____SERVERTALK____
#define ____SERVERTALK____

//

extern bool volatile b_GRefreshConfig;
extern unsigned long volatile UDP_Serial;

enum TxHeader
{
	Tx_CONNECT = 0x01,
	Tx_Data = 0x02,
	Tx_ETX = 0x03,
	Tx_RTData = 0x04,
	Tx_ACK = 0x06,
	Tx_NAK = 0x15,
	Tx_TIMESYNC = 0x16,
	Tx_INFOSPLIT =  0x1c,
	Tx_DATASPLIT = 0x1F,
	Tx_DISCONNECT = 0x7F
};

//With Parking Server
unsigned int calculatePort(unsigned long ClientID);
bool PingHost(char *hostIP);

void UDPInitial(char ip[],int port,bool bBlock);
void UDPPlateServerInitial(char ip[], int port, bool bBlock); // nick add 20150727 //
void UDPNewClientInitial(char ip[], int port, bool bBlock);
void UDPProcessCommand(void);
bool ConnectServerCheck();
bool DisConnectServer();
void ACKServer(unsigned long sn);
void ACKNewClient(unsigned long sn);
void NAKServer(unsigned long sn);
bool SendInCarData(HTicketData Hdata);
bool SendOutCarData(HTicketData Hdata);
bool SendBlackListData(HTicketData Hdata,bool bIsOut);
bool SendManualOpenBarrier(OpenBrData BrData);
bool QuestDiscount(HTicketData Hdata);
bool QuestIDCardStatus(TicketData Hdata, IDCardStatus* Status); // nick add 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
void Process1017(txParktron tx);
bool SendAlarm(unsigned long AlarmID , char Message[]);
bool SendStatus(void);

bool SendDiscountData(HDiscountData data);
bool SendPaymentData(HTicketData Hdata);
//With Plate Server
char GetPlateNumber(char *plate,TicketData Hdata,bool bIsOut);

char CheckValueTicketPaid(TicketData *ticketData);					//Frank add 20120525
void SendValuePayment(TicketData *ticketData , int ValuePay);					//Frank add 20120525
void ClearVIS(void);					//Frank add 20120904
void *SendSoftTrigger2Vis(void *args); // nick add 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //

bool SendHealthCheckToNewTerimal();
bool SendLoop1TriggerToNewTerimal(unsigned long volatile udpSerial,	int isEnable);
bool SendButtonTriggerToNewTerimal(unsigned long udpSerial,	int isEnable);
bool SendCarEnterStatusToNewTerimal(unsigned long udpSerial,	int isEnter);
bool SendCarFullTriggerToNewTerimal(unsigned long udpSerial,	int fullStatus);
bool SendLoop2TriggerToNewTerimal(unsigned long udpSerial,	int isEnable);
bool SendCarEnterStartToNewTerimal(unsigned long udpSerial,	int ticketType);
bool SendRFInTriggerToNewTerimal(unsigned long udpSerial,	int isEnable);
bool SendToNewTerimal(unsigned long udpSerial,char sendData[]);

#endif
