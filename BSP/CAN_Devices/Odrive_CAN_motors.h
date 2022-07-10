#ifndef __BSP_CAN_H
#define __BSP_CAN_H
#include "bsp_can.h"

class Odrive_CAN_motors
{
public:
    enum Odrive_ID_e
    {
        AXIS0_NODE_ID = 1 << 5,
        AXIS1_NODE_ID = 2 << 5,
        AXIS2_NODE_ID = 3 << 5,
    };

    enum Odrive_Controller_inputMode
    {
        ODRV_CONTROL_INPUTMODE_DISABLE = 0,
        ODRV_CONTROL_INPUTMODE_PASSTHROUGH = 1,
        ODRV_CONTROL_INPUTMODE_VEL_RAMP = 2,
        ODRV_CONTROL_INPUTMODE_POS_FILTER = 3,
        ODRV_CONTROL_INPUTMODE_TRAP_TRAJ = 5,
        ODRV_CONTROL_INPUTMODE_TORQUE_RAMP = 6,
    };

    enum Odrive_Controller_ctrlMode
    {
        ODRV_CONTROL_CTRLMODE_VOLTAGE = 0,
        ODRV_CONTROL_CTRLMODE_TORQUE = 1,
        ODRV_CONTROL_CTRLMODE_VELOCITY = 2,
        ODRV_CONTROL_CTRLMODE_POSITION = 3,
    };

    struct OdriveAxisGetState_t
    {
        uint32_t axis_error;
        uint32_t axis_current_state;
        uint32_t motor_error;
        uint32_t encoder_error;
        uint32_t sensorless_error;
        float encoder_pos_estimate;
        float encoder_vel_estimate;
        int32_t encoder_shadow_count;
        int32_t encoder_cpr_count;
        float iq_setpoint;
        float iq_measured;
        float sensorless_pos_estimate;
        float sensorless_vel_estimate;
        float vbus_voltage;
    };

    struct OdriveAxisSetState_t
    {
        uint16_t axis_node_id;
        uint32_t requested_state;
        int32_t control_mode;
        int32_t input_mode;
        int16_t vel_ff;
        int16_t current_ff;
        int32_t input_pos;
        float input_vel;
        float torque_vel;
        int32_t input_current;
        float vel_limit;
        float traj_vel_limit;
        float traj_accel_limit;
        float traj_decel_limit;
        float traj_a_per_css;
    };

    enum Odrive_Command
    {
        MSG_CO_NMT_CTRL = 0x000, // CANOpen NMT Message REC
        MSG_ODRIVE_HEARTBEAT,
        MSG_ODRIVE_ESTOP,
        MSG_GET_MOTOR_ERROR, // Errors
        MSG_GET_ENCODER_ERROR,
        MSG_GET_SENSORLESS_ERROR,
        MSG_SET_AXIS_NODE_ID,
        MSG_SET_AXIS_REQUESTED_STATE,
        MSG_SET_AXIS_STARTUP_CONFIG,
        MSG_GET_ENCODER_ESTIMATES,
        MSG_GET_ENCODER_COUNT,
        MSG_SET_CONTROLLER_MODES,
        MSG_SET_INPUT_POS,
        MSG_SET_INPUT_VEL = 0x00D,
        MSG_SET_INPUT_CURRENT,
        MSG_SET_VEL_LIMIT,
        MSG_START_ANTICOGGING,
        MSG_SET_TRAJ_VEL_LIMIT,
        MSG_SET_TRAJ_ACCEL_LIMITS,
        MSG_SET_TRAJ_A_PER_CSS,
        MSG_GET_IQ,
        MSG_GET_SENSORLESS_ESTIMATES,
        MSG_RESET_ODRIVE,
        MSG_GET_VBUS_VOLTAGE,
        MSG_CLEAR_ERRORS,
        MSG_CO_HEARTBEAT_CMD = 0x700, // CANOpen NMT Heartbeat  SEND
    };

    struct Odrive_motor_t
    {
        OdriveAxisSetState_t odrive_set_axis;
        OdriveAxisGetState_t odrive_get_axis;
    };

    struct __packed Odrive_HeartBeat_ret_msg_t{
        uint32_t axis_err;
        uint8_t axis_state;
        uint8_t __reserve1;
        uint8_t __reserve2;
        uint8_t controller_state;
    };

    struct __packed Odrive_HeartBeat_enc_est_t{
        float pos;
        float vel;
    };
    

    Odrive_CAN_motors(FDCAN_HandleTypeDef *_hfdcan);
    // void Tmotor_set_mode(Odrive_Controller_inputMode imode, Odrive_Controller_ctrlMode cmode);
    void setSpeed(uint8_t index, float speed);
    void setTorque(uint8_t index, float torque);

    Odrive_motor_t T_motor[3];

private:
    uint8_t send_msg(uint8_t index, Odrive_Command cmd);
    bsp_can_device_t can_odrv_devices;
};

#endif