#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/i2c.h>  
#include <linux/slab.h>  
#include <linux/delay.h>  
#include <linux/videodev2.h>  
#include <media/v4l2-chip-ident.h>  
#include <media/v4l2-subdev.h>  
#include <media/soc_camera.h>  
#include <media/soc_mediabus.h>  
#include <linux/miscdevice.h>
#include <asm/io.h>  
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include "ov2640.h"
#include "sccb.h"

#define DEVICE_NAME "ov2640"


#define SDA_PIN     3*32+21
#define SCL_PIN     3*32+27


#define SCCB_SDA_IN()  {  gpio_direction_input(SDA_PIN);}
#define SCCB_SDA_OUT() {  gpio_direction_output(SDA_PIN,0); }


#define SCCB_SCL(x)        gpio_set_value(SCL_PIN,x)    //SCL
#define SCCB_SDA(x)        gpio_set_value(SDA_PIN,x)    //SDA 
#define SCCB_READ_SDA      gpio_get_value(SDA_PIN)     //  

#define SCCB_ID   			0X60    //OV2640ID
#define WAIT                100
void SCCB_Init(void)
{				
    gpio_free(SDA_PIN);
    gpio_request(SDA_PIN,"SDA");
    gpio_direction_output(SDA_PIN,0);

    gpio_free(SCL_PIN);
    gpio_request(SCL_PIN,"SCL");
    gpio_direction_output(SCL_PIN,0);
  
}			 


void SCCB_Start(void)
{
    SCCB_SDA(1);   
    SCCB_SCL(1);	    
    udelay(WAIT);  
    SCCB_SDA(0);
    udelay(WAIT); 
    SCCB_SCL(0);  
}


void SCCB_Stop(void)
{
    SCCB_SDA(0);
    udelay(WAIT);	 
    SCCB_SCL(1);	
    udelay(WAIT); 
    SCCB_SDA(1);	
    udelay(WAIT);
}  

void SCCB_No_Ack(void)
{
	udelay(WAIT);
	SCCB_SDA(1);	
	SCCB_SCL(1);	
	udelay(WAIT);
	SCCB_SCL(0);	
	udelay(WAIT);
	SCCB_SDA(0);	
	udelay(WAIT);
}
 
u8 SCCB_WR_Byte(u8 dat)
{
	u8 j,res;	 
	for(j=0;j<8;j++)
	{
		if(dat&0x80)SCCB_SDA(1);	
		else SCCB_SDA(0);
		dat<<=1;
		udelay(WAIT);
		SCCB_SCL(1);	
		udelay(WAIT);
		SCCB_SCL(0);		   
	}			 
	SCCB_SDA_IN();
	udelay(WAIT);
	SCCB_SCL(1);
	udelay(WAIT);
	if(SCCB_READ_SDA)res=1;
	else res=0;
	SCCB_SCL(0);		 
	SCCB_SDA_OUT(); 
	return res;  
}	 

u8 SCCB_RD_Byte(void)
{
	u8 temp=0,j;
	SCCB_SDA_IN();		
	for(j=8;j>0;j--)
	{		     	  
		udelay(WAIT);
		SCCB_SCL(1);
		temp=temp<<1;
		if(SCCB_READ_SDA)temp++;   
		udelay(WAIT);
		SCCB_SCL(0);
	}	
	SCCB_SDA_OUT();  
	return temp;
} 							    

u8 SCCB_WR_Reg(u8 reg,u8 data)
{
	u8 res=0;
	SCCB_Start(); 					
	if(SCCB_WR_Byte(SCCB_ID))res=1;		  
	udelay(WAIT);
  	if(SCCB_WR_Byte(reg))res=1;			  
	udelay(WAIT);
  	if(SCCB_WR_Byte(data))res=1; 		 
  	SCCB_Stop();	  
  	return	res;
}		  					    

u8 SCCB_RD_Reg(u8 reg)
{
	u8 val=0;
	SCCB_Start(); 				
	SCCB_WR_Byte(SCCB_ID);	
	udelay(WAIT);	 
  	SCCB_WR_Byte(reg);	  
	udelay(WAIT);	  
	SCCB_Stop();   
	udelay(WAIT);
	SCCB_Start();
	SCCB_WR_Byte(SCCB_ID|0X01);	  
	udelay(WAIT);
  	val=SCCB_RD_Byte();
  	SCCB_No_Ack();
  	SCCB_Stop();
  	return val;
}


