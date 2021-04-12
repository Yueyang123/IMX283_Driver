/*
 * The PS/2 keyboard driver. use GPIO interrput
 *
 * Copyright (C) 2007 ShenZhen forsafe System Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
*/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/irq.h>
//#include <asm/hardware.h>
#include <linux/workqueue.h>


#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/kbd_ll.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/kd.h>
#include <linux/pm.h>
#include <linux/wait.h>
#include <linux/proc_fs.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/io.h>

#include <linux/pc_keyb.h>
#include <mach/hardware.h>
#include <mach/platform.h>

#include <asm/arch/lpc32xx_gpio.h>

#include "ps2.h"


#define DRIVER_NAME		"ps2"
#define VERSION			"1.0.0"


static struct io_ps2 *gpiops2_dev;

static unsigned char keycode_translate[256] =
{
/* 00 */  K_NONE, K_F9  , K_NONE, K_F5  , K_F3  , K_F1  , K_F2  , K_F12 ,
/* 08 */  K_A, K_F10 , K_F8  , K_F6  , K_F4  , K_TAB , K_AGR , K_E,
/* 10 */  K_L, K_LALT, K_LSFT, K_B, K_LCTL, K_Q   , K_1   , K_F,
/* 18 */  K_M, K_G, K_Z   , K_S   , K_A   , K_W   , K_2   , K_N,
/* 20 */  K_H, K_C   , K_X   , K_D   , K_E   , K_4   , K_3   , K_O,
/* 28 */  K_I, K_SPCE, K_V   , K_F   , K_T   , K_R   , K_5   , K_P,
/* 30 */  K_C, K_N   , K_B   , K_H   , K_G   , K_Y   , K_6   , K_Q,
/* 38 */  K_J, K_D, K_M   , K_J   , K_U   , K_7   , K_8   , K_K,
/* 40 */  K_R, K_COMA, K_K   , K_I   , K_O   , K_0   , K_9   , K_F7,
/* 48 */  K_F8, K_DOT , K_FSLH, K_L   , K_SEMI, K_P   , K_MINS, K_F9,
/* 50 */  K_F10, K_NONE, K_SQOT, K_NONE, K_LSBK, K_EQLS, K_NONE, K_NONE,
/* 58 */  K_CAPS, K_RSFT, K_ENTR, K_RSBK, K_NONE, K_HASH, K_NONE, K_NONE,
/* 60 */  K_UP  , K_BSLH, K_NONE, K_NONE, K_NONE, K_NONE, K_BKSP, K_NONE,
/* 68 */  K_DOWN, KP_1  , K_NONE, KP_4  , KP_7  , K_NONE, K_NONE, K_NONE,
/* 70 */  KP_0  , KP_DOT, KP_2  , KP_5  , KP_6  , KP_8  , K_ESC , K_NUML,
/* 78 */  K_F11 , KP_PLS, KP_3  , KP_MNS, KP_STR, KP_9  , K_SCRL, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_F7  , K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
	  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE
};

static int mouse_state=0;//1=send 0=receive
static int mouse_counter=0;/*当前已经接收位数计数器*/
static unsigned char mouse_data=0;
static int mouse_checksum=0;

static unsigned char status;
static unsigned char ncodes;
static unsigned char bi;
static unsigned char buffer[4];


static struct aux_queue *queue; /* Mouse data buffer. */
spinlock_t kbd_controller_lock = SPIN_LOCK_UNLOCKED;
unsigned short state_DataAndCLK=0;
unsigned short IsKeyReceived=0;

