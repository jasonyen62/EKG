/* SDL function to Show bmp and play audio server */

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <sys/ipc.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_thread.h"

#include "SDL_prim.h"
#include <queue>
using namespace std;

#define WIDTH 640
#define HEIGHT 480
#define BPP 8
#define DEPTH 32
//#define BUFSIZ 128

typedef struct cmd_msg_ST
{
	long int msg_type;
	char fileName[80];
}cmd_msg_st;


/*  Structure  for  loaded  sounds.  */
typedef  struct  sound_s  
{
	Uint8  *samples;
	Uint32  length;
}sound_t, *sound_p;

/*  Structure  for  a  currently  playing  sound.  */
typedef  struct  playing_s  {
	int  active;
	sound_p  sound;
	Uint32  position;
}playing_t, *playing_p;

/*  Array  for  all  active  sound  effects.  */
#define  MAX_PLAYING_SOUNDS 2
playing_t  playing[MAX_PLAYING_SOUNDS];
SDL_mutex *value_mutex;		//mutex to lock variables
#define  VOLUME_PER_SOUND SDL_MIX_MAXVOLUME  /  2

void ExecuteCommand(SDL_Surface* screen, sound_t* explosion, SDL_AudioSpec* obtained,bool b_audioRun);
bool FileExists(char* strFilename);

void AudioCallback(void  *user_data,  Uint8  *audio,  int  length)
{
	int  i;
 
	memset(audio,  0,  length);
   
	for  (i  =  0;  i  <  MAX_PLAYING_SOUNDS;  i++)  
	{
		if  (playing[i].active)  
		{
			Uint8  *sound_buf;
			Uint32  sound_len;
			
			sound_buf  =  playing[i].sound->samples;
			sound_buf  +=  playing[i].position;

			if  ((playing[i].position  +  length)  > playing[i].sound->length)  
			{
				sound_len  =  playing[i].sound->length - playing[i].position;
			}  
			else
			{
				sound_len  =  length;
			}
			
			SDL_MixAudio(audio,  sound_buf,  sound_len, VOLUME_PER_SOUND);
			playing[i].position  +=  length;
			
			if  (playing[i].position  >=  playing[i].sound->length)  
			{
				playing[i].active  =  0;       /*  mark  it  inactive  */
			}
		}
	}
}

int LoadAndConvertSound(char  *filename,  SDL_AudioSpec  *spec,sound_p  sound)
{
	SDL_AudioCVT  cvt;
	SDL_AudioSpec  loaded;
	Uint8  *new_buf;
    
    /*  Load  the  WAV  file  in  its  original  sample  format.  */
	if  (SDL_LoadWAV(filename, &loaded,  &sound->samples, &sound->length)  ==  NULL)  
	{
		printf("Unable to load sound:%s  Err:%s\n",filename, SDL_GetError());
		return  1;
	}

	if  (SDL_BuildAudioCVT(&cvt,  loaded.format,loaded.channels,  loaded.freq,spec->format,  spec->channels, spec->freq) < 0)
	{
		printf("Unable  to  convert  sound:  %s\n",  SDL_GetError());
		return  1;
	}

	cvt.len  =  sound->length;
	new_buf  =  (Uint8  *)  malloc(cvt.len  *  cvt.len_mult);
	
	if (new_buf  ==  NULL)
	{
		printf("Memory  allocation  failed.\n");
		SDL_FreeWAV(sound->samples);
		return  1;
	}

	memcpy(new_buf,  sound->samples,  sound->length);

	cvt.buf  =  new_buf;
	
	if (SDL_ConvertAudio(&cvt) < 0)
	{
		printf("Audio  conversion  error:  %s\n",  SDL_GetError());
		free(new_buf);
		SDL_FreeWAV(sound->samples);
		return  1;
	}
 
	SDL_FreeWAV(sound->samples);
	sound->samples  =  new_buf;
	sound->length  =  sound->length  *  cvt.len_mult;
	
	//printf("・%s・  was  loaded  and  converted  successfully.\n", filename);
	return  0;
}

