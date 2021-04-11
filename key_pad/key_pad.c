//C:1.24,1.25,3.22,3.23:R4  R3   R2  R1
//R:3.24,3.25,3.26,3.28

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define DEVICE_NAME  "key_pad"

struct input_dev *inputdev;
int key;
//R:1.17,1.18,2.4,2.5
struct Row{
    int gpio;
    char* describe
}R[]={
    {1*32+17,"R1"},
    {1*32+18,"R2"},
    {2*32+4,"R3"},
    {2*32+5,"R4"},
};

//C:2.7,3.20,3.22,3.26
struct Cow{
    int gpio;
    char* describe;
    struct work_struct work;
}C[]={
    {2*32+7,"C1"},
    {3*32+20,"C2"},
    {3*32+22,"C3"},
    {2*32+6,"C4"},
};

static int key_val[4][4]=
{
    {KEY_A,KEY_B,KEY_C,KEY_D},
    {KEY_E,KEY_F,KEY_G,KEY_H},
    {KEY_I,KEY_J,KEY_K,KEY_L},
    {KEY_M,KEY_N,KEY_O,KEY_P}
};

static irqreturn_t key_pad_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    int i = (int)dev_id;
    int j =0;
    int gpio = C[i].gpio; /* 获取按键的 GPIO */

    mdelay(20);
    if (gpio_get_value(gpio)) {
    return IRQ_HANDLED;
    }
    mdelay(20);
    for(j=0;j<4;j++)
    {
        gpio_set_value(R[j].gpio,1);
        if(gpio_get_value(C[i].gpio)==1)
        {
            gpio_set_value(R[j].gpio,0);
            printk(KERN_INFO "OK");
            break;
        }
        else
        gpio_set_value(R[j].gpio,0);  
    }
    if(j==4)j=3;
    key=key_val[j][i];
    printk(KERN_INFO "%d %d %dwas pressed",key,i,j);
    // input_report_key(inputdev, key_val[j][i], 1);  /* 先报告键按下事件 */
    // input_sync(inputdev);
    // schedule_work(&(C[i].work));  /* 提交工作队列，实现中断的下半部处理 */
    return IRQ_HANDLED;
}


static void key_pad_scan(struct work_struct *_work)
{
    struct Cow *key_tmp = container_of(_work, struct Cow, work);
    int gpio = key_tmp->gpio;
    int code = key;
    /* 每隔 10mS 检查按键是否已经提起，如果没有提起就一直等待 */
    while(gpio_get_value(gpio)){ 
    mdelay(10); 
    }
    input_report_key(inputdev, code, 0);  /* 报告按键提起事件 */
    input_sync(inputdev);
}

//SET R TO OUTPUT
void R_init()
{
    int i=0;
    for(i=0;i<4;i++)
    {
        gpio_free(R[i].gpio);
        gpio_request(R[i].gpio,R[i].describe);
        gpio_direction_output(R[i].gpio,0);
    }
}

void C_init()
{
    int i=0;
    int irq_no = 0;
    int ret;
    for(i=0;i<4;i++)
    {
        gpio_free(C[i].gpio);
        gpio_request(C[i].gpio,C[i].describe);
        gpio_direction_input(C[i].gpio);

        irq_no = gpio_to_irq(C[i].gpio);
        set_irq_type(C[i].gpio, IRQ_TYPE_EDGE_FALLING);
        ret = request_irq(irq_no, key_pad_interrupt, IRQF_DISABLED, "key_pad", (void *)i);
            if (ret) {
            printk("request irq faile %d!\n", irq_no);
            return -EBUSY;
            }

        // /* 为每个按键都初始化工作队列  */
        INIT_WORK(&(C[i].work), key_pad_scan);
    }
}


static int __devinit key_pad_init(void)
{
    int i = 0, ret = 0;
    int irq_no = 0;
    int code, gpio;
    R_init();
    C_init();
    inputdev = input_allocate_device(); /* 为输入设备驱动对象申请内存空间*/
    if (!inputdev) {
    return -ENOMEM;
    }
    inputdev->name = DEVICE_NAME;
    set_bit(EV_KEY, inputdev->evbit); /* 设置输入设备支持按键事件 */

    input_register_device(inputdev); /* 注册设备驱动  */
    printk("EasyARM-i.MX28x key driver up \n");
    return 0;
}

static void __exit key_pad_exit(void)
{
    int i = 0;
    int irq_no;
    for (i = 0; i < 4; i++) {
    irq_no = gpio_to_irq(C[i].gpio); /* 为每个按键释放 GPIO */
    free_irq(irq_no, (void *)i); /* 为每个按键卸载中断处理函数 */
    }
    input_unregister_device(inputdev); /* 注销输入设备驱动 */
    printk("key driver remove \n");
}

module_init(key_pad_init);
module_exit(key_pad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YangYue");