static void handle_rawcode(int keyval)
{
	int keysym;

	if (keyval > 0x83) {
		switch (keyval) {
		case KBD_ESCAPEE0:
			ncodes = 2;
			bi = 0;
			break;
		case KBD_ESCAPEE1:
			ncodes = 3;
			bi = 0;
			break;
		case KBD_BREAK:
			status |= CODE_BREAK;
			return;
		default:
			return;
		}
	}

	if (ncodes) {
		buffer[bi++] = keyval;
		ncodes -= 1;
		if (ncodes)
			return;
		keysym = K_NONE;
		switch (buffer[0] << 8 | buffer[1]) {
		case ESCE0(0x11): keysym = K_RALT; break;
		case ESCE0(0x14): keysym = K_RCTL; break;
		/*
		 * take care of MS extra keys (actually
		 * 0x7d - 0x7f, but last one is already K_NONE
		 */
		case ESCE0(0x1f): keysym = 124;    break;
		case ESCE0(0x27): keysym = 125;    break;
		case ESCE0(0x2f): keysym = 126;    break;
		case ESCE0(0x4a): keysym = KP_SLH; break;
		case ESCE0(0x5a): keysym = KP_ENT; break;
		case ESCE0(0x69): keysym = K_END;  break;
		case ESCE0(0x6b): keysym = K_LEFT; break;
		case ESCE0(0x6c): keysym = K_HOME; break;
		case ESCE0(0x70): keysym = K_INS;  break;
		case ESCE0(0x71): keysym = K_DEL;  break;
		case ESCE0(0x72): keysym = K_DOWN; break;
		case ESCE0(0x74): keysym = K_RGHT; break;
		case ESCE0(0x75): keysym = K_UP;   break;
		case ESCE0(0x7a): keysym = K_PGDN; break;
		case ESCE0(0x7c): keysym = K_PRNT; break;
		case ESCE0(0x7d): keysym = K_PGUP; break;
		case ESCE1(0x14):
			if (buffer[2] == 0x77)
				keysym = K_BRK;
			break;
		case ESCE0(0x12):		/* ignore escaped shift key */
			status = 0;
			return;
		}
	} else {
		bi = 0;
		keysym = keycode_translate[keyval]|status;
		status = 0;
	}

	if (keysym != K_NONE) {
		int head = queue->head;
		queue->buf[head] = keysym|status;
		status=0;
		//DEBUGMSG("key=0x%x", queue->buf[head]);
		head = (head + 1) & (AUX_BUF_SIZE-1);
		if (head != queue->tail) {
			queue->head = head;
			if (queue->fasync)
			kill_fasync(&queue->fasync, SIGIO, POLL_IN);
			wake_up_interruptible(&queue->proc_list);
		} else {
			//DEBUGMSG("key buff full!!\n");
		}
	}
}

#if 1
static int lpc32xx_gpio_irq_set(int state)
{
    u32 *sic2_er;
    u32 *sic2_rsr;
    u32 *sic2_atr;
    u32 val=0;

    sic2_er = (u32 *)io_p2v(SIC2_BASE + INTC_MASK);
    sic2_rsr = (u32 *)io_p2v(SIC2_BASE + INTC_RAW_STAT);
    sic2_atr = (u32 *)io_p2v(SIC2_BASE + INTC_ACT_TYPE);
    if (state != Enable) {
        val = __raw_readl(sic2_er);
        val &= (~(_BIT(PS2CK_IRQ_NO)));
        __raw_writel(_BIT(PS2CK_IRQ_NO), sic2_rsr); //clear interrupt flag
        __raw_writel(val, sic2_er); //disable GPIO_00 interrupt
    } else {
        __raw_writel(_BIT(PS2CK_IRQ_NO), sic2_rsr); //clear interrupt flag
        /*set atr*/
		val = __raw_readl(sic2_atr);
		val |= _BIT(PS2CK_IRQ_NO);
        __raw_writel(val, sic2_atr);

        /*set er*/
        val = __raw_readl(sic2_er);
        val |= _BIT(PS2CK_IRQ_NO);        
        __raw_writel(val, sic2_er); //enable GPIO_00 interrupt            
        //PS2CK_DIR(IN);
    }        

    return 0;
}
#else
static int lpc32xx_gpio_irq_set(int state)
{
    if (state != Enable) {
    	disable_irq(PS2CK_IRQ_NO);
    } else {
    	set_irq_type(PS2CK_IRQ_NO, IRQ_TYPE_EDGE_FALLING);
    	enable_irq(PS2CK_IRQ_NO);
    }

    return 0;
}
#endif

