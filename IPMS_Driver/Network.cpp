/* TCP/IP UDP Transaction function */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "Network.h"
#include <string.h>
#include "../CommonDef.h"					//Frank add 20111020

bool UDPSocket::Initial(char ip[],short port,short myport,bool bBlock)
{
	int flags;
	struct sockaddr_in myaddr;
	
	if (sockfd > 0) // nick add 20150727 //
		return true; // nick add 20150727 //
	
	hostinfo = gethostbyname(ip);
	if(!hostinfo)
	{
		printf("No host: %s\n",ip);
		return false;
	}
	
	udpSends.clear();
	
	// nick mark 20150727 //sockfd = socket(PF_INET,SOCK_DGRAM,0);
	sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // nick add 20150727 //
	
	printf("UDP Initial() -> sockfd = %d\n", sockfd);
	
	bzero(&myaddr, sizeof(myaddr));
	//memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myport);
	//bOnlyClient == false &&
	if ( bind(sockfd, (struct sockaddr *)&myaddr,sizeof(myaddr)) <0)
	{
		perror("bind failed!");
	}
	
	//set non block
	if(bBlock != true)
	{
		flags = fcntl(sockfd, F_GETFL,0);
		flags |= (O_NONBLOCK);
		fcntl(sockfd, F_SETFL, flags);
	}
	// send Initial
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	//address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	bcopy(hostinfo->h_addr_list[0], (caddr_t)&address.sin_addr, hostinfo->h_length);
	//len = sizeof(address);
	return true;
}

