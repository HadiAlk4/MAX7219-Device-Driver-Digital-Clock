#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <string.h>


#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2_WRITE     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3_READ     _IOW(SCREEN_TO_MAGIC,3,char)
#define DEVICE_NAME "max_screen"


int main()
{
    int fd = open("/dev/max_screen", O_RDWR);
    char fgString[20] = {0};

    fd_set read; // check the origin of this 
    struct timeval halt; // check the struct of this 

    time_t raw_time_int;
    struct tm *time_data;
    char time_string[20];
    char alarm_string[20];
    int mode = 0;
    char brightness = 0x01;
    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);


    if(fd < 0)
    {
        perror("failed to open device");
        return -1;
    }
    printf("testing start \n");

    while(1)
    {

        time(&raw_time_int);
        time_data = localtime(&raw_time_int);

        int normal_time = time_data->tm_hour % 12;
        if (normal_time == 0) 
        {
            normal_time = 12; 
        }

        if(mode == 0) sprintf(time_string, "%02d-%02d-%02d", normal_time, time_data->tm_min, time_data->tm_sec);
        if(mode == 1) sprintf(time_string, "%02d-%02d-%02d", time_data->tm_mday, time_data->tm_mon, time_data->tm_year);
        
        sprintf(alarm_string, "%02d-%02d-%02d", normal_time, time_data->tm_min, time_data->tm_sec + 10);
        
        

        lseek(fd, 0, SEEK_SET);
        write(fd, time_string, 8);

        FD_ZERO(&read);
        FD_SET(0, &read);

        halt.tv_sec = 0;
        halt.tv_usec = 100000;

        if(select(1, &read, NULL, NULL, &halt) > 0)
        {

            if(fgets(fgString, sizeof(fgString), stdin) != NULL)
            {
                if(fgString[0] == 27)
                {
                    printf("\n Exiting digital clock...\n");
                    break;
                }
                printf("Command Typed: %s", fgString);
                fgString[strcspn(fgString, "\r\n")] = 0;
                if(strcmp(fgString, "date") == 0)
                {
                    mode = 1; 
                }
                else if(strcmp(fgString, "time") == 0)
                {
                    mode = 0; 
                }
                
                else if(fgString[0] == '>')
                {
                    brightness += 0x01 ;
                    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);                    
                }
                else if(fgString[0] == '<')
                {
                    brightness -= 0x01;
                    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
                }
                
            }
        }
    }
    return 0;
}
