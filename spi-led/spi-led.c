#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <mach/gpio.h>                                                  
#include <asm/io.h>                                                 
#include "mach/../../mx28_pins.h"
#include <mach/pinctrl.h>
#include "mach/mx28.h"
#include <linux/fs.h>
#include <linux/io.h>
#include <asm/uaccess.h>                                     
#include <linux/miscdevice.h>                          
#include <linux/irq.h>                          
#include <linux/sched.h>                   
#include <linux/interrupt.h>              
#include <linux/timer.h>

#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/device.h>

#define GPIO_74HC595_DATA_PIN        MXS_PIN_TO_GPIO(PINID_SSP3_MOSI)
#define GPIO_74HC595_STCP_PIN        MXS_PIN_TO_GPIO(PINID_SAIF0_LRCLK)
#define GPIO_74HC595_SHCP_PIN        MXS_PIN_TO_GPIO(PINID_SSP3_SCK)


int nxp74hc595_open(struct inode *, struct file *);
int nxp74hc595_release(struct inode *, struct file *);

ssize_t nxp74hc595_read(struct file *, char *, size_t, loff_t *);
ssize_t nxp74hc595_write(struct file *, const char *, size_t, loff_t *);


int dev_major = 666; 
int dev_minor = 0; 


char data[2]={0xFF,0x0f};//data[1]选择段码位 data[0]段码内容

static struct class *firstdrv_class;
static struct device *firstdrv_class_dev;
struct cdev *nxp74hc595_cdev; 


//将文件操作与分配的设备号相连
struct file_operations nxp74hc595_fops = {
    owner    : THIS_MODULE, 
    open    : nxp74hc595_open,
    release    : nxp74hc595_release,
    write    : nxp74hc595_write,
};

//初始化io引脚
static int __init gpio_74hc595_drv_init(void)
{
    int iRet;

    gpio_free(GPIO_74HC595_DATA_PIN);
    gpio_free(GPIO_74HC595_STCP_PIN);
    gpio_free(GPIO_74HC595_SHCP_PIN);


    iRet = gpio_request(GPIO_74HC595_DATA_PIN, "74HC595_DATA");
    if (iRet != 0) {
        printk("request 74HC595_DATA failed \n");
        return -EBUSY;
    }

    iRet = gpio_request(GPIO_74HC595_SHCP_PIN, "74HC595_SHCP");
    if (iRet != 0) {
        printk("request 74HC595_SHCP failed \n");
        return -EBUSY;
    }


    iRet = gpio_request(GPIO_74HC595_STCP_PIN, "74HC595_STCP");
    if (iRet != 0) {
        printk("request 74HC595_STCP failed \n");
        return -EBUSY;
    }

    gpio_direction_output(GPIO_74HC595_DATA_PIN,0);
    gpio_direction_output(GPIO_74HC595_STCP_PIN,0);
    gpio_direction_output(GPIO_74HC595_SHCP_PIN,0);

    printk("init all 74HC595 ctl gpio succ \n");
    return 0;

}


//向74HC595写入数据
void Write_74HC595(unsigned int ChipNum,unsigned char *DataBuf)
{
    unsigned char i = 0;
    unsigned char DataBufTmp = 0;
    
    gpio_set_value(GPIO_74HC595_STCP_PIN,0);//STCP
            
            
            DataBufTmp = *DataBuf;
            for(i=0; i<8; i++){
                gpio_set_value(GPIO_74HC595_SHCP_PIN,0);//SHCP
                if (DataBufTmp & 0x80){
                    gpio_set_value(GPIO_74HC595_DATA_PIN,1);
                }else{
                    gpio_set_value(GPIO_74HC595_DATA_PIN,0);
                }
                udelay(5);
                gpio_set_value(GPIO_74HC595_SHCP_PIN,1);
                udelay(5);        
                DataBufTmp = DataBufTmp << 1;
            }


            DataBufTmp = 8;
            gpio_set_value(GPIO_74HC595_STCP_PIN,0);
            for(i=0; i<8; i++){
            gpio_set_value(GPIO_74HC595_SHCP_PIN,0);//SHCP
            if (DataBufTmp & 0x80){
                gpio_set_value(GPIO_74HC595_DATA_PIN,1);
            }else{
                gpio_set_value(GPIO_74HC595_DATA_PIN,0);
            }
            udelay(5);
            gpio_set_value(GPIO_74HC595_SHCP_PIN,1);
            udelay(5);        
            DataBufTmp = DataBufTmp << 1;
            }

    gpio_set_value(GPIO_74HC595_STCP_PIN,1);
    udelay(10);
    gpio_set_value(GPIO_74HC595_STCP_PIN,0);
}