static void init_gpio_port(void)
{
	/*conifg P2_MUX_CLR [0 1] is GPIO_02 GPIO_03*/
	__raw_writel(P2_GPIO02_KEYROW6, GPIO_P2_MUX_CLR(GPIO_IOBASE));//set GPIO_02 IO
	__raw_writel(P2_GPIO03_KEYROW7, GPIO_P2_MUX_CLR(GPIO_IOBASE));//set GPIO_03 IO

	PS2CK_DIR(IN);
	PS2DT_DIR(IN);
	lpc32xx_gpio_irq_set(Disable);
}

static int fasync_aux(int fd, struct file *filp, int on)
 {
	int retval;

	retval = fasync_helper(fd, filp, on, &queue->fasync);
	if (retval < 0)
		return retval;
	return 0;
} 


static inline int queue_empty(void)
{
	if((queue->head == queue->tail))return 1;
	return 0;
} 
static unsigned char get_from_queue(void)
{
	unsigned char result=0;
	unsigned long flags;

	spin_lock_irqsave(&kbd_controller_lock, flags);
	if(queue->tail==queue->head){
	}else{
		result = queue->buf[queue->tail];
		queue->tail = (queue->tail + 1) & (AUX_BUF_SIZE-1);
	}
	spin_unlock_irqrestore(&kbd_controller_lock, flags);
	return result;
}

static int wait_fall_edge(void)
{
#define WAITETIME_MARCO		20000
	u32 waitus=0;

	//wait for high
	do {
		waitus++;
		if (waitus > WAITETIME_MARCO) {
		    DEBUGMSG("wait for hig error");
		    break;
		}
	}while (!PS2CK_STATE());

	//DEBUGMSG("wait for high:%d",waitus);

	waitus=0;
	//wait for low
	do {
		waitus++;
		if (waitus > WAITETIME_MARCO) {
			DEBUGMSG("wait for low error");
		    return 0;
		}
	}while (PS2CK_STATE());
	//DEBUGMSG("wait for low:%d", waitus);

	return 1;
}



int SendData(unsigned char dt) 
{
	unsigned int i;
	int ret_value=0;
	unsigned char mouse_data_send;
	unsigned long flags;
	mouse_data_send=dt;
	
	lpc32xx_gpio_irq_set(Disable);
	local_irq_save(flags);
	
	PS2CK_OUTP(Low); PS2DT_OUTP(High);
	mouse_state=1;
	mouse_counter=0;
	mouse_checksum=0;
	udelay(150);
	PS2DT_OUTP(Low);
	PS2CK_DIR(IN);

	for(i=0;i<8;i++) {
		if(wait_fall_edge()){
			if(mouse_data_send&0x01) {
				PS2DT_OUTP(High);
				mouse_checksum = !mouse_checksum;
			} else {
				PS2DT_OUTP(Low);
			}
			mouse_data_send >>= 1;
			ret_value++;
		} else {
			goto endofsend;							
		}
	}
	if(wait_fall_edge()) {
		mouse_checksum=!mouse_checksum;
		mouse_checksum ? (PS2DT_OUTP(High)) : (PS2DT_OUTP(Low));
		ret_value++;
	} else {
		goto endofsend;
	}
	if(wait_fall_edge()){
		PS2DT_OUTP(High);
		ret_value++;
	} else {
		goto endofsend;
	}
	if(wait_fall_edge()) {
		PS2DT_DIR(IN);
		udelay(10);
		if(PS2DT_STATE() == 0) {
		    //ack ok
    		ret_value++;
		} else {
			//DEBUGMSG("data is high");
		}
	}
endofsend:	
	PS2CK_OUTP(High);PS2DT_OUTP(High);
	PS2DT_DIR(IN);PS2CK_DIR(IN);
	udelay(10);
	mouse_state=0;
	mouse_counter=0;
	mouse_checksum=0;
	//DEBUGMSG("send value=%d", ret_value);
	lpc32xx_gpio_irq_set(Enable);
	local_irq_restore(flags);
	return ret_value;
}

static int release_aux(struct inode * inode, struct file * file)
{
	DEBUGMSG("%s", __func__);

	return 0;
} 


/*
 * Install interrupt handler.
 * Enable auxiliary device.
 */

static int open_aux(struct inode * inode, struct file * file)
{
	DEBUGMSG("%s", __func__);
	lpc32xx_gpio_irq_set(Enable);
	mdelay(50);
	SendData(KBD_RESET);
	return 0;
}