/*********** start of OV2640 ********************/ 

void OV2640_PWDN_Set(u8 sta)
{
	OV2640_PWDN(sta);
}

u8 OV2640_Init(void)
{ 
	u16 i=0;
	u16 reg;

    SCCB_Init();
    gpio_free(RST_PIN);
    gpio_request(RST_PIN,"RST");
    gpio_direction_output(RST_PIN,0);
 

    gpio_free(PWDN_PIN);
    gpio_request(PWDN_PIN,"RST");
    gpio_direction_output(PWDN_PIN,0);
   
    gpio_set_value(PWDN_PIN,0);

 	//OV2640_PWDN_Set(0);
	mdelay(WAIT);
	OV2640_RST(0);
	mdelay(WAIT);
	OV2640_RST(1);
    SCCB_Init();
	SCCB_WR_Reg(OV2640_DSP_RA_DLMT, 0x01);
 	SCCB_WR_Reg(OV2640_SENSOR_COM7, 0x80);
	mdelay(WAIT); 
	reg=SCCB_RD_Reg(OV2640_SENSOR_MIDH);
	reg<<=8;
	reg|=SCCB_RD_Reg(OV2640_SENSOR_MIDL);
	if(reg!=OV2640_MID)
	{
		printk("MID:%d\r\n",reg);
		return 1;
	}
	reg=SCCB_RD_Reg(OV2640_SENSOR_PIDH);
	reg<<=8;
	reg|=SCCB_RD_Reg(OV2640_SENSOR_PIDL);
	if(reg!=OV2640_PID)
	{
		printk("HID:%d\r\n",reg);
		return 2;
	}   

	for(i=0;i<sizeof(ov2640_sxga_init_reg_tbl)/2;i++)
	{
	   	SCCB_WR_Reg(ov2640_sxga_init_reg_tbl[i][0],ov2640_sxga_init_reg_tbl[i][1]);
 	} 
  	return 0x00; 	//ok
}

