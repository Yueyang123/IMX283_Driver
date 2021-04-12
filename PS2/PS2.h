/*
 * @Author: your name
 * @Date: 2021-04-11 19:10:30
 * @LastEditTime: 2021-04-11 19:11:40
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /DriverDefine/PS2/PS2.h
 */
#ifndef __PSTWO_H
#define __PSTWO_H

//CLK 	    PB3
//CS		PB5
//CMD		PB7
//DAT		PB9

#define PS_GROUP GPIOB

//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2         9
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
//#define WHAMMY_BAR		8
//These are stick values
#define PSS_RX 5             
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

#endif
