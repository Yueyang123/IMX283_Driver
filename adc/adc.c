#include<linux/module.h>                                                /* module                       */
#include<linux/fs.h>                                                    /* file operation               */
#include<asm/uaccess.h>                                                 /* get_user()                   */
#include<linux/miscdevice.h>                                            /* miscdevice                   */
#include<asm/io.h>                                                      /* ioctl                        */
#include <mach/regs-lradc.h>						/* #define                      */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysdev.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <mach/device.h>
#include <mach/regs-lradc.h>
#include <mach/lradc.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include "adc.h"

#define  ZLG_IMX_283   0
#define  ZLG_IMX_287   1
#define  ZLG_IMX_280   2

extern int zlg_board_type;

static void __iomem *adc_base = NULL;

#define DEVICE_NAME  "magic-adc"
/*********************************************************************************************************
 文件操作接口
*********************************************************************************************************/
static int adc_open (struct inode *inode, struct file *fp) 
{

	try_module_get(THIS_MODULE);    

	//modified by cxf 2014-10-22 for imx280 ADCs
	if(ZLG_IMX_280 == zlg_board_type) {
		writel(BM_LRADC_CTRL0_SFTRST,adc_base + HW_LRADC_CTRL0_SET);
		udelay(1);
		writel(BM_LRADC_CTRL0_SFTRST,adc_base + HW_LRADC_CTRL0_CLR);
		/* Clear the Clock Gate for normal operation */
		writel(BM_LRADC_CTRL0_CLKGATE,adc_base + HW_LRADC_CTRL0_CLR);
	}
	writel(0xC0000000, adc_base + HW_LRADC_CTRL0_CLR);              /* 开启CLK，关闭复位模式        */
	writel(0x18430000, adc_base + HW_LRADC_CTRL1_CLR);              /* 关闭0-1-6通道及按键中断      */ 
	writel(0x30000000, adc_base + HW_LRADC_CTRL3_SET);              /* 忽略上电前三次采集数据       */     
	writel(0x76543210, adc_base + HW_LRADC_CTRL4);                  /* 设置对应的数据存放位置       */
	
	return 0;
}
static int adc_release (struct inode *inode, struct file *fp)
{
	module_put(THIS_MODULE);
    
	return 0;
}
/*********************************************************************************************************
**   cmd: 
**	不开启除2：
**	10:CH0 	11:CH1 	12:CH2 	13:CH3 	14:CH4  15:CH5  16:CH6 	 17:Vbat(内部除4)	  
**	开启除2: 
**      20:CH0	21:CH1  22:CH2  23:CH3  24:CH4  25:CH5  26:CH6 
**
**   被注释部分为3.0以上内核函数定义形式
*********************************************************************************************************/

