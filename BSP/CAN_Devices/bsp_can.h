#ifndef BSP_CAN_H
#define BSP_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fdcan.h"


typedef enum {
    BSP_CAN_RX_CB_VALUE_VALID   = 0,
    BSP_CAN_RX_CB_VALUE_INVALID = 1
} bsp_can_rx_cb_ret_e;

typedef struct _bsp_can_device_t
{
    struct _bsp_can_device_t * next;
    FDCAN_HandleTypeDef * hfdcan;
    bsp_can_rx_cb_ret_e (*rx_cb)(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);
} bsp_can_device_t;

void bsp_can_add_device(bsp_can_device_t * device);
void bsp_can_delete_device(const bsp_can_device_t * device);

void bsp_can_send_message(bsp_can_device_t * dev, FDCAN_TxHeaderTypeDef * head, uint8_t *data);
void bsp_can_send_message8(bsp_can_device_t * dev, uint32_t id, uint8_t *data);

#ifdef __cplusplus 
}
#endif

#endif