short UDPSocket::parseCommand(txParktron* tx, char* str)
{
	unsigned char BCC = 0x5A;
	short parms = 0;
	short i,len,forward = 1;//,n = 0;
//	short infosplit = 0,datasplit = 0;
	char buf[64];
	char *s = NULL;
	short command = 0;
	
	for(i=0;i<(short)strlen(str)-3;i++)
	{
		BCC ^= str[i];
	}
	//if(BCC != 0x00)
		//printf("BCC:%02X\n",BCC);
	tx->header = (unsigned char)str[0];
	memset(buf,'\0',sizeof(buf));
	// Serial number
	s = strchr(str+forward,0x1c);
	len = s - str - forward;
	memcpy(buf,(str+forward),len);
	sscanf(buf,"%ld",&tx->sn);
	forward += (strlen(buf)+1);
	// Client ID
	s = strchr(str+forward+1,0x1c);
	memset(buf,'\0',sizeof(buf));
	len = s - str - forward;
	memcpy(buf,(str+forward+1),len-1);
	sscanf(buf,"%ld",&tx->clientID);
	forward += (strlen(buf)+1);
	// Command ID
	s = strchr(str+forward+1,0x1c);
	memset(buf,'\0',sizeof(buf));
	len = s - str - forward;
	memcpy(buf,(str+forward)+1,len-1);
	sscanf(buf,"%hd",&command);
	tx->commandID = command;
	
	// params
	forward += (strlen(buf)+1);
	
	memset(tx->parms,'\0',sizeof(tx->parms));
	memcpy(tx->parms,(str+forward+1),strlen(str)-3-forward-1);
	s = strchr(str+forward+1,0x1f);
	
	while(1)
	{
		parms++;
		if(s == NULL)
			break;
		s = strchr(s+1,0x1f);
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
	
	return parms;
}

bool UDPSocket::GetParm(char* str ,short index,txParktron tx)
{
	char buf[64];
//	char *s = NULL;
//	char *s1 = NULL;
	short i,len,p=0;
	short point[25];
	short fix=0;
	
	point[p++] = 0;
	memset(buf,'\0',sizeof(buf));
	len = strlen(tx.parms);
	tx.parms[len-1]=0;
	//printf("parms:%s len:%d\n",tx.parms,len);
	
	for(i=0; i<len; i++)
	{
		if(tx.parms[i] == 0x1c && index == 1)
		{
			fix = i+1;
		}
		if(tx.parms[i] == 0x1F)
		{
			point[p] = i + 1;
			tx.parms[i] = 0;
			tx.parms[i-1] = 0;
			p++;
		}
	}
	
	sprintf(str,"%s",tx.parms+point[index-1]+fix);
	//printf("%d parm:%s\n", p, str);
	
	if(p >= index)
		return true;
	else
		return false;
}

UDPSocket::UDPSocket(char ip[],short port,short myport)
{
	if(Initial(ip,port,myport) == false)
		printf("UDP init fail!\n");
}

void UDPSocket::udpSend(char data[],short len)
{
//	short i;
	sendto(sockfd,data,len,0,(struct sockaddr *)&address,sizeof(address));
}

void UDPSocket::udpSendWithBccEnd(char data[],short len)
{
	short i;
	char buf[8];
	unsigned char BCC = 0x5A;

	memset(&buf, '\0', sizeof(buf));
	
	//printf("Send:");
	for(i=1;i<len;i++)
	{
		BCC ^= data[i];
	//	printf("%02x ",data[i]);
	}
	
	sprintf(buf,"%02X",BCC);
	data[len] = buf[0];
	data[len+1] = buf[1];
	data[len+2] = 0x03;
	//printf("BCC: %02X\n",BCC);
	sendto(sockfd,data,len+3,0,(struct sockaddr *)&address,sizeof(address));
}

int UDPSocket::udpRecv(char* recvData)
{
	short receiveLen;
	
	socklen_t len = 0;
	receiveLen = recvfrom(sockfd,recvData,UDPRecvBuffSize-1,0,(struct sockaddr *)&address,&len);					//Frank add 20111020
	
	if((receiveLen < 0))
		return -1;
		
//	printf("udpRecv() :: ");
//	
//	for (int i=0; i<receiveLen; i++)
//		printf("%02X ", *(recvData+i));
//	
//	printf("\n");
	
	return receiveLen;
}

bool UDPSocket::checkBCC(char data[],short len)
{
	unsigned char BCC = UDP_BCC;
	short i;
	
	if(data[len-1] != 0x03)
		return false;
	// print for debug
	//printf("Recv:");
	for(i=0; i<len-1; i++)
	{
		//printf("%02X ",(unsigned char)data[i]);
		BCC ^= data[i];
	}
	//printf("BCC:%02X\n",BCC);
	
	if(BCC == 0x00)
		return true;
	return false;
}

//Frank add s 20120904
void UDPSocket::Clearbuf(void)
{
	char recvData[UDPRecvBuffSize];
	
	while(1)
	{
		if(udpRecv(recvData) < 0)
			break;
		printf("Clearbuf");
		usleep(1); // nick add 20140910 Ver:000-000-GIO_V2-135101-0006-13B251 //
	}
		
}
//Frank add e 20120904

//¥¼§¹¦¨
//************************************************//
void UDPSocket::addToHash(char data[],short len)
{
	char test[256];
	udpSends[123456] = test;
	udpSends.insert(make_pair(123456L, test));
}

void UDPSocket::removeFromHash(long serial)
{
//	char test[256];
	if(udpSends.empty() == true)
		return;
	map<long,char*>::iterator it;
	it=udpSends.find(serial);
	if(it == udpSends.end())
		return;
	udpSends.erase (it);
}

#include <fstream>
#include <iostream>
using namespace std;
#define MST (-7)
void GetPrimaryIp(char* buffer, size_t buflen)
{
	//assert(buflen >= 16);
/*
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	//assert(sock != -1);
	const char* kGoogleDnsIp = "192.168.0.164";
	uint16_t kDnsPort = 53;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
	serv.sin_port = htons(kDnsPort);
	
	int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
	//assert(err != -1);
	
	sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (sockaddr*) &name, &namelen);
	//assert(err != -1);
	
	const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
	//assert(p);
	
	close(sock);
	*/
	hostent * record = gethostbyname("localhost");
	if(record == NULL)
	{
		printf("%s is unavailable\n", "localhost");
		//exit(1);
	}
	in_addr * address = (in_addr * )record->h_addr;
	string ip_address = inet_ntoa(* address);
	
	// get the current time
	time_t rawtime;
	tm * ptm;
	time ( &rawtime );
	ptm = gmtime ( &rawtime );
	
	cout << "localhost" << " (" << ip_address << ")\n";
	
	// log this information to ipaddr.log
	ofstream ipaddr_log("ipaddr.log", ios::app);
	ipaddr_log << (ptm->tm_hour+MST%24) << ":" << (ptm->tm_min) << " " << "localhost" << " (" << ip_address << ")" << endl;
	ipaddr_log.close();
}

// 20110627 Tony add s
// receive buffer max length = 65535
VideoServerProtocol::VideoServerProtocol( unsigned short RecvBufferLen)
{
	BufferLength = RecvBufferLen;
	sockfd = 0;
	if( RecvBufferLen > 0 )
		ReceiveBuffer = new char[RecvBufferLen];
	bzero( &address, sizeof(address));
	SerialNumber = 0;
}
//
//
VideoServerProtocol::~VideoServerProtocol(void)
{
	if( sockfd > 0 )
		close(sockfd);
	if( ReceiveBuffer )
		delete [] ReceiveBuffer;
};
//
//
void VideoServerProtocol::InitialNetwork( char *ServerIP, unsigned short ServerPort)
{
	if( sockfd > 0 )
	{
		close(sockfd);
		sockfd = 0;
	}
	//
	address.sin_family			= AF_INET;
	address.sin_port			= htons(ServerPort);
	address.sin_addr.s_addr	= inet_addr(ServerIP);
}
//
//
// if return = false, connect is faild.
bool VideoServerProtocol::ConnectToServer(void)
{
	if( sockfd > 0 )
		CloseConnect();
	usleep(100000);	// delay 100ms
	//
	if( (sockfd = socket( AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("socket");
		return false;
	}
	//
	if( connect( sockfd, (struct sockaddr *)&address, sizeof(address)) < 0 )
	{
		perror("connect");
		return false;
	}
	
	//recv( sockfd, ReceiveBuffer, sizeof(ReceiveBuffer), 0);
	return true;
}
//
//
bool VideoServerProtocol::SendNormalDataToServer( char *SendData)
{
	if( send( sockfd, SendData, strlen(SendData), 0) < 0 )
        {
            perror("send");
            return false;
        }
	return true;
}
//
//
bool VideoServerProtocol::SendCommandPackageToServer( char *SendCommand)
{
	//unsigned char	SendBuffer[128];
	char	SendBuffer[128];
	int			datalen, a, doxor;
	//
	memset( SendBuffer, 0, 128);
	SendBuffer[0] = DEF_STX_CODE;
	sprintf( SendBuffer+3, "%05d%s", SerialNumber, SendCommand);
	
	//datalen = 8 + strlen(SendCommand);
	
	datalen = 5 + strlen(SendCommand) + 2; // SerialNumber + SendCommand + CRC
	
	SendBuffer[1] = datalen>>8;	//	length
	SendBuffer[2] = datalen;	//	length
	
	datalen += 1;
	//
	for( a = 0, doxor = START_CRC; a < datalen; a++)
		doxor ^= SendBuffer[a];
	//
	SendBuffer[datalen] = ((doxor>>4) & 0x0F);

	if (SendBuffer[datalen]>0x09)
		SendBuffer[datalen]+=0x37;
	else
		SendBuffer[datalen]+=0x30;
	
	SendBuffer[datalen+1] = (doxor & 0x0F);

	if (SendBuffer[datalen + 1]>0x09)
		SendBuffer[datalen + 1]+=0x37;
	else
		SendBuffer[datalen + 1]+=0x30;
	
	//
	datalen += 2;
	//
	if( send( sockfd, SendBuffer, datalen, 0) < 0 )
	//if( send( sockfd, SendBuffer, strlen(datalen), 0) < 0 )
	{
		perror("send");
		return false;
	}
	//
	SerialNumber++;
	if( SerialNumber > 99999 )
		SerialNumber = 0;
	//
	return true;
}
//
//
 void VideoServerProtocol::CloseConnect(void)
{
	if( sockfd > 0 )
	{
		close(sockfd);
		sockfd = 0;
	}
}
//
//
int VideoServerProtocol::GetBufferLength(void)
{
	return BufferLength;
}
//

// 20110627 Tony add e
