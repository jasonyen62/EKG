/* FileName: ServerTalk.cpp
	Server Communucation Functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in_systm.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <setjmp.h>
#include <errno.h>
#include <typeinfo>

#include "CommonDef.h"
#include "Network.h"
#include "ServerTalk.h"
#include "Datafile.h"
#include "traceLog.h"

#include <map>
#include <utility> // make_pair

//#define ICMP_ECHO   	0
//#define ICMP_ECHOREPLY  8

#define PACKET_SIZE     4096
#define MAX_WAIT_TIME   5
#define MAX_NO_PACKETS  3

UDPSocket udpServer;
UDPSocket udpPlateServer; // nick add 20150807 Ver:000-000-GIO_V2-13B251-0005-13C241 //
UDPSocket udpNewTerimal;

unsigned long volatile UDP_Serial = 0L;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];
int sockfd,datalen = 56;
int nsend = 0, nreceived = 0;
struct sockaddr_in dest_addr;
pid_t pid;
struct sockaddr_in from;
struct timeval tvrecv;

bool volatile b_GServerConnect = false;
bool volatile b_GRefreshConfig = false;
bool volatile G_bMaintain = false; // nick add 20130121 //

void ConnectACK(void);

void UDPInitial(char ip[],int port,bool bBlock)
{
	int myport=0;
	
	myport = calculatePort(G_ParkingConfig.ClientID);
	printf("ServerIP:%s, port:%d, My Port:%d \n",ip,port,myport);
	udpServer.Initial(ip,port,myport,bBlock);
}

void UDPPlateServerInitial(char ip[], int port, bool bBlock) // nick add 20150727 //
{
	int myport=0;
	
	if (strlen(ip) > 7 && port > 0)
	{
		myport = calculatePort(G_ParkingConfig.ClientID) + 1;
		printf("PlateServerIP:%s, port:%d, My Port:%d \n",ip,port,myport);
		udpPlateServer.Initial(ip,port,myport,bBlock);
	}
}

void UDPNewClientInitial(char ip[], int port, bool bBlock)
{
	int myport=0;
	
	if (strlen(ip) > 7 && port > 0)
	{
		myport = calculatePort(G_ParkingConfig.ClientID) + 2;
		printf("UDPNewClientInitial:%s, port:%d, My Port:%d \n",ip,port,myport);
		udpNewTerimal.Initial(ip,port,myport,bBlock);
	}
}

// Initial Upd socket

static unsigned long syncBios=0;

void UDPProcessCommand()
{
	int len = 0, i, parms;
	int i_sysRtn=0;
	char recvData[UDPRecvBuffSize];					//Frank add 20111020
	char splitdata[5][256];
	char buf[512];
	char cmdTime[24];
	char serialNum[15];
	char LogBuffer[1024]; // nick add 20150504 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	bool bGetServerCmd = false; // nick add 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
	
	txParktron tx;
	time_t now = 0;
	static time_t lastTime = 0;
	serverCommand cmd;
	serverAck ack;
//	enum ControlCommand remoteCmd;
	 
//	struct tm *tm_ptr = NULL;
//	char *p = NULL;    
	
	memset(buf,'\0',sizeof(buf));
	memset(recvData,'\0',sizeof(recvData));
	memset(splitdata,'\0',sizeof(splitdata));
	memset(serialNum,'\0',sizeof(serialNum));
	memset(cmdTime,'\0',sizeof(cmdTime));
	memset(LogBuffer, 0, sizeof(LogBuffer)); // nick add 20150504 Ver:000-000-GIO_V2-13B251-0004-13C241 //
	
	now = time((time_t *)0);
	
	if((len = udpServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		bGetServerCmd = true; // nick add 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
		
		switch(recvData[0])
		{
			case Tx_CONNECT:
				ConnectACK();
				// ========================================================== //
				// nick mark s 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
				//lastTime = now;
				//cmd.Command = CMD_CONNECT;
				//G_SvrCmdQue.push(cmd);
				// nick mark e 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
				// ========================================================== //
				break;
			case Tx_RTData:
//nick mark 20101217				ACKServer(tx.sn);
			case Tx_Data:
				parms = udpServer.parseCommand(&tx, recvData);
				if (recvData[0] == Tx_RTData) ACKServer(tx.sn);	//nick add 20101217
				
				if(tx.commandID == 1001)
				{
					len = strlen(recvData);
					
					if(udpServer.GetParm(buf,1,tx))
					{
						cmd.Command = (enum ControlCommand)atoi(buf);
                  				
						if(udpServer.GetParm(buf,2,tx))
						{	
							sprintf(cmd.datas,"%s",buf);
						}
					
						G_SvrCmdQue.push(cmd);
					}
				}
				else if(tx.commandID == 1039)
				{					
					//if (G_NoTicketSystem == false)
					//{	
						if (G_bMaintain == false) // nick add 20130121 //
							Process1017(tx);						
					//}
				}
				else if(tx.commandID == 1014) //Refresh ini
				{
					b_GRefreshConfig = true;
				}
				else if(tx.commandID == 1054) //Status 
				{
					SendStatus();
				}
				else if(tx.commandID == 1070) //Clear reticket or Add Ticket
				{
					if(udpServer.GetParm(buf,1,tx))
					{
						if(IsINMachine==true)
						{
							cmd.Command = CMD_ADDTICKET;
						}
						else
						{
							cmd.Command = CMD_CLEAR_RETICKET;
						}
						
						sprintf(cmd.datas,"%s", buf);
						G_SvrCmdQue.push(cmd);
					}
				}
				else if(tx.commandID == 1017) //can delete data
				{
				}
				// ========================================================= //
				// nick add s 20150429 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				else if (tx.commandID == 1036)
				{
					IDCardStatus IDStatus;
					struct tm LastModifyTime;
					
					memset(&IDStatus, 0, sizeof(IDCardStatus));
					
					if (udpServer.GetParm(buf, 1, tx))
						IDStatus.TicketID = atol(buf);
					
					printf("CMD:1036, TicketID Buf:[%s], Value:[%ld]\n", buf, IDStatus.TicketID);
					
					if (udpServer.GetParm(buf, 2, tx))
						IDStatus.Status = atoi(buf);
					
					printf("CMD:1036, Status Buf:[%s], Value:[%d]\n", buf, IDStatus.Status);
					
					if (udpServer.GetParm(buf, 3, tx))
					{
						printf("GetParam:[%s]\n", buf);
						
						sscanf(buf, "%4d/%2d/%2d %2d:%2d:%2d", &LastModifyTime.tm_year, &LastModifyTime.tm_mon,
							&LastModifyTime.tm_mday, &LastModifyTime.tm_hour, &LastModifyTime.tm_min,
							&LastModifyTime.tm_sec);
						
						LastModifyTime.tm_year -= 1900;
						LastModifyTime.tm_mon -= 1;
						IDStatus.Last_Entry_Exit_Time = mktime(&LastModifyTime);
					}
					
					printf("CMD:1036, Status Buf:[%s], Value:[%ld]\n", buf, IDStatus.Last_Entry_Exit_Time);
					sprintf(LogBuffer, "Get ID Status :: TicketID:[%ld], Time:[%ld], Status:[%d]", 
						IDStatus.TicketID, IDStatus.Last_Entry_Exit_Time, IDStatus.Status);
					ShowMessage(LogBuffer);
					G_SvrIDCardStatusQue.push(IDStatus);
				}
				// nick add e 20150429 Ver:000-000-GIO_V2-13B251-0004-13C241 //
				// ========================================================= //			
				else
				{
					printf("recv:");
					
					for(i=0;i<len;i++)
					{
						printf("%02x ",recvData[i]);
					}
					
					printf("\n");
				}
				
				break;
			case Tx_TIMESYNC:
				i_sysRtn = system((char *)"./TimeSync.sh >/dev/null 2>&1");					//Frank add 20111116
				
				if(syncBios == 0)
				{
					i_sysRtn = system((char *)"./WBiosTime.sh >/dev/null 2>&1");					//Frank add 20111116
				}
				
				syncBios++;
				
				if(syncBios >= 86400L)
					syncBios=0;
				
				break;
			case Tx_ACK:
				ack.num = atol(recvData+1);
				G_SvrAckQue.push(ack);
				break;
			default:
				len = strlen(recvData);
				sprintf(buf,"Recv Len:%d . Head:%02x\n",len,recvData[0]);
				printf("%s :",buf);
				
				for(i=0;i<len;i++)
				{
					printf("%02X ",recvData[i]);
				}
				
				printf("\n");
				bGetServerCmd = false; // nick add 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
				break;
		}
		
		// ========================================================= //
		// nick add s 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
		if (bGetServerCmd)
		{
			lastTime = now;
			
			if (b_GServerConnect == false)
			{
				cmd.Command = CMD_CONNECT;
				G_SvrCmdQue.push(cmd);
			}
		}
		// nick add e 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 //
		// ========================================================= //
	}
	else
	{
		if((now - lastTime) > 20L)
		{   // 現在時間-上次ConnectACK時間 超過30秒則判斷為斷線
			b_GServerConnect = false;
		}
	}
}

bool ConnectServerCheck()
{
	char msg[64];
	char recvData[UDPRecvBuffSize];					//Frank add 20111020
	int len = 0;
//	unsigned char BCC = UDP_BCC;
	
	sprintf(msg,"\x01\x1c""%ld\x1c""%lX\x1c",UDP_Serial,G_ParkingConfig.ClientID);
	
	memset(msg,'\0',sizeof(msg));
	memset(recvData,'\0',sizeof(recvData));
	
	udpServer.udpSend(msg,strlen(msg));
	
	if((len=udpServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		if(udpServer.checkBCC(recvData,len) == true)
		{
//			serverCommand sc;
			return true;
		}
	}
	
	return false;
}

bool DisConnectServer()
{
	char msg[128];
	char recvData[UDPRecvBuffSize];			//Frank add 20111020
	int len = 0;
//	unsigned char BCC = UDP_BCC;
	
	memset(msg,'\0',sizeof(msg));
	memset(recvData,'\0',sizeof(recvData));
	
	sprintf(msg,"\x7F\x1c""%ld\x1c""%lX\x1c",UDP_Serial,G_ParkingConfig.ClientID);
	udpServer.udpSend(msg,14);
	
	if((len=udpServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		if(udpServer.checkBCC(recvData,len) == true)
		{
//			serverCommand sc;
			return true;
		}
	}
	
	return false;
}

void ACKServer(unsigned long sn)
{
	char msg[128];
//	unsigned char BCC = UDP_BCC;
	
	memset(msg,'\0',sizeof(msg));
	sprintf(msg,"\x06""%ld",sn);
	udpServer.udpSend(msg,strlen(msg));
}

void ACKNewClient(unsigned long sn)
{
	char msg[128];
	memset(msg,'\0',sizeof(msg));
	sprintf(msg,"\x06""%ld",sn);
	udpNewTerimal.udpSend(msg,strlen(msg));
}

void ConnectACK()
{
	char msg[128];
	
	memset(msg,'\0',sizeof(msg));
	sprintf(msg,"\x01""%08lX",G_ParkingConfig.ClientID);
	udpServer.udpSend(msg,strlen(msg));
}

void NAKServer(unsigned long sn)
{
	char msg[128];
//	unsigned char BCC = UDP_BCC;
	
	memset(msg,'\0',sizeof(msg));
	sprintf(msg,"\x15\x1c""%6ld\x1c""%4X\x1c",sn,G_ParkingConfig.MachineID);
	udpServer.udpSend(msg,strlen(msg));
}

unsigned int calculatePort(unsigned long ClientID)
{
	unsigned int port;
	unsigned long lbit6 = (0x1000000);
	unsigned long lbit4 = (0x10000);
	port = ClientID / lbit6 *100 + (ClientID % lbit6) / lbit4 + (ClientID % lbit6) % lbit4;
	return port;
}

bool SendInCarData(HTicketData Hdata)
{	// 傳送入車資料
	bool bRet = false;
	int i;
	char SendData[256],recvData[256];
	char datetimeString[30];
	struct tm *in_DateTime = NULL;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	in_DateTime = localtime((time_t*)&Hdata.in_time);
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		in_DateTime->tm_year + 1900, in_DateTime->tm_mon+1, in_DateTime->tm_mday,
		in_DateTime->tm_hour,in_DateTime->tm_min,in_DateTime->tm_sec);
		
	//nick add s 20110124
	char buf[128];
	
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "Get Temp Hourly InData ID:%ld  InTime:%s  Ticks:%ld", Hdata.TicketID, datetimeString, Hdata.in_time);
	ShowMessage(buf);
	//nick add e 20110124
	
	// 20120925 Tony mark s
	//sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%ld""s\x1F""%8s""s\x1F%8ss",
	//	UDP_Serial,G_ParkingConfig.ClientID,1047L,Hdata.TicketID, datetimeString,Hdata.Plate);
	// 20120925 Tony mark e

	// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%09ld""s\x1F""%8s""s\x1F%8ss",
	// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //	UDP_Serial,G_ParkingConfig.ClientID,1047L,Hdata.TicketID, datetimeString,Hdata.Plate);	// 20120925 Tony add
	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	//sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%09ld""s\x1F""%8s""s\x1F%8ss""\x1F%ds",
	//	UDP_Serial, G_ParkingConfig.ClientID, 1047L, Hdata.TicketID, datetimeString, Hdata.Plate, Hdata.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

	sprintf(SendData,
			"\x04"
			"%ld""\x1c"
			"%lX""\x1c"
			"%ld""\x1c"
			"%09ld""s\x1F"
			"%8s""s\x1F"
			"%8s""s\x1F"
			"%d""i\x1F"
			"%s""s\x1F"
			"%ld""i"
			,UDP_Serial
			,G_ParkingConfig.ClientID
			,1047L
			,Hdata.TicketID
			,datetimeString
			,Hdata.Plate
			,Hdata.AreaID
			,Hdata.TagID
			,Hdata.in_time
			);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	usleep(15000L);
	
	serverAck ack;
	
	unsigned long sn=0L;
	
	for(i=0;i<10;i++)
	{	//1秒 沒回應	
		if(G_SvrAckQue.empty() == false)
		{	//Do Server Command
			ack = (serverAck)(G_SvrAckQue.front());
			sn = ack.num;
			G_SvrAckQue.pop();
			bRet = true;
			break;
		}
		
		usleep(100000L);
	}
	
	UDP_Serial++;
	return bRet;
}

bool SendOutCarData(HTicketData Hdata)
{	// 傳送出車資料
	bool bRet=false;
	int i;
	char SendData[256],recvData[256];
	char datetimeString[30];
	struct tm *out_DateTime = NULL;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	out_DateTime = localtime((time_t*)&Hdata.in_time);
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		out_DateTime->tm_year + 1900, out_DateTime->tm_mon+1, out_DateTime->tm_mday,
		out_DateTime->tm_hour,out_DateTime->tm_min,out_DateTime->tm_sec);
		
	//nick add s 20110124
	char buf[128];
	
	memset(buf, '\0', sizeof(buf));
	sprintf(buf, "Get Temp Hourly OutData ID:%ld  InTime:%s  Ticks:%ld", Hdata.TicketID, datetimeString, Hdata.in_time);
	ShowMessage(buf);
	//nick add e 20110124
	
	// 20120925 Tony mark s
	//sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%ld""s\x1F""%8s""s\x1F%8ss",
	//	UDP_Serial++,G_ParkingConfig.ClientID,1048L,Hdata.TicketID, datetimeString,Hdata.Plate);
	// 20120925 Tony mark e
	
	// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%09ld""s\x1F""%8s""s\x1F%8ss",
	// nick mark 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //	UDP_Serial++,G_ParkingConfig.ClientID,1048L,Hdata.TicketID, datetimeString,Hdata.Plate);	// 20120925 Tony add

	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	//sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%09ld""s\x1F""%8s""s\x1F%8ss""\x1F%ds",
	//	UDP_Serial++, G_ParkingConfig.ClientID, 1048L, Hdata.TicketID, datetimeString, Hdata.Plate, Hdata.AreaID); // nick add 20160524 Ver:000-000-GIO_V2-13B251-0013-13C241 //
	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)

	sprintf(SendData,
			"\x04"
			"%ld\x1c"
			"%lX""\x1c"
			"%ld""\x1c"
			"%09ld""s\x1F"
			"%8s""s\x1F"
			"%8s""s\x1F"
			"%d""i\x1F"
			"%s""s\x1F"
			"%ld""i"
			,UDP_Serial++
			,G_ParkingConfig.ClientID
			,1048L
			,Hdata.TicketID
			,datetimeString
			,Hdata.Plate
			,Hdata.AreaID
			,Hdata.TagID
			,Hdata.in_time
			);	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output)
	
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	usleep(10000L);
	
	serverAck ack;
	
	unsigned long sn = 0L;
	
	for(i=0; i<10; i++)
	{	//1秒 沒回應	
		if(G_SvrAckQue.empty() == false)
		{	//Do Server Command
			ack = (serverAck)(G_SvrAckQue.front());
			sn = ack.num;	
			G_SvrAckQue.pop();
			bRet = true;
			break;
		}
		
		usleep(100000L);
	}
	
	return bRet;
}

bool SendBlackListData(HTicketData Hdata,bool bIsOut)
{ 	// 送 黑名單
	bool bRet=false;
	int i;
	char SendData[256],recvData[256];
	char datetimeString[30];
	struct tm *out_DateTime = NULL;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	//if(bIsOut)
	//	out_DateTime = localtime((time_t*)&Hdata.out_time);
	//else
		out_DateTime = localtime((time_t*)&Hdata.in_time);
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		out_DateTime->tm_year+1900, out_DateTime->tm_mon+1, out_DateTime->tm_mday,
		out_DateTime->tm_hour,out_DateTime->tm_min,out_DateTime->tm_sec);
		
	sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%ld""s\x1F""%8s""s\x1F%8sd",
		UDP_Serial++,G_ParkingConfig.ClientID,1049L,Hdata.TicketID,Hdata.Plate, datetimeString);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	usleep(10000L);
	
	serverAck ack;
	
	unsigned long sn=0L;
	
	for(i=0;i<10;i++)
	{	//1秒 沒回應
		if(G_SvrAckQue.empty() == false)
		{	//Do Server Command
			ack = (serverAck)(G_SvrAckQue.front());
			sn = ack.num;
			G_SvrAckQue.pop();
			bRet = true;
			break;
		}
		
		usleep(100000L);
	}
	
	return bRet;
}

bool SendManualOpenBarrier(OpenBrData BrData)
{	//  傳送手動開柵欄資料
	bool bRet = false;
	int i;
	char SendData[256],recvData[256];
	char datetimeString[30];
	struct tm *out_DateTime=NULL;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	out_DateTime = localtime((time_t*)&BrData.OpenTime);
	
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		out_DateTime->tm_year+1900, out_DateTime->tm_mon+1, out_DateTime->tm_mday,
		out_DateTime->tm_hour,out_DateTime->tm_min,out_DateTime->tm_sec);
	//printf("Open barrier timestring:%s\n",datetimeString);
	sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%ld""s\x1F""%8s""s\x1F%dd""\x1F""i\x1F""********s",
		UDP_Serial,G_ParkingConfig.ClientID,1050L,BrData.sn, datetimeString,1);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	usleep(100000L);
	
	serverAck ack;
	
	unsigned long sn=0L;
	
	for(i=0;i<10;i++)
	{	//1秒 沒回應
		if(G_SvrAckQue.empty() == false)
		{	//Do Server Command
			ack = (serverAck)(G_SvrAckQue.front());
			sn = ack.num;
			G_SvrAckQue.pop();
			bRet = true;
			break;
		}
		
		usleep(100000L);
	}
	
	UDP_Serial++;
	return bRet;
}

bool QuestDiscount( HTicketData Hdata)
{   //要求折扣資料
	char SendData[256];
	char recvData[UDPRecvBuffSize];					//Frank add 20111020
	char datetimeString[30];
	struct tm *in_DateTime = NULL;
	int len;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	in_DateTime = localtime((time_t*)&Hdata.in_time);
	sprintf(datetimeString,"%04d//%02d//%02d %02d:%02d:%02d",
		in_DateTime->tm_year+1900, in_DateTime->tm_mon+1, in_DateTime->tm_mday,
		in_DateTime->tm_hour,in_DateTime->tm_min,in_DateTime->tm_sec);
		
	sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%d""i\x1F""%ld""i\x1F%sd""\x1F""%ss",
		UDP_Serial++, G_ParkingConfig.ClientID, 1035L, G_ParkingConfig.AreaID, Hdata.TicketID, datetimeString, Hdata.Plate);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	if((len=udpServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		printf("Send Discount Receive:\n");
	}
	
	return true;
}

// ========================================================= //
// nick add s 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
bool QuestIDCardStatus(TicketData Hdata, IDCardStatus* TickStatus)
{
	char SendData[256];
	unsigned long Ticks;
	bool bRtn = false;
	serverAck ack;
	IDCardStatus CardStatus;

	memset(TickStatus, 0, sizeof(IDCardStatus));
	memset(&CardStatus, 0, sizeof(IDCardStatus));
	memset(&ack, 0, sizeof(serverAck));
	memset(SendData,'\0',sizeof(SendData));
	
	// 清空 G_SvrAckQue //
	while (G_SvrAckQue.empty() == false)
	{
		G_SvrAckQue.pop();
		usleep(1);
	}
	//
	// 清空 G_SvrIDCardStatusQue //
	while (G_SvrIDCardStatusQue.empty() == false)
	{
		G_SvrIDCardStatusQue.pop();
		usleep(1);
	}
	//
	Ticks = GetTickCount();
	sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%ld""i",
		UDP_Serial++, G_ParkingConfig.ClientID, 1036L, Hdata.ticketID);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	// 等待Server Ack //
	while(CheckTimeout(&Ticks, 1000) == false)
	{
		if (G_SvrAckQue.empty() == false)
		{
			ack = (serverAck)G_SvrAckQue.front();
			G_SvrAckQue.pop();
			break;
		}
		
		usleep(1);
	}
	//
	
	if (ack.num == 0)
		goto FUNCEXIT;
	
	// 等待Server回應 //
	Ticks = GetTickCount();
	
	while(CheckTimeout(&Ticks, 3000) == false)
	{
		if (G_SvrIDCardStatusQue.empty() == false)
		{
			CardStatus = (IDCardStatus)(G_SvrIDCardStatusQue.front());
			
			if (CardStatus.TicketID == Hdata.ticketID)
			{
				memcpy(TickStatus, &CardStatus, sizeof(IDCardStatus));
				printf("Get Server Reply, TicketID:[%ld], SeasonCardID:[%ld]\n", CardStatus.TicketID, Hdata.ticketID);
				bRtn = true;
				break;
			}
			
			G_SvrIDCardStatusQue.pop();
		}
		
		usleep(1);
	}
	//
	
FUNCEXIT:
	
	return (bRtn);
}
// nick add e 20150428 Ver:000-000-GIO_V2-13B251-0004-13C241 //
// ========================================================= //

bool SendDiscountData(HDiscountData Ddata)
{   //傳送折扣資料
	bool bRet = false;
	char SendData[256];
	char recvData[256];
	char datetimeString[30];
	struct tm *in_DateTime = NULL;
	int i;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	in_DateTime = localtime((time_t*)&Ddata.DiscountTime);
	//Frank mark 20130114 sprintf(datetimeString,"%04d//%02d//%02d %02d:%02d:%02d",
	sprintf(datetimeString , "%04d/%02d/%02d %02d:%02d:%02d" ,					//Frank add 20130115
		in_DateTime->tm_year+1900, in_DateTime->tm_mon+1, in_DateTime->tm_mday,
		in_DateTime->tm_hour,in_DateTime->tm_min,in_DateTime->tm_sec);
		
	sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c"
					"%di\x1F"
					"%ss\x1F"
					"%di\x1F"
					"%sd\x1F"
					"%di\x1F"
					"%di\x1F"
					"%di\x1F"
					"%di\x1F"
					"%di\x1F"
					"%di\x1F"
					"%ss\x1F"
					//Frank mark 20130114 "%di\x1F"
					"%ldi\x1F"
					"%ldi\x1F"
					"%di\x1F"
					"%ss\x1F"
					"%di\x1F"
					"%di\x1F"
					"%di\x1F"
					"%ss",
		UDP_Serial, G_ParkingConfig.ClientID, 1034L, 
		G_ParkingConfig.AreaID,
		"0",
		Ddata.AID,
		datetimeString,
		0,
		Ddata.Type,
		0,
		0,
		Ddata.DiscountMinute,
		0,
		"",
		//Frank mark 20130114 0,
		Ddata.Point ,					//Frank add 20130114
		Ddata.TicketID,
		Ddata.VID,
		"",
		Ddata.SID,
		Ddata.RealDiscountMinute,
		0,
		""
	);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	usleep(100000L);
	
	serverAck ack;
	
	unsigned long sn=0L;
	
	for(i=0;i<10;i++)
	{	//1秒 沒回應	
		if(G_SvrAckQue.empty() == false)
		{	//Do Server Command
			ack = (serverAck)(G_SvrAckQue.front());
			sn = ack.num;
			G_SvrAckQue.pop();
			bRet = true;
			break;
		}
		
		usleep(100000L);
	}
	
	UDP_Serial++;
	return bRet;
}

bool SendPaymentData(HTicketData Hdata)
{   //傳送繳費資料
	char SendData[256];
	char recvData[256];
	char datetimeString[30];
	char PayTimeString[30];					//Frank add 20130116
	struct tm *in_DateTime = NULL;
	struct tm *Pay_DateTime = NULL;					//Frank add 20130116
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	memset(PayTimeString , '\0' , sizeof(PayTimeString));					//Frank add 20130116
	
	in_DateTime = localtime((time_t*)&Hdata.in_time);
	// 20110712 Tony mark sprintf(datetimeString,"%04d//%02d//%02d %02d:%02d:%02d",
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		in_DateTime->tm_year+1900, in_DateTime->tm_mon+1, in_DateTime->tm_mday,
		in_DateTime->tm_hour,in_DateTime->tm_min,in_DateTime->tm_sec);
		
	//Frank add s 20130116
	Pay_DateTime = localtime((time_t*)&Hdata.pay_time);
	sprintf(PayTimeString , "%04d/%02d/%02d %02d:%02d:%02d" , Pay_DateTime ->tm_year+1900 , Pay_DateTime ->tm_mon+1 ,
						Pay_DateTime ->tm_mday , Pay_DateTime ->tm_hour , Pay_DateTime ->tm_min , Pay_DateTime ->tm_sec);
	//Frank add e 20130116
	
	//Frank mark 20130116 sprintf(SendData,"\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%d""i\x1F""%ld""i\x1F%sd""\x1F""%ss",
		//Frank mark 20130116 UDP_Serial++, G_ParkingConfig.ClientID, 1035L, G_ParkingConfig.AreaID, Hdata.TicketID, datetimeString, Hdata.Plate);

	// ==================== //
	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) //
	//Frank add s 20130116
	//sprintf(SendData , "\x02""%ld\x1c""%lX""\x1c""%ld""\x1c"
	//				"%ld""i\x1F"
	//				"%8s""d\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%s""s\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%8s""d\x1F"
	//				"%8s""s\x1F"
	//				"%d""i\x1F"
	//				"%s""s\x1F"
	//				"%s""s\x1F"
	//				"%x""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%d""i\x1F"
	//				"%sd",
	//				UDP_Serial++ , G_ParkingConfig.ClientID , 1032L ,
	//				Hdata.TicketID ,							//tick_id
	//				PayTimeString ,							//payment_date_time
	//				0 ,										//Amount
	//				0 ,										//DeValue
	//				0 ,										//Discount
	//				0 ,										//Shift_id
	//				0 ,										//invoice_id
	//				"" ,										//invoice_track
	//				0 ,										//disc_type
	//				0 ,										//Charge_amount
	//				0 ,										//payment_card_id
	//				0 ,										//OnDuty
	//				datetimeString ,							//payment_in_time
	//				Hdata.Plate ,								//plate_number
	//				0 ,										//debt_amount
	//				"STOP_PAYMENT" ,							//payment_subject
	//				"" ,										//Note
	//				G_ParkingConfig.AreaID ,					//Area_ID
	//				0 ,										//ShowNote
	//				0 ,										//Remaining_Amount
	//				0 ,										//CancelInvObj
	//				"1900/01/01 00:00"						//CancelInvTime
	//);
	//Frank add e 20130116
	// Tony mark 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) //
	// ==================== //

	// =================== //
	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) //
	sprintf(SendData , "\x02""%ld\x1c""%lX""\x1c""%ld""\x1c"
						"%ld""i\x1F"
						"%8s""d\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%s""s\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%8s""d\x1F"
						"%8s""s\x1F"
						"%d""i\x1F"
						"%s""s\x1F"
						"%s""s\x1F"
						"%x""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%d""i\x1F"
						"%s""d\x1F"
						"%s""s\x1F"
						"%s""s\x1F"
						"%s""s\x1F"
						"%s""s",
						UDP_Serial++ , G_ParkingConfig.ClientID , 1032L ,
						Hdata.TicketID ,							//tick_id
						PayTimeString , 						//payment_date_time
						0 , 									//Amount
						0 , 									//DeValue
						0 , 									//Discount
						0 , 									//Shift_id
						0 , 									//invoice_id
						"" ,										//invoice_track
						0 , 									//disc_type
						0 , 									//Charge_amount
						0 , 									//payment_card_id
						0 , 									//OnDuty
						datetimeString ,							//payment_in_time
						Hdata.Plate ,								//plate_number
						0 , 									//debt_amount
						"STOP_PAYMENT" ,							//payment_subject
						"" ,										//Note
						G_ParkingConfig.AreaID ,					//Area_ID
						0 , 									//ShowNote
						0 , 									//Remaining_Amount
						0 , 									//CancelInvObj
						"1900/01/01 00:00",						//CancelInvTime
						"",
						"",
						"",
						Hdata.TagID								//TagID 
		);

	// Tony add 20161122 Ver:000-000-GIO_V2-13C241-0001-16B221 (2-Output) //
	// =================== //
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	return true;
}

bool SendStatus()
{
	int TxID = 1006;
	char SendData[512];
	char datetimeString[30];
	struct tm* now_DateTime = NULL;
	time_t now;
	int tickets=0;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(datetimeString,'\0',sizeof(datetimeString));
	
	now = time((time_t *)0);
	now_DateTime = localtime(&now);
	
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		now_DateTime->tm_year+1900, now_DateTime->tm_mon+1, now_DateTime->tm_mday,
		now_DateTime->tm_hour,now_DateTime->tm_min,now_DateTime->tm_sec);
		
	if(IsINMachine==true)
	{
		TxID = 1005;
	}
	if(IsINMachine==true)
	{
		tickets = G_ParkingStatus.TicketReserve;
	}
	else
	{
		tickets = G_ParkingStatus.Out_Retrieved;
	}
	
	sprintf(SendData,"\x02%06ld\x1c""%08lX""\x1c""%04d""\x1c" 
		"%s""d\x1F""%d""i\x1F"
		"%d""i\x1F"
		"%d""i\x1F"
		"%d""i\x1F"
		"%s""b\x1F"
		"%s""b\x1F"
		"%s""b\x1F"
		"%s""b\x1F"
		"%s""b\x1F"
		"%s""b\x1F""%s""b\x1F""%s""b\x1F""%s""b\x1F""%d""i\x1F""%d""i",
        UDP_Serial++,G_ParkingConfig.ClientID,TxID,
        datetimeString, ((G_ParkingStatus.status & STATUS_GATE_ON_SERVICE)==0) ? 0:1,
        ((G_ParkingStatus.status & STATUS_PARKING_FREE)==0) ? 1:0,
        tickets,
        G_ParkingStatus.status & STATUS_READER_CONNECT ? 1:0,
        G_ParkingStatus.status & STATUS_BARRIER_DOWN ? "True":"False",
        G_ParkingStatus.status & STATUS_GATE_ON_SERVICE ? "False":"True",
        G_ParkingStatus.status & STATUS_MACHINE_OPEN ? "True":"False",
        G_ParkingStatus.status & STATUS_TICKET_LOW ? "True":"False",
        G_ParkingStatus.status & STATUS_TICKET_LOW ? "True":"False",
        "False","False","False","False",0,0);

	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	return true;
}

bool SendAlarm(unsigned long AlarmID , char Message[])
{
//	int len,i;
	char SendData[256], recvData[256];
	char datetimeString[30];
	struct tm *now_DateTime = NULL;
	
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(datetimeString,'\0',sizeof(datetimeString));

	// ========================================================= //
	// nick add s 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
	char buff[256];
	
	memset(buff, 0, sizeof(buff));
	
	if (AlarmID <= 1000)
		sprintf(buff, "Alarm Enable:%ld", AlarmID);
	else
		sprintf(buff, "Alarm Disnable:%ld", AlarmID);
	
	ShowMessage(buff);
	// nick add e 20131113 Ver:000-000-GIO_V2-133181-0103-135101 //
	// ========================================================= //
	
	if(b_GServerConnect == false)
		return false;
	
	time_t now;
	now = time((time_t *)0);
	now_DateTime = localtime(&now);
	
	sprintf(datetimeString,"%04d/%02d/%02d %02d:%02d:%02d",
		now_DateTime->tm_year+1900, now_DateTime->tm_mon+1, now_DateTime->tm_mday,
		now_DateTime->tm_hour,now_DateTime->tm_min,now_DateTime->tm_sec);
	
	sprintf(SendData,"\x02""%ld\x1c""%08lX""\x1c""%ld""\x1c""%ld""i\x1F""%s""d\x1F%ss",
		UDP_Serial++,G_ParkingConfig.ClientID,1045L,AlarmID, datetimeString,Message);
	udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
	
	return true;
}

void Process1017(txParktron tx)
{   //回應 1049 黑名單
	char SendData[256];
	char cmdTime[24];
	char serialNum[15];
	char SQL[256];
	
	memset(SQL,'\0',sizeof(SQL));
	memset(SendData,'\0',sizeof(SendData));
	
	if(udpServer.GetParm(SQL,3,tx)==true)
	{
		if(RunSQL(SQL) == true)
		{	//send 1017
			memset(cmdTime,'\0',sizeof(cmdTime));
			memset(serialNum,'\0',sizeof(serialNum));
			udpServer.GetParm(serialNum,4,tx);
			udpServer.GetParm(cmdTime,5,tx);
			//printf("1017ret %ld : [%s ,%s]\n",tx.commandID,serialNum,cmdTime);
			sprintf(SendData,"\x02""%ld\x1c""%08lX""\x1c""%ld""\x1c""%hd""i\x1F""%s""i\x1F%sd",
				UDP_Serial++, G_ParkingConfig.ClientID, 1017L, tx.commandID, serialNum,cmdTime);
				
			udpServer.udpSendWithBccEnd(SendData,strlen(SendData));
		}
	}
}

/* ================================================================
	To Car Plate Server
   ================================================================
   return
   -1:不允許進(出)場
   -2: 通訊錯
	-3: server 有回應ack無須retry					//Frank add 20120827
   0: 伺服器無回應
   1: 可(出)進場
	2: 不需要車牌辨識 // nick add 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
*/

