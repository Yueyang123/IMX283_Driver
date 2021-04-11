#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/bcd.h>
#include <linux/capability.h>
#include <linux/rtc.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>


static const struct i2c_device_id FM24C02A_id[] = {
    { "FM24C02A", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, FM24C02A_id); /* 进一步初始化 FM24C02A_id */
#define DEVICE_NAME "FM24C02A"
static struct i2c_client *g_client;



#define CMD_SET_ROM_ADDR 0x1
static char rom_addr; /* 用于保存 FM24C02A 内部储存器的读/写地址 */
static int FM24C02A_ioctl(struct inode *inode,struct file *flip,unsigned int command,unsigned long arg)
{
if (command == CMD_SET_ROM_ADDR) {
rom_addr = arg; /* FM24C02A内部储存器的读/写地址在arg参数中传入*/
}
return 0;
}

static ssize_t FM24C02A_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    int ret = 0;
    char data_buf[256 + 1];
    data_buf[0] = rom_addr;  

    copy_from_user(data_buf + 1, buf, count);  /* 在用户空间复制要写入的数据  */
    ret = i2c_master_send(g_client, data_buf, count);  /* 把要写入的数据发送到 FM24C02A*/
    return ret;
}

static ssize_t FM24C02A_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    char data_buf[256];
    ret = i2c_master_send(g_client, &rom_addr, 1);  /* 向 FM24C02A 发送要读取数据的起始地址*/
    ret = i2c_master_recv(g_client, data_buf, count);  /* 在 FM24C02A 连续读取数据 */
    copy_to_user(buf, data_buf, count);  /* 把读取的数据返回给用户空间  */
    return ret;
}


static struct file_operations FM24C02A_fops={
    .owner = THIS_MODULE,
    .write = FM24C02A_write,
    .read = FM24C02A_read,
    .ioctl = FM24C02A_ioctl,
};

static struct miscdevice FM24C02A_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &FM24C02A_fops,
};



static int FM24C02A_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err = 0;
    printk("FM24C02A device is detected \n");
    g_client = client;
    err = misc_register(&FM24C02A_miscdev); /* 生成字符设备  */
    if (err) {
    printk("register FM24C02A device faile \n");
    return -1;
    }
    return 0;
}

static struct i2c_driver FM24C02A_driver = {
    .driver  = {
    .name  = "FM24C02A",
    },
    .probe  = FM24C02A_probe,
    .id_table = FM24C02A_id,
};


static int __init FM24C02A_init(void)
{
    return i2c_add_driver(&FM24C02A_driver);
}

static void __exit FM24C02A_exit(void){
    i2c_del_driver(&FM24C02A_driver);
}
module_init(FM24C02A_init);
module_exit(FM24C02A_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YangYue");