#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include <asm/ioctls.h>
#include <time.h>
#include <pthread.h>


int main(int argc ,char** argv)
{
	int fd;
	
	fd = open("/dev/NXP74HC595-1", O_RDWR);
	if (fd < 0) {
		perror("open /dev/NXP74HC595-1");
	}

        printf("test write....\n");
        write(fd, argv[1], 2);
    return 0;    
}
