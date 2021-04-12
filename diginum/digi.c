/*
 * @Author: your name
 * @Date: 2021-04-11 16:18:34
 * @LastEditTime: 2021-04-11 18:22:55
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/diginum/digi.c
 */
#include <stdint.h> 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <pthread.h>
#define SPI_DEVICE "/dev/spidev1.0"
#define GPIO_DEVICE "/dev/gpio-P3.21"  /* gpio3.21 的属性文件  */
#define RUN_VALUE "/tmp/digi"
/* 显示数值和位选值的对照表 0 1 2 3 4 5 6 7 8 9 */
uint8_t led_value_table[] = {0xC0, 0xF9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 10000;
static uint16_t delay = 0;
enum {
	SET_GPIO_HIGHT = 9,
	SET_GPIO_LOW,
	GET_GPIO_VALUE,
};
static void show_led_num(int fd_spi, int fd_gpio, int value, int num)
{
        int ret;
        uint8_t tx[] = {
        led_value_table[value], /* 把显示数值转化为位选值  */
        (1 << num), /*把数字选择值转化为段选值  */
        };
        struct spi_ioc_transfer tr_txrx[] = {
        {
        .tx_buf = (unsigned long)tx,
        .rx_buf = 0,
        .len = 2,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
        },
        };
        /* 把位选值和段选值通过 SPI 总线发送到 U4 和 U6 的移位寄存器 */
        ret = ioctl(fd_spi, SPI_IOC_MESSAGE(1), &tr_txrx[0]);
        if (ret == 1) {
        printf("can't revieve spi message");
        return;
        }
        ioctl(fd_gpio,SET_GPIO_LOW);
        usleep(100);
        ioctl(fd_gpio,SET_GPIO_HIGHT);

}
int fd_spi = 0;
int fd_gpio = 0;
int led_value[4] = {0};
void *Print(void *threadid){ /* 线程函数  */
    int i=0;
    while(1)
    {
    for(i=0;i<4;i++)
    show_led_num(fd_spi, fd_gpio, led_value[i], i); 
    }
}

int main(int argc, char *argv[])
{
    int ret = 0,i=0,rc=0;
    pthread_t thread;
    long t;
    int value;
    int led_num = 0;
    void *status;
    if (argc != 2) {  /* 输入参数必须为两个  */
    printf("cmd : ./spi_led_test led_value \n ");
    return -1;
    }
    
    value = atoi(argv[1]);  /*获取程序输入参数的数码管的显示值  */
    led_value[0]=value/1000;      
    led_value[1]=(value%1000)/100;
    led_value[2]=(value%100)/10;
    led_value[3]=(value%10);
    
    fd_spi = open(SPI_DEVICE, O_RDWR);  /* 打开 SPI 总线的设备文件  */
    if (fd_spi < 0) {
    printf("can't open %s \n", SPI_DEVICE);
    return -1;
    }

    fd_gpio = open(GPIO_DEVICE, O_RDWR); /* 打开 GPIO 设备的属性文件 */
    if (fd_gpio < 0) {
    printf("can't open %s device\n", GPIO_DEVICE);
    return -1;
    }
    ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
    printf("can't set wr spi mode\n");
    return -1;
    }
    ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits); /* 设置 SPI 的数据位 */
    if (ret == -1) {
    printf("can't set bits per word\n");
    return -1;
    }
    ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed); /* 设置 SPI 的最大总线频率  */
    if (ret == -1)  {
    printf("can't set max speed hz\n");
    return -1;
    }
    rc = pthread_create(&thread, NULL, Print, (void *)t); /* 创建线程  */
    if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
    }
    rc = pthread_join(thread, &status);
    close(fd_gpio);
    close(fd_spi);
    return ret;
}