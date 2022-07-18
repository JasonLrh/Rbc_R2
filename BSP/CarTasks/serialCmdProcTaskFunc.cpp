#include "CarTasks.h"

#include "usart.h"
#include "cJSON.h"

extern osMessageQId qSerialPackHandle;

#define UART_BUFF_SIZE 256
static uint8_t __attribute__((section(".dma_data"))) uart_cmd_buff[2][UART_BUFF_SIZE];
static uint8_t *uart_point_buff = uart_cmd_buff[0];

void cmd_server_rx_callback(UART_HandleTypeDef *huart, uint16_t pos)
{
    // ! clean DCache to sync data space
    // SCB_CleanDCache_by_Addr((uint32_t *)uart_point_buff + pos, 1);
    // SCB_InvalidateDCache_by_Addr((uint32_t *)uart_point_buff, pos);
    // SCB_InvalidateDCache_by_Addr(uart_point_buff, (pos / 32 + 1) * 32 );
    uart_point_buff[pos] = '\0';
    // SCB_CleanDCache_by_Addr((uint32_t *)(uart_point_buff + pos), 1);
    xQueueSendFromISR(qSerialPackHandle, (void *)&uart_point_buff, NULL);
    // ! memory management
    if (uart_point_buff == uart_cmd_buff[0])
    {
        uart_point_buff = uart_cmd_buff[1];
    }
    else
    {
        uart_point_buff = uart_cmd_buff[0];
    }
    HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE);
}

void cmd_server_error_callback(UART_HandleTypeDef *huart){
    ST_LOGE("Uart(%d) Error: %8x", huart->Instance == UART8 ? 8 : -1, huart->ErrorCode);
    Error_Handler();
}

void cmd_server_start(UART_HandleTypeDef *huart)
{
    assert_param(HAL_UART_RegisterRxEventCallback(huart, cmd_server_rx_callback) == HAL_OK);
    assert_param(HAL_UART_RegisterCallback(huart, HAL_UART_ERROR_CB_ID, cmd_server_error_callback) == HAL_OK);
    assert_param(HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE) == HAL_OK);
}

// volatile float angle_test = 0.f;
// static void process_input(const char * cmd, uint16_t pos){
//     float a;
//     sscanf(cmd, "%f", &a);
//     if (fabsf(a) < 180){
//         angle_test = a;
//         ST_LOGI("update angle to : %.2f", angle_test);
//     }
// }

remote_input_t serial_input;
static void process_json(const char * cmd){
    cJSON * root = cJSON_Parse(cmd);
    if (cJSON_GetErrorPtr() != NULL){
        ST_LOGE("cJSON parse error");
    }

    cJSON * subNode = cJSON_GetObjectItem(root, "m");
    // for (int i = 0; i < 2; i++){
        // cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, i) );
    // }
    serial_input.move.angle = cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, 0) );
    serial_input.move.speed = cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, 1) );
    serial_input.move.type  = (int)cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, 2) );
    

    subNode = cJSON_GetObjectItem(root, "f");
    serial_input.zhua.height = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "h") );
    serial_input.zhua.rotate_angle = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "r") );
    serial_input.zhua.expand_angle = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "e") );

    subNode = cJSON_GetObjectItem(root, "s");
    serial_input.puller.height = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "h") );
    serial_input.puller.len = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "x") );
    serial_input.puller.isSuckerOn = (cJSON_IsTrue(cJSON_GetObjectItem(subNode, "isSuck")) == cJSON_True)? true : false ;
    serial_input.puller.pState = 1;

    cJSON_Delete(root);
}


struct __packed binary_data_arch_t{
    char __prechar;
    uint8_t mvType; 
    // 2
    int8_t zhuaA_e;
    int8_t zhuaA_r;

    // 4

    float mvAngle;
    float mvSpeed;
    // 12

    uint16_t zhuaH1;
    uint16_t zhuaH2;
    // 16

    uint8_t pullLen;
    uint8_t pullState;

    // 18
    char __end_pack;
};
// 19


static void process_binary(const char * bin){
    binary_data_arch_t * ptr = (binary_data_arch_t *)bin;
    if (ptr->__end_pack == '\n') {
        serial_input.move.angle = ptr->mvAngle;
        serial_input.move.speed = ptr->mvSpeed;
        serial_input.move.type  = ptr->mvType;
        
        serial_input.zhua.height = ptr->zhuaH1;
        serial_input.zhua.rotate_angle = ptr->zhuaA_r;
        serial_input.zhua.expand_angle = ptr->zhuaA_e;

        serial_input.puller.height = ptr->zhuaH2;
        serial_input.puller.len = ptr->pullLen;
        serial_input.puller.isSuckerOn = (ptr->pullState) >> 4;
        serial_input.puller.pState = (ptr->pullState) & 0x0f;
    } else {
        ST_LOGE("invalid binary pack");
    }
}

void serialCmdProcTaskFunc(void const * argument) {
    char *dog_cmd_buff = NULL;
    ST_LOGI("CMD server start");
    cmd_server_start(&huart8);
    for (;;) {
        dog_cmd_buff = NULL;
        if (xQueueReceive(qSerialPackHandle, &(dog_cmd_buff), portMAX_DELAY) == pdPASS) {
            uint16_t pos;
            // ST_LOGI("$ %s", dog_cmd_buff);
            // ST_LOGI("$ <");
            for (pos = 0; pos < UART_BUFF_SIZE; pos++) {
                if (dog_cmd_buff[pos] == '\0') {
                    break;
                }
            }
            if (pos > 0) {
                // ! cmd process code 
                switch (dog_cmd_buff[0])
                {
                case 'J':{
                    process_json(dog_cmd_buff + 1);
                    // ST_LOGI("Height:%.2f", serial_input.zhua.height);
                } break;

                case 'B':{
                    // if (pos >= 18){
                        process_binary(dog_cmd_buff);
                    // } else {
                        // ST_LOGE("E");
                    // }
                } break;
                
                default:
                    ST_LOGE("Error Input");
                    break;
                }
            }
        }
    }
}