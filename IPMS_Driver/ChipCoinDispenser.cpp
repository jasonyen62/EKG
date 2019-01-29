/* FileName: ChipCoinDispenser.cpp */

#include <stdio.h>

#include "../CommonDef.h"
#include "rs232.h"
#include "ChipCoinDispenser.h"

ChipCoinDispenser::ChipCoinDispenser()
{
	//comport.PortInit(COM2,19200,'N',8,1);
	comport.PortInit2(COM2,19200,'N',8,1);
}

ChipCoinDispenser::~ChipCoinDispenser()
{
	comport.PortClose();
}

bool ChipCoinDispenser::Initial(enum COMPORT port)
{
	//comport.PortInit(port,19200,'N',8,1);
	comport.PortInit2(port,19200,'N',8,1);
	return true;
}

bool ChipCoinDispenser::Issue(short id)
{
	return true;
}

void ChipCoinDispenser::Reset(short id)
{
	
}

bool ChipCoinDispenser::CheckCoinLow(short id)
{
	return false;
}

