/*  CRT 350 Reader Control Programe Header file */

#ifndef ____CRT350____
#define ____CRT350____

#include "rs232.h"

enum CRT350_ERR
{
	CRT350_CMDERR = 0x00,
	CRT350_PARAMERR= 0x01,
	CRT350_EXECERR = 0x02,
	CRT350_DATAERR = 0x04,
	CRT350_VOLERR = 0x05,
	CRT350_ABNORERR = 0x06,
	CRT350_JAMEERR = 0x07,
	CRT350_SHUTTERERR = 0x08,
	CRT350_MOTORERR = 0x09,
	CRT350_HOOKERR = 0x0A
};

enum CRT350_COMMAND
{
	CRT350_CMD_RESET = 0x20,
	CRT350_CMD_STATUS = 0x21,
	CRT350_CMD_SET_IN = 0x22,
	CRT350_CMD_SET_STAY = 0x23,
	CRT350_CMD_MOVE = 0x24,
	CRT350_CMD_READ = 0x45,
	CRT350_CMD_WRITE = 0x46
};
enum CRT350_TRACKMODE
{
	CRT350_TRACK1 = 0x31,
	CRT350_TRACK2 = 0x32,
	CRT350_TRACK3 = 0x33,
	CRT350_TRACK12 = 0x34,
	CRT350_TRACK23 = 0x35,
	CRT350_TRACK13 = 0x36,
	CRT350_TRACK123 = 0x37
};

enum CRT350_MovePlace
{
	CRT350_MOVE_FRONT_HOLD = 0x38,
	CRT350_MOVE_FRONT      = 0x39,
	CRT350_MOVE_FRONT_MAG  = 0x3A,
	CRT350_MOVE_ICCARD     = 0x3B,
	CRT350_MOVE_AFTER_MAG  = 0x3C,
	CRT350_MOVE_RAER_HOLD  = 0x3D,
	CRT350_MOVE_REAR       = 0x3E,
	CRT350_MOVE_EJECT      = 0x35 //rear side
};

enum CRT350_WhichStatus
{
	STATUS_READER = 0x30,
	STATUS_SENSOR = 0x31,
	STATUS_SHUTTER = 0x34
};

class CRT350
{
public :	
	CRT350(void){ ReaderID=0x00;};
	~CRT350(void){ comport.PortClose();};

	void init(enum COMPORT port);
	bool Reset(char* versionString,char ResetValue='0');
	bool GetStatus(enum CRT350_WhichStatus whichStatus,char* status);
	bool MoveTicket(enum CRT350_MovePlace place);
	bool SetCardInControl(enum CRT350_MovePlace place);
	bool SetStopPosition();
	bool ReadTrack();
	bool WriteTrack();
	void ClearBuff(void);					//Frrank add 20111116
	
private:
	RS232 comport;
	unsigned char ReaderID;
	
	void SendCommand(int  len,char* data);
	bool  GetResponse(char* rData);	
};

#endif
