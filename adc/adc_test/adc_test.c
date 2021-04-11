/*
 * @Author: your name
 * @Date: 2021-04-02 22:58:30
 * @LastEditTime: 2021-04-02 23:01:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/adc/adc_test/adc_test.c
 */
#include<stdio.h>  /* using printf() */
#include<stdlib.h> /* using sleep() */
#include<fcntl.h> /* using file operation */
#include<sys/ioctl.h> /* using ioctl() */
#include "adc_test.h"
int main(int argc, char *argv[])
{
    int fd;
    int iRes;
    int time = 50;
    double val;
    fd = open("/dev/magic-adc", 0);
    if(fd < 0) {
    printf("open error by APP- %d\n",fd);
    close(fd);
    return 0;
    }
    while(time--) {
    sleep(1); 
    ioctl(fd,IMX28_ADC_CH0_DIV2, &iRes);  /* 开启除 2 CH0 */
    val = (iRes * 3.7) / 4096.0;
    printf("CH0:%.2f ", val);
    ioctl(fd,IMX28_ADC_CH1, &iRes);  /* 不开除 2 CH1 */
    val = (iRes * 1.85) / 4096.0;
    printf("CH1:%.2f ", val);
    #if 0 /* 对于 EasyARM-iMX280A，请设置为 1 */
        ioctl(fd,IMX28_ADC_CH2_DIV2, &iRes);  /* 开启除 2 CH2 */
        val = (iRes * 3.7) / 4096.0;
        printf("CH2:%.2f ", val);
        ioctl(fd, IMX28_ADC_CH3, &iRes);  /* 不开除 2 CH3 */
        val = (iRes * 1.85) / 4096.0;
        printf("CH3:%.2f ", val);
        ioctl(fd,IMX28_ADC_CH4_DIV2, &iRes);  /* 开启除 2 CH4 */
        val = (iRes * 3.7) / 4096.0;
        printf("CH4:%.2f ", val);
        ioctl(fd,IMX28_ADC_CH5, &iRes);  /* 不开除 2 CH5 */
        val = (iRes * 1.85) / 4096.0;
        printf("CH5:%.2f ", val);
    #endif
    ioctl(fd,IMX28_ADC_CH6_DIV2, &iRes);  /* 开启除 2 CH6 */
    val = (iRes * 3.7) / 4096.0;
    printf("CH6:%.2f ", val);
    ioctl(fd,IMX28_ADC_VBAT_DIV4, &iRes); /* 电池电压默认除 4 */
    val = (iRes * 7.4) / 4096.0;
    printf("Vbat:%.2f ", val);
    printf("\n");
}
close(fd);
}