void  ClearPlayingSounds(void)
{
	int  i;
	
	for  (i = 0; i < MAX_PLAYING_SOUNDS; i++)
	{
		playing[i].active = 0;
	}
}

int PlaySound(sound_p sound)
{
	int  i;
	
	/*  Find  an  empty  slot  for  this  sound.  */
	for (i = 0; i < MAX_PLAYING_SOUNDS; i++)
	{
		if (playing[i].active == 0)
			break;
	}
	
	/*  Report  failure  if  there  were  no  free  slots.  */
	if  (i == MAX_PLAYING_SOUNDS)
		return  1;
	
	SDL_LockAudio();
	playing[i].active = 1;
	playing[i].sound  = sound;
	playing[i].position = 0;

	SDL_UnlockAudio();
	return  0;
}

//===============================================================================
void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 *pixmem32=NULL;
	Uint32 colour;  

	colour = SDL_MapRGB( screen->format, r, g, b );

	pixmem32 = (Uint32*) screen->pixels  + y + x;
	*pixmem32 = colour;
}

void DrawScreen(SDL_Surface* screen, int h)
{ 
	int x, y, ytimesw;
	
	if(SDL_MUSTLOCK(screen)) 
	{
		if(SDL_LockSurface(screen) < 0) return;
	}

	for(y = 0; y < screen->h; y++ ) 
	{
		ytimesw = y*screen->pitch/BPP;
		
		for( x = 0; x < screen->w; x++ ) 
		{
			setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
		}
	}

	//if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
	SDL_UnlockSurface(screen);
	SDL_Flip(screen); 
}

void ShowBMP(char filename[],SDL_Surface* screen )
{
	SDL_Surface* hello = NULL;
	SDL_Rect  dest,source;
	hello = SDL_LoadBMP( filename);
	
	if (hello != NULL)
	{
		dest.x = 0;
		dest.y = 0;
		dest.w = 800;
		dest.h = 550;
		source.x = 0;
		source.y = 0;
		source.w = 800;
		source.h = 550;
		
		//Apply image to screen
		SDL_BlitSurface( hello, &source, screen, &dest );
	
		//Update Screen
		SDL_Flip( screen );
	
		SDL_FreeSurface( hello );
	}
	//else
	//{
	//	printf("Load Image Error %s", *filename);
	//}
}

const char* bmpFile[30]=
{
    "parktron.bmp", "InsertTicket.bmp", "TakeTicket.bmp", "PushButton.bmp", "ParkingFull.bmp",
    "StopService.bmp", "NoPay.bmp", "Leave.bmp", "AlreadyIn.bmp", "AlreadyOut.bmp",
    "AreaError.bmp", "Fault.bmp", "NoTicket.bmp", "TicketDataError.bmp", "PlateError.bmp",
    "TicketOver.bmp", "PlateProcess.bmp", "PleaseEnter.bmp", "TicketProcess.bmp", "TicketReadError.bmp",
    "", "", "", "", "",
    "", "", "", "", ""
};
    
typedef struct Madia_msg_st
{
    long msg_type; //0:Show bmp  1:sound  2:mpeg
    char fileName[BUFSIZ];   // index of array 0~29
}CMD_msg;

void drawText(SDL_Surface* screen,char* string, int size, int x, int y, int fR, int fG, int fB)
{
   TTF_Font* font = TTF_OpenFont("./font/Acknowledge.ttf", size);
   SDL_Color foregroundColor = { fR, fG, fB };
   SDL_Color backgroundColor = { 0, 0, 0 };
   SDL_Surface* textSurface = TTF_RenderText_Shaded(font, string, 
		foregroundColor, backgroundColor);
	//SDL_Surface* textSurface = TTF_RenderText_Solid(font, string, 
	//	foregroundColor);
   SDL_Rect textLocation = { x, y, 0, 0 };
   SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
   SDL_Flip( screen );
   SDL_FreeSurface(textSurface);
   TTF_CloseFont(font);
}

