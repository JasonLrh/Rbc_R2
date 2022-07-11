#ifndef BSP_CAN_H
#define BSP_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fdcan.h"

class CanDevice {
public:
    enum bsp_can_rx_cb_ret_e {
        BSP_CAN_RX_CB_VALUE_VALID   = 0,
        BSP_CAN_RX_CB_VALUE_INVALID = 1
    };
    CanDevice(FDCAN_HandleTypeDef * _hfdcan);
    ~CanDevice();
    CanDevice * next;

    void send_msg8(uint32_t id, uint8_t *data);
    void send_msg_raw(FDCAN_TxHeaderTypeDef * head, uint8_t *data);
    virtual bsp_can_rx_cb_ret_e rx_cb(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData){
        return BSP_CAN_RX_CB_VALUE_INVALID;
    };
private:
    FDCAN_HandleTypeDef * hfdcan;
    // void add_to_bus(void);
    // void delete_from_bus(void);
};

#ifdef __cplusplus 
}
#endif

#endif