#include "RCSwitch.h"
#include "TCPServer.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>

#define SYSTEMCODE      "11011"
#define CRONFILENAME    "/home/pi/cronFile.txt"

TCPServer tcp;
RCSwitch mySwitch = RCSwitch();
int minute_mo_fr=0, minute_we=0, hour_mo_fr=0, hour_we=0;

char * int2charBin(int in, int bits)
{
    char out[bits+1];
    unsigned int mask = 1U << (bits-1);
    int i;
    for (i = 0; i < bits; i++) {
        out[i] = (in & mask) ? '1' : '0';
        in <<= 1;
    }
    out[bits]=' ';
    return out;
}

int editCronjob()
{
    char com[100];
    FILE *cronFile = fopen(CRONFILENAME, "w+");

    fprintf(cronFile, "%i %i * * 1-5 /home/pi/Haussteuerung/Morgenlicht/bin/Release/Morgenlicht \n", minute_mo_fr, hour_mo_fr);
    fprintf(cronFile, "%i %i * * 6-7 /home/pi/Haussteuerung/Morgenlicht/bin/Release/Morgenlicht \n", minute_we, hour_we);
    fclose(cronFile);
    system("sudo crontab -r");
    sprintf(com, "sudo crontab %s", CRONFILENAME);
    system(com);

    return 0;
}

void * loop(void * m)
{
    char msg[1024], sendBuffer[50];
    //char systemcode[6], farbe[2];
    //int wert;
    string message;
    int bit = 0, msgVal = 0;
    char *sVal = (char*) malloc(sizeof(char*) * 20);
    char sWert[20];
    char msgInfo[20], msgData[255], msgCom[50], msgDev[50];
    bool running = true;

    pthread_detach(pthread_self());

	while(running)
	{
        message = tcp.getMessage();
        strcpy(msg, message.c_str());
        memset((char*)msgInfo, 0, sizeof(char)*20);
        memset((char*)msgData, 0, sizeof(char)*255);
        memset((char*)msgCom, 0, sizeof(char)*50);
        memset((char*)msgDev, 0, sizeof(char)*50);
        memset((char*)sendBuffer, 0, sizeof(char)*50);
        memset((char*)sVal, 0, sizeof(char)*20);

		if( message != "" )
		{
            printf("Nachricht empfangen: %s\n", msg);
            fflush(stdout);
            sscanf(msg, "%s %s", msgInfo, msgData);

            if( !strcmp(msgInfo, "INFO:") ){
                printf("%s\n", msgData);
            }
            else if( !strcmp(msgInfo, "COM:") ){
                sscanf(msg, "%*s %s %s %i", msgCom, msgDev, &msgVal);
                sVal = int2charBin(msgVal, 8);
                strcpy(sWert, sVal);
                if( !strcmp(msgCom, "set") ){
                    if( !strcmp(msgDev, "red") ){
                        sprintf(sendBuffer, "%s 11 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                    }
                    else if( !strcmp(msgDev, "blue") ){
                        sprintf(sendBuffer, "%s 01 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                    }
                    else if( !strcmp(msgDev, "green") ){
                        sprintf(sendBuffer, "%s 10 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                    }
                    else if( !strcmp(msgDev, "all") ){
                        sprintf(sendBuffer, "%s 01 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                        sprintf(sendBuffer, "%s 11 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                        sprintf(sendBuffer, "%s 10 %s ", SYSTEMCODE, sWert);
                        mySwitch.send(sendBuffer);
                    }
                    else printf("Error: Device ""%s"" not available", msgDev);
                }
                else if( !strcmp(msgCom, "stop") ){
                    if( !strcmp(msgDev, "app") ){
                        running = false;
                        printf("Verbindung beendet\n");
                    }
                }
                else if( !strcmp(msgCom, "mli") ){
                    if( !strcmp(msgDev, "mofr")){
                        sscanf(msg, "%*s %*s %*s %i:%i", &hour_mo_fr, &minute_mo_fr);
                        editCronjob();
                    }
                    else if( !strcmp(msgDev, "saso")){
                        sscanf(msg, "%*s %*s %*s %i:%i", &hour_we, &minute_we);
                        editCronjob();
                    }
                    else printf("Error: Device ""%s"" is not available", msgDev);
                }
                else printf("Error: Command ""%s"" is not available", msgCom);
            }
            else{
                printf("Error:\n");
            }
            tcp.clean();
		}
		usleep(1000);
	}
	tcp.detach();
}

int main() {

    int PIN = 0;


    if (wiringPiSetup () == -1) return 1;

	mySwitch.enableTransmit(PIN);

    pthread_t msg;

	tcp.setup(9999);

	if( pthread_create(&msg, NULL, loop, (void *)0) == 0)
	{
		tcp.receive();
	}

	return 0;
}
