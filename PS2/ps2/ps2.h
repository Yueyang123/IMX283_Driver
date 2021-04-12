#ifndef _PS2_H_
#define _PS2_H_

#define GPIO_IOBASE     io_p2v(GPIO_BASE)
#define CLKPWR_IOBASE 	io_p2v(CLK_PM_BASE)

#undef CONFIG_PS2_DEBUG

/* Define makro for driver debug-messages */
#ifdef CONFIG_PS2_DEBUG
#define DEBUGMSG(fmt,args...) printk(KERN_NOTICE "PS/2 (DEBUG): " fmt "\r\n",##args)
#else
#define DEBUGMSG(fmt,args...)		
#endif


/*The PS/2 keyboard define */
/*-------------------------------------------------------------------------*/

#define KBD_REPORT_ERR
#define KBD_REPORT_UNKN

#define KBD_ESCAPEE0	0xe0		/* in */
#define KBD_ESCAPEE1	0xe1		/* in */

#define ESCE0(x)		(0xe000|(x))
#define ESCE1(x)		(0xe100|(x))

#define KBD_BAT			0xaa		/* in */
#define KBD_SETLEDS		0xed		/* out */
#define KBD_ECHO		0xee		/* in/out */
#define KBD_BREAK		0xf0		/* in */
#define KBD_TYPRATEDLY	0xf3		/* out */
#define KBD_SCANENABLE	0xf4		/* out */
#define KBD_DEFDISABLE	0xf5		/* out */
#define KBD_DEFAULT		0xf6		/* out */
#define KBD_ACK			0xfa		/* in */
#define KBD_DIAGFAIL	0xfd		/* in */
#define KBD_RESEND		0xfe		/* in/out */
#define KBD_RESET		0xff		/* out */

#define CODE_BREAK		0x80
#define CODE_ESCAPEE0	2
#define CODE_ESCAPEE1	4
#define CODE_ESCAPE12	8

#define SEND_SUCCESS 	11


#define K_NONE		0x7f
#define K_ESC		0x01
#define K_F1		0x3b
#define K_F2		0x3c
#define K_F3		0x3d
#define K_F4		0x3e
#define K_F5		0x3f
#define K_F6		0x40
#define K_F7		0x41
#define K_F8		0x42
#define K_F9		0x43
#define K_F10		0x44
#define K_F11		0x57
#define K_F12		0x58
#define K_PRNT		0x54
#define K_SCRL		0x46
#define K_BRK		0x77
#define K_AGR		0x29
#define K_1			0x02
#define K_2			0x03
#define K_3			0x04
#define K_4			0x05
#define K_5			0x06
#define K_6			0x07
#define K_7			0x08
#define K_8			0x09
#define K_9			0x0a
#define K_0			0x0b
#define K_MINS		0x0c
#define K_EQLS		0x0d
#define K_BKSP		0x0e
#define K_INS		0x6e
#define K_HOME		0x66
#define K_PGUP		0x68
#define K_NUML		0x45
#define KP_SLH		0x62
#define KP_STR		0x37
#define KP_MNS		0x4a
#define K_TAB		0x0f
#define K_Q			0x10
#define K_W			0x11
#define K_E			0x12
#define K_R			0x13
#define K_T			0x14
#define K_Y			0x15
#define K_U			0x16
#define K_I			0x17
#define K_O			0x18
#define K_P			0x19
#define K_LSBK		0x1a
#define K_RSBK		0x1b
#define K_ENTR		0x1c
#define K_DEL		111
#define K_END		0x6b
#define K_PGDN		0x6d
#define KP_7		0x47
#define KP_8		0x48
#define KP_9		0x49
#define KP_PLS		0x4e
#define K_CAPS		0x3a
#define K_A			0x1e
#define K_S			0x1f
#define K_D			0x20
#define K_F			0x21
#define K_G			0x22
#define K_H			0x23
#define K_J			0x24
#define K_K			0x25
#define K_L			0x26
#define K_SEMI		0x27
#define K_SQOT		0x28
#define K_HASH		K_NONE
#define KP_4		0x4b
#define KP_5		0x4c
#define KP_6		0x4d
#define K_LSFT		0x2a
#define K_BSLH		0x2b
#define K_Z			0x2c
#define K_X			0x2d
#define K_C			0x2e
#define K_V			0x2f
#define K_B			0x30
#define K_N			0x31
#define K_M			0x32
#define K_COMA		0x33
#define K_DOT		0x34
#define K_FSLH		0x35
#define K_RSFT		0x36
#define K_UP		0x67
#define KP_1		0x4f
#define KP_2		0x50
#define KP_3		0x51
#define KP_ENT		0x1c  //0x1c
#define K_LCTL		0x1d
#define K_LALT		0x38
#define K_SPCE		0x39
#define K_RALT		0x64
#define K_RCTL		0x61
#define K_LEFT		0x69
#define K_DOWN		0x6c
#define K_RGHT		0x6a
#define KP_0		0x52
#define KP_DOT		0x53


