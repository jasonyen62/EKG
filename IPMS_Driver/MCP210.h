#ifndef ____MCP210____
#define ____MCP210____

#include "rs232.h"

enum MCP210_Command
{
	MCP210_C_READ			= 0x31,
	MCP210_C_PRINT		= 0x36,
	MCP210_C_EJECT		= 0x3C,
	MCP210_C_COLLECT		= 0x3D,
	MCP210_C_STATUS		= 0x42,
	MCP210_C_SETFONT		= 0x46,
	MCP210_C_RESET		= 0x4C
};

class MCP210
{
public:
	MCP210(void);
	~MCP210(void);
	
	void		Init(enum COMPORT port);
	void		Close(void);
	void		ClearBuff(void);	
	//Frank mark 20121102 void		Reset(void);
	bool		Reset(void);					//Frank add 20121102
	char		GetStatus(void);
	char		EjectCardOut(char* strPrint);
	char		RetrieveCard(void);
	char		InsertTicket(void);
	
private:
	RS232	comport;
	int		BufferSize;
	char		buf[128];
	char 	SendCommand(enum MCP210_Command cmd, char* sData, int DataLen);
	bool 	Response(char* RecvBuff, int mTimeout);
	void		Restart(void);
};

#endif
