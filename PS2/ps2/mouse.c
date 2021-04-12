#include <linux/config.h>

#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/fs.h>


#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/kbd_ll.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
//#include <linux/kbd_kern.h>
#include <linux/vt_kern.h>
#include <linux/smp_lock.h>
#include <linux/kd.h>
#include <linux/pm.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>

#define pk(a,b...) index+=sprintf(debug_string+index,a, ## b)
//#define SEND_USE_INTERRUPT 0

//#include <asm/keyboard.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/system.h>

#include <asm/io.h>
#if CONFIG_MODVERSIONS==1
//#define MODVERSIONS
//#include <linux/modversions.h>
#endif        

/* Some configuration switches are present in the include file... */

#include <linux/pc_keyb.h>
#include "mcf5272.h"
#include "cpld.h"

#define MCF_CLK 66000000


static struct timer_list timer_keyboard_dog;
static unsigned long flags;
/*
 *	Define the TIMER register set addresses.
 */
#define	MCFTIMER_TMR		0x00		/* Timer Mode reg (r/w) */
#define	MCFTIMER_TRR		0x02		/* Timer Reference (r/w) */
#define	MCFTIMER_TCR		0x04		/* Timer Capture reg (r/w) */
#define	MCFTIMER_TCN		0x06		/* Timer Counter reg (r/w) */
#define	MCFTIMER_TER		0x11		/* Timer Event reg (r/w) */


/*
 *	Bit definitions for the Timer Mode Register (TMR).
 *	Register bit flags are common accross ColdFires.
 */
#define	MCFTIMER_TMR_PREMASK	0xff00		/* Prescalar mask */
#define	MCFTIMER_TMR_DISCE	0x0000		/* Disable capture */
#define	MCFTIMER_TMR_ANYCE	0x00c0		/* Capture any edge */
#define	MCFTIMER_TMR_FALLCE	0x0080		/* Capture fallingedge */
#define	MCFTIMER_TMR_RISECE	0x0040		/* Capture rising edge */
#define	MCFTIMER_TMR_ENOM	0x0020		/* Enable output toggle */
#define	MCFTIMER_TMR_DISOM	0x0000		/* Do single output pulse  */
#define	MCFTIMER_TMR_ENORI	0x0010		/* Enable ref interrupt */
#define	MCFTIMER_TMR_DISORI	0x0000		/* Disable ref interrupt */
#define	MCFTIMER_TMR_RESTART	0x0008		/* Restart counter */
#define	MCFTIMER_TMR_FREERUN	0x0000		/* Free running counter */
#define	MCFTIMER_TMR_CLKTIN	0x0006		/* Input clock is TIN */
#define	MCFTIMER_TMR_CLK16	0x0004		/* Input clock is /16 */
#define	MCFTIMER_TMR_CLK1	0x0002		/* Input clock is /1 */
#define	MCFTIMER_TMR_CLKSTOP	0x0000		/* Stop counter */
#define	MCFTIMER_TMR_ENABLE	0x0001		/* Enable timer */
#define	MCFTIMER_TMR_DISABLE	0x0000		/* Disable timer */

/*
 *	Bit definitions for the Timer Event Registers (TER).
 */
#define	MCFTIMER_TER_CAP	0x01		/* Capture event */
#define	MCFTIMER_TER_REF	0x02		/* Refernece event */

/****************************************************************************/


extern int set_pa0_pa7(unsigned short cmd,unsigned long arg);
#define imm 0x10000000
#define MCFEXT_IRQ6_VECTOR 91
#define PS2DataLineState (MCF5272_RD_GPIO_PADAT(imm)&0x4000)
#define RECEIVE_OK 4
#define AckOk    2
#define AckNotOk 3
#define PortEnable 1
#define PortDisable 0 

#define InitDataPort()  {	MCF5272_WR_GPIO_PACNT(imm,MCF5272_RD_GPIO_PACNT(imm)&0xcfffffff);\
				MCF5272_WR_GPIO_PADDR(imm,MCF5272_RD_GPIO_PADDR(imm)&(~0x4000));\
				MCF5272_WR_GPIO_PACNT(imm,MCF5272_RD_GPIO_PACNT(imm)&0xffff3fff);\
                                MCF5272_WR_GPIO_PADDR(imm,MCF5272_RD_GPIO_PADDR(imm)|(0x0080));\
				set_pa0_pa7(PS2_DATA_OUT,1);\
				set_pa0_pa7(PS2_DATA_CTRL,PortDisable);}
				
