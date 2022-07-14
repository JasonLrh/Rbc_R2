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
volatile float angle_test = 0.f;
static void process_input(const char * cmd, uint16_t pos){
    float a;
    sscanf(cmd, "%f", &a);
    if (fabsf(a) < 180){
        angle_test = a;
        ST_LOGI("update angle to : %.2f", angle_test);
    }
}

extern remote_input_t remote_input;
static void process_json(const char * cmd){
    cJSON * root = cJSON_Parse(cmd);
    if (cJSON_GetErrorPtr() != NULL){
        ST_LOGE("cJSON parse error");
    }

    cJSON * subNode = cJSON_GetObjectItem(root, "m");
    // for (int i = 0; i < 2; i++){
        // cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, i) );
    // }
    remote_input.move.angle = cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, 0) );
    remote_input.move.speed = cJSON_GetNumberValue( cJSON_GetArrayItem(subNode, 1) );
    

    subNode = cJSON_GetObjectItem(root, "f");
    remote_input.zhua.height = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "h") );
    remote_input.zhua.rotate_angle = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "r") );
    remote_input.zhua.expand_angle = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "e") );

    subNode = cJSON_GetObjectItem(root, "s");
    remote_input.puller.height = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "h") );
    remote_input.puller.len = cJSON_GetNumberValue( cJSON_GetObjectItem(subNode, "x") );
    remote_input.puller.isSuckerOn = (cJSON_IsTrue(cJSON_GetObjectItem(subNode, "isSuck")) == cJSON_True)? true : false ;
    remote_input.puller.pState = 1;

    cJSON_Delete(root);
}

void serialCmdProcTaskFunc(void const * argument) {
    char *dog_cmd_buff = NULL;
    ST_LOGI("CMD server start");
    cmd_server_start(&huart8);
    for (;;) {
        dog_cmd_buff = NULL;
        if (xQueueReceive(qSerialPackHandle, &(dog_cmd_buff), portMAX_DELAY) == pdPASS) {
            uint16_t pos;
            ST_LOGI("$ %s", dog_cmd_buff);
            for (pos = 0; pos < UART_BUFF_SIZE; pos++) {
                if (dog_cmd_buff[pos] == '\0') {
                    break;
                }
            }
            if (pos > 0) {
                // ! cmd process code 
                switch (dog_cmd_buff[0])
                {
                case 'J':
                    process_json(dog_cmd_buff + 1);
                    // ST_LOGI("Height:%.2f", remote_input.zhua.height);
                    break;
                
                default:
                    ST_LOGE("Error Input");
                    break;
                }
            }
        }
    }
}