char GetPlateNumber(char *plate,TicketData Hdata, bool bIsOut)
{	// Get Plate Number from Plate Server
	//Frank mark 20120814 int iRet=0,i;
	int iRet = 0;					//Frank add 20120814
	char CarPlate[10];
	char Pass[10];
	char SendData[256];
	char recvData[UDPRecvBuffSize];					//Frank add 20111020
	int myport,len;
	//UDPSocket udpPlateServer;
	txParktron tx;
	char buf[UDPRecvBuffSize];					//Frank add 20120806
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Ticks = GetTickCount();					//Frank add 20120814
	unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	// nick mark 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //myport = calculatePort(G_ParkingConfig.ClientID);
	// nick mark 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //udpPlateServer.Initial(G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort, myport+1,false);
	//printf("plate IP:%s port:%d myport:%d\n",G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort,myport+1);
	
	memset(CarPlate,'\0',sizeof(CarPlate));
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(Pass,'\0',sizeof(Pass));
	memset(buf,'\0',sizeof(buf));					//Frank add 20120806
	
	// ========================================================= //
	// nick add s 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
	if (strlen(G_ParkingConfig.PlateServerIP) <= 7) return(2);
	
	if ((Hdata.ticketID % 10L) == 2 || (Hdata.ticketID % 10L) == 3)
	{
		if (G_ParkingConfig.VISforSeason == false) return(2);
	}
	else
	{
		if (G_ParkingConfig.VISforHourly == false) return(2);
	}
	
	myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial(G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort, myport+1,false);
	// nick add e 20140127 Ver:000-000-GIO_V2-135101-0000-13B250 //
	// ========================================================= //
	
	udpPlateServer.Clearbuf();					//Frank add 20120904
	
	//Command ID = 1046
	//Frank mark 20120806 if(bIsOut)
	if(bIsOut || (Hdata.ticketID % 10L) == 2 || (Hdata.ticketID % 10L) == 3)	//Frank add 20120806
	{
		//Frank mark 20120814 sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%ld""i\x1F""%8ss",UDP_Serial,G_ParkingConfig.ClientID,1046,Hdata.ticketID, Hdata.plate);
		sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%ld""i\x1F""%ss" ,
						UDP_Serial,G_ParkingConfig.ClientID,1046,Hdata.ticketID, Hdata.plate);//Frank add 20120814
	}
	else
	{
		sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%ld""i\x1F""%8ss" , 
						UDP_Serial,G_ParkingConfig.ClientID,1046,Hdata.ticketID, "********");
	}
	
	udpPlateServer.udpSendWithBccEnd(SendData,strlen(SendData));
	UDP_Serial++;
	usleep(200000L);
	
	if((len = udpPlateServer.udpRecv(recvData)) > 0)
	{ 	//Check format.  Do server command
		if(recvData[0] == 0x06)
		{
			//Frank mark 20120814 for(i=0;i<G_ParkingConfig.PlateTimeout*10;i++)
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //			while(CheckTimeout(Ticks, (long)(G_ParkingConfig.PlateTimeout * 1000)) == false)			//Frank add 20120814
			while(CheckTimeout(&Ticks, (unsigned long)(G_ParkingConfig.PlateTimeout * 1000)) == false) // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101
			{
				//Frank mark 20120814 usleep(150000L);
				usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
				
				if((len=udpPlateServer.udpRecv(recvData)) > 0)
				{
					//printf("Recv: plate len%d\n",len);
					//for(j=0;j<strlen(recvData);j++)
					//{
					//	printf("%02X ",recvData[j]);
					//}
					//printf("\n");
					
					if(recvData[0] == 0x04)
					{
						udpPlateServer.parseCommand(&tx, recvData);
						
						memset(SendData,'\0',sizeof(SendData));
						
						//nick mark 20120905 sprintf(SendData,"\x06""1");
						sprintf(SendData , "\x06""%ld" , tx.sn);		//nick add 20120905
						//Frank mark 20120905 udpPlateServer.udpSend(SendData,2);
						udpPlateServer.udpSend(SendData , strlen(SendData));					//Frank add 20120905
						udpPlateServer.GetParm(CarPlate,2,tx);
						udpPlateServer.GetParm(Pass,4,tx);
						//printf("Pass:[%s] \n",Pass);
						memcpy(plate, CarPlate,strlen(CarPlate));
						
						if(strcmp(Pass,"True")==0)
						{
							iRet = 1;
						}
						else
						{
							iRet = -1;
						}
						
						break;
					}
					else
						continue;
				}
			}
			if (iRet == 0)	iRet = -3;					//Frank add 20120827
		}
		else
		{
			iRet = -2;
			printf("Plate response Error Format!!!\n");
		}
	}
	else
	{
		//ShowMessage("Plate Server No ACK!!");
		iRet = 0;
	}
	
	return iRet;
}