bool ReadCMD(CMD_msg *cmd)
{
	FILE* fh = NULL;
	char* buffer = NULL;
	CMD_msg CMD;
	
	fh = fopen("/tmp/cmd.txt","rb");
    
	if(fh != NULL)
	{
		buffer = (char*)(&CMD);
		fread (buffer,1,sizeof(Madia_msg_st),fh);
		fclose(fh);
		cmd->msg_type = CMD.msg_type;
		sprintf(cmd->fileName,"%s",CMD.fileName);
		//printf("read: %ld,%s\n",cmd->msg_type,cmd->fileName);
		return true;
	}

	return false;
}

void WriteCMD(CMD_msg CMD)
{
    FILE* fh = NULL;
    char * buffer = NULL;
    
    fh = fopen("/tmp/cmd.txt","wb");
    
    if(fh != NULL)
    {
        buffer = (char*)(&CMD);
        fwrite(buffer,1,sizeof(Madia_msg_st),fh);
        fclose(fh);
        //printf("write\n");
    }
}

/* ==== Status define ==== */
#define STATUS_TCP_CONNECT   	0x0001
#define STATUS_PARKING_FREE     0x0002 /* Parking Free is need take ticket , no Fee */
#define STATUS_GATE_OPEN        0x0004 /* Gate Open is no need take ticket */
#define STATUS_GATE_ON_SERVICE  0x0008
#define STATUS_BARRIER_DOWN     0x0010
#define STATUS_MACHINE_OPEN     0x0020
#define STATUS_TICKET_LOW       0x0040
#define STATUS_STRIKE           0x0080
#define STATUS_READER_CONNECT   0x0100
#define STATUS_HOPPER_SELECT    0x0200

void GetStatus(unsigned int* status)
{
	FILE* fh = NULL;
	bool  bReadOK = false;
	short retry = 0;
	
	do
	{
		fh = fopen("/tmp/Status","rb");
		
		if(fh == NULL)
		{
			//printf("Read status file fail.\n");
			usleep(200000L);
			
			if(retry++ >3)
			{
				break;
			}
			
			continue;
		}
		
		size_t Fsize = fread(status,sizeof(unsigned int),1,fh);
		fclose(fh);
		bReadOK = true;
	}while(bReadOK==false);
}

void ShowStatus(SDL_Surface* screen,int x,int y,unsigned int status)
{
	SDL_Rect src;
	char string[40];
	
	src.x  =  x;
	src.y  =  y;
	src.w  =  3;
	src.h  =  3;
	
	if( (status & STATUS_TCP_CONNECT)>0)
	{
		SDL_fillCircle(screen, x,y,5, SDL_MapRGB(screen->format, 0, 250, 0));
	}
	else
	{
		SDL_fillCircle(screen, x,y,5, SDL_MapRGB(screen->format, 250, 0, 0));
		//sprintf(string,"%04x",status);
		//drawText(screen,string,40,500,550,255,255,255); 
	}
}

int global_data = 0;
queue <cmd_msg_st> G_CmdQue;

int thread_func1(void *unused)
{
	cmd_msg_st cmddata;
	long int msg_to_receive = 0;
	cmd_msg_st cmd;
	int msgid;
	
	msgid = msgget((key_t)1234,0666 | IPC_CREAT);
	
	if(msgid == -1)
	{
		printf("create mq1 error \n");
		return 1;
	}
	
	while ( global_data != -1 )
	{
		if(msgrcv(msgid,(void*)&cmddata,80,msg_to_receive,0) == -1)
		{
			printf("recv MQ error\n");
		}
		
		//printf("MQ:%ld,%s \n",cmddata.msg_type,cmddata.fileName);
		SDL_mutexP ( value_mutex ); //lock before upgrading 
		G_CmdQue.push(cmddata);
		SDL_mutexV ( value_mutex );
		SDL_Delay(99);
	}
	
	if(msgctl(msgid,IPC_RMID,0) == -1)
	{
		printf("MQ remove error\n");
	}
	
	printf("Thread1 quitting\n");
	return(0);
}

