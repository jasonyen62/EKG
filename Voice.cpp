
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <sys/msg.h>

#include "CommonDef.h"
#include "IPMS_Driver/rs232.h"
#include "IPMS_Driver/IC8255.h"
#include "traceLog.h"
#include "Voice.h"
#include "Dio.h"

#define IO_VOICE true


void AudioOut(char filename[])
{ 	//語音檔輸出
	int msgid;
	struct cmd_msg_st cmddata;
	
	if(G_ParkingConfig.UseMQ ==0 )
		return;
	
	msgid = msgget((key_t)1234,0666 | IPC_CREAT);
	
	if(msgid == -1)
	{
//nick mark 20110705		printf("create mq error \n");
		printf("<<  AudioOut() : create mq error  >>\n");		//nick add 20110705
		return;
	}
	
	cmddata.msg_type = 1;
//nick mark 20110705	sprintf(cmddata.fileName,"%s",filename);
	//nick add s 20110705
	if (strlen(G_ParkingConfig.ParkLanguage) == 0)
		sprintf(cmddata.fileName,"%s",filename);
	else
		sprintf(cmddata.fileName, "%s/%s", G_ParkingConfig.ParkLanguage, filename);
	//nick add e 20110705
	
	if(msgsnd(msgid,(void*)&cmddata,80,0) == -1)
	{
//nick mark 20110705		printf("Send MQ erroe\n");
		printf("<<  AudioOut() : Send MQ erroe  >>\n");		//nick add 20110705
		return;
	}
}

void VoiceOn(enum VOICEDEF index)
{	// 語音板輸出
#ifndef IO_VOICE

#else
	IOVoiceOut(index);
#endif
}

void AudioOn(enum AUDIODEF index)
{
	char name[128];

	memset(name,'\0',sizeof(name));
//nick mark 20120420 	sprintf(name,"J%03d.wav",index);

	//nick add s 20120420
	if (index == AUDIO_SEASON_W_SUC)
		sprintf(name, "rfbeep.wav");
	else
		sprintf(name,"J%03d.wav",index);
	//nick add e 20120420

	AudioOut(name);
}

