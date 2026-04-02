#include "../../Core/Inc/bsp.h"
#ifdef HAL_UART_MODULE_ENABLED

/* 包含头文件 ----------------------------------------------------------------*/
#include "../user_stp23.h"
#include "../../User_Architect/user_codec.h"

/* 私有变量 ------------------------------------------------------------------*/
static STP23_DRIVES* stp23_drives[STP23_NUM];
static uint8_t stp23_num = 0;
uint8_t count = 0;
float delta_angle_fortest = 0.45f;

/* 私有函数声明 --------------------------------------------------------------*/
static void STP23_Handle(void* user_uart);

/* 函数体 --------------------------------------------------------------------*/

/**
* @brief 初始化 STP23 激光测距模块
* @param user_stp23  STP23 驱动结构体指针
* @param user_uart   UART 驱动结构体指针
*/
void STP23_Init(STP23_DRIVES *user_stp23, UART_DRIVES* user_uart, uint8_t id) {

    user_stp23->uart = user_uart;
    user_stp23->id = id;
    UART_RegisterCallback(user_uart, STP23_Handle);
    STP23_RegisterCallback(user_stp23, Stp_To_Circle);

    stp23_drives[stp23_num] = user_stp23;
    stp23_num++;
}

/**
* @brief 处理 STP23 接收数据
* @param user_uart UART 驱动结构体指针
* @note 会在初始化时自动注册到 UART 接收回调函数表
*/
static void STP23_Handle(void* user_uart) {
    const UART_DRIVES* uart = (UART_DRIVES*)user_uart;

    for (uint8_t stp23_index = 0; stp23_index < stp23_num; stp23_index++) {
        STP23_DRIVES *stp23 = stp23_drives[stp23_index];

        if (stp23->uart->huart != uart->huart)
            continue;

        const uint8_t Head[3] = {0x54, 0x2C, 0x00};
        if (UART_GetDataWithHLen(stp23->uart, stp23->rx_buffer, (char*)&Head[0],STP23_DATA_FRAME_LEN)) {
            CODEC codec = {0};
            Codec_Init(&codec, stp23->rx_buffer, Little_Endian);
            Codec_Decode_Skip(&codec, 16);
            stp23->temperature = Codec_Decode_Unsigned(&codec, 16);
            Codec_Decode_Skip(&codec, 16);

            float distance = 0.0f;
            for (uint8_t i = 0; i < 12; i++) {
                stp23->raw_distance[i] = Codec_Decode_Unsigned(&codec, 16);
                distance += (float)stp23->raw_distance[i] / 12.0f;
                Codec_Decode_Skip(&codec, 8);
            }

            stp23->distance = distance;
            // count++;
            // if (count == 50) {
            //     count = 0;
            //     uint8_t dis = (uint8_t)(stp23->distance/10.0f);
            //     HAL_UART_Transmit(&hlpuart1, &dis, 1, 100);
            // }

            for (uint8_t i = 0; i < stp23->callback_num; i++) {
                stp23->callbacks[i](stp23);
            }
        }
    }
}





/**
* @brief 注册 ADC 转换完成回调函数
* @param user_stp23    ADC 驱动结构体指针
* @param callback    转换完成回调函数
*/
void STP23_RegisterCallback(STP23_DRIVES* user_stp23, const STP23_Callback callback) {
    user_stp23->callbacks[user_stp23->callback_num] = callback;
    user_stp23->callback_num++;
}

/**
* @brief 将STP23的距离数据传入数组
* @param user_stp23   STP23驱动结构体指针
* @note 会在stp23初始化时自动注册到 STP23 接收回调函数表
*/
void Stp_To_Circle(void* user_stp23) {
    const STP23_DRIVES* stp23 = (STP23_DRIVES*)user_stp23;
    uint8_t id = stp23->id;
    uint16_t ini_angle = (id - 1) * 45;
    // Circle_Update(stp23->raw_distance,id,ini_angle,
    // &user_distance_control.delta_angle,user_distance_control.delta_angle_old);

    //测试用
    Circle_Update(stp23->raw_distance,id,ini_angle,
    &delta_angle_fortest,user_distance_control.delta_angle_old);
}
#endif /* HAL_UART_MODULE_ENABLED */