char GetPlateNumberTcp(char *plate,TicketData Hdata, bool bIsOut)
{	// Get Plate Number from Plate Server
	int iRet=0,i;
	char CarPlate[10];
	char Pass[10];
	char SendData[256];
	char recvData[256];
	char buf[5];
	int myport,len;
	int sockfd;
	struct sockaddr_in dest;
	UDPSocket udpPlateServer;
	txParktron tx;
	
	myport = calculatePort(G_ParkingConfig.ClientID);
	//udpPlateServer.Initial(G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort, myport+1,false);
	//printf("plate IP:%s port:%d myport:%d\n",G_ParkingConfig.PlateServerIP,G_ParkingConfig.PlateServerPort,myport+1);
	/* create socket */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	
	/* initialize value in dest */
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(G_ParkingConfig.PlateServerPort);
	inet_aton(G_ParkingConfig.PlateServerIP, &dest.sin_addr);
	
	if(connect(sockfd, (struct sockaddr*)&dest, sizeof(dest))==0)
		return 0;
	
	memset(CarPlate,'\0',sizeof(CarPlate));
	memset(SendData,'\0',sizeof(SendData));
	memset(recvData,'\0',sizeof(recvData));
	memset(Pass,'\0',sizeof(Pass));
	
	//Command ID = 1046
	if(bIsOut)
	{
		sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%ld""i\x1F""%8ss",UDP_Serial,G_ParkingConfig.ClientID,1046,Hdata.ticketID, Hdata.plate);
	}
	else
	{
		sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%ld""i\x1F""%8ss",UDP_Serial,G_ParkingConfig.ClientID,1046,Hdata.ticketID, "********");
	}
	
	unsigned char BCC = 0x5A;
	
	len = strlen(SendData);
	
	for(i=1;i<len;i++)
	{
		BCC ^= SendData[i];
	}
	
	sprintf(buf,"%02X",BCC);
	SendData[len] = buf[0];
	SendData[len+1] = buf[1];
	SendData[len+2] = 0x03;
	
	send(sockfd, SendData, strlen(SendData), 0);
	UDP_Serial++;
	usleep(200000L);
	
	if((len = recv(sockfd, recvData, sizeof(recvData), 0)) > 0)
	{ 	//Check format.  Do server command
		if(recvData[0] == 0x06)
		{		
			for(i=0;i<G_ParkingConfig.PlateTimeout*10;i++)
			{
				usleep(150000L);
				bzero(recvData, sizeof(recvData));
				
				if((len=recv(sockfd, recvData, sizeof(recvData), 0)) > 0)
				{
					//printf("Recv: plate len%d\n",len);
					//for(j=0;j<strlen(recvData);j++)
					//{
					//	printf("%02X ",recvData[j]);
					//}
					//printf("\n");
					
					if(recvData[0] == 0x04)
					{
						udpPlateServer.parseCommand(&tx, recvData);
						
						memset(SendData,'\0',sizeof(SendData));
						//nick mark 20120905 sprintf(SendData,"\x06""1");
						sprintf(SendData,"\x06""%ld", tx.sn);		//nick add 20120905
						//udpPlateServer.udpSend(SendData,2);
						//Frank mark 20120905 send(sockfd, SendData, 2, 0);  //回應
						send(sockfd , SendData , strlen(SendData) , 0);  //回應					//Frank add 20120905
						udpPlateServer.GetParm(CarPlate,2,tx);
						udpPlateServer.GetParm(Pass,4,tx);
						//printf("Pass:[%s] \n",Pass);
						memcpy(plate, CarPlate,strlen(CarPlate));
						
						if(strcmp(Pass,"True")==0)
						{
							iRet = 1;
						}
						else
						{
							iRet = -1;
						}
						
						break;
					}
					else 
						continue;
				}
			}
		}
		else 
		{
			iRet = -2;
			printf("Plate response Error Format!!!\n");
		}
	}
	else
	{
		//ShowMessage("Plate Server No ACK!!");
		iRet = 0;
	}
	
	close(sockfd);
	return iRet;
}

