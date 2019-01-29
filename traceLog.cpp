#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>	//nick add 20110311
#include <errno.h> // nick add 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //

#include "CommonDef.h" // nick add 20130318 //

void ShowMessage(char msg[],short level)
{	
//	char buf[256];
	char buf[4096];
	char logFilename[80];
	time_t now;
	char *logLevel = NULL;
	FILE *fh = NULL;
	short logLV=0;
	int iRet = 0; // nick add 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
    
	logLevel = getenv("LOG_LEVEL");
	
	if(logLevel)
	{
		logLV = atoi(logLevel);
	}

	now = time((time_t *)0);
	struct tm *tm_ptr;

	memset(buf,'\0',sizeof(buf));
	memset(logFilename,'\0',sizeof(logFilename));

	tm_ptr = localtime(&now);
	//sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d  %s",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
	//	tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec,msg);
	sprintf(buf,"%02d:%02d:%02d  %s",tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec,msg);
	printf("%s\n",buf);
	sprintf(logFilename,"./log/%04d%02d%02d.log",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
//	printf("<< ShowMessage:  LOG_LEVEL=%d, level=%d, s_LOG_LEVEL=%s >>\n", logLV, level, logLevel);
	
	if(logLV > level)
	//if (logLV == -1)
	{
		pthread_mutex_lock(&Log_mutex); // nick add 20130318 //
		
		int RetryOpenCnt = 0;
		
		while(1)
		{
			fh = fopen(logFilename,"a");
			
			if (fh != NULL)
			{
				fprintf(fh,"%s\n",buf);
				fclose(fh);
				fh = NULL;
				break;
			}
			// ========================================================= //
			// nick add s 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
			else
			{
				if (errno == 30)
				{
// nick mark 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //					iRet = system((char *)"reboot");
					iRet = system((char *)"mount -o,remount rw /Data"); // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
					usleep(100000); // nick add 20131107 Ver:000-000-GIO_V2-133181-0102-135101 //
				}
			}
			// nick add e 20131017 Ver:000-000-GIO_V2-133181-0101-135101 //
			// ========================================================= //
			
			RetryOpenCnt++;
			if (RetryOpenCnt > 10) break;			
			usleep(2000L);
		}

		pthread_mutex_unlock(&Log_mutex); // nick add 20130318 //
		//usleep(3000L); // nick edit 20140118 Ver:000-000-GIO_V2-135101-0000-13B250 from 10 to 3000 //
		usleep(10000L);
	}
}

void KillLog(short dayAgo)
{
	char logFilename[80];
	time_t now,ago;
	struct tm *tm_ptr = NULL;
//	char *logLevel = NULL;
//	FILE *fh = NULL;
    
	now = time((time_t *)0);
	ago = now - (dayAgo * 86400L);
	memset(logFilename,'\0',sizeof(logFilename));
    
	tm_ptr = localtime(&ago);
	sprintf(logFilename,"./log/%04d%02d%02d.log",tm_ptr->tm_year+1900,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
	unlink(logFilename);
}

// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //long GetTickCount()		//nick add 20110302
unsigned long GetTickCount() // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
{
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long currentTime;
	unsigned long currentTime; // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	struct timeval current;
	char buf[100];

	memset(&current, 0, sizeof(timeval)); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	if (gettimeofday(&current, NULL) == -1)
	{
		memset(buf, 0, sizeof(buf));
		currentTime = current.tv_sec * 1000 + current.tv_usec / 1000;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //		sprintf(buf, "GetTime Error!! TimeValue=%ld", currentTime);
		sprintf(buf, "GetTime Error!! TimeValue=%ld", currentTime); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
		ShowMessage(buf, 0);
		return(currentTime); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	}

	currentTime = current.tv_sec * 1000 + current.tv_usec / 1000;
	return(currentTime);
}

// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //bool CheckTimeout(long Ticks, long timeout)
bool CheckTimeout(unsigned long *Ticks, unsigned long timeout) // nick add 20131012 Ver:000-000-GIO_V2-135101-0005-13B251 //
{
	bool bRet = false;
// nick mark 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //	long nowTicks = GetTickCount();
	unsigned long nowTicks = GetTickCount(); // nick add 20131012 Ver:000-000-GIO_V2-133181-0101-135101 //
	
	if (nowTicks - *Ticks >= timeout)
		bRet = true;
	else if (nowTicks < *Ticks)
		*Ticks = nowTicks;

	return(bRet);
}

