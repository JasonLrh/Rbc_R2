#ifndef F4_CAN_TEMPBOARD_H
#define F4_CAN_TEMPBOARD_H


#include "bsp_can.h"

class TemperBoard {
public:
    TemperBoard(FDCAN_HandleTypeDef *_hfdcan);
    
    bool set_angle_routate(float angle);
    bool set_angle_expand(float angle);
    bool set_height_lower(float height);
    bool set_height_higher(float height);
    bool set_pull(bool p);
    void output(void);
private:
    typedef union _temper_board_tx_msg_t{
        struct __packed{
            int8_t a_e;
            int8_t a_r;
            uint16_t pull;
            uint16_t h1;
            uint16_t h2;
        }val;
        uint8_t raw[8];
    } temper_board_tx_msg_t;

    temper_board_tx_msg_t info;


    bsp_can_device_t can_devices;
};




#endif