void OV2640_JPEG_Mode(void) 
{
	u16 i=0;

	for(i=0;i<(sizeof(ov2640_yuv422_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_yuv422_reg_tbl[i][0],ov2640_yuv422_reg_tbl[i][1]); 
	} 

	for(i=0;i<(sizeof(ov2640_jpeg_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_jpeg_reg_tbl[i][0],ov2640_jpeg_reg_tbl[i][1]);  
	}  
}

void OV2640_RGB565_Mode(void) 
{
	u16 i=0;
	for(i=0;i<(sizeof(ov2640_rgb565_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1]); 
	} 
} 

const static u8 OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
	{
		0xFF,0x01,
		0x24,0x20,
		0x25,0x18,
		0x26,0x60,
	},
	{
		0xFF,0x01,
		0x24,0x34,
		0x25,0x1c,
		0x26,0x00,
	},
	{
		0xFF,0x01,	
		0x24,0x3e,	
		0x25,0x38,
		0x26,0x81,
	},
	{
		0xFF,0x01,
		0x24,0x48,
		0x25,0x40,
		0x26,0x81,
	},
	{
		0xFF,0x01,	
		0x24,0x58,	
		0x25,0x50,	
		0x26,0x92,	
	},
}; 

void OV2640_Auto_Exposure(u8 level)
{  
	u8 i;
	u8 *p=(u8*)OV2640_AUTOEXPOSURE_LEVEL[level];
	for(i=0;i<4;i++)
	{ 
		SCCB_WR_Reg(p[i*2],p[i*2+1]); 
	} 
}  

void OV2640_Light_Mode(u8 mode)
{
	u8 regccval=0X5E;//Sunny 
	u8 regcdval=0X41;
	u8 regceval=0X54;
	switch(mode)
	{ 
		case 0://auto 
			SCCB_WR_Reg(0XFF,0X00);	 
			SCCB_WR_Reg(0XC7,0X10);//AWB ON 
			return;  	
		case 2://cloudy
			regccval=0X65;
			regcdval=0X41;
			regceval=0X4F;
			break;	
		case 3://office
			regccval=0X52;
			regcdval=0X41;
			regceval=0X66;
			break;	
		case 4://home
			regccval=0X42;
			regcdval=0X3F;
			regceval=0X71;
			break;	
	}
	SCCB_WR_Reg(0XFF,0X00);	 
	SCCB_WR_Reg(0XC7,0X40);	//AWB OFF 
	SCCB_WR_Reg(0XCC,regccval); 
	SCCB_WR_Reg(0XCD,regcdval); 
	SCCB_WR_Reg(0XCE,regceval);  
}

void OV2640_Color_Saturation(u8 sat)
{ 
	u8 reg7dval=((sat+2)<<4)|0X08;
	SCCB_WR_Reg(0XFF,0X00);		
	SCCB_WR_Reg(0X7C,0X00);		
	SCCB_WR_Reg(0X7D,0X02);				
	SCCB_WR_Reg(0X7C,0X03);			
	SCCB_WR_Reg(0X7D,reg7dval);			
	SCCB_WR_Reg(0X7D,reg7dval); 		
}

void OV2640_Brightness(u8 bright)
{
  SCCB_WR_Reg(0xff, 0x00);
  SCCB_WR_Reg(0x7c, 0x00);
  SCCB_WR_Reg(0x7d, 0x04);
  SCCB_WR_Reg(0x7c, 0x09);
  SCCB_WR_Reg(0x7d, bright<<4); 
  SCCB_WR_Reg(0x7d, 0x00); 
}

void OV2640_Contrast(u8 contrast)
{
	u8 reg7d0val=0X20;
	u8 reg7d1val=0X20;
  	switch(contrast)
	{
		case 0://-2
			reg7d0val=0X18;	 	 
			reg7d1val=0X34;	 	 
			break;	
		case 1://-1
			reg7d0val=0X1C;	 	 
			reg7d1val=0X2A;	 	 
			break;	
		case 3://1
			reg7d0val=0X24;	 	 
			reg7d1val=0X16;	 	 
			break;	
		case 4://2
			reg7d0val=0X28;	 	 
			reg7d1val=0X0C;	 	 
			break;	
	}
	SCCB_WR_Reg(0xff,0x00);
	SCCB_WR_Reg(0x7c,0x00);
	SCCB_WR_Reg(0x7d,0x04);
	SCCB_WR_Reg(0x7c,0x07);
	SCCB_WR_Reg(0x7d,0x20);
	SCCB_WR_Reg(0x7d,reg7d0val);
	SCCB_WR_Reg(0x7d,reg7d1val);
	SCCB_WR_Reg(0x7d,0x06);
}
    
void OV2640_Special_Effects(u8 eft)
{
	u8 reg7d0val=0X00;
	u8 reg7d1val=0X80;
	u8 reg7d2val=0X80; 
	switch(eft)
	{
		case 1:
			reg7d0val=0X40; 
			break;	
		case 2:
			reg7d0val=0X18; 
			break;	 
		case 3:
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XC0; 
			break;	
		case 4:
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0X40; 
			break;	
		case 5:
			reg7d0val=0X18; 
			reg7d1val=0XA0;
			reg7d2val=0X40; 
			break;	
		case 6:
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XA6; 
			break;	 
	}
	SCCB_WR_Reg(0xff,0x00);
	SCCB_WR_Reg(0x7c,0x00);
	SCCB_WR_Reg(0x7d,reg7d0val);
	SCCB_WR_Reg(0x7c,0x05);
	SCCB_WR_Reg(0x7d,reg7d1val);
	SCCB_WR_Reg(0x7d,reg7d2val); 
}

void OV2640_Color_Bar(u8 sw)
{
	u8 reg;
	SCCB_WR_Reg(0XFF,0X01);
	reg=SCCB_RD_Reg(0X12);
	reg&=~(1<<1);
	if(sw)reg|=1<<1; 
	SCCB_WR_Reg(0X12,reg);
}

void OV2640_Window_Set(u16 sx,u16 sy,u16 width,u16 height)
{
	u16 endx;
	u16 endy;
	u8 temp; 
	endx=sx+width/2;	//V*2
 	endy=sy+height/2;
	
	SCCB_WR_Reg(0XFF,0X01);			
	temp=SCCB_RD_Reg(0X03);				
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WR_Reg(0X03,temp);				
	SCCB_WR_Reg(0X19,sy>>2);		
	SCCB_WR_Reg(0X1A,endy>>2);		
	
	temp=SCCB_RD_Reg(0X32);			
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WR_Reg(0X32,temp);				
	SCCB_WR_Reg(0X17,sx>>3);			
	SCCB_WR_Reg(0X18,endx>>3);		
}

u8 OV2640_OutSize_Set(u16 width,u16 height)
{
	u16 outh;
	u16 outw;
	u8 temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	outw=width/4;
	outh=height/4; 
	SCCB_WR_Reg(0XFF,0X00);	
	SCCB_WR_Reg(0XE0,0X04);			
	SCCB_WR_Reg(0X5A,outw&0XFF);	
	SCCB_WR_Reg(0X5B,outh&0XFF);	
	temp=(outw>>8)&0X03;
	temp|=(outh>>6)&0X04;
	SCCB_WR_Reg(0X5C,temp);			
	SCCB_WR_Reg(0XE0,0X00);	
	return 0;
}

u8 OV2640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height)
{
	u16 hsize;
	u16 vsize;
	u8 temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	hsize=width/4;
	vsize=height/4;
	SCCB_WR_Reg(0XFF,0X00);	
	SCCB_WR_Reg(0XE0,0X04);					
	SCCB_WR_Reg(0X51,hsize&0XFF);		
	SCCB_WR_Reg(0X52,vsize&0XFF);
	SCCB_WR_Reg(0X53,offx&0XFF);
	SCCB_WR_Reg(0X54,offy&0XFF);
	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07; 
	SCCB_WR_Reg(0X55,temp);
	SCCB_WR_Reg(0X57,(hsize>>2)&0X80);
	SCCB_WR_Reg(0XE0,0X00);	
	return 0;
} 

