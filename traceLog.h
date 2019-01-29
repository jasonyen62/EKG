/*
Trace log file
for debug
*/

#ifndef ____IPMS_TRACELOG____
#define ____IPMS_TRACELOG____

void ShowMessage(char msg[],short level=0);
void KillLog(short dayAgo);
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long GetTickCount(void);		//nick add 20110302
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //bool CheckTimeout(long Ticks, long timeout);
unsigned long GetTickCount(void); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
bool CheckTimeout(unsigned long *Ticks, unsigned long timeout); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //

#endif