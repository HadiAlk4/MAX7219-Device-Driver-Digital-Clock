#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

#define bits 6


#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)


int main()
{
    int fd = open("/dev/max_screen", O_RDWR);
    
    char temp[9] = {0};
    if(fd < 0)
    {
        perror("failed to open device");
        return -1;
    }
    
    write(fd, "12345E", bits);
    sleep(2);
    
    lseek(fd, 0, SEEK_SET);
        sleep(2);
            ioctl(fd, SCREEN_CTL1_DISABLE);
    sleep(2);


    write(fd, "34", 2);
        sleep(2);

    read(fd, temp, bits);
        sleep(2);

    printf("%s\n", temp);
    
    ioctl(fd, SCREEN_CTL1_DISABLE);
    sleep(2);

    
    
    close(fd);
}
