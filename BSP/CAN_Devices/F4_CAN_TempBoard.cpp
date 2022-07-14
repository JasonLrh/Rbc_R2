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

    memcpy(&(temperBoard.state), pRxData, 8);
    

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
    temper_board_tx_msg_t initcode = {
        .val = {
            .a_e = 0,
            .a_r = 0,
            .pull_len = 0,
            .pull_state = 0,
            .h1 = 0,
            .h2 = 0
        }
    };
    bsp_can_send_message8(&can_devices, COMMAND_PACK_ID, info.raw);
}


bool TemperBoard::set_angle_routate(float angle){
    float m = (angle / 2);
    if (m > 180.f || m < -180.f){
        ST_LOGE("Not valid input");
        return false;
    }

    info.val.a_r = (int8_t)(m);
    return true;
}

bool TemperBoard::set_angle_expand(float angle){
    float m = (angle / 2);
    if (m > 180.f || m < 0.f){
        ST_LOGE("Not valid input");
        return false;
    }

    info.val.a_e = (int8_t)(m);
    return true;
}

bool TemperBoard::set_height_lower(float height){
    if (height > H1_LIMIT || height < -0.1f){
        ST_LOGE("Not valid input: %.2f", height);
        return false;
    }

    info.val.h1 = (int16_t)(height);
    return true;
}
 
bool TemperBoard::set_height_higher(float height){
    if (height > H1_LIMIT || height < -0.1f){
        ST_LOGE("Not valid input: %.2f", height);
        return false;
    }

    info.val.h2 = (int16_t)(height);
    return true;
}

bool TemperBoard::set_sucker(bool on){
    sucker_switch = (on == true)? SUCKER_STATE_ON : SUCKER_STATE_OFF;
    // info.val.pull_state &= 0x0f;
    // info.val.pull_state |= sucker_switch;
    return true;
}

bool TemperBoard::set_puller_force(float val){
    info.val.pull_state = PULLER_STATE_TORQUE ;
    info.val.pull_len = val < 0 ? 0: val > 3200.f ? 3200 : (uint8_t)val;
    return true;

}

bool TemperBoard::set_puller_position(float len){
    info.val.pull_state = PULLER_STATE_POSITION ;
    info.val.pull_len = len < 0.f ?  0 : len > 240.f ? 240 : (uint8_t)(len);
    return true;
}

void TemperBoard::output(void){
    bsp_can_send_message8(&can_devices, COMMAND_PACK_ID, info.raw);
}