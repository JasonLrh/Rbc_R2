#ifndef F4_CAN_TEMPBOARD_H
#define F4_CAN_TEMPBOARD_H


#include "bsp_can.h"

#define PULLER_STATE_TORQUE 0
#define PULLER_STATE_POSITION 1

#define SUCKER_STATE_OFF (0 << 4)
#define SUCKER_STATE_ON  (1 << 4)

class TemperBoard 
{
public:
    union temper_board_tx_msg_t {
        struct __packed{
            int8_t a_e; // 180 -> 90
            int8_t a_r; // -90 
            // uint16_t pull;

            uint8_t pull_len;
            uint8_t pull_state;
            uint16_t h1;
            uint16_t h2;
        }val;
        uint8_t raw[8];
    };

    TemperBoard(FDCAN_HandleTypeDef *_hfdcan);
    
    bool set_angle_routate(float angle);
    bool set_angle_expand(float angle);
    bool set_height_lower(float height);
    bool set_height_higher(float height);
    bool set_sucker(bool on);
    bool set_puller_force(float val);
    bool set_puller_position(float len);

    void output(void);

    temper_board_tx_msg_t state;

private:
    uint32_t sucker_switch = SUCKER_STATE_OFF;
    temper_board_tx_msg_t info;

    bsp_can_device_t can_devices;
};

#endif