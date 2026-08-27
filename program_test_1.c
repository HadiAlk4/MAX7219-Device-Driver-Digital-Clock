#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2_WRITE     _IOR(SCREEN_TO_MAGIC,2,char)
#define SCREEN_CTL3_READ     _IOW(SCREEN_TO_MAGIC,3,char)
#define SCREEN_CTL4_TOGGLE     _IOW(SCREEN_TO_MAGIC, 4, int)
#define DEVICE_NAME "max_screen"


int main()
{
    int fd = open("/dev/max_screen", O_RDWR);
    char temp_buf[16] = {0};
    unsigned char brightness;
    int cursor_toggle;

    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf(" Testing IOCTL Connection (Reading default brightness)...\n");
    if (ioctl(fd, SCREEN_CTL3_READ, &brightness) == 0) {
        printf("   -> SUCCESS: Connected! Default brightness is 0x%X\n", brightness);
    } else {
        perror("   -> FAILED: IOCTL connection issue");
        close(fd);
        return -1;
    }
    sleep(2);

    printf("   Writing '12345-EL'.\n");
    write(fd, "12456", 6);
    sleep(3);
    lseek(fd, 4, SEEK_CUR);
    write(fd, "12456", 2);



    printf("\n Testing Brightness IOCTLs...\n");
    printf("   Setting brightness to LOW (0x01)...\n");
    brightness = 0x01;
    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
    sleep(2);

    printf("   Setting brightness to MAX (0x0F)...\n");
    brightness = 0x0F;
    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
    sleep(2);


    printf("   Clearing screen \n");
    ioctl(fd, SCREEN_CTL1_DISABLE);
    sleep(2);
    printf("   Writing 'HELP' \n");
    write(fd, "HELP", 4);
    sleep(2);

    printf("\n--- TESTS COMPLETE. CLOSING DEVICE ---\n");
    ioctl(fd, SCREEN_CTL1_DISABLE);
    close(fd);
    return 0;
}