u8 OV2640_ImageSize_Set(u16 width,u16 height)
{ 
	u8 temp; 
	SCCB_WR_Reg(0XFF,0X00);			
	SCCB_WR_Reg(0XE0,0X04);			
	SCCB_WR_Reg(0XC0,(width)>>3&0XFF);
	SCCB_WR_Reg(0XC1,(height)>>3&0XFF);
	temp=(width&0X07)<<3;
	temp|=height&0X07;
	temp|=(width>>4)&0X80; 
	SCCB_WR_Reg(0X8C,temp);	
	SCCB_WR_Reg(0XE0,0X00);				 
	return 0;
}

static unsigned int get_camera_clk(void)  
{  
    return OV2640_MAX_CLK;  
}

/********* end of OV2640 ********************/


/********** start of S/W YUV2RGB ************/ 

#define XLATTABSIZE      256  
#define MulDiv(x, y, z) ((long)((int) x * (int) y) / (int) z)  
  
//#define CLIP(x)   min_t(int, 255, max_t(int, 0, (x)))  
#define CLIP(x) {if(x<0) x=0;if(x>255) x=255;}  
  
#define RED_REGION      0xf800  
#define GREEN_REGION    0x07e0  
#define BLUE_REGION     0x001f  
  
int XlatY[XLATTABSIZE] = { 0 };  
int XlatV_B[XLATTABSIZE] = { 0 };  
int XlatV_G[XLATTABSIZE] = { 0 };  
int XlatU_G[XLATTABSIZE] = { 0 };  
int XlatU_R[XLATTABSIZE] = { 0 };  
  
#define ORIG_XLAT 1  
void __init init_yuvtable (void)  
{  
    int i, j;  
  
    for (i = 0; i < XLATTABSIZE; i++) {  
#if ORIG_XLAT  
        j = min_t(int, 253, max_t(int, 16, i));  
#else  
        j = (255 * i + 110) / 220;  // scale up  
        j = min_t(int, 255, max_t(int, j, 16));  
#endif  
        // orig: XlatY[i] = (int ) j;  
        XlatY[i] = j-16;  
    }  
  
    for (i = 0; i < XLATTABSIZE; i++) {  
#if ORIG_XLAT  
        j = min_t(int, 240, max_t(int, 16, i));  
        j -= 128;  
#else  
        j = i - 128;        // make signed  
        if (j < 0)  
            j++;            // noise reduction  
        j = (127 * j + 56) / 112;   // scale up  
        j = min_t(int, 127, max_t(int, -128, j));  
#endif  
  
        XlatV_B[i] = MulDiv (j, 1000, 564); /* j*219/126 */  
        XlatV_G[i] = MulDiv (j, 1100, 3328);  
        XlatU_G[i] = MulDiv (j, 3100, 4207);  
        XlatU_R[i] = MulDiv (j, 1000, 713);  
    }  
}  
  
