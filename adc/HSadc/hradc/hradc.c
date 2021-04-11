#include<stdio.h>	/* using printf()        */
#include<stdlib.h>      /* using sleep()         */
#include<fcntl.h>       /* using file operation  */
#include<sys/ioctl.h>   /* using ioctl()         */
#include <asm/ioctls.h>
#include <unistd.h> //sleep  write read close
 #include <time.h>
int main(int argc, const char * argv [ ])
{
  int fd;
  int i=0,value = 0;
  unsigned char buf[2*8192];
clock_t start, end;        //用于保存当前时间
   
  fd = open("/dev/imx283_hsadc",O_RDWR);
  if(fd < 0)
  {
     printf("open imx283_hsadc error %d\n",fd);
	 return 0;
  }
  start = clock();           /*记录起始时间*/
    for(i=0;i<(sizeof(buf)/sizeof(buf[0]))/2;i++){
        read(fd,buf+2*i,2);
        value = buf[0+2*i]<<8 | buf[1+2*i];
    }
    end = clock();
    double seconds  =(double)(end - start)/CLOCKS_PER_SEC;
    printf("Use time is: %.10f\n", seconds);
  return 0;
}
