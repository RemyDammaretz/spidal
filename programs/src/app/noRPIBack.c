#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define MAX_DATA_LEN 100

int out;
char data[MAX_DATA_LEN];
int length;
int btn1Status = 0;
int btn2Status = 0;
int pedal1Status = 0;
int pedal2Status = 0;
int remoteStatus = 0;

void sendData();

int main(int argc, char * argv[]) {
    char action;
    struct timespec time;

    if (argc != 2) {
        out = STDOUT_FILENO;
    } else {
        out = atoi(argv[1]);
    }

    while(1) {
		scanf("%s", &action);
	    clock_gettime(CLOCK_REALTIME, &time);
		switch(action) {
            case 'a':
                btn1Status = (btn1Status + 1) % 2;
                length = sprintf(data, "BTN1:%d:%ld:%ld\n", btn1Status, time.tv_sec, time.tv_nsec);
                sendData();
                break;
            case 'z':
                btn2Status = (btn2Status + 1) % 2;
                length = sprintf(data, "BTN2:%d:%ld:%ld\n", btn2Status, time.tv_sec, time.tv_nsec);
                sendData();
                break;
            case 'q':
                pedal1Status = (pedal1Status + 1) % 2;
                length = sprintf(data, "PEDAL1:%d:%ld:%ld\n", pedal1Status, time.tv_sec, time.tv_nsec);
                sendData();
                break;
            case 's':
                pedal2Status = (pedal2Status + 1) % 2;
                length = sprintf(data, "PEDAL2:%d:%ld:%ld\n",pedal2Status, time.tv_sec, time.tv_nsec);
                sendData();
                break;
            case 'e':
                remoteStatus = (remoteStatus + 1) % 2;
                length = sprintf(data, "REMOTE:%d:0:0\n",remoteStatus);
                sendData();
                break;
        }
    }

}

void sendData() {
    write(out, data, length+1);
}