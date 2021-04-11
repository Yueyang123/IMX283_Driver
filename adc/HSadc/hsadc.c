#include<linux/module.h>      /* module */
#include<linux/fs.h>          /* file operation*/
#include<asm/uaccess.h>       /* get_user()*/
#include<linux/miscdevice.h>  /* miscdevice*/
#include<asm/io.h>            /* ioctl*/
 
#include <linux/kernel.h>
#include <linux/sysdev.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/cdev.h>
 
#include <../arch/arm/mach-mx28/regs-clkctrl.h>
 
#define DEBUG_T 0
 
#define DEVICE_NAME	"imx283_hsadc"//驱动名称
#define HSADC_BASE  0x80002000 //HSADC基地址 实际物理地址
#define HW_CLKCTRL_BASE  0x80040000//时钟控制模块基地址
#define HSADC_CLKCTRL_BASE 0x80040150//HSADC CLK控制寄存器基地址
 
static void __iomem *hsadc_base = NULL;//映射后HSADC基地址
static void __iomem *hsadc_clk_base = NULL;
static void __iomem *CLKCTRL_FRAC0_BASE=NULL;
 
 
static int hsadc_open(struct inode *inode, struct file *fp)
{
    /* 清0分频器配置位  再将       分频值配置为30 主clk=480*18/30=288MHz*/
    writel((0x3F<<8),CLKCTRL_FRAC0_BASE+0x18);//CLR 
    writel((0x1E<<8),CLKCTRL_FRAC0_BASE+0x14);//SET
	
    /*使能HSADC CLOCK*/
	writel(1<<15,CLKCTRL_FRAC0_BASE+0x18);//HW_CLKCTRL_FRAC1_SET
 
	/*RESETB=1 Divider normal work  FREQDIV=11 Divide by 72*/
    writel((1<<30)|(3<<28),hsadc_clk_base);//hsadc_clk_base
 
    /*SFTRST=0 HSADC退出复位进入正常模式*/
	writel(1<<31,hsadc_base+0x08);//HW_HSADC_CTRL0_CLR
	
	/*CLKGATE=0 进入正常模式*/
	writel(1<<30,hsadc_base+0x08);//HW_HSADC_CTRL0_CLR
 
    /*HSADC由软件触发转换*/
	writel(3<<28,hsadc_base+0x08);//HW_HSADC_CTRL0_CLR
    
    /*HSADC忽略初次上电4次采样值 */
    writel(3<<19,hsadc_base+0x04);//HW_HSADC_CTRL0_SET
 
    /*HSADC输出12bit结果*/
	writel(1<<18, hsadc_base+0x04);//HW_HSADC_CTRL0_SET
	writel(1<<17, hsadc_base+0x08);//HW_HSADC_CTRL0_CLR
	
    /*小端数据模式*/
	writel(1<<16,hsadc_base+0x08);//HW_HSADC_CTRL0_CLR
  
    /*使能ADC_DONE中断*/
    writel((1<<30),hsadc_base+0x14);//HW_HSADC_CTRL1_CLR
	
    /*清除POWER_DOWN位    ADC模块上电*/
	writel(1<<13,hsadc_base+0x28);//HW_HSADC_CTRL2_CLR
	
	/*ADC_PRECHARGE置1 使能HSADC预充电*/
    writel(1<<0,hsadc_base+0x24);//HW_HSADC_CTRL2_SET
 
	/*选择HSADC0作为ADC输入源*/        
	writel(7<<1,hsadc_base+0x24);//HW_HSADC_CTRL2_SET
	
#if (DEBUG_T)	
	printk("HW_HSADC_CTRL0=%08X\n",readl(hsadc_base+0));
	printk("HW_HSADC_CTRL1=%08X\n",readl(hsadc_base+0X10));
	printk("HW_HSADC_CTRL2=%08X\n",readl(hsadc_base+0x20));
	printk("HW_HSADC_CLK=%08x\n",readl(hsadc_clk_base)); 	  
    printk("HW_HSADC_CLK2=%08x\n",readl(CLKCTRL_FRAC0_BASE+0x10)); 
#endif	
	
	return 0; 
}
 
 
static int hsadc_release(struct inode * inode, struct file * file)
{
   //失能HSADC    
   writel((1<<31)|(1<<30),hsadc_base+0x04);//HW_HSADC_CTRL0_SET
   return 0;
}
 
 
static int hsadc_read(struct file *filp,  char __user *buf, size_t count,
                loff_t *f_pos)
{
    int ret;
	int hsadc_res;
	unsigned char databuf[2];
    unsigned int timeout = 1000;
	/*清除所有flag*/
	writel(3<<26,hsadc_base+0x14);//HW_HSADC_CTRL1_SET
 
	/*HSADC_RUN=1  启动HSADC等待触发*/
	writel(1<<0,hsadc_base+0x04);//HW_HSADC_CTRL0_SET
 
	/*SOFTWARE_TRIGGER=1 软件触发HSADC进行转换*/
    writel(1<<27,hsadc_base+0x04);//HW_HSADC_CTRL0_SET
 
	while(((readl(hsadc_base+0x10)&0x01)==0)&&(timeout > 0))//HW_HSADC_CTRL1等待HSADC转换完成 
	{
	   timeout--;
	   if(timeout == 0)
	   {    
            printk("timeout! \n");   
			break;
	   }
	   
	   //cpu_relax();
	}
#if (DEBUG_T)	
	printk("HW_HSADC_CTRL0=%08X\n",readl(hsadc_base+0));
	printk("HW_HSADC_CTRL1=%08X\n",readl(hsadc_base+0X10));
	printk("HW_HSADC_CTRL2=%08X\n",readl(hsadc_base+0x20));
#endif	
	/*清除所有flag*/
	writel(3<<26,hsadc_base+0x14);//HW_HSADC_CTRL1_SET 
 
	/*读取转换结果 12bit*/
	hsadc_res = readl(hsadc_base+0x50) & 0xFFF;//HW_HSADC_FIFO_DATA 读取ADC数据
   	
	databuf[0] = (hsadc_res >> 8) & 0xFF;
	databuf[1] = hsadc_res & 0xFF;
	ret = copy_to_user(buf, databuf, 2);
	if(ret < 0)
	{
      printk("kernel read error \n");
	  return ret;
	}
	return 0;
  
}
 
 
static struct file_operations hsadc_fops =
{
  .owner   = THIS_MODULE,
  .open    = hsadc_open,
  .release = hsadc_release,
  .read    = hsadc_read,
};
 