#define K_VIEW_FIRE    113 
#define K_VIEW_ERROR   114
#define K_VIEW_ACTION  115
#define K_ACK_FIRE     116
/*-------------------------------------------------------------------------*/

#define OUT		1
#define IN		0

#define High	1
#define Low		0

#define Enable	1
#define Disable 0

#define PS2CK_IRQ_NO 	2
#define PS2CK_IRQ_NAME	IRQ_GPIO_02
#define GPIO_02			2
#define GPIO_03			3

/*PS2CK pin dir set: 1=out 0=in*/
#define PS2CK_DIR(dir)		((dir)? \
							(__raw_writel(OUTP_STATE_GPIO(GPIO_02), GPIO_P2_DIR_SET(GPIO_IOBASE))) :\
							(__raw_writel(OUTP_STATE_GPIO(GPIO_02), GPIO_P2_DIR_CLR(GPIO_IOBASE))))
							
/*PS2DT pin dir set: 1=out 0=in*/
#define PS2DT_DIR(dir)		((dir)? \
							(__raw_writel(OUTP_STATE_GPIO(GPIO_03), GPIO_P2_DIR_SET(GPIO_IOBASE))) :\
							(__raw_writel(OUTP_STATE_GPIO(GPIO_03), GPIO_P2_DIR_CLR(GPIO_IOBASE))))


/*PS2CK/PS2DT pin state*/
#define PS2CK_STATE()		(__raw_readl(GPIO_P3_INP_STATE(GPIO_IOBASE)) & INP_STATE_GPIO_02)
#define PS2DT_STATE()		(__raw_readl(GPIO_P3_INP_STATE(GPIO_IOBASE)) & INP_STATE_GPIO_03)


/*PS2CK pin set: 1:high, 0:low*/
#define PS2CK_OUTP(state)	{\
								PS2CK_DIR(OUT);\
								(state) ? \
								(__raw_writel(OUTP_STATE_GPIO(GPIO_02), GPIO_P3_OUTP_SET(GPIO_IOBASE))):\
								(__raw_writel(OUTP_STATE_GPIO(GPIO_02), GPIO_P3_OUTP_CLR(GPIO_IOBASE)));\
							}
							

/*PS2DT pin set: 1:high, 0:low*/
#define PS2DT_OUTP(state)	{\
								PS2DT_DIR(OUT);\
								(state) ? \
								(__raw_writel(OUTP_STATE_GPIO(GPIO_03), GPIO_P3_OUTP_SET(GPIO_IOBASE))):\
								(__raw_writel(OUTP_STATE_GPIO(GPIO_03), GPIO_P3_OUTP_CLR(GPIO_IOBASE)));\
							}



struct io_ps2 {
    dev_t dev; /* device number */
    int irq;
    struct cdev cdev;
    struct device sdev;
    struct class *ps2_class;
};


#endif