static void __exit nxp74hc595_exit(void) 
{
    char data[2]={0xFF,0x0f};

    dev_t devno = MKDEV(dev_major, dev_minor); 
    cdev_del(nxp74hc595_cdev); 
    kfree(nxp74hc595_cdev); 

    unregister_chrdev_region(devno, 1); 
    device_unregister(firstdrv_class_dev);
    class_destroy(firstdrv_class);


    Write_74HC595(2,data);
    gpio_free(GPIO_74HC595_DATA_PIN);
    gpio_free(GPIO_74HC595_STCP_PIN);
    gpio_free(GPIO_74HC595_SHCP_PIN);
    printk("NXP74hc595 unregister success\n");

}

static int __init nxp74hc595_init(void) 
{
    int ret, err;
    dev_t devno;


    ret=alloc_chrdev_region(&devno, dev_minor, 2, "NXP74hc595");
    dev_major=MAJOR(devno);

    if(ret<0){
    printk("my74hc595 register failure\n");
        return ret;
    }else{
        printk("74hc595 register success\n");
    }

    nxp74hc595_cdev = kmalloc(sizeof(struct cdev), GFP_KERNEL);

    cdev_init(nxp74hc595_cdev, &nxp74hc595_fops);

    nxp74hc595_cdev->owner = THIS_MODULE;         
    err = cdev_add(nxp74hc595_cdev, devno, 1);     
    if(err<0){
        printk("add device failure\n"); 
    }

    firstdrv_class = class_create(THIS_MODULE, "74HC595");
    firstdrv_class_dev = device_create(firstdrv_class, NULL, MKDEV(dev_major, 0), NULL,"NXP74HC595-%d", 1);

    gpio_74hc595_drv_init();
    Write_74HC595(2,data);


    printk("register 74HC595 dev OK\n");
    return 0;
}


int nxp74hc595_open(struct inode *inode, struct file *filp)
{
    printk("open my74hc595 dev OK\n");
    return 0;
}

int nxp74hc595_release(struct inode *inode, struct file *filp)
{
    Write_74HC595(2,data);
    gpio_free(GPIO_74HC595_DATA_PIN);
    gpio_free(GPIO_74HC595_STCP_PIN);
    gpio_free(GPIO_74HC595_SHCP_PIN);
    printk("close NXP74HC595 dev OK\n");
    return 0;
}


ssize_t nxp74hc595_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    char dataa[2]    = {0};
    char tmp[2];
    if(copy_from_user(tmp, buf, 2)){
        return -EFAULT;
    }
    dataa[0]=(tmp[0]-48);
    dataa[1]=(tmp[1]-48);
    printk("dataa[0] = %d , dataa[1] = %d\n",dataa[0],dataa[1]);


    switch(dataa[0]){
        case 1:data[0] = ~0x06;break;
        case 2:data[0] = ~0x5b;break;
        case 3:data[0] = ~0x4f;break;
        case 4:data[0] = ~0x66;break;
        case 5:data[0] = ~0x6d;break;
        case 6:data[0] = ~0x7d;break;
        case 7:data[0] = ~0x07;break;
        case 8:data[0] = ~0x7f;break;
        case 9:data[0] = ~0x6f;break;
    }


    
    printk("data[0] = %d  \n",data[0]);
    Write_74HC595(data[1],data);

    printk("write nxp74hc595 dev OK\n");
    return sizeof(int); 
}


MODULE_LICENSE("GPL"); 
module_init(nxp74hc595_init); 
module_exit(nxp74hc595_exit);