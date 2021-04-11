#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#define I2C_SLAVE 0x0703
#define I2C_TENBIT 0x0704
#define I2C_ADDR 0xA0
#define DATA_LEN 8
#define I2C_DEV_NAME "/dev/i2c-1"
int main(int arg,char*args[])
{
unsigned int ret,len;
int i,flag=0;
int fd;
char tx_buf[DATA_LEN + 1];  /* 用于储存数据地址和发送数据  */
char rx_buf[DATA_LEN];  /* 用于储存接收数据 */
char addr[1];  /* 用于储存读/写的数据地址  */
addr[0] = 0;  /* 数据地址设置为 0 */
fd = open(I2C_DEV_NAME, O_RDWR); /* 打开 I 2 C 总线设备 */
if(fd < 0) {
printf("open %s failed\n", I2C_DEV_NAME);
return -1;
}
ret = ioctl(fd, I2C_SLAVE, I2C_ADDR >> 1); /* 设置从机地址  */
if (ret < 0) {
printf("setenv address faile ret: %x \n", ret);
return -1;
}
/* 由于没有设置从机地址长度，所以使用默认的地址长度为 8 */
tx_buf[0] = addr[0]; /* 发数据时，第一个发送是数据地址 */
for (i = 1; i < DATA_LEN; i++)  /* 初始化要写入的数据：0、1„„7 */
tx_buf[i] = i;
len = write(fd, tx_buf, DATA_LEN + 1); /* 把数据写入到 FM24C02A， */
if (len < 0) {
printf("write data faile \n");
return -1;
}
usleep(1000*100);  /* 需要延迟一段时间才能完成写入 EEPROM  */
len = write(fd, addr, 1); /* 设置数据地址  */
if (len < 0) {
printf("write data addr faile \n");
return -1;
}
len = read(fd, rx_buf, DATA_LEN); /* 在设置的数据地址连续读入数据  */
if (len < 0) {
printf("read data faile \n");
return -1;
}
printf("read from eeprom:");
for(i = 0; i < DATA_LEN - 1; i++) {  /* 对比写入数据和读取的数据 */
printf(" %x", rx_buf[i]);

if (rx_buf[i] != tx_buf[i+1]) flag = 1;
}
printf("\n");
if (!flag) { /* 如果写入/读取数据一致，打印测试成功 */
printf("eeprom write and read test sussecced!\r\n");
} else {  /* 如果写入/读取数据不一致，打印测试失败  */
printf("eeprom write and read test failed!\r\n");
}
return 0;
}