//Frank add s 20120525
char CheckValueTicketPaid(TicketData *ticketData)
{	// 1:扣款成功  0:server 無回應  -1:餘額不足  -2:BCC 錯誤
	int len , i , ValuePay , iRet = 0 , x;
	char SendData[256] , RecvData[UDPRecvBuffSize] , InDateData[32] , OutDateData[32] , ValueTen[2] , Valueid[2] , buf[128];
	txParktron tx;
	struct tm *tm_ptr = NULL;
	time_t now;
	unsigned char BCC = UDP_BCC;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long Ticks = GetTickCount();
	unsigned long Ticks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	if((long)(ticketData->value) < 0)
		return iRet = -1;
		
	memset(SendData , '\0' , sizeof(SendData));
	memset(InDateData , '\0' , sizeof(InDateData));
	memset(OutDateData , '\0' , sizeof(OutDateData));
	memset(ValueTen , '\0' , sizeof(ValueTen));
	memset(Valueid , '\0' , sizeof(Valueid));
	
	now = time((time_t *)0);
	tm_ptr = localtime(&now);
	ticketData->out_year		= tm_ptr->tm_year + 1900;
	ticketData->out_month	= tm_ptr->tm_mon + 1;
	ticketData->out_day		= tm_ptr->tm_mday;
	ticketData->out_hour		= tm_ptr->tm_hour;
	ticketData->out_min		= tm_ptr->tm_min;
	ticketData->out_sec		= tm_ptr->tm_sec;					//Frank add 20120725
	
	sprintf(InDateData , "%4d/%02d/%02d %02d:%02d" ,
		ticketData->in_year , ticketData->in_month , ticketData->in_day , ticketData->in_hour , ticketData->in_min);
	sprintf(OutDateData , "%4d/%02d/%02d %02d:%02d" ,
		ticketData->out_year , ticketData->out_month , ticketData->out_day , ticketData->out_hour , ticketData->out_min);
	sprintf(SendData , "\x04""%ld\x1c""%lX""\x1c""%ld""\x1c""%8s""d\x1F""%8s""d\x1F""%ldi",
					UDP_Serial++ , G_ParkingConfig.ClientID , 1075L , InDateData , OutDateData , ticketData->ticketID);
					
	udpServer.udpSendWithBccEnd(SendData , strlen(SendData));
	
	while(CheckTimeout(&Ticks, (unsigned long)5000) == false)
	{
		memset(RecvData , '\0' , sizeof(RecvData));
		
		len = (udpServer.udpRecv(RecvData));
		
		if(len > 0)
		{
			if (RecvData[0] == Tx_Data)
			{
				udpServer.parseCommand(&tx , RecvData);
				
				if(tx.commandID == 1075)
				{
					for(i = 1 ; i < len - 3 ; i++)
					{
						BCC ^= RecvData[i];
					}
					
					ValueTen[0] = RecvData[len - 3];
					Valueid[0] = RecvData[len - 2];
					
					if(Valueid[0] >= 'A' && Valueid[0] <= 'F')
					{
						switch(Valueid[0])
						{
							case 'A':
								x = 10;
								break;
							case 'B':
								x = 11;
								break;
							case 'C':
								x = 12;
								break;
							case 'D':
								x = 13;
								break;
							case 'E':
								x = 14;
								break;
							case 'F':
								x = 15;
								break;
						}
					}
					else
					{
						x = atoi(Valueid);
					}
					
					if((int)BCC != (atoi(ValueTen) * 16 + x))
					{
						ShowMessage((char *)"BCC fail!");
						return iRet = -2;
					}
					
					ValuePay = atoi(tx.parms);
					ticketData->value = ticketData->value - ValuePay;
					
					if((long)(ticketData->value)  >= 0)
					{
						sprintf(buf , "After pay value is : %ld" , ticketData->value);
						ShowMessage(buf);
						SendValuePayment(ticketData , ValuePay);
						WriteTempValueData(ticketData , ValuePay);
						return iRet = 1;
					}
					else
					{
						return iRet = -1;
					}
				}
			}
		}
		
		if(b_GServerConnect == 0)
			break;

		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	ShowMessage((char *)"Server not response!");
	return iRet;
}

void SendValuePayment(TicketData *ticketData , int ValuePay)
{
	char SendData[512] , OutDateData[32] , InDateData[32];
	
	memset(SendData , '\0' , sizeof(SendData));
	memset(InDateData , '\0' , sizeof(InDateData));
	memset(OutDateData , '\0' , sizeof(OutDateData));
	
	sprintf(InDateData,"%4d/%02d/%02d %02d:%02d",
		ticketData->in_year , ticketData->in_month , ticketData->in_day , ticketData->in_hour , ticketData->in_min);
	sprintf(OutDateData,"%4d/%02d/%02d %02d:%02d" ,
		ticketData->out_year , ticketData->out_month , ticketData->out_day , ticketData->out_hour , ticketData->out_min);
	sprintf(SendData , "\x02""%ld\x1c""%lX""\x1c""%ld""\x1c"
					"%ld""i\x1F"
					"%8s""d\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%s""s\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%d""i\x1F"
					"%8s""d\x1F"
					"%8s""s\x1F"
					"%d""i\x1F"
					"%s""s\x1F"
					"%s""s\x1F"
					"%x""i\x1F"
					"%d""i\x1F"
					"%ld""i\x1F"
					"%d""i\x1F"
					"%sd",
					UDP_Serial++ , G_ParkingConfig.ClientID , 1032L ,
					ticketData->ticketID ,						//TicketID
					OutDateData ,								//Time
					0 ,										//Money
					ValuePay ,								//DeValue
					0 ,										//Discount
					0 ,										//ReMoney
					0 ,										//InvoiceID
					"" ,										//InvoiceTrack
					0 ,										//DiscType
					0 ,										//ChargeAmount
					0 ,										//MemberID
					0 ,										//OnDuty
					InDateData ,								//InTime
					ticketData->plate ,						//PlateNum
					0 ,										//DebtAmount
					"STOP_PAYMENT" ,							//PaySubType
					"" ,										//Note
					G_ParkingConfig.AreaID ,					//AreaID
					0 ,										//ShowNoteFlag
					ticketData->value ,						//Remaining_Amount
					0 ,										//CancelObj
					"1900/01/01 00:00"						//CancelTime
			);
			
	udpServer.udpSendWithBccEnd(SendData , strlen(SendData));
}
//Frank add e 20120525

//Frank add s 20120904
void ClearVIS()
{
	char SendData[256];
	int myport;
	UDPSocket udpPlateServer;

	if (strlen(G_ParkingConfig.PlateServerIP) <= 7 || G_ParkingConfig.PlateServerPort <= 0) return; // nick add 20130205 //
	
	myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial(G_ParkingConfig.PlateServerIP , G_ParkingConfig.PlateServerPort , myport + 1 , false);
	
	memset(SendData , '\0' , sizeof(SendData));
	
	// nick mark 20150523 Ver:000-000-GIO_V2-13B251-0005-13C241 //sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%di" ,
	// nick mark 20150523 Ver:000-000-GIO_V2-13B251-0005-13C241 //				UDP_Serial,G_ParkingConfig.ClientID , 1052 , G_ParkingConfig.MachineID);
	
	sprintf(SendData,"\x02""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%di" ,
		UDP_Serial,G_ParkingConfig.ClientID , 1052 , G_ParkingConfig.MachineID); // nick add 20150523 Ver:000-000-GIO_V2-13B251-0005-13C241 //
	
	udpPlateServer.udpSendWithBccEnd(SendData , strlen(SendData));
	UDP_Serial++;
}
//Frank add e 20120904

void *SendSoftTrigger2Vis(void *args) // nick add 20150521 Ver:000-000-GIO_V2-13B251-0005-13C241 //
{
	char SendData[256];
	char buf[512];
	char recvData[UDPRecvBuffSize];
	int myport = 0;
	int i = 0;
	int len = 0;
	bool bRtn = false;
	unsigned long ul_tmpSerial = 0;
	unsigned long Ticks = 0;
	// nick mark 20150727 Ver:000-000-GIO_V2-Smart_Traffic-13C241-0003-155061 //UDPSocket udpPlateServer;
	serverAck ack;

	pthread_detach(pthread_self()); // nick add 20160503 Ver:000-000-GIO_V2-13B251-0011-13C241 //
	
	if (strlen(G_ParkingConfig.PlateServerIP) <= 7 || G_ParkingConfig.PlateServerPort <= 0)
	{
		printf("Plate Server Setting Error, SettingValue:[%s:%d]\n", G_ParkingConfig.PlateServerIP, G_ParkingConfig.PlateServerPort);
		goto FUNCEXIT;
	}
	
	myport = calculatePort(G_ParkingConfig.ClientID);
	udpPlateServer.Initial(G_ParkingConfig.PlateServerIP , G_ParkingConfig.PlateServerPort , myport + 1 , false);
	
	memset(SendData , '\0' , sizeof(SendData));
	memset(recvData, 0, UDPRecvBuffSize);
	memset(buf, 0, sizeof(buf));
	memset(&ack, 0, sizeof(serverAck));
	
	for (i = 0; i < 3; i++)
	{
		sprintf(SendData,"\x04""%06ld\x1c""%08lX""\x1c""%04d""\x1c""%di" ,
			UDP_Serial, G_ParkingConfig.ClientID, 1081, G_ParkingConfig.MachineID);
		udpPlateServer.udpSendWithBccEnd(SendData , strlen(SendData));
		ul_tmpSerial = UDP_Serial;
		UDP_Serial++;
		
		printf("Plate Server Wait ACK Count:[%d]!\n", i);
		
		Ticks = GetTickCount();
		
		// 等待Server Ack //
		while(CheckTimeout(&Ticks, 200) == false)
		{
			if((len = udpPlateServer.udpRecv(recvData)) > 0)
			{
				switch(recvData[0])
				{
					case Tx_ACK:
						ack.num = atol(recvData + 1);
						break;
					default:
						len = strlen(recvData);
						sprintf(buf,"Recv Len:%d . Head:%02x\n",len,recvData[0]);
						printf("%s :",buf);
						
						for(i=0;i<len;i++)
						{
							printf("%02X ",recvData[i]);
						}
						
						printf("\n");
						break;
				}
				
				printf("Recv Ack Number:[%ld], mySend Ack Number:[%ld]\n", ack.num, ul_tmpSerial);
				
				if (ack.num == ul_tmpSerial)
				{
					bRtn = true;
					break;
				}
			}
			
			usleep(1);
		}
		//
		
		if (bRtn)
			break;
	}
	
	FUNCEXIT:
	
	pthread_exit(NULL);
}


bool SendHealthCheckToNewTerimal()
{
	char sendData[256];
	bool status= false;
	unsigned long udpSerial;

	//ShowMessage((char *)"Send Health Check To New Terimal.");
	udpSerial = (GetTickCount() % 1000000L);
	memset(sendData,'\0',sizeof(sendData));	
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""0s",udpSerial,G_ParkingConfig.ClientID,9999L); 			
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}


bool SendLoop1TriggerToNewTerimal(unsigned long udpSerial,	int isEnable)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Loop1 Trigger To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));	
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1083L,isEnable); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}