/*
 * Put bytes from input queue to buffer.
 */

static ssize_t read_aux(struct file * file, char * buffer,
			size_t count, loff_t *ppos)
{
	DECLARE_WAITQUEUE(wait, current);
	ssize_t i = count;
	unsigned char c;
	if (queue_empty()) {
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		add_wait_queue(&queue->proc_list, &wait);
repeat:
		current->state = TASK_INTERRUPTIBLE;
		if (queue_empty() && !signal_pending(current)) {//信号到来 
			schedule();
			goto repeat;
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&queue->proc_list, &wait);

		return 0;
	}
	while ((i > 0) &&(!queue_empty())) {
		c = get_from_queue();
		put_user(c, buffer++);
		i--;
	} 
	if (count-i) {
		file->f_dentry->d_inode->i_atime = CURRENT_TIME;
		return count-i;
	 }
	if (signal_pending(current))
		return -ERESTARTSYS;
	return 0;
}


/*
 * Write to the aux device.
 */
static ssize_t write_aux(struct file * file, const char * buffer,
			 size_t count, loff_t *ppos)
{
	ssize_t retval = 0;
	
	if (count) {
		ssize_t written = 0;

		if (count > 32)
			count = 32; /* Limit to 32 bytes. */
		do {
			unsigned char c;
			get_user(c, buffer++);
			if(SendData(c)!= SEND_SUCCESS)
				SendData(c);
			else DEBUGMSG("write :SUCCESS:0x%x\n",c);
//			aux_write_dev(c);
			written++;
 		} while (--count);
		retval = -EIO;
 		if (written) {
			retval = written;
			file->f_dentry->d_inode->i_mtime = CURRENT_TIME;
		}
 	}
	return retval;
} 

static unsigned int aux_poll(struct file *file, poll_table * wait)
 { 
	poll_wait(file, &queue->proc_list, wait);
	 if (!queue_empty())
		return POLLIN | POLLRDNORM;
	return 0;
  }




int mouse_read_procmem(char *buf, char **start, off_t offset,
		                   int count, int *eof, void *data)
 {
	int len = 0;
 	while(!queue_empty()){
//		len+=sprintf(buf+len,"0x%x",get_from_queue());
		len+=sprintf(buf+len,"0x%x\n",get_from_queue());
	}
//	mouse_send_one_byte(0x55);
	
	*eof = 1;
	return len;
}


static irqreturn_t gpiops2_keyboard_irq(int irq, void *dev_id)//, struct pt_regs *regs)
{  
    u32 *sic2_rsr;
	unsigned long flags;
	static unsigned short scancode=0;

    
    sic2_rsr =(u32 *)io_p2v(SIC2_BASE + INTC_RAW_STAT);
    __raw_writel((1<<PS2CK_IRQ_NO), sic2_rsr); //clear interrupt flag

	if (PS2CK_STATE()) {
		udelay(5);
		if (PS2CK_STATE()) {
		    DEBUGMSG("irq: but PS2CK is high");
		    goto err;
		}
	}
	lpc32xx_gpio_irq_set(Disable);
	local_irq_save(flags);
	if(mouse_state == 0) {
		/*Don`t use interrupt*/
		u32 k;
		(PS2DT_STATE()) ? (scancode=0x80) : (scancode=0x00);
		mouse_data=0;
		mouse_checksum=0;

		for(k=0;k<8;k++) {
			if(wait_fall_edge()) {				
				(PS2DT_STATE()) ? (scancode=0x80) : (scancode=0x00);
				mouse_data >>= 1;
				mouse_data |= scancode;
				if(scancode) mouse_checksum = !mouse_checksum;			
			} else {
				break;
			}
		}
		if(wait_fall_edge()) {
			(PS2DT_STATE()) ? (scancode=0x80) : (scancode=0x00);
			if(scancode) mouse_checksum = !mouse_checksum;
			if(mouse_checksum) {
				k++;
			} else {
				goto receive_end;
			}
		} else {
			goto receive_end;
		}
		if(wait_fall_edge()) {
			k++;
		} else {
			goto receive_end;
		}
		
receive_end:
		
		if(k == 10) {
			handle_rawcode(mouse_data);
		} else {
			SendData(KBD_RESEND);			
		}
		
	}
   local_irq_restore(flags);
   lpc32xx_gpio_irq_set(Enable);

err:

    return IRQ_HANDLED;
}


