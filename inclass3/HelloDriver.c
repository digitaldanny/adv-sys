#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 

int main(void)
{
    char string[] = "Hello Device Driver!";
    int len = sizeof(string);

    int fd1 = open("/dev/tux0", O_RDWR);
    int fd2 = open("/dev/tux0", O_RDWR);

    write(fd1, string, len);
    read(fd2, string, len);

    close(fd1);
    close(fd2);
    return 0;
}