/* TCP/IP Transaction function Header file */

#ifndef ____IPMS_DRIVER_NETWORK____
#define ____IPMS_DRIVER_NETWORK____

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <utility> // make_pair
//bool InitUDP(char ip[],int port);
#define UDP_BCC		0x5A
#define DEF_STX_CODE	0x02	// 20110627 Tony add
#define START_CRC		0x62	// 20110627 Tony add

using namespace std;

typedef map<long,char*> udpSendData;
typedef struct TxParktron
{
	unsigned char   header;
	unsigned long   sn;
	unsigned long   clientID;
	unsigned short  commandID;
	char            parms[2048];
}txParktron;

class UDPSocket
{
public:
	UDPSocket(void){};
	UDPSocket(char ip[],short port,short myport);

	~UDPSocket(void){ close(sockfd);};

	bool  Initial(char ip[],short port, short myport,bool bBlock=false);
	void  udpSend(char data[],short len);
	void  udpSendWithBccEnd(char data[],short len);
	int   udpRecv(char* recvData);
	bool  checkBCC(char data[],short len);
	void  addToHash(char data[],short len);
	void  removeFromHash(long serial);
	short parseCommand(txParktron* tx, char* str);
	bool  GetParm(char* str ,short index,txParktron tx);
	void	Clearbuf(void);					//Frank add 20120904

private:
	udpSendData udpSends;
	int 	sockfd;
	struct 	sockaddr_in address;  // remote address
	struct 	hostent *hostinfo;
	struct 	servent *servinfo;
};


void GetPrimaryIp(char* buffer, size_t buflen=20);

class TCPSocket
{
public:
	TCPSocket(void){};
	
	~TCPSocket(void){ close(sockfd);};

private:
	int 	sockfd;
	struct 	sockaddr_in address;  // remote address
	struct 	hostent *hostinfo;
	struct 	servent *servinfo;	
};

// 20110627 Tony add  s
class VideoServerProtocol
{	private:
		int					sockfd;
		struct sockaddr_in		address;
		char					*ReceiveBuffer;
		int					BufferLength;
		unsigned int			SerialNumber;
		//
	public:
		//
	private:
		//
	public:
		VideoServerProtocol( unsigned short RecvBufferLen = 128);
		~VideoServerProtocol(void);
		void		InitialNetwork( char *ServerIP, unsigned short ServerPort);
		bool		ConnectToServer(void);
		bool		SendNormalDataToServer( char *SendData);
		bool		SendCommandPackageToServer( char *SendCommand);
		void		CloseConnect(void);
		int		GetBufferLength(void);
	//
};
// 20110627 Tony add  e

#endif