#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <sys/select.h>


#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3     _IOW(SCREEN_TO_MAGIC,3,char)
#define DEVICE_NAME "max_screen"


int main()
{
    int fd = open("/dev/max_screen", O_RDWR);
    char fgString[20] = {0};

    fd_set read;
    struct timeval halt;

    if(fd < 0)
    {
        perror("failed to open device");
        return -1;
    }
    printf("testing start \n");

    while(1)
    {
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