#define MORE_QUALITY 1  
void inline yuv_convert_rgb16(unsigned char *rawY, unsigned char *rawU,  
        unsigned char *rawV, unsigned char *rgb, int size)  
{  
    unsigned short  buf1, buf3;  
    int   red;  
    int   blue;  
    int   green;  
    unsigned long   cnt;  
    int    Y, U, V;  
    unsigned short  data;  
    unsigned short  data2;  
  
    for ( cnt = 0 ; cnt < size; cnt +=2){  
        buf1 = *(rawY+cnt) & 0xff;  // Y data  
        buf3 = *(rawY+cnt+1) & 0xff;  // Y data  
  
        U = *(rawV+cnt/2) & 0xff;  
        V = *(rawU+cnt/2) & 0xff;  
  
#if MORE_QUALITY  
        Y = buf1;  
#else  
        Y = ((buf1+buf3)/2);  
#endif  
  
        red = XlatY[Y] + XlatU_R[U];  
        CLIP(red);  
        green = XlatY[Y] - XlatV_G[V] - XlatU_G[U];  
        CLIP(green);  
        blue = XlatY[Y] + XlatV_B[V];  
        CLIP(blue);  
  
        data = ((red << 8) & RED_REGION)  
                | ((green << 3) & GREEN_REGION)  
                | (blue >> 3);  
  
#if MORE_QUALITY  
        Y = buf3;  
        red = XlatY[Y] + XlatU_R[U];  
        CLIP(red);  
        green = XlatY[Y] - XlatV_G[V] - XlatU_G[U];  
        CLIP(green);  
        blue = XlatY[Y] + XlatV_B[V];  
        CLIP(blue);  
  
        data2 = ((red << 8) & RED_REGION)  
                | ((green << 3) & GREEN_REGION)  
                | (blue >> 3);  
#else  
        data2 = data;  
#endif  
  
        *(unsigned short *)(rgb + 2 * cnt) = data;  
        *(unsigned short *)(rgb + 2 * (cnt + 1))= data2;  
    }  
}  
  
void inline yuv_convert_rgb32(unsigned char *rawY, unsigned char *rawU,   
        unsigned char *rawV, unsigned char *rgb, int size)  
{  
    unsigned short  buf1, buf3;  
    int   red;  
    int   blue;  
    int   green;  
    unsigned long   cnt;  
    int    Y, U, V;  
  
    for ( cnt = 0 ; cnt < size; cnt +=2){  
        buf1 = *(rawY+cnt) & 0xff;  // Y data  
        buf3 = *(rawY+cnt+1) & 0xff;  // Y data  
  
        U = *(rawV+cnt/2) & 0xff;  
        V = *(rawU+cnt/2) & 0xff;  
  
#if MORE_QUALITY  
        Y = buf1;  
#else  
        Y = ((buf1+buf3)/2);  
#endif  
  
        red = XlatY[Y] + XlatU_R[U];  
        CLIP(red);  
        green = XlatY[Y] - XlatV_G[V] - XlatU_G[U];  
        CLIP(green);  
        blue = XlatY[Y] + XlatV_B[V];  
        CLIP(blue);  
  
        *(unsigned int*)(rgb) = (red << 16) | (green << 8) | blue;  
        rgb += 4;  
  
#if MORE_QUALITY  
        Y = buf3;  
        red = XlatY[Y] + XlatU_R[U];  
        CLIP(red);  
        green = XlatY[Y] - XlatV_G[V] - XlatU_G[U];  
        CLIP(green);  
        blue = XlatY[Y] + XlatV_B[V];  
        CLIP(blue);  
#endif  
        *(unsigned int*)(rgb) = (red << 16) | (green << 8) | blue;  
        rgb += 4;  
    }  
}  
  
/********** end of S/W YUV2RGB ******************/ 


static int cam_open(struct inode *inode, struct file *file)  
{  

  

  
    return 0;  
}  

static struct file_operations cam_fops = {  
    owner  :  THIS_MODULE,  
    open   :  cam_open,
};  
  
static struct miscdevice cam_dev = {  
    minor :  MISC_DYNAMIC_MINOR,  
    name  :  DEVICE_NAME,  
    fops  :  &cam_fops  
};  


static int __devinit init_imx_ov2640(void)
{
    printk(DEVICE_NAME "up");

    u8 i= OV2640_Init();
    printk("ok? :%d \n",i);

    misc_register(&cam_dev);

    return 0;
}

static void __exit exit_imx_ov2640(void)
{
    printk(DEVICE_NAME "remove");
    misc_deregister(&cam_dev);
}


module_init(init_imx_ov2640);  
module_exit(exit_imx_ov2640); 

MODULE_LICENSE("GPL"); 