bool SendButtonTriggerToNewTerimal(unsigned long udpSerial,	int isPush)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Button Trigger To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));	
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1084L,isPush); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}

bool SendCarEnterStatusToNewTerimal(unsigned long udpSerial,	int isEnter)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Car Enter Status To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));	
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1085L,isEnter); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}

bool SendCarFullTriggerToNewTerimal(unsigned long udpSerial,	int fullStatus)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Car Full Trigger To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1086L,fullStatus); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}

bool SendLoop2TriggerToNewTerimal(unsigned long udpSerial,	int isEnable)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Loop2 Trigger To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1087L,isEnable); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}

bool SendCarEnterStartToNewTerimal(unsigned long udpSerial,	int ticketType)
{
	char sendData[256];
	bool status= false;

	//ShowMessage((char *)"Send Car Enter Start To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1088L,ticketType); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}


bool SendRFInTriggerToNewTerimal(unsigned long udpSerial, int isEnable)
{
	char sendData[256];
	bool status= false;
	
	//ShowMessage((char *)"Send RFIn Trigger To New Terimal.");
	memset(sendData,'\0',sizeof(sendData));
	sprintf(sendData,"\x04""%ld\x1c""%lX\x1c""%ld\x1c""%ds",udpSerial,G_ParkingConfig.ClientID,1089L,isEnable); 		
	status =  SendToNewTerimal(udpSerial,sendData);
	
	return status;
}


bool SendToNewTerimal(unsigned long udpSerial,char sendData[])
{
	bool status = false;
	int len = 0;
	int i = 0;
	char recvData[UDPRecvBuffSize];
	char buf[512];
	char log[256];
	unsigned long Ticks = 0;	
	serverAck ack;
	char sendBuffer[512];
	unsigned long myudpSerial;

	if (strlen(G_ParkingConfig.NewTerminalIP) <= 7 || G_ParkingConfig.NewTerminalPort <= 0)
	{	
		return false;
	}

	pthread_mutex_lock(&NewTerminal_mutex);

	memset(&log, '\0', sizeof(log));
	memset(&ack, '\0', sizeof(serverAck));
	memset(&recvData, '\0', sizeof(recvData));
	memset(&buf, '\0', sizeof(buf));

	memset(&sendBuffer,'\0',sizeof(sendBuffer));
	memcpy(sendBuffer,sendData,sizeof(sendBuffer)-1);
	myudpSerial = udpSerial;

	ShowMessage(sendBuffer);

	for (i = 0; i < 200; i++)
	{
		// Send Udp 
		udpNewTerimal.udpSendWithBccEnd(sendBuffer,strlen(sendBuffer));

		Ticks = GetTickCount();
		
		// 等待Server Ack //
		while(CheckTimeout(&Ticks, 300) == false)
		{
			memset(&ack, '\0', sizeof(serverAck));
			memset(&recvData, '\0', sizeof(recvData));
			memset(&buf, '\0', sizeof(buf));
		
			if((len = udpNewTerimal.udpRecv(recvData)) > 0)
			{
				//ShowMessage((char *)"SendToNewTerimal() :: recvData :");
				ShowMessage(recvData);
			
				switch(recvData[0])
				{
					case Tx_ACK:
						//ShowMessage((char *)"SendToNewTerimal() :: Get Ack");
		
						ack.num = atol(recvData + 1);
						break;
					default:
						len = strlen(recvData);
						sprintf(buf,"Recv Len:%d . Head:%02x\n",len,recvData[0]);
						printf("%s :",buf);
						
						for(i=0;i<len;i++)
						{
							printf("%02X ",recvData[i]);
						}
						
						printf("\n");
						break;
				}
				
				if (ack.num == myudpSerial)
				{
					status = true;
					break;
				}
			}
			
			usleep(1);
		}

		if(status == true) break;
		
		memset(&log, '\0', sizeof(log));
		sprintf(log,"SendToNewTerimal() :: Send Count [ %d ], Recv Ack Number:[%ld], mySend Ack Number:[%ld]",i, ack.num, udpSerial);
		ShowMessage(log);
	}

	if(status == false)
	{
		FILE *fh = NULL;				
		fh = fopen("/Data/LocalSeting/SendToNewTerimalNoReboot","r");
		
		if(fh != NULL)
		{		
			ShowMessage((char *)"SendToNewTerimal :: false.");
			fclose(fh);
		}
		else
		{
			int systemResult = system((char *)"reboot");
			memset(&log, '\0', sizeof(log));
			sprintf(log,"SendToNewTerimal :: false, Reboot : %d",systemResult);
			ShowMessage(log);
		}
	}
	
	pthread_mutex_unlock(&NewTerminal_mutex);
	return status;
}