int thread_func2(void *unused)
{
	cmd_msg_st cmddata;
	long int msg_to_receive = 0;
	cmd_msg_st cmd;
	int msgid;
	
	msgid = msgget((key_t)1235,0666 | IPC_CREAT);
	
	if(msgid == -1)
	{
		printf("create mq1 error \n");
		return 1;
	}
	
	while ( global_data != -1 )
	{
		SDL_mutexP ( value_mutex ); //lock before upgrading 
		
		if(msgrcv(msgid,(void*)&cmddata,80,msg_to_receive,0) == -1)
		{
			printf("recv MQ error\n");
		}
		
		G_CmdQue.push(cmddata);
		SDL_mutexV ( value_mutex );
		SDL_Delay(112);
	}
	
	if(msgctl(msgid,IPC_RMID,0) == -1)
	{
		printf("MQ remove error.\n");
	}
	
	printf("Thread2 quitting\n");
	return(0);
}

int main(int argc, char* argv[])
{
	long count=0;
	bool bSt=false;
	bool b_audioRun=true;
	int  quit_flag  =  0;     /*  we・ll  set  this  when  we  want  to  exit.  */  
	unsigned int status;
	
	SDL_Surface* hello  = NULL;
	SDL_Surface* screen = NULL;
	SDL_Thread*  thread1 = NULL;
	SDL_Thread*  thread2 = NULL;
	SDL_AudioSpec  desired,  obtained;
	SDL_Rect     src,dest;
	SDL_Event    event;
	sound_t  explosion;
	CMD_msg CMD;	
	
	char timeString[50];	
	time_t now, outDeadline;
	struct tm *tm_ptr = NULL;
 
	//Start SDL
	//SDL_putenv("SDL_VIDEO_X11_WMCLASS=littlegptracker");
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO );
	/*  Make  sure  SDL_Quit  gets  called  when  the  program  exits.  */
	atexit(SDL_Quit);
	/*  We  also  need  to  call  this  before  we  exit.  SDL_Quit  does
	not  properly  close  the  audio  device  for  us.  */
	atexit(SDL_CloseAudio);
	
	TTF_Init();
	
	//Set up screen
	screen = SDL_SetVideoMode( 800, 600, 16, SDL_SWSURFACE );
	
	//Load image
	CMD.msg_type = 0;
	sprintf(CMD.fileName,"parktron.bmp");
	WriteCMD(CMD);
	SDL_Color cirlceColor = { 200, 200, 10 };
	
	desired.freq      = 44100; /*  desired  output  sample  rate  */
	desired.format    = AUDIO_S16SYS;    /*  request  signed  16-bit  samples  */
	desired.samples   = 8192; /*  this  is  somewhat  arbitrary  */
	desired.channels  = 2; /*  ask  for  stereo  */
	desired.callback  = AudioCallback;
	desired.userdata  = NULL; /*  we  don・t  need  this  */
	//Mix_Volume(-1,MIX_MAX_VOLUME/2);
	
	if (SDL_OpenAudio(&desired,  &obtained) < 0)
	{
		printf("Unable to open audio device: %s\n", SDL_GetError());
		//return  1;
		b_audioRun = false;
	}
	
	value_mutex = SDL_CreateMutex();
	thread1 = SDL_CreateThread(thread_func1, NULL);
	//thread2 = SDL_CreateThread(thread_func2, NULL);
	
	if ( thread1 == NULL )
	{
		fprintf(stderr, "Unable to create thread1: %s\n", SDL_GetError());
		quit_flag = 1;
	}

	while( quit_flag  ==  0)
	{
		now = time((time_t *)0);
		tm_ptr = localtime(&now);
		sprintf(timeString," %04d/%02d/%02d %02d:%02d:%02d  ",
		tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday, 
		tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
		ExecuteCommand(screen,&explosion,&obtained,b_audioRun);
      
		SDL_Delay(10);
		
		if(count = 50L)
		{
			count = 0;
			GetStatus(&status);
			
			drawText(screen,timeString,40,6,550,255,255,255);
			ShowStatus(screen,785,585,status);
		}
		
		count++;
		//bSt = !bSt;
		
		if(SDL_PollEvent(&event) != 0)
		{
			SDL_keysym  keysym;		
			
			switch(event.type)
			{		
				case  SDL_KEYDOWN:
					keysym  =  event.key.keysym;
					//keyHit = event.key.keysym.sym;
					
					if  (keysym.sym == SDLK_q)
					{
						printf("・Q・  pressed,  exiting.\n");
						quit_flag = 1;
					}
				
					break;
				case  SDL_QUIT:
					printf("Quit  event.  Bye.\n");
					//quit_flag  =  1;
				default:
					break;
			}
		}
	}
	
	global_data = -1;
	SDL_PauseAudio(1);
	SDL_LockAudio();
	free(explosion.samples);
	SDL_UnlockAudio();
	TTF_Quit();
	SDL_Quit();  
	
	SDL_DestroyMutex ( value_mutex );	//release the resources
	return 0;
}

