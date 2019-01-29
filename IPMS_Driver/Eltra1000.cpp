#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */

#include "../CommonDef.h"
#include "Eltra1000.h"

Eltra1000::Eltra1000()
{
	ReaderID=0;
}

Eltra1000::~Eltra1000()
{
	comport.PortClose();
}

void Eltra1000::init(enum COMPORT port)
{
	//comport.PortInit(port,9600,'N',8,1);
	comport.PortInit2(port,9600,'N',8,1);
}

void Eltra1000::ClearBuff(void)					//Frank add s 20111116
{
	comport.Clear232Bufer();
}					//Frank add e 20111116