static struct cdev *hsadc_cdev = NULL;//cdev
static struct class *hsadc_class = NULL;//类
static struct device *hsadc_device = NULL;//设备
static dev_t  dev_id;//设备号
static int major;//主设备号
 
static int __init hsadc_init(void)
{
  int ret;
  hsadc_clk_base = ioremap(HSADC_CLKCTRL_BASE, 4);
  hsadc_base = ioremap(HSADC_BASE,0xC0*4);//地址映射
  CLKCTRL_FRAC0_BASE = ioremap(HW_CLKCTRL_BASE+HW_CLKCTRL_FRAC0,32);
  
  ret = alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME); //申请设备号
  if(ret < 0)
  {
     printk(KERN_ERR "alloc dev_id error %d \n", ret);
	 return ret;
  }
  major = MAJOR(dev_id);//获取主设备号
  hsadc_cdev=cdev_alloc();//申请cdev结构
  if(hsadc_cdev != NULL)
  {
     cdev_init(hsadc_cdev,&hsadc_fops);//初始化hsadc_cdev结构体
	 ret = cdev_add(hsadc_cdev, dev_id, 1);//添加hsadc设备到hsadc_cdev结构体 
	 if(ret != 0)
	 {
       printk("cdev add error %d \n",ret);
	   goto error;
	 }
  }
  else
  {
     printk("cdev_alloc error \n");
     return -1;
  }
  //创建hsadc_class类
  hsadc_class = class_create(THIS_MODULE, "HSADC_CLASS");
  if(hsadc_class !=NULL)
  {
     //在hsadc_class类下面创建1个设备
     hsadc_device=device_create(hsadc_class, NULL, dev_id, NULL, DEVICE_NAME);
  }
	 printk("module init ok\n");
	 return 0;
error:
     cdev_del(hsadc_cdev);//删除hsadc_cdev结构体
     unregister_chrdev_region(dev_id, 1);//释放设备号
     return ret;	 
}
 
 
static void __exit hsadc_exit(void)
{
   iounmap(hsadc_base);//取消地址映射
   cdev_del(hsadc_cdev);//删除hsadc_cdev结构体
   unregister_chrdev_region(dev_id, 1);//释放设备号
   device_del(hsadc_device);//删除设备
   class_destroy(hsadc_class);//删除类
   printk("module exit ok\n");
}
 
 
module_init(hsadc_init);
module_exit(hsadc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xzx2020");