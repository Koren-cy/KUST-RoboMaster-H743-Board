#include "../../Core/Inc/bsp.h"
#ifdef HAL_FDCAN_MODULE_ENABLED
/* 包含头文件 ----------------------------------------------------------------*/
#include "../user_fdcan.h"

/* 私有变量 ------------------------------------------------------------------*/
static FDCAN_DRIVES *fdcan_drives[CAN_NUM];
static uint8_t fdcan_num = 0;
static uint8_t is_init_loop_event_sign = 0;

/* 私有函数 ------------------------------------------------------------------*/

/**
* @brief 处理 CAN 发送队列
* @note  该函数会自动注册在全局注册表
*/
static void CAN_QueueHandle(void) {
    for (uint8_t can_index = 0; can_index < fdcan_num; can_index++) {
        FDCAN_DRIVES *fdcan = fdcan_drives[can_index];

        // 处理发送队列
        FDCAN_Tx_Msg tx_msg;
        if (Circular_Queue_Peek(&fdcan->tx_queue, (uint8_t*)&tx_msg)) {
            FDCAN_TxHeaderTypeDef tx_header;
            tx_header.Identifier = tx_msg.id;
            tx_header.IdType = fdcan->tx_conf.IdType;
            tx_header.TxFrameType = fdcan->tx_conf.FrameType;
            tx_header.DataLength = tx_msg.len << 16; // 0~8 字节映射为 FDCAN_DLC_BYTES_x
            tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
            tx_header.BitRateSwitch = FDCAN_BRS_OFF;
            tx_header.FDFormat = FDCAN_CLASSIC_CAN;
            tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
            tx_header.MessageMarker = 0;

            if (HAL_FDCAN_AddMessageToTxFifoQ(fdcan->hfdcan, &tx_header, tx_msg.data) == HAL_OK) {
                Circular_Queue_Discard(&fdcan->tx_queue);
            }
        }
    }
}

/* 函数体 --------------------------------------------------------------------*/

/**
* @brief 初始化 CAN 总线
* @param user_fdcan CAN 总线驱动结构体指针
* @param hfdcan    硬件句柄
*/
void CAN_Init(FDCAN_DRIVES* user_fdcan, FDCAN_HandleTypeDef* hfdcan){

    user_fdcan->hfdcan = hfdcan;

    user_fdcan->tx_conf.IdType = FDCAN_STANDARD_ID;
    user_fdcan->tx_conf.FrameType = FDCAN_DATA_FRAME;

    Circular_Queue_Init(&user_fdcan->tx_queue, sizeof(FDCAN_Tx_Msg), 6);

    FDCAN_FilterTypeDef can_filter;
    can_filter.IdType = FDCAN_STANDARD_ID;
    can_filter.FilterIndex = 0;
    can_filter.FilterType = FDCAN_FILTER_MASK;
    can_filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    can_filter.FilterID1 = 0x0000;
    can_filter.FilterID2 = 0x0000;

    HAL_FDCAN_ConfigFilter(hfdcan, &can_filter);
    HAL_FDCAN_Start(hfdcan);
    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    fdcan_drives[fdcan_num] = user_fdcan;
    fdcan_num ++;

    // 注册到全局事件循环
    if (is_init_loop_event_sign == 0) {
        loop_event[loop_event_num] = &CAN_QueueHandle;
        loop_event_num++;
        is_init_loop_event_sign = 1;
    }
}

/**
* @brief 注册 CAN 接收回调函数
* @param user_can CAN 总线驱动结构体指针
* @param callback 用户自定义的 can 总线消息接收回调函数
*/
void CAN_RegisterCallback(FDCAN_DRIVES* user_can, const FDCAN_Callback callback){
    if (user_can->callback_num < CAN_CALLBACK_NUM) {
        user_can->callbacks[user_can->callback_num] = callback;
        user_can->callback_num++;
    }
}

/**
* @brief 发送消息
* @param user_can CAN 总线驱动结构体指针
* @param id       报文标识符
* @param data     报文数据
* @param len      报文数据长度
*/
void CAN_Send(FDCAN_DRIVES* user_can, const uint32_t id, const uint8_t *data, const uint8_t len){
    FDCAN_Tx_Msg tx_msg;
    tx_msg.id = id;
    tx_msg.len = (len > 8) ? 8 : len;
    
    memcpy(tx_msg.data, data, tx_msg.len);
    Circular_Queue_Enqueue(&user_can->tx_queue, (uint8_t*)&tx_msg);
}


/* 覆写中断回调函数 -----------------------------------------------------------*/

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        for (uint8_t can_index = 0 ; can_index < fdcan_num ; can_index++) {
            FDCAN_DRIVES* can_drive = fdcan_drives[can_index];
            if (hfdcan == can_drive->hfdcan) {
                FDCAN_RxHeaderTypeDef RxHeader;

                if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, can_drive->rx_msg.Data) != HAL_OK)
                    continue;

                can_drive->rx_msg.Identifier = RxHeader.Identifier;
                can_drive->rx_msg.IdType = RxHeader.IdType;
                can_drive->rx_msg.FrameType = RxHeader.RxFrameType;
                can_drive->rx_msg.DataLength = RxHeader.DataLength;

                for (uint8_t i = 0; i < can_drive->callback_num; i++) {
                    can_drive->callbacks[i](can_drive);
                }
            }
        }
    }
}

#endif /* HAL_FDCAN_MODULE_ENABLED */