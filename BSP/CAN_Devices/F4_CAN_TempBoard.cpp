#include "F4_CAN_TempBoard.h"

#include <string.h>

#define COMMAND_PACK_ID 0x107
#define RETURNS_PACK_ID 0x496

#define H1_LIMIT 1700.0
#define H2_LIMIT 8000.0


TemperBoard temperBoard(&hfdcan1);

static bsp_can_rx_cb_ret_e __temper_board_rx_process(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData){
    if (pRxHeader->Identifier != RETURNS_PACK_ID){
        return BSP_CAN_RX_CB_VALUE_INVALID;
    }

    // memcpy(&(temperBoard.state), pRxData, 8);
    

    return BSP_CAN_RX_CB_VALUE_VALID;
}


TemperBoard::TemperBoard(FDCAN_HandleTypeDef *_hfdcan){
    can_devices.hfdcan = _hfdcan;
    can_devices.rx_cb = __temper_board_rx_process;
    sucker_switch = SUCKER_STATE_OFF;
    bsp_can_add_device(&can_devices);

    // for (int i = 0; i <  8; i++){
    //     info.raw[i] = 0;
    // }
}

void TemperBoard::init(void){
    static FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = COMMAND_PACK_ID;
	txHeader.IdType = FDCAN_STANDARD_ID;
	txHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	txHeader.DataLength = FDCAN_DLC_BYTES_0;
	txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	txHeader.MessageMarker = 0;
    bsp_can_send_message(&can_devices, &txHeader, NULL);
}


// bool TemperBoard::set_angle_routate(float angle){
//     float m = (angle / 2);
//     if (m > 180.f || m < -180.f){
//         ST_LOGE("Not valid input");
//         return false;
//     }

//     info.val.a_r = (int8_t)(m);
//     return true;
// }

// bool TemperBoard::set_angle_expand(float angle){
//     float m = (angle / 2);
//     if (m > 180.f || m < 0.f){
//         ST_LOGE("Not valid input");
//         return false;
//     }

//     info.val.a_e = (int8_t)(m);
//     return true;
// }

// bool TemperBoard::set_height_lower(float height){
//     if (height > H1_LIMIT || height < -0.1f){
//         ST_LOGE("Not valid input: %.2f", height);
//         return false;
//     }

//     info.val.h1 = (int16_t)(height);
//     return true;
// }
 
// bool TemperBoard::set_height_higher(float height){
//     if (height > H1_LIMIT || height < -0.1f){
//         ST_LOGE("Not valid input: %.2f", height);
//         return false;
//     }

//     info.val.h2 = (int16_t)(height);
//     return true;
// }

// bool TemperBoard::set_sucker(bool on){
//     sucker_switch = (on == true)? SUCKER_STATE_ON : SUCKER_STATE_OFF;
//     // info.val.pull_state &= 0x0f;
//     // info.val.pull_state |= sucker_switch;
//     return true;
// }

// bool TemperBoard::set_puller_force(float val){
//     info.val.pull_state = PULLER_STATE_TORQUE ;
//     info.val.pull_len = val < 0 ? 0: val > 3200.f ? 3200 : (uint8_t)val;
//     return true;

// }

// bool TemperBoard::set_puller_position(float len){
//     info.val.pull_state = PULLER_STATE_POSITION ;
//     info.val.pull_len = len < 0.f ?  0 : len > 240.f ? 240 : (uint8_t)(len);
//     return true;
// }

void TemperBoard::output(void){
    static uint8_t cnt = 0;
    bsp_can_send_message8(&can_devices, COMMAND_PACK_ID, info.raw);

    if (cnt++ > 80){
        cnt = 0;
        uint8_t * a = (uint8_t *)info.raw;
        ST_LOGD("%02x %02x %02x %02x %02x %02x %02x %02x",  a[0],
                                                            a[1],
                                                            a[2],
                                                            a[3],
                                                            a[4],
                                                            a[5],
                                                            a[6],
                                                            a[7]);

    }

}