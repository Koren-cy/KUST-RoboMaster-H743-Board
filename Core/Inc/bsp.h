#ifndef USER_BSP_H
#define USER_BSP_H

/* 包含头文件 ----------------------------------------------------------------*/
#include "main.h"
#include "../../SEGGER_RTT/SEGGER_RTT.h"
#include"../../User_Drives/user_can.h"
/* 全局注册表 ----------------------------------------------------------------*/
#define MAX_LOOP_EVENT 32
void LOOP_EVENT_Handle(void);
typedef void (*LOOP_Event)(void);
extern LOOP_Event loop_event[MAX_LOOP_EVENT];
extern uint8_t loop_event_num;

/* SysTick 系统滴答定时器回调函数表 ------------------------------------------------------------*/
#include "../../User_Architect/user_systick.h"

/* JScope ------------------------------------------------------------------*/
#include "../../SEGGER_RTT/user_JScope_Transmit.h"
#ifdef HAL_TIM_MODULE_ENABLED
extern JScope_Transmit_t jscope_transmit;
extern uint8_t JScope_RTT_UpBuffer[BUFFER_SIZE_UP];
#endif /* HAL_TIM_MODULE_ENABLED */

/* 接口定义 ------------------------------------------------------------------*/
// 调试串口
#include "../../User_Drives/user_uart.h"
extern UART_DRIVES user_lp_uart;
extern UART_DRIVES user_stp1_uart;
extern UART_DRIVES user_stp2_uart;
extern UART_DRIVES user_stp3_uart;
extern UART_DRIVES user_stp4_uart;
extern UART_DRIVES user_stp5_uart;
extern UART_DRIVES user_stp6_uart;
extern UART_DRIVES user_stp7_uart;
extern UART_DRIVES user_stp8_uart;

extern

//激光模块
#include "../../User_Drives/user_stp23.h"
extern STP23_DRIVES user_stp23_num[8];



#include "../../User_Algorithm/user_distance_control.h"
extern DistanceControl user_distance_control;



#endif // USER_BSP_H