#define InitClkPort()   {	MCF5272_WR_GPIO_PACNT(imm,MCF5272_RD_GPIO_PACNT(imm)&0x3fffffff);\
				MCF5272_WR_GPIO_PADDR(imm,MCF5272_RD_GPIO_PADDR(imm)&(~0x8000));\
				set_pa0_pa7(PS2_CLK_OUT,1);\
				set_pa0_pa7(PS2_CLK_CTRL,PortDisable);}
				
#define SetDataPortAsOut()   MCF5272_WR_GPIO_PADDR(imm,MCF5272_RD_GPIO_PADDR(imm)|0x4000)
#define SetDataPortAsIn()    MCF5272_WR_GPIO_PADDR(imm,MCF5272_RD_GPIO_PADDR(imm)&(~0x4000))


#define SetClkPortAsOut()     {	MCF5272_WR_SIM_ICR4(imm,0x800000);/*关中断*/}
#define SetClkPortAsIn()      {	MCF5272_WR_SIM_ICR4(imm,0xD00000);/*开中断*/}
				
				
#define	InitPs2Icr()     {	MCF5272_WR_SIM_ICR4(imm,0xD00000);/*中断优先级5*/\
				MCF5272_WR_SIM_PITR(imm,MCF5272_RD_SIM_PITR(imm)&(~0x0020));}

#define OutClk(dt) 	{	set_pa0_pa7(PS2_CLK_OUT,dt);\
				set_pa0_pa7(PS2_CLK_CTRL,PortEnable);\
				set_pa0_pa7(PS2_CLK_OUT,dt);state_DataAndCLK|=1;}
				
#define OutData(dt) 	{	set_pa0_pa7(PS2_DATA_OUT,dt);\
				set_pa0_pa7(PS2_DATA_CTRL,PortEnable);\
				set_pa0_pa7(PS2_DATA_OUT,dt);state_DataAndCLK|=2;}
				
#define ReleaseClkPort() {	set_pa0_pa7(PS2_CLK_OUT,1);\
				set_pa0_pa7(PS2_CLK_CTRL,PortDisable);state_DataAndCLK&=0xfe;}
				
#define ReleaseDataPort(){	set_pa0_pa7(PS2_DATA_OUT,1);\
				 set_pa0_pa7(PS2_DATA_CTRL,PortDisable);state_DataAndCLK&=0xfd;}

#define DisableTimeOut() MCF5272_WR_TIMER2_TMR(imm,MCFTIMER_TMR_DISABLE)
#define timeout_send 1
#define timeout_receive 2
#define SetTimeOut_us(pa,pa1) {\
	KeyboardTimeout=pa1;\
	MCF5272_WR_TIMER2_TMR(imm,MCFTIMER_TMR_DISABLE);\
	MCF5272_WR_TIMER2_TCN(imm,0);\
	MCF5272_WR_TIMER2_TRR(imm,(unsigned short) ((MCF_CLK / 16) / (1000000/pa)));\
	MCF5272_WR_TIMER2_TMR(imm,(MCFTIMER_TMR_ENORI | MCFTIMER_TMR_CLK16 |\
		                MCFTIMER_TMR_RESTART | MCFTIMER_TMR_ENABLE));}

				
extern struct file_operations psaux_fops;








#define VERSION 100




#define CLK_IS_HIGH (MCF5272_RD_GPIO_PADAT(imm)&0x8000)

















static DECLARE_WAIT_QUEUE_HEAD(mouse_w_wait);


#define send_data(baba)	SendData(baba)

//	/*printk("send %x :%d\n",baba,__LINE__);*/while(mouse_state);}






#ifdef SEND_USE_INTERRUPT
static void timer2_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
#if 0		
	int j;
	//测试,在中断中向pa7发一个窄脉冲
	//if((MCF5272_RD_GPIO_PADAT(imm)&0x8000)==0)
	MCF5272_WR_GPIO_PADAT(imm,MCF5272_RD_GPIO_PADAT(imm)|0x80);
	j=10;	while(j--);
	MCF5272_WR_GPIO_PADAT(imm,MCF5272_RD_GPIO_PADAT(imm)&~0x80);
	
#endif
	MCF5272_WR_TIMER2_TMR(imm,MCFTIMER_TMR_DISABLE);
	MCF5272_WR_TIMER2_TER(imm,MCFTIMER_TER_CAP | MCFTIMER_TER_REF);
	MCF5272_WR_SIM_ICR1(imm,0x00c0);
	if(KeyboardTimeout==timeout_send)
	{	//发送超时 look :  SendData
		AskResent(0);
//					printk("timeout_send:%d\n",__LINE__);
		//KeyboardTimeout=0;
	}
	else if(KeyboardTimeout==timeout_receive)
	{	//接收超时 look :  mouse_interrupt
		
		SendData(KBD_RESEND);		
//					printk("send KBD_RESEND:%d\n",__LINE__);
	}
}
#endif


