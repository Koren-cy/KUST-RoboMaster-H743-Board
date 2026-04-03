#ifndef USER_FDCAN_H
#define USER_FDCAN_H
#include "main.h"
#ifdef HAL_FDCAN_MODULE_ENABLED

/* 包含头文件 ----------------------------------------------------------------*/
#include "../../Core/Inc/bsp_config.h"
#include "../../User_Architect/user_circular_queue.h"

/* 类型定义 ------------------------------------------------------------------*/
/**
* @brief CAN 发送消息结构体
*/
typedef struct {
    uint32_t id;      /* 标准标识符或扩展标识符 */
    uint8_t data[8];  /* 数据段 */
    uint8_t len;      /* 数据长度 (0 ~ 8) */
} FDCAN_Tx_Msg;

/**
* @brief CAN 接收回调函数类型
* @param user_can CAN 驱动总线结构体指针
*/
typedef void (*FDCAN_Callback)(void* user_fdcan);

/**
* @brief 发送配置
*/
typedef struct {
    uint32_t IdType;      /* 标准标识符或扩展标识符 (FDCAN_STANDARD_ID / FDCAN_EXTENDED_ID) */
    uint32_t FrameType;   /* 帧类型 (FDCAN_DATA_FRAME / FDCAN_REMOTE_FRAME) */
} FDCAN_Tx_Config;

/**
* @brief 接收的消息
*/
typedef struct {
    uint32_t Identifier;  /* 标识符 */
    uint32_t IdType;      /* 标识符类型 (FDCAN_STANDARD_ID / FDCAN_EXTENDED_ID) */
    uint32_t FrameType;   /* 帧类型 (FDCAN_DATA_FRAME / FDCAN_REMOTE_FRAME) */
    uint32_t DataLength;  /* 数据长度编码 (FDCAN_DLC_BYTES_0 ~ FDCAN_DLC_BYTES_8) */
    uint8_t Data[8];      /* 数据段的内容 */
} FDCAN_Rx_Msg;

/**
* @brief can 总线驱动结构体
*/
typedef struct  {
    FDCAN_HandleTypeDef* hfdcan;                  /* FDCAN 硬件句柄 */
    FDCAN_Callback callbacks[CAN_CALLBACK_NUM];   /* 接收回调函数数组 */
    uint8_t callback_num;                       /* 已注册的回调函数数量 */
    FDCAN_Tx_Config tx_conf;                      /* 发送配置 */
    FDCAN_Rx_Msg rx_msg;                          /* 接收的消息 */
    CIRCULAR_QUEUE tx_queue;                    /* 发送队列 */
} FDCAN_DRIVES;

typedef FDCAN_DRIVES CAN_DRIVES; /* 为保持代码一致性，添加别名 */

/* 函数声明 ------------------------------------------------------------------*/
void CAN_Init(FDCAN_DRIVES* user_fdcan, FDCAN_HandleTypeDef* hfdcan);
void CAN_RegisterCallback(FDCAN_DRIVES* user_can, FDCAN_Callback callback);
void CAN_Send(FDCAN_DRIVES* user_can, uint32_t id, const uint8_t *data, uint8_t len);

#endif /* HAL_FDCAN_MODULE_ENABLED */
#endif // USER_FDCAN_H