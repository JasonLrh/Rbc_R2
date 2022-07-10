#include "CarTasks.h"

#include "usart.h"

extern osMessageQId qSerialPackHandle;

#define UART_BUFF_SIZE 256
static uint8_t __attribute__((section(".dma_data"))) uart_cmd_buff[2][UART_BUFF_SIZE];
static uint8_t *uart_point_buff = uart_cmd_buff[0];

void dog_cmd_rx_callback(UART_HandleTypeDef *huart, uint16_t pos)
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

void dog_cmd_start(UART_HandleTypeDef *huart)
{
    assert_param(HAL_UART_RegisterRxEventCallback(huart, dog_cmd_rx_callback) == HAL_OK);
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

void serialCmdProcTaskFunc(void const * argument)
{
    char *dog_cmd_buff = NULL;
    ST_LOGI("CMD server start");
    dog_cmd_start(&huart8);
    for (;;)
    {
        dog_cmd_buff = NULL;
        if (xQueueReceive(qSerialPackHandle, &(dog_cmd_buff), portMAX_DELAY) == pdPASS)
        {
            uint16_t pos;
            ST_LOGI("$ %s", dog_cmd_buff);
            for (pos = 0; pos < UART_BUFF_SIZE; pos++)
            {
                if (dog_cmd_buff[pos] == '\0')
                {
                    break;
                }
            }
            if (pos > 0){
                // ! cmd process code 
                process_input(dog_cmd_buff, pos);
            }
        }
    }
}