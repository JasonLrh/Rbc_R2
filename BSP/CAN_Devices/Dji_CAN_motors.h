#ifndef DJI_CAN_MOTORS_H
#define DJI_CAN_MOTORS_H

#include "ControllerPID/MotorPID.h"
#include "bsp_can.h"

class DjiMotor {
public:
    DjiMotor();

    struct motor_measure_t{
        uint16_t ecd;
        int16_t speed_rpm;
        int16_t given_current;
        uint8_t temperate;
        uint16_t last_ecd;
        int16_t circle;
    };

    motor_measure_t currentState = {
        .circle = 0,
    };

    float get_angle(bool if_cal_circle);

    float update(float _input, MotorPID::peng_ctrl_type_t _type);

    float angle;
    float speed;

    float output;

    float ratio;
    float base_angle;


    bool is_update = false;
    MotorPID pid;
private:
    


};



class DjiMotorGroup {
public:
    DjiMotorGroup(FDCAN_HandleTypeDef * _hfdcan, bool _isLowerIdentityGroup);
    

    void SetInput(uint8_t id, float _input, MotorPID::peng_ctrl_type_t _type);
    void output(void);
    void stop(void);
    
    DjiMotor motor[4];

private:
    uint32_t ID_tx;
    bool is_force_stop = false;
    void setCurrent(int16_t val[4]); 
    bsp_can_device_t can_devices;
};


#endif