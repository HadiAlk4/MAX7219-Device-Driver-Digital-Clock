#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>


#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3     _IOW(SCREEN_TO_MAGIC,3,char)
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

        sprintf(time_string, "%02d-%02d-%02d", normal_time, time_data->tm_min, time_data->tm_sec);


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
                printf("Command Typed: %s", fgString);
            }
        }
    }
    return 0;
}
