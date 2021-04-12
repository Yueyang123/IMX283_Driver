/*
 * @Author: your name
 * @Date: 2021-04-11 23:03:45
 * @LastEditTime: 2021-04-11 23:04:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/PS2/test/ps2.c
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>

int main(int argc, char **argv)
{
    int fd;    
    struct input_event ev;

    // 判断参数
    if (argc < 2) {
        printf("Usage: %s <input device>\n", argv[0]);
        return 0;
    }

    // 打开设备
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        printf("open %s", argv[1]);
        fflush(stdout);
        perror(" ");
        return 0;
    }

    // 循环读取
    while(1) {
        // 读取数据
        read(fd, &ev, sizeof(struct input_event));
        // 打印当前触发类型
        printf("ev ==  %x \n",ev.type );   
switch(ev.type) {
        case EV_SYN:
            printf("-------------------------\n");    
            break;

        // 按键
        case EV_KEY:
            printf("key down / up: %d \n",ev.code );    
            break;

        // 鼠标
        case EV_REL:
            printf("mouse: ");    
            if (ev.code == REL_X) {
                printf(" x -- %d\n", ev.value);
            } else if (ev.code == REL_Y) {
                printf(" y -- %d\n", ev.value);
            }
            break;

        // 触摸屏
        case EV_ABS:
            printf("ts: ");            
            if(ev.code == ABS_X) {
                printf(" x -- %d\n", ev.value);
            } else if (ev.code == ABS_Y) {
                printf(" y -- %d\n", ev.value);
            } else if (ev.code == ABS_PRESSURE) {
                printf(" pressure: %d\n", ev.value);
            }
            break;
        }        
    }
    close(fd);
    return 0;
}