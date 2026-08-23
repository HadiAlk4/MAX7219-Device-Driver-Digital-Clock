
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

/*
 * This test program only ever writes characters the MAX7219's Code B
 * decode mode can actually render: '0'-'9', '-', E, H, L, P (upper or
 * lower case). Any other character (A, B, C, ..., I, O, S, T, ...) is
 * rendered blank by the driver - that is a hardware decode-mode
 * limitation (see Table 5 of the datasheet), not something this test
 * should rely on.
 */

/* Match the IOCTL definitions from char_driver.c exactly */
#define SCREEN_TO_MAGIC 'k'
#define SCREEN_CTL1_DISABLE _IO(SCREEN_TO_MAGIC, 1)
#define SCREEN_CTL2_WRITE   _IOW(SCREEN_TO_MAGIC, 2, unsigned char)
#define SCREEN_CTL3_READ    _IOR(SCREEN_TO_MAGIC, 3, unsigned char)
#define SCREEN_CTL4_TOGGLE  _IOW(SCREEN_TO_MAGIC, 4, int)

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
    printf("\n--- STARTING COMPREHENSIVE DRIVER TEST ---\n\n");

    // 1. EARLY IOCTL CONNECTION TEST
    printf("1. Testing IOCTL Connection (Reading default brightness)...\n");
    if (ioctl(fd, SCREEN_CTL3_READ, &brightness) == 0) {
        printf("   -> SUCCESS: Connected! Default brightness is 0x%X\n", brightness);
    } else {
        perror("   -> FAILED: IOCTL connection issue");
        close(fd);
        return -1;
    }
    sleep(2);

    // 2. WRITE & WRAP-AROUND TEST
    // 10 characters written to an 8-digit display: the last 2 wrap
    // around and overwrite the first 2. Every character here (digits,
    // '-', E, L) is a valid Code B character, so what you see IS what
    // was written - no blanks.
    printf("\n2. Testing Write and Wrap-Around...\n");
    printf("   Writing '1234567-EL'. Screen should show 'EL34567-'.\n");
    write(fd, "1234567-EL", 10);
    sleep(3);

    // 3. SEEK MODES TEST
    printf("\n3. Testing Seek Modes (Watch the decimal point cursor move!)...\n");
    printf("   SEEK_SET to 0...\n");
    lseek(fd, 0, SEEK_SET);
    sleep(2);
    printf("   SEEK_CUR by +4...\n");
    lseek(fd, 4, SEEK_CUR);
    sleep(2);
    printf("   SEEK_END by -1...\n");
    lseek(fd, -1, SEEK_END);
    sleep(2);

    // 4. BOUNDS-CHECKED READ TEST
    // read() returns the characters from the left-hand side up to the
    // CURRENT cursor position, so we read right after the write while
    // the cursor still sits just past what we wrote. Seeking to 0 first
    // would make read() correctly return nothing, since there'd be
    // nothing between the left edge and a cursor sitting at 0.
    printf("\n4. Testing Bounds-Checked Read...\n");
    lseek(fd, 0, SEEK_SET);      // Reset to a known start position
    write(fd, "1279", 4);        // Cursor is now at position 4
    printf("   Cursor is at 4. Reading back what we just wrote...\n");
    int bytes_read = read(fd, temp_buf, 8);
    printf("   -> Driver returned %d byte(s). Data: '%s'\n", bytes_read, temp_buf);
    sleep(3);

    // 5. BRIGHTNESS IOCTL TEST
    printf("\n5. Testing Brightness IOCTLs...\n");
    printf("   Setting brightness to LOW (0x01)...\n");
    brightness = 0x01;
    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
    sleep(2);

    printf("   Setting brightness to MAX (0x0F)...\n");
    brightness = 0x0F;
    ioctl(fd, SCREEN_CTL2_WRITE, &brightness);
    sleep(2);

    // 6. CURSOR TOGGLE IOCTL TEST
    printf("\n6. Testing Cursor Toggle IOCTL...\n");
    printf("   Turning cursor OFF...\n");
    cursor_toggle = 0;
    ioctl(fd, SCREEN_CTL4_TOGGLE, &cursor_toggle);
    sleep(2);

    printf("   Turning cursor ON...\n");
    cursor_toggle = 1;
    ioctl(fd, SCREEN_CTL4_TOGGLE, &cursor_toggle);
    sleep(2);

    // 7. RESET & CLEAR IOCTL TEST
    printf("\n7. Testing Reset/Clear IOCTL...\n");
    printf("   Clearing screen and resetting cursor...\n");
    ioctl(fd, SCREEN_CTL1_DISABLE);
    sleep(2);
    printf("   Writing 'HELP' to verify cursor was reset to 0...\n");
    write(fd, "HELP", 4);
    sleep(2);

    printf("\n--- TESTS COMPLETE. CLOSING DEVICE ---\n");
    ioctl(fd, SCREEN_CTL1_DISABLE);
    close(fd);
    return 0;
}
