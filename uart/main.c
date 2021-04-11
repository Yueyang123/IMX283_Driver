#include "usart.h"
#include<stdio.h>      /*标准输入输出定义*/    
#include<stdlib.h>     /*标准函数库定义*/    
#include<unistd.h>     /*Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*文件控制定义*/    
#include<termios.h>    /*PPSIX 终端控制定义*/    
#include<errno.h>      /*错误号定义*/    
#include<string.h>
#include <pthread.h>

int fd = -1; 
char rcv_buf[100];  
void *uart_read()
{
    int len; 
    while(1)
    {
            len = UART0_Recv(fd, rcv_buf,sizeof(rcv_buf));    
            if(len > 0)    
            {    
                rcv_buf[len] = '\0';     
                printf("%s\n",rcv_buf); 
            }      
    }
}

int main(int argc, char **argv)    
{    
    int err;               //返回调用函数的状态    
    pthread_t th;  
    int i;             
    char send_buf[40];
    if(argc != 3)    
    {    
        printf("Usage: %s /dev/ttySn 1(send data)/1 (receive data) \n",argv[0]);
        printf("open failure : %s\n", strerror(errno));
        return FALSE;    
    }    
     fd = UART0_Open(fd,argv[1]); //打开串口，返回文件描述符   
     do  
    {    
        err = UART0_Init(fd,115200,0,8,1,'N');    
        printf("Set Port Exactly!\n");    
    }while(FALSE == err || FALSE == fd);
      
      pthread_create(&th,NULL,(void*)uart_read,NULL);
        while (1) //循环读取数据    
        {   
            
            UART0_Send(fd,"1234",4);
            sleep(2);    
        }                
        UART0_Close(fd);     
}    