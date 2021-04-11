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


#include <../arch/arm/mach-mx28/mx28_pins.h>


#define DEVICE_NAME  "input_key"

struct input_dev *inputdev;

struct imx28x_key_struct {
    int key_code;  /* 按键能产生的键值*/
    int gpio;  /* 按键连接的 GPIO */
    struct work_struct work;  /* 按键的工作队列  */
};

struct imx28x_key_struct keys_list[] ={
    {.key_code = KEY_A, .gpio = MXS_PIN_TO_GPIO(PINID_LCD_D17)},
    {.key_code = KEY_B, .gpio = MXS_PIN_TO_GPIO(PINID_LCD_D18)},
    {.key_code = KEY_C, .gpio = MXS_PIN_TO_GPIO(PINID_SSP0_DATA4)},
    {.key_code = KEY_D, .gpio = MXS_PIN_TO_GPIO(PINID_SSP0_DATA5)},
    {.key_code = KEY_E, .gpio = MXS_PIN_TO_GPIO(PINID_SSP0_DATA6)}
};

static irqreturn_t imx28x_key_intnerrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    int i = (int)dev_id;
    int gpio = keys_list[i].gpio; /* 获取按键的 GPIO */
    int code = keys_list[i].key_code; /* 获取按键的键值  */
    /*
    * 延迟 20uS，看按键是不是按下，如果不是，就是抖动
    */
    printk(KERN_INFO "%d was pressed",code);
    udelay(20);
    if (gpio_get_value(gpio)) {
    return IRQ_HANDLED;
    }
    input_report_key(inputdev, code, 1);  /* 先报告键按下事件 */
    input_sync(inputdev);
    schedule_work(&(keys_list[i].work));  /* 提交工作队列，实现中断的下半部处理 */
    return IRQ_HANDLED;
}


static void imx28x_scankeypad(struct work_struct *_work)
{
    /* 通过工作队列指针而获得它所属的 imx28x_key_struct 类型的对象 */
    struct imx28x_key_struct *key_tmp = container_of(_work, struct imx28x_key_struct, work);
    int gpio = key_tmp->gpio;
    int code = key_tmp->key_code;
    /* 每隔 10mS 检查按键是否已经提起，如果没有提起就一直等待 */
    while(!gpio_get_value(gpio)){ 
    mdelay(10); 
    }
    input_report_key(inputdev, code, 0);  /* 报告按键提起事件 */
    input_sync(inputdev);
}

static int __devinit iMX28x_key_init(void)
{
    int i = 0, ret = 0;
    int irq_no = 0;
    int code, gpio;
    inputdev = input_allocate_device(); /* 为输入设备驱动对象申请内存空间*/
    if (!inputdev) {
    return -ENOMEM;
    }
    inputdev->name = DEVICE_NAME;
    set_bit(EV_KEY, inputdev->evbit); /* 设置输入设备支持按键事件 */
        for (i = 0; i < sizeof(keys_list)/sizeof(keys_list[0]); i++) {
        code = keys_list[i].key_code;
        gpio = keys_list[i].gpio;
        /* 为每个按键都初始化工作队列  */
        INIT_WORK(&(keys_list[i].work), imx28x_scankeypad);
        set_bit(code, inputdev->keybit); /* 设置输入设备支持的键值  */
        /* 为每个按键都初始化 GPIO */
        gpio_free(gpio);
        ret = gpio_request(gpio, "key_gpio");
            if (ret) {
            printk("request gpio failed %d \n", gpio);
            return -EBUSY;
            }
        /* 当 GPIO 被设置为输入工作状态后，就可以检测中断信号 */
        gpio_direction_input(gpio);
        /* 把每个 GPIO 中断响应方式都设置为下降沿响应 */
        irq_no = gpio_to_irq(gpio);
        set_irq_type(gpio, IRQF_TRIGGER_FALLING);
        /* 为每个按键的中断都安装中断处理函数，其私有数据为按键信息在 keys_list 数组下的索引 */
        ret = request_irq(irq_no, imx28x_key_intnerrupt, IRQF_DISABLED, "imx28x_key", (void *)i);
            if (ret) {
            printk("request irq faile %d!\n", irq_no);
            return -EBUSY;
            }
        }
    input_register_device(inputdev); /* 注册设备驱动  */
    printk("EasyARM-i.MX28x key driver up \n");
    return 0;
}

static void __exit iMX28x_key_exit(void)
{
    int i = 0;
    int irq_no;
    for (i = 0; i < sizeof(keys_list)/sizeof(keys_list[0]); i++) {
    irq_no = gpio_to_irq(keys_list[i].gpio); /* 为每个按键释放 GPIO */
    free_irq(irq_no, (void *)i); /* 为每个按键卸载中断处理函数 */
    }
    input_unregister_device(inputdev); /* 注销输入设备驱动 */
    printk("EasyARM-i.MX28x key driver remove \n");
}

module_init(iMX28x_key_init);
module_exit(iMX28x_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YangYue");