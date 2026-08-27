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
#define SCREEN_CTL4_TOGGLE      _IOW(SCREEN_TO_MAGIC, 4, int)  
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

    time_t alarm_time = 0;
    int alarm_active = 0;


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

        if(alarm_active && raw_time_int >= alarm_time)
        {
            int alarm_stopped = 0;
            int scroll_pos = 0;
            char msg[] = "        -----        "; // check over this 
            int msg_len = strlen(msg);

            while(!alarm_stopped)
            {
                char frame[9] = {0};
                strncpy(frame, &msg[scroll_pos], 8);
                lseek(fd, 0, SEEK_SET);
                write(fd, frame, 8);

                scroll_pos++;

                if(scroll_pos > msg_len - 8) scroll_pos = 0;

                FD_ZERO(&read);
                FD_SET(0, &read);

                halt.tv_sec = 0;
                halt.tv_usec = 200000; 

                if(select(1, &read, NULL, NULL, &halt) > 0)
                {
                    if(fgets(fgString, sizeof(fgString), stdin) != NULL)
                    {
                        if(fgString[0] == '#')
                        {
                            alarm_active = 0;
                            alarm_stopped = 1;
                            printf("alarm stop");
                        }
                    }
                }
            }
        }

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

        lseek(fd, 7, SEEK_SET);

        int dot = (time_data->tm_sec % 2);
        ioctl(fd, SCREEN_CTL4_TOGGLE, &dot);

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
                    ioctl(fd, SCREEN_CTL1_DISABLE);
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
                    if(brightness < 0x0F) brightness += 0x01 ;
                    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);                    
                }
                else if(fgString[0] == '<')
                {
                    if(brightness > 0x00) brightness -= 0x01;
                    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
                } 
                else if(strncmp(fgString, "alarm ", 6) == 0)  // check over this 
                {
                    int offset = atoi(fgString + 6); 
                    alarm_time = raw_time_int + offset;
                    alarm_active = 1;
                    printf("-> Alarm set for %d seconds.\n", offset);
                }
                
            }
        }
    }
    close(fd);
    return 0;
}