#ifdef SEND_USE_INTERRUPT
static int dog=0;
static int dog2=0;
static void
TimerKeyboardDog(unsigned long unused_ptr_info)
{
//	printk(".\n");

	dog++;
	if(dog>20)
	{
		dog=0;
//		waitus(100);
#if debugtime		
		wait_fall_edge();
#else
//		dog2=SendData(0xee);
//		if(dog2!=10)
//			printk("error :retvalue = %d \n",dog2);			
#endif
	}
#if 0	
	if(state_DataAndCLK)
	{
		dog++;
		if(dog>1)
		{
			ReleaseClkPort();
			ReleaseDataPort();
			mouse_state=0;
			dog=0;
		}
		
	}	
	if(!IsKeyReceived)
	{
		dog2++;			
		if(dog2>100)
		{
			if(dog2>101)
			{
				dog2=0;
				printk("baaa\n");
				send_data(KBD_RESET);
				
			}
			else
			{
				printk("aaa\n");
				send_data(KBD_DEFAULT);
				dog2=0;
			}
		}
	}
	else
	{
		dog2=0;
		IsKeyReceived=0;
	}
#endif	
	timer_keyboard_dog.expires = jiffies + 5;
	add_timer(&timer_keyboard_dog);
}
#endif

int init_mouse_module(void) 
{ 
#ifdef SEND_USE_INTERRUPT
	//use timer0 to do timeout check and process 
	MCF5272_WR_SIM_ICR1(imm,0x00c0);
	if (request_irq(71, timer2_interrupt,SA_INTERRUPT, "Mouse_timerout_checker", 0)) 
	{ 
		printk(KERN_ERR "Mouse_timerout_checkerInit: Unable to get IRQ.\n");
		return -EBUSY;
	}
#endif

	register_chrdev(42, "myKeyboard", &psaux_fops);
	queue = (struct aux_queue *) kmalloc(sizeof(*queue), GFP_KERNEL);
        memset(queue, 0, sizeof(*queue));
	queue->head = queue->tail = 0;
	init_waitqueue_head(&queue->proc_list);
	//queue->proc_list=NULL;
		
	 if (request_irq(MCFEXT_IRQ6_VECTOR, mouse_interrupt,SA_INTERRUPT, "Mouse", 0)) 
	  {
		printk(KERN_ERR "MouseInit: Unable to get IRQ.\n");
		return -EBUSY;
 	}
	
	printk("ret_value:%d\n",SendData(KBD_RESET));
	InitDataPort();
	InitClkPort();
	SetDataPortAsIn();
	SetClkPortAsIn();
	InitPs2Icr();
	create_proc_read_entry(		"mouse", 0 /* default mode */, 
					NULL /* parent dir */, 
					mouse_read_procmem, 
					NULL /* client data */);
#ifdef SEND_USE_INTERRUPT
	timer_keyboard_dog.function=TimerKeyboardDog;
	init_timer(&timer_keyboard_dog);
	mod_timer(&timer_keyboard_dog, jiffies+HZ/2);
#endif
	
	printk("Register mouse_irq successfully\n");

	return 0;
	
}  

void cleanup_mouse_module(void) 
{
	del_timer(&timer_keyboard_dog);
	free_irq(MCFEXT_IRQ6_VECTOR,0);
	free_irq(71,0);
	remove_proc_entry("mouse", NULL /* parent dir */);
	unregister_chrdev(42,"myKeyboard");
	printk("<1>Cleanup mouse:ok!\n"); 
}
module_init(init_mouse_module);      /*   added by ron so driver can compile directly into kernel */
module_exit(cleanup_mouse_module);  /*   added by ron so driver can compile directly into kernel */


#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/pci.h>

#include <linux/init.h>

#include <linux/ioport.h>

#include <linux/netdevice.h>

#include <linux/etherdevice.h>

#include <linux/delay.h>

#include <linux/ethtool.h>

#include <linux/mii.h>

#include <linux/crc32.h>

#include <linux/io.h>

#include <linux/interrupt.h>

#include <linux/module.h>

#include <linux/kernel.h>

#include <linux/types.h>

#include <linux/miscdevice.h>

#include <linux/ioport.h>

#include <linux/fcntl.h>

#include <linux/init.h>

#include <linux/poll.h>