//static int adc_ioctl(struct file *fp, struct file *flip, unsigned int cmd, unsigned long arg)
static int adc_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
    int iRes;
	int iTmp;
	
	iTmp = _IOC_NR(cmd) % 10;

	switch(cmd){
	case IMX28_ADC_CH0:
		writel(0x01000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;

	case IMX28_ADC_CH0_DIV2:
		writel(0x01000000, adc_base + HW_LRADC_CTRL2_SET);
       		break; 

	case IMX28_ADC_CH1:
		writel(0x02000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;	
	
	case IMX28_ADC_CH1_DIV2:
        writel(0x02000000, adc_base + HW_LRADC_CTRL2_SET);
		break;
	
	case IMX28_ADC_CH2:
		writel(0x04000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;
	
	case IMX28_ADC_CH2_DIV2:
		writel(0x04000000, adc_base + HW_LRADC_CTRL2_SET);
       		break; 
	
	case IMX28_ADC_CH3:
		writel(0x08000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;	
	
	case IMX28_ADC_CH3_DIV2:
                writel(0x08000000, adc_base + HW_LRADC_CTRL2_SET);
		break;
	
	case IMX28_ADC_CH4:
		writel(0x10000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;
	
	case IMX28_ADC_CH4_DIV2:
		writel(0x10000000, adc_base + HW_LRADC_CTRL2_SET);
       		break; 
	
	case IMX28_ADC_CH5:
		writel(0x20000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;	
	
	case IMX28_ADC_CH5_DIV2:
                writel(0x20000000, adc_base + HW_LRADC_CTRL2_SET);
		break;
	
	case IMX28_ADC_CH6:
		writel(0x40000000, adc_base + HW_LRADC_CTRL2_CLR);
		break;
        
	case IMX28_ADC_CH6_DIV2:
                writel(0x40000000, adc_base + HW_LRADC_CTRL2_SET);
		break;

	case IMX28_ADC_VBAT:	
        case IMX28_ADC_VBAT_DIV4:
		break;

	default:
		printk("adc control cmd invalid!!\n");
		return -1;
	}

        /* Clear the accumulator & NUM_SAMPLES */
        __raw_writel(0xFFFFFFFF,
                     adc_base + HW_LRADC_CHn_CLR(iTmp));

        /* Clear the interrupt flag */
        __raw_writel(1 << iTmp,
                     adc_base + HW_LRADC_CTRL1_CLR);
        
        __raw_writel(BF_LRADC_CTRL0_SCHEDULE(1 << iTmp),
                     adc_base + HW_LRADC_CTRL0_SET);

        /* wait for completion */
        while ((__raw_readl(adc_base + HW_LRADC_CTRL1)
                & (1 << iTmp)) != (1 << iTmp))
                cpu_relax();

        /* Clear the interrupt flag */
        __raw_writel(1 << iTmp,
                     adc_base + HW_LRADC_CTRL1_CLR);

        iRes =  __raw_readl(adc_base + HW_LRADC_CHn(iTmp)) &
                           BM_LRADC_CHn_VALUE;
   	copy_to_user((void *)arg, (void *)(&iRes), sizeof(int));
    
    	return 0;
}
/*********************************************************************************************************
Device Struct
*********************************************************************************************************/
struct file_operations adc_fops = 
{
	.owner		= THIS_MODULE,
	.open		= adc_open,
	.release	= adc_release,
    .ioctl      	= adc_ioctl,
};
static struct miscdevice adc_miscdev = 
{
	.minor	        = MISC_DYNAMIC_MINOR,
   	.name	        = "magic-adc",
    .fops	        = &adc_fops,
};
/*********************************************************************************************************
Module Functions
*********************************************************************************************************/
const char irq_type[5]={
    IRQ_TYPE_EDGE_RISING,
    IRQ_TYPE_EDGE_FALLING,
    IRQ_TYPE_EDGE_BOTH,
    IRQ_TYPE_LEVEL_HIGH,
    IRQ_TYPE_LEVEL_LOW
};
#define KEY_GPIO_IRQ(x)  gpio_to_irq(x)

static irqreturn_t key_irq_handler(unsigned int irq,void *dev_id)
{
	udelay(20);
    if (gpio_get_value(3*32+26)) {
    return IRQ_HANDLED;
    }
	printk(KERN_INFO DEVICE_NAME" pressed!\n");
    module_put(THIS_MODULE);
    return 0;
}


static int __init adcModule_init (void)
{
    int iRet=0;
    gpio_request(3*32+26, "SW");
    gpio_direction_input(3*32+26);
    if(request_irq(KEY_GPIO_IRQ(3*32+26),key_irq_handler,IRQF_DISABLED,"key_irq",NULL))
    {
        printk(KERN_WARNING DEVICE_NAME":can't get irq1\n");
    }
	set_irq_type(KEY_GPIO_IRQ(3*32+26),irq_type[1]);

	if (ZLG_IMX_283 == zlg_board_type) {
		printk("zlg EasyARM-imx283 adc driver up. \n");
	} else if(ZLG_IMX_287 == zlg_board_type) {
		printk("zlg EasyARM-imx287 adc driver up. \n");
	} else if(ZLG_IMX_280 == zlg_board_type) {
		//printk("zlg EasyARM-imx280 adc driver up. \n");
	} 	
	adc_base = ioremap(0x80050000, 0x180*4);    
    	iRet = misc_register(&adc_miscdev);
	if (iRet) {
		printk("register failed!\n");
	} 
	return iRet;
}
static void __exit adcModule_exit (void)                                /* warning:return void          */
{
	printk("zlg EasyARM-imx28xx adc driver down.\n");
	misc_deregister(&adc_miscdev);
	gpio_free(3*32+26);
}
/*********************************************************************************************************
Driver Definitions
*********************************************************************************************************/
module_init(adcModule_init);
module_exit(adcModule_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("admin@9crk.com");
MODULE_DESCRIPTION("EasyARM283 By zhouhua");
/*********************************************************************************************************
End File
*********************************************************************************************************/


