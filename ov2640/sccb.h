#ifndef __SCCB_H
#define __SCCB_H


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

#define SDA_PIN     3*32+21
#define SCL_PIN     3*32+27

#define delay_us(x)  udelay(x)
#define delay_ms(x)  mdelay(x)

#define SCCB_SDA_IN()  {    gpio_free(SDA_PIN); \
                            gpio_request(SDA_PIN,"SDA"); \
                            gpio_direction_input(SDA_PIN); \
                        }
#define SCCB_SDA_OUT()  {   gpio_free(SDA_PIN); \
                            gpio_request(SDA_PIN,"SDA"); \
                            gpio_direction_output(SDA_PIN,0); \
                        }


#define SCCB_SCL(x)        gpio_set_value(SCL_PIN,x)    //SCL
#define SCCB_SDA(x)        gpio_set_value(SDA_PIN,x)    //SDA 
#define SCCB_READ_SDA      gpio_get_value(SDA_PIN)     //  

#define SCCB_ID   			0X60    //OV2640ID

///////////////////////////////////////////
void SCCB_Init(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
u8 SCCB_WR_Byte(u8 dat);
u8 SCCB_RD_Byte(void);
u8 SCCB_WR_Reg(u8 reg,u8 data);
u8 SCCB_RD_Reg(u8 reg);
#endif