void ExecuteCommand(SDL_Surface* screen, sound_t* explosion, SDL_AudioSpec* obtained,bool b_audioRun)
{
	char fileName[128];
	
	cmd_msg_st cmd;
	
	if(G_CmdQue.empty() == false)
	{	//Do Server Command
		SDL_mutexP ( value_mutex );		//lock to read value
		cmd = (cmd_msg_st)(G_CmdQue.front());
          
		G_CmdQue.pop();
		
		switch(cmd.msg_type)
		{
			case 2:	
				if( strncmp(cmd.fileName ,"end",3) == 0)
            {
					break;
            }
            
            if(strncmp(cmd.fileName ,"clear",5) == 0)
            {
					SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
            }
            else
            {
					sprintf(fileName,"./jpg/%s",cmd.fileName);		
					
					//if(FileExists(fileName) == true)
					if (access(fileName, R_OK) == 0)
					{
						ShowBMP(fileName,screen);
					}
					else 
					{
						//printf("File %s not exist\n",fileName);
					}
            }
            
				break;
			case 1:
				if(b_audioRun == true)
				{
					sprintf(fileName,"./wav/%s",cmd.fileName);
					
					if (access(fileName, R_OK) == 0)
					{
						SDL_PauseAudio(1);
						SDL_LockAudio();
						free(explosion->samples);
						SDL_UnlockAudio();
						//sprintf(fileName,"./wav/%s",cmd.fileName);
						
						if (LoadAndConvertSound(fileName,obtained, explosion) != 0)
						{
							//printf("Unable to load sound. %s\n",fileName);
							ClearPlayingSounds();
							//printf("Unable to load sound. %s\n",fileName);
							//return  1;
						}
						else
						{
							ClearPlayingSounds();
							/*  SDL・s  audio  is  initially  paused.  Start  it.  */
							SDL_PauseAudio(0);
							//printf("Play %s \n",fileName);
							PlaySound(explosion);
						}
					}
				}
				
				break;
		}
			
		SDL_mutexV ( value_mutex );
	}
}

bool FileExists(char* strFilename) 
{ 
	struct stat stFileInfo;
	bool blnReturn; 
	int intStat; 

	// Attempt to get the file attributes 
	intStat = stat(strFilename,&stFileInfo); 
	
	if(intStat == 0) { 
		// We were able to get the file attributes 
		// so the file obviously exists. 
		blnReturn = true; 
	}
	else
	{
		// We were not able to get the file attributes. 
		// This may mean that we don't have permission to 
		// access the folder which contains this file. If you 
		// need to do that level of checking, lookup the 
		// return values of stat which will give you 
		// more details on why stat failed. 
		blnReturn = false; 
	}
	
	return(blnReturn); 
}