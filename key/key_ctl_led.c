//LED:3.26(1) ,3.22(2) ,3.20(3), 2.7(4)
//KEY:2.6(5) ,2.5(4) ,2.4(3),1.18(2),1.17(1)  
//I need to use the key to control the led. Every time I press the key ,the Led will change
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#define DEVICE_NAME  "key_ctl_led"

struct KEY_INFO{
    int id;
    char* describe;
}KEY_GPIO[]= {
              {1*32+17,"KEY_GPIO1"},
              {1*32+18,"KEY_GPIO2"},
              {2*32+4 ,"KEY_GPIO3"},    
              {2*32+5 ,"KEY_GPIO4"},
              {2*32+6 ,"KEY_GPIO5"},
            };

struct LED_INFO{
    int id;
    char* describe;
}LED_GPIO[]= {
              {3*32+26 ,"LED_GPIO1"},
              {3*32+22 ,"LED_GPIO2"},
              {3*32+20 ,"LED_GPIO3"},
              {2*32+7  ,"LED_GPIO4"},
            };

#define KEY_GPIO_IRQ(x)  gpio_to_irq(KEY_GPIO[x].id)


const char irq_type[5]={
    IRQ_TYPE_EDGE_RISING,
    IRQ_TYPE_EDGE_FALLING,
    IRQ_TYPE_EDGE_BOTH,
    IRQ_TYPE_LEVEL_HIGH,
    IRQ_TYPE_LEVEL_LOW
};


static irqreturn_t key_irq1_handler(unsigned int irq,void *dev_id)
{
    printk(KERN_INFO DEVICE_NAME" pressed!\n");
    int data= gpio_get_value(LED_GPIO[0].id);
    
    gpio_direction_output(LED_GPIO[0].id,!data);
    module_put(THIS_MODULE);
    return 0;
}

static irqreturn_t key_irq2_handler(unsigned int irq,void *dev_id)
{
    printk(KERN_INFO DEVICE_NAME" pressed!\n");
    int data= gpio_get_value(LED_GPIO[1].id);
    
    gpio_direction_output(LED_GPIO[1].id,!data);
    module_put(THIS_MODULE);
    return 0;
}

static irqreturn_t key_irq3_handler(unsigned int irq,void *dev_id)
{
    printk(KERN_INFO DEVICE_NAME" pressed!\n");
    int data= gpio_get_value(LED_GPIO[2].id);
    
    gpio_direction_output(LED_GPIO[2].id,!data);
    module_put(THIS_MODULE);
    return 0;
}

static irqreturn_t key_irq4_handler(unsigned int irq,void *dev_id)
{
    printk(KERN_INFO DEVICE_NAME" pressed!\n");
    int data= gpio_get_value(LED_GPIO[3].id);
    
    gpio_direction_output(LED_GPIO[3].id,!data);
    module_put(THIS_MODULE);
    return 0;
}

static irqreturn_t key_irq5_handler(unsigned int irq,void *dev_id)
{
    printk(KERN_INFO DEVICE_NAME" pressed!\n");
    int data= gpio_get_value(LED_GPIO[3].id);
    gpio_direction_output(LED_GPIO[3].id,!data);
    module_put(THIS_MODULE);
    return 0;
}

void led_init()
{
    int i=0;
    for(i=0;i<4;i++){
    gpio_free(LED_GPIO[i].id);
    gpio_request(LED_GPIO[i].id, LED_GPIO[i].describe);
    gpio_direction_output(LED_GPIO[i].id, 0);
    }
}

void key_init()
{
    int i=0;
    for(i=0;i<5;i++){
    gpio_free(KEY_GPIO[i].id);
    gpio_request(KEY_GPIO[i].id, KEY_GPIO[i].describe);
    gpio_direction_input(KEY_GPIO[i].id);
    }

        if(request_irq(KEY_GPIO_IRQ(0),key_irq1_handler,IRQF_DISABLED,"key_irq1",NULL))
      {
          printk(KERN_WARNING DEVICE_NAME":can't get irq1\n");
      }
        if(request_irq(KEY_GPIO_IRQ(1),key_irq2_handler,IRQF_DISABLED,"key_irq2",NULL))
      {
          printk(KERN_WARNING DEVICE_NAME":can't get irq2\n");
      }
        if(request_irq(KEY_GPIO_IRQ(2),key_irq3_handler,IRQF_DISABLED,"key_irq3",NULL))
      {
          printk(KERN_WARNING DEVICE_NAME":can't get irq3\n");
      }
        if(request_irq(KEY_GPIO_IRQ(3),key_irq4_handler,IRQF_DISABLED,"key_irq4",NULL))
      {
          printk(KERN_WARNING DEVICE_NAME":can't get irq4\n");
      }
        if(request_irq(KEY_GPIO_IRQ(4),key_irq5_handler,IRQF_DISABLED,"key_irq5",NULL))
      {
          printk(KERN_WARNING DEVICE_NAME":can't get irq5\n");
      }

      for(i=0;i<5;i++)
      {
          set_irq_type(KEY_GPIO_IRQ(i),irq_type[1]);
          disable_irq(KEY_GPIO_IRQ(i));
          enable_irq(KEY_GPIO_IRQ(i));
      }
}


static int key_led_open(struct inode *inode,struct file *file)
{
    try_module_get(THIS_MODULE);//count how many times the module has been opened
    printk(KERN_INFO DEVICE_NAME" opened!\n");
    return 0;
}

static int key_led_release(struct inode *inode,struct file *file)
{
    printk(KERN_INFO DEVICE_NAME" closed!\n");

    return 0;
}

struct cdev *keyirq;
static dev_t devno;
static struct class *key_irq_class;

struct file_operations key_irq_fops=
{
    .owner =THIS_MODULE,
    .open =key_led_open,
    .release=key_led_release,
};





static int __init key_led_init(void)
{
    int major,minor;

    int ret;

    led_init();
    key_init();

    ret =alloc_chrdev_region(&devno,minor,1,DEVICE_NAME);//get device id
    major=MAJOR(devno);
    if(ret<0)
    {
        printk(KERN_ERR"can not get id");
        return -1;
    }

    keyirq=cdev_alloc();
    if(keyirq!=NULL)
    {
        cdev_init(keyirq,&key_irq_fops);
        keyirq->owner=THIS_MODULE;
        if(cdev_add(keyirq,devno,1)!=0){
            printk(KERN_ERR "add error !\n");
            goto error;
        }
    }else{
        printk(KERN_ERR " alloc error !\n");
        return -1;
    }

    key_irq_class=class_create(THIS_MODULE,"key_irq_class");
    if(IS_ERR(key_irq_class))
    {
    
        printk(KERN_ERR "create class error ! \n");
        return -1;
    }

    device_create(key_irq_class,NULL,devno,NULL,DEVICE_NAME);
    return 0;

error:
    unregister_chrdev_region(devno,1);
    
    return ret;
}

static void __exit key_led_exit(void)
{
    int i=0;
    for(i=0;i<4;i++){
    gpio_free(LED_GPIO[i].id);
    }
    for(i=0;i<5;i++){
    gpio_free(KEY_GPIO[i].id);
    disable_irq(KEY_GPIO_IRQ(i));
    free_irq(KEY_GPIO_IRQ(i),NULL);
    }
    cdev_del(keyirq);
    unregister_chrdev_region(devno,1);
    device_destroy(key_irq_class,devno);
    class_destroy(key_irq_class);

}

module_init(key_led_init);
module_exit(key_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YangYue");

