#include <sys/ioctl.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef K_UNICODE
#define K_UNICODE 0x03
#endif

const char* tty = "/dev/tty";

int main(int argc, char** argv)
{
    int fd;
    if (argc > 1) {
        tty = argv[1];
    }
    fd = open(tty, O_RDONLY);
    if (fd < 0) {
        perror(tty);
        return 1;
    }
    if (ioctl(fd, KDSKBMODE, K_UNICODE) != 0) {
        perror("KDSKBMODE");
    }
    return 0;
}
