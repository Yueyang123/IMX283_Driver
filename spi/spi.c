#include <stdint.h> 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev1.0"
#define GPIO_DEVICE "/sys/class/gpio/gpio117/value"  /* gpio3.21 的属性文件  */
/* 显示数值和位选值的对照表 0 1 2 3 4 5 6 7 8 9 */
uint8_t led_value_table[] = {0xC0, 0xF9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 10000;
static uint16_t delay = 0;
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

    write(fd_gpio, "0", 1);
    usleep(100);
    write(fd_gpio, "1", 1);
}
int main(int argc, char *argv[])
{
    int ret = 0;
    int fd_spi = 0;
    int fd_gpio = 0;
    int led_value = 0;
    int led_num = 0;

    if (argc != 3) {  /* 输入参数必须为两个  */
    printf("cmd : ./spi_led_test led_value led_num \n ");
    return -1;
    }
    led_value = atoi(argv[1]);  /* 获取程序输入参数的数码管的显示值  */
    if ((led_value) < 0 || (led_value > 9)) { /* 该值必须在 0 ~ 9 之间 */
    printf("led num just in 0 ~ 9 \n");
    return -1;
    }
    led_num = atol(argv[2]);  /* 获取程序输入参数的数字选择值  */
    if ((led_num < 0) || (led_num > 3)) {  /* 该值必须在 0 ~ 3 之间 */
    printf("led number just in 0 ~ 3");
    return -1;
    }
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
    /*
    * 这里 mode 的值为 0，这是 SPI 总线的 SPI_CLK 在上升沿阶段，SPI_DIN 的信号有效，
    * 这符合 74HC595 芯片把输入数据送入移位寄存器的要求。
    */
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
    show_led_num(fd_spi, fd_gpio, led_value, led_num);  /* 实现数码管的控制 */
    close(fd_spi);
    return ret;
}