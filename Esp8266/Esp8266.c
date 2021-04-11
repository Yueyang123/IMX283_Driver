#include"usart.h"
#include<stdio.h>      /*标准输入输出定义*/    
#include<stdlib.h>     /*标准函数库定义*/    
#include<unistd.h>     /*Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*文件控制定义*/    
#include<termios.h>    /*PPSIX 终端控制定义*/    
#include<errno.h>      /*错误号定义*/    
#include<string.h>

int fp = -1; 
char rcv_buf[100];  
pthread_mutex_t mutex;
void *uart_read()
{
    int len; 
    while(1)
    {
        len = UART0_Recv(fp, rcv_buf,sizeof(rcv_buf));    
        if(len > 0)    
        {    
            rcv_buf[len] = '\0';     
            printf("%s\n",rcv_buf); 
        }      
    }
}

char *AT_TCP_CLINT[6]={
                       "AT+CWMODE=1\r\n",
                       "AT+CWJAP=\"YY\",\"12345678\"\r\n",
                       "AT+CIPMUX=0\r\n",
                       "AT+CIPSTART=\"TCP\",\"47.114.134.173\",8081\r\n",
                       "AT+CIPMODE=1\r\n",
                       "AT+CIPSEND\r\n"};
void *uart_send()
{
    UART0_Send(fp,AT_TCP_CLINT[0],13);
    sleep(1);
    UART0_Send(fp,AT_TCP_CLINT[1],26);
    sleep(15);
    UART0_Send(fp,AT_TCP_CLINT[2],13);
    sleep(1);
    UART0_Send(fp,AT_TCP_CLINT[3],41);
    sleep(1); 
    UART0_Send(fp,AT_TCP_CLINT[4],14);
    sleep(1); 
    UART0_Send(fp,AT_TCP_CLINT[5],12);
    sleep(1);
    while(1)
    {
        UART0_Send(fp,"alive",5);
    }
}


void EspInit()
{
    int err;
    pthread_t th,th1;
    pthread_mutex_init(&mutex,NULL);
    fp = UART0_Open(fp,"/dev/ttySP1"); //打开串口，返回文件描述符 
    do  
    {    
        err = UART0_Init(fp,115200,0,8,1,'N');    
        printf("Set Port Exactly!\n");    
    }while(FALSE == err || FALSE == fp);

    pthread_create(&th,NULL,(void*)uart_read,NULL);
    pthread_create(&th1,NULL,(void*)uart_send,NULL);

    pthread_join(th1,NULL);
    
}