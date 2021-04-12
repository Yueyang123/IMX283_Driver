/*
 * @Author: your name
 * @Date: 2021-04-11 18:25:16
 * @LastEditTime: 2021-04-11 23:46:33
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/PS2/PS2.c
 */
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
#include <linux/timer.h>

#define DEVICE_NAME  "joystick"

#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16
#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      16

#define PSS_RX          5           
#define PSS_RY          6
#define PSS_LX          7
#define PSS_LY          8

static struct timer_list ps2timer; 
u16 Handkey;
u8 Comd[2]={0x01,0x42};	
u8 Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //Êý¾Ý´æ´¢Êý×é
u16 MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
};	

typedef struct  
{
    int data_pin;
    int cmd_pin;
    int cs_pin;
    int clk_pin;
}ps2_handler;

ps2_handler ps_define={
    2*32+5,2*32+7,2*32+13,2*32+15,
};

#define DI      gpio_get_value(ps_define.data_pin)
#define DO_H    gpio_direction_output(ps_define.cmd_pin,1)
#define DO_L    gpio_direction_output(ps_define.cmd_pin,0)

#define CS_H    gpio_direction_output(ps_define.cs_pin,1)    
#define CS_L    gpio_direction_output(ps_define.cs_pin,0)    

#define CLK_H   gpio_direction_output(ps_define.clk_pin,1)     
#define CLK_L   gpio_direction_output(ps_define.clk_pin,0)   


typedef struct 
{
   int PHYKEY;
   int LOGICKEY;
}KEY_MAP;
KEY_MAP key_map[16]={
    {PSB_SELECT,    KEY_1},
    {PSB_L3,        KEY_A},
    {PSB_R3,        KEY_B},
    {PSB_START,     KEY_C},
    {PSB_PAD_UP,    KEY_A},
    {PSB_PAD_RIGHT, KEY_A},
    {PSB_PAD_DOWN,  KEY_A},
    {PSB_PAD_LEFT,  KEY_A},
    {PSB_L2,        KEY_1},
    {PSB_R2,        KEY_2},
    {PSB_L1,        KEY_3},
    {PSB_R1,        KEY_4},
    {PSB_GREEN,     KEY_A},
    {PSB_RED,       KEY_ENTER},
    {PSB_BLUE,      KEY_7},
    {PSB_PINK,      KEY_8},
};

struct input_dev *inputdev;
void PS2_Cmd(u8 CMD)
{
	volatile u16 ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{
		if(ref&CMD)
		{
			DO_H;               
		}
		else DO_L;
		CLK_H;                     
		udelay(50);
		CLK_L;
		udelay(50);
		CLK_H;
		if(DI)
			Data[1] = ref|Data[1];
	}
}
void PS2_ReadData(void)
{
	volatile u8 byte=0;
	volatile u16 ref=0x01;
	CS_L;
	PS2_Cmd(Comd[0]); 
	PS2_Cmd(Comd[1]);  

	for(byte=2;byte<9;byte++)        
	{
		for(ref=0x01;ref<0x100;ref<<=1)
		{
			CLK_H;
			CLK_L;
			udelay(50);
			CLK_H;
		      if(DI)
		      Data[byte] = ref|Data[byte];
		}
        udelay(50);
	}
	CS_H;	
}


void PS2_ClearData()
{
	u8 a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}
u8 PS2_DataKey()
{
	u8 index;
	PS2_ClearData();
	PS2_ReadData();
	Handkey=(Data[4]<<8)|Data[3];    
	for(index=0;index<16;index++)
	{	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
		return index+1;
	}
	return 0;      
}
u8 PS2_AnologData(u8 button)
{
	return Data[button];
}
void PS2_ShortPoll(void)
{
	CS_L;
	udelay(16);
	PS2_Cmd(0x01);
	PS2_Cmd(0x42);
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x00);
	CS_H;
	udelay(16);
}

