//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <errno.h>
//#include <unistd.h>
//
//#define bits 6
//
//
//#define SCREEN_TO_MAGIC 'k'
//#define SCREEN_CTL1_DISABLE     _IO(SCREEN_TO_MAGIC, 1)
//
//
//int main()
//{
//    int fd = open("/dev/max_screen", O_RDWR);
//    
//    char temp[9] = {0};
//    if(fd < 0)
//    {
//        perror("failed to open device");
//        return -1;
//    }
//    
//    write(fd, "12345E", bits);
//    sleep(2);
//    
//    lseek(fd, 0, SEEK_SET);
//        sleep(2);
//            ioctl(fd, SCREEN_CTL1_DISABLE);
//    sleep(2);
//
//
//    write(fd, "34", 2);
//        sleep(2);
//
//    read(fd, temp, bits);
//        sleep(2);
//
//    printf("%s\n", temp);
//    
//    ioctl(fd, SCREEN_CTL1_DISABLE);
//    sleep(2);
//
//    
//    
//    close(fd);
//}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

#define bits 6

// Make sure these match your char_driver.c file exactly
#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2         _IOR(SCREEN_TO_MAGIC, 2, char)
#define SCREEN_CTL3         _IOW(SCREEN_TO_MAGIC, 3, char)

int main()
{
    int fd = open("/dev/max_screen", O_RDWR);
    char temp[9] = {0};
    unsigned char brightness; // Variable to send brightness data
    unsigned char status;     // Variable to receive dummy data

    if(fd < 0)
    {
        perror("failed to open device");
        return -1;
    }
    
    printf("1. Writing 12345E to screen...\n");
    write(fd, "12345E", bits);
    sleep(2);
    
    // --- NEW IOCTL TESTS ---
    printf("2. Testing IOCTL 2: Setting brightness to LOW (0x01)...\n");
    brightness = 0x01;
    ioctl(fd, SCREEN_CTL2, &brightness);
    sleep(2); // You should see the LEDs physically dim

    printf("3. Testing IOCTL 2: Setting brightness to MAX (0x0F)...\n");
    brightness = 0x0F;
    ioctl(fd, SCREEN_CTL2, &brightness);
    sleep(2);

    printf("4. Testing IOCTL 3: Reading dummy status...\n");
    ioctl(fd, SCREEN_CTL3, &status);
    printf("   -> Driver returned status: 0x%X\n", status);
    sleep(2);
    // -----------------------

    printf("5. Disabling/Clearing screen...\n");
    lseek(fd, 0, SEEK_SET);
    ioctl(fd, SCREEN_CTL1_DISABLE);
    sleep(2);

    printf("6. Writing 34...\n");
    write(fd, "34", 2);
    sleep(2);

    printf("7. Reading data back from memory...\n");
    lseek(fd, 0, SEEK_SET); // IMPORTANT: Move cursor back to start before reading
    read(fd, temp, 2);      // Read exactly the 2 bytes we just wrote
    printf("   -> Memory buffer output: %s\n", temp);
    sleep(2);
    
    printf("8. Disabling screen and closing...\n");
    ioctl(fd, SCREEN_CTL1_DISABLE);
    close(fd);
    
    return 0;
}