static  struct file_operations gpiops2_fops = {
	.owner 	= THIS_MODULE,
	.read 	= read_aux,
	.write 	= write_aux,
	.poll 	= aux_poll,
	.open 	= open_aux,
	.release = release_aux,
	.fasync = fasync_aux,
};


/*
 * Initialization
 */
static int __init gpiops2_init(void)
{
    int ret;
    
    DEBUGMSG(DRIVER_NAME" init\n");
    
    gpiops2_dev = kmalloc(sizeof(struct io_ps2),GFP_KERNEL);
    if (gpiops2_dev == NULL) {
        printk(KERN_WARNING "LPC32xx PS2: cannot allocate memmory for ps2dev");
        ret = -1;
        goto err_kmalloc_gpiops2_dev;
    }

    queue = (struct aux_queue *) kmalloc(sizeof(*queue), GFP_KERNEL);
    memset(queue, 0, sizeof(*queue));
    queue->head = queue->tail = 0;
    init_waitqueue_head(&queue->proc_list);


    
    set_irq_type(IRQ_GPIO_02, IRQ_TYPE_EDGE_FALLING);
    gpiops2_dev->irq=IRQ_GPIO_02;
    ret = request_irq(gpiops2_dev->irq, gpiops2_keyboard_irq, IRQF_DISABLED|IRQF_TRIGGER_FALLING,DRIVER_NAME, NULL);
    if (ret < 0) {
        dev_err(&gpiops2_dev->sdev, "request irq %d failed (ret = %d)\n", gpiops2_dev->irq, ret);
        goto error_irq;
    }

    /* Figure out our device number. */
	ret = alloc_chrdev_region(&gpiops2_dev->dev,/*base minor*/ 0, 1, DRIVER_NAME);
    
    if (ret < 0) {
	    printk(KERN_WARNING "gpiops2: unable to get major %d", gpiops2_dev->dev);
	    goto err_major;
    }

	gpiops2_dev->ps2_class = class_create(THIS_MODULE, DRIVER_NAME);
	cdev_init(&gpiops2_dev->cdev, &gpiops2_fops);
	gpiops2_dev->cdev.owner = THIS_MODULE;
	gpiops2_dev->cdev.ops = &gpiops2_fops;
	
	ret = cdev_add (&gpiops2_dev->cdev, gpiops2_dev->dev, 1);
	/* Fail gracefully if need be */
	if (ret)
		printk (KERN_NOTICE "Error %d adding can_lpc2xxx\n", ret);
	if (device_create(gpiops2_dev->ps2_class, NULL, 
						MKDEV(MAJOR(gpiops2_dev->dev), 0), gpiops2_dev, DRIVER_NAME) == NULL) {
		printk("err----%s %d \n",__FILE__,__LINE__);
	}

	init_gpio_port();
    return 0;

err_major:
DEBUGMSG("err_major");
    kfree(&gpiops2_dev->cdev);
    free_irq(IRQ_GPIO_02, NULL);
error_irq:
DEBUGMSG("error_irq");
DEBUGMSG("err_kmalloc_buf");
    kfree(gpiops2_dev);
err_kmalloc_gpiops2_dev:

DEBUGMSG("err_kmalloc_gpiops2_dev");
    printk(KERN_WARNING "gpiops2 init FAILURE\n");
    return ret;
}


/*
 * Cleanup
 */
static void __exit gpiops2_cleanup(void)
{

    free_irq(gpiops2_dev->irq, NULL);
    cdev_del(&gpiops2_dev->cdev);
    kfree(&gpiops2_dev->cdev);
    unregister_chrdev_region(MKDEV(gpiops2_dev->dev, 0), 1);

    DEBUGMSG(KERN_INFO "lpc32xx GPIO PS/2 exit");
}


module_init(gpiops2_init);
module_exit(gpiops2_cleanup);

MODULE_DESCRIPTION("lpc32xx GPIO PS/2 controller driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("JiawenZhou");

