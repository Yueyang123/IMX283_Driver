/*
 * @Author: your name
 * @Date: 2021-04-02 22:58:30
 * @LastEditTime: 2021-04-04 23:13:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/adc/adc_test/adc_test.c
 */
#include<stdio.h> 
#include<stdlib.h> 
#include<fcntl.h> 
#include<sys/ioctl.h> 
#include <time.h>   
#include "adc_test.h"
int main(int argc, char *argv[])
{
    int fd;
    int i,RES;
    clock_t start, end;        //用于保存当前时间
    start = clock();           /*记录起始时间*/
    double val[8192];
    fd = open("/dev/magic-adc", 0);
    if(fd < 0) {
    printf("open error by APP- %d\n",fd);
    close(fd);
    return 0;
    }
    start = clock();
    for(i=0;i<sizeof(val)/sizeof(val[0]);i++){
        ioctl(fd,IMX28_ADC_CH0_DIV2, &RES);  /* 开启除 2 CH0 */
        val[i] = (RES * 3.7) / 4096.0;
    }
    end = clock();
    double seconds  =(double)(end - start)/CLOCKS_PER_SEC;
    printf("Use time is: %.10f\n", seconds);
close(fd);
}