#include <linux/proc_fs.h>

#include <linux/seq_file.h>

#include <linux/spinlock.h>

#include <linux/sched.h>

#include <linux/sysctl.h>

#include <linux/wait.h>

#include <linux/cdev.h>

#include <linux/fs.h>

#include <linux/delay.h>

#include <linux/uaccess.h>

#include <linux/device.h>

#include <linux/err.h>

#include <linux/fs.h>

#include <asm/io.h>

#include <asm/current.h>

#include <asm/system.h>

#include <linux/mm.h>

#include <linux/mman.h>

#include <linux/miscdevice.h>

#include <linux/proc_fs.h>

#include <linux/device.h>

#include <linux/fs.h>

#include <linux/slab.h>

#include <linux/mm.h>

#include <linux/slab.h>



#define DEVICE_NAME "testc"

#define MODNAME "pmc-test"

#define MMAPBUF_LEN (16*1024*1024)



struct my_testdev{

    dev_t testc_dev_num;

    struct class * testc_class;

    unsigned long testc_kernel_virt_addr;



    struct cdev test_cdev;

    unsigned int current_pointer; /*char device offset ,目前同时只能一个程序读取 */

    unsigned long mmap_phyaddr;

    dma_addr_t mmap_pcibus_addr;

    unsigned long mmap_kvirt_addr;

};



//不得已的一个全局变量

struct my_testdev *priv;



//仅仅是为了测试程序 手头只有这个卡 

static DEFINE_PCI_DEVICE_TABLE(netdrv_pci_tbl) = {

    {0x8086, 0x10b9, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

    {0,}

};

MODULE_DEVICE_TABLE(pci, netdrv_pci_tbl);





static int testc_open(struct inode *inode, struct file *file)

{

    return 0;

}



static int testc_release(struct inode *inode, struct file *file)

{



    return 0;

}





static ssize_t testc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)

{



    

    return 0;

}    



static ssize_t testc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)

{



    return 0;

}



static loff_t testc_llseek(struct file * file,loff_t offset,int orig)

{

    return 0;

}





static int testc_mmap(struct file *file, struct vm_area_struct *vma)

{

    

    unsigned long size = vma->vm_end - vma->vm_start;

    struct my_testdev *pri=priv;

    printk("inmmap ,mmap_phyaddr %lx , mmap_kvirt_addr %lx \n",pri->mmap_phyaddr,pri->mmap_kvirt_addr);

    printk("start %lx end %lx off %lx \n",vma->vm_start,vma->vm_end,vma->vm_pgoff);

//pci_alloc_consistent申请到的内存已经是物理地址连续的 ，只要一个remap_pfn_range

    remap_pfn_range(vma,vma->vm_start,(pri->mmap_phyaddr)>> PAGE_SHIFT,size,vma->vm_page_prot);



    vma->vm_flags |= VM_RESERVED;    /* avoid to swap out this VMA */

    return 0;



}



static const struct file_operations testc_fops = {

    .read        = testc_read,

    //.aio_read    = generic_file_aio_read,

    .write        = testc_write,

    //.aio_write    = blkdev_aio_write,

    //.fsync        = blkdev_fsync,

    .mmap        = testc_mmap,

    .open        = testc_open,

    .release    = testc_release,

    //.unlocked_ioctl = raw_ioctl,

    .llseek        = testc_llseek,

    .owner        = THIS_MODULE,

};





static int __devinit pmc_probe(struct pci_dev *pdev,const struct pci_device_id *ent)

{    

    int ret;

    int i;

    struct device *x;

    struct my_testdev *pri;

    printk("--------%s %d\n",__FUNCTION__,__LINE__);

    pri=kmalloc(sizeof(struct my_testdev),GFP_KERNEL);

    memset(pri,0,sizeof(struct my_testdev));

    priv=pri;

    pci_set_drvdata(pdev, pri);

    

    



    //char device

    if(alloc_chrdev_region(&(pri->testc_dev_num), 0, 1,DEVICE_NAME))

        {

            printk("err----%s %d \n",__FILE__,__LINE__);

            return -1;

        }

    pri->testc_class=class_create(THIS_MODULE,DEVICE_NAME);

    cdev_init(&(pri->test_cdev),&testc_fops);

    pri->test_cdev.owner=THIS_MODULE;



    ret=cdev_add(&(pri->test_cdev),pri->testc_dev_num,1);

    if(ret)

        {

            printk("err----%s %d \n",__FILE__,__LINE__);

            return -1;

        }

    //设备节点自动生成

    x=device_create(pri->testc_class,NULL,MKDEV(MAJOR(pri->testc_dev_num),0),pri,DEVICE_NAME);

    if(x==NULL)

        {

            printk("err----%s %d \n",__FILE__,__LINE__);

            return -1;

        }

    priv=pri;

    priv->current_pointer=0;





//mmap 

    pri->mmap_kvirt_addr=(unsigned long )pci_alloc_consistent(pdev,MMAPBUF_LEN,&(pri->mmap_pcibus_addr));

    if((void *)pri->mmap_kvirt_addr==NULL)

        {

            return -1;

        }

    memset((void *)pri->mmap_kvirt_addr,0x0,MMAPBUF_LEN);

//写入有意义的数据，方便在应用里验证确实映射了16MB

    for(i=0;i<MMAPBUF_LEN/4;i++)

        {

            *(((u32 *)pri->mmap_kvirt_addr)+i)=i;

        }



    pri->mmap_phyaddr=virt_to_phys((void *)pri->mmap_kvirt_addr);

    printk("mmap_phyaddr %lx , mmap_kvirt_addr %lx \n",pri->mmap_phyaddr,pri->mmap_kvirt_addr);

    

    return 0;

}

static void __devexit pmc_remove(struct pci_dev *pdev)

{

    struct my_testdev *pri = pci_get_drvdata(pdev);

    pci_free_consistent(pdev, MMAPBUF_LEN,(void *)pri->mmap_kvirt_addr, pri->mmap_pcibus_addr);

    

}





static struct pci_driver netdrv_pci_driver = {

    .name        = MODNAME,

    .id_table    = netdrv_pci_tbl,

    .probe        = pmc_probe,

    .remove        = __devexit_p(pmc_remove),



};



static int __init pmc_init_module(void)

{

    return pci_register_driver(&netdrv_pci_driver);

}





static void __exit pmc_cleanup_module(void)

{

    pci_unregister_driver(&netdrv_pci_driver);

}



module_init(pmc_init_module);

module_exit(pmc_cleanup_module);





MODULE_AUTHOR("deep_pro");

MODULE_LICENSE("GPL");


简单的应用程序，注意mmap的最后一个参数是dma缓冲区的物理地址，演示程序里根据驱动的打印写的硬编码，最终还是要靠ioctl等机制实现自动从驱动取得。


点击(此处)折叠或打开
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <errno.h>

#include <limits.h>

#include <linux/kernel.h>



#include <byteswap.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/mman.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>



#include <sys/ioctl.h>

#define MMAP_LEN 16*1024*1024



int main()

{

        int fd,i,ret;



        char buf[0x20]={0};

        unsigned int *mmap_data;



        fd=open("/dev/testc",O_RDWR);

        if(fd<0)

        {

                perror("open:");

                return -1;

        }

//0xf7000000 这个是缓冲区的物理地址，要从驱动里得到

        mmap_data=(unsigned int *)mmap(NULL,MMAP_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0xf7000000);

        if(mmap_data==MAP_FAILED)

        {

                perror("mmap");

                return -1;

        }

//测试mmap后的缓冲区读写 ，先写再读,这样执行第二遍应用就能验证写入成功

for(i=14*1024*1024/4;i<(14*1024*1024+0x120)/4;i++)

        {

                //mmap_buf[i]=0;

                printf("mmap_buf %02x :%x \n",i,*(mmap_data+i));

                   *(mmap_data+i)+=1;

        }



        munmap(mmap_data,MMAP_LEN);



        close(fd);

}

执行2遍应用即能验证读写和确实映射了16MB








分享到： 新浪微博 QQ空间 开心网 豆瓣 人人网 twitter fb 0 
   顶 热门产品推荐IBM System x3650 M3(...IBM System x3650 M3(...惠普 ProLiant DL380 G...联想 万全R525 G2(Xeon ...戴尔 PowerEdge R210(Xeo...IBM System x3100 M4(...IBM System x3100 M3(...IBM System X3200 M3(...
阅读(21)┊ 评论 (0)┊收藏(0)┊举报┊打印 
前一篇：uboot使用串口下载镜像
[发评论] 评论 重要提示：警惕虚假中奖信息! 

亲，您还没有登录,请[登录]或[注册]后再进行评论关于我们 | 关于IT168 | 联系方式 | 广告合作 | 法律声明 | 免费注册 
Copyright ? 2001-2010 ChinaUnix.net All Rights Reserved 北京皓辰网域网络信息技术有限公司. 版权所有 
感谢所有关心和支持过ChinaUnix的朋友们
京ICP证041476号 京ICP证060528号 

 