void PS2_EnterConfing(void)
{
	CS_L;
	udelay(16);
	PS2_Cmd(0x01);
	PS2_Cmd(0x43);
	PS2_Cmd(0X00);
	PS2_Cmd(0x01);
	PS2_Cmd(0x00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	udelay(16);
}

void PS2_TurnOnAnalogMode(void)
{
    CS_L;
    PS2_Cmd(0x01);
    PS2_Cmd(0x44);
    PS2_Cmd(0X00);
    PS2_Cmd(0x01);
    PS2_Cmd(0xEE); 
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    udelay(16);
}

void PS2_VibrationMode(void)
{
	CS_L;
	udelay(16);
	PS2_Cmd(0x01);
	PS2_Cmd(0x4D);
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0X01);
	CS_H;
	udelay(16);
}

void PS2_ExitConfing(void)
{
	CS_L;
	udelay(16);
	PS2_Cmd(0x01);
	PS2_Cmd(0x43);
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	CS_H;
	udelay(16);
}

void PS2_SetInit(void)
{
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_EnterConfing(); 
	PS2_TurnOnAnalogMode(); 
	PS2_VibrationMode(); 
	PS2_ExitConfing();
}

void PS2_Vibration(u8 motor1, u8 motor2)
{
	CS_L;
	udelay(16);
	PS2_Cmd(0x01); 
	PS2_Cmd(0x42); 
	PS2_Cmd(0X00);
	PS2_Cmd(motor1);
	PS2_Cmd(motor2);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	CS_H;
	udelay(16);
}


void ps2_timer_function(unsigned long data)
{
    u8 val;
    val=PS2_DataKey();
    printk("%d \t\n",val);
    if(val==0)
    {
        input_event(inputdev,EV_KEY,key_map[val].LOGICKEY, 0);  //上报EV_KEY类型,button按键,0(没按下)
        input_sync(inputdev);
    }else
    {
        input_event(inputdev,EV_KEY,key_map[val].LOGICKEY, 1);  //上报EV_KEY类型,button按键,0(没按下)
        input_event(inputdev,EV_REL,10, 1); 
        input_sync(inputdev);        
        //printk("%d \t\n",key_map[val].LOGICKEY);
    }
    mod_timer(&ps2timer,jiffies+ HZ*100/1000  );
}

static int PS2_init(void)
{
    int i = 0;
    inputdev = input_allocate_device(); /* 为输入设备驱动对象申请内存空间*/
    if (!inputdev) {return -ENOMEM;}
    inputdev->name = DEVICE_NAME;
    
    set_bit(EV_KEY, inputdev->evbit); /* 设置输入设备支持按键事件 */
    set_bit(EV_REL, inputdev->evbit); 

    set_bit(KEY_A,inputdev->keybit);                  //支持按键 L
    set_bit(KEY_B,inputdev->keybit);                //支持按键 S
    set_bit(KEY_C,inputdev->keybit);      //支持按键 空格
    set_bit(KEY_ENTER,inputdev->keybit);


    //GPIO
    gpio_request(ps_define.data_pin, "data");
    gpio_direction_input(ps_define.data_pin);
    gpio_request(ps_define.cs_pin, "cs");
    gpio_direction_output(ps_define.cs_pin,0); 
    gpio_request(ps_define.clk_pin, "clk");
    gpio_direction_output(ps_define.clk_pin,0);
    gpio_request(ps_define.cmd_pin, "cmd");
    gpio_direction_output(ps_define.cmd_pin,0);
    PS2_SetInit();
    input_register_device(inputdev); /* 注册设备驱动  */
    
    init_timer(&ps2timer);
    ps2timer.function=ps2_timer_function;
    ps2timer.expires=jiffies +HZ*100/1000;
    add_timer(&ps2timer);

    printk("YURI PS2 driver up \n");
    return 0;
}

static void PS2_exit(void)
{
    del_timer(&ps2timer);
    input_unregister_device(inputdev); /* 注销输入设备驱动 */
    input_free_device(inputdev);
    printk("PS2 driver remove \n");
}

module_init(PS2_init);
module_exit(PS2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YURI YANG");