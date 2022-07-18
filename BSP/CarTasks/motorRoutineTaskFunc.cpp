#include "CarTasks.h"

#include "CAN_Devices/Dji_CAN_motors.h"
#include "CAN_Devices/Odrive_CAN_motors.h"
#include "CAN_Devices/F4_CAN_TempBoard.h"
#include "Uart_Devices/relay_switches.h"
#include "tim.h"

#define R_WHEELS 63
#define D_WHEELS 650
#define __SQRT_3 1.732050f
#define PARAM_ANGLE D_WHEELS / (R_WHEELS * __SQRT_3 * 360.f)

extern DjiMotorGroup djiMotorGroupLowerId;
// extern DjiMotorGroup djiMotorGroupHigherId;
extern Odrive_CAN_motors odrv_motors;
extern TemperBoard temperBoard;

extern osMessageQId qMotorTimeupHandle;
// extern TemperBoard temperBoard;

extern volatile float angle_test;
extern volatile uint8_t is_init_ok;

static uint8_t __last_state;
static float __angle_mode_offset[3];

motors_output_t motor_values;
remote_input_t remote_input = {
    .move = {
        .angle = 0.f,
        .speed = 0.f,
        .type = CTRL_TYPE_SPEED},
    .zhua = {.rotate_angle = 0, .expand_angle = 90, .height = 0},
    .puller = {.height = 0, .len = 0, .pState = 0, .isSuckerOn = 1}};

static void transfer_remote_input_data(void)
{
    if (__last_state != remote_input.move.type)
    {
        __last_state = remote_input.move.type;
        switch (remote_input.move.type)
        {
        case CTRL_TYPE_ANGLE:
        {
            for (int i = 0; i < 3; i++)
            {
                // __angle_mode_offset[i] = odrv_motors.T_motor[i].odrive_get_axis.encoder_pos_estimate;
            }
            // ST_LOGD("%.2f\t%.2f\t%.2f", __angle_mode_offset[0], __angle_mode_offset[1], __angle_mode_offset[2]);
        }
        break;

        default:
            break;
        }
    }

    switch (remote_input.move.type)
    {
    case CTRL_TYPE_SPEED:
    {
        for (int i = 0; i < 3; i++)
        {
            motor_values.rudder_motors[i] = remote_input.move.angle;
            motor_values.vel_motors[i] = remote_input.move.speed;
        }
    }
    break;

    case CTRL_TYPE_ANGLE:
    {
        motor_values.rudder_motors[0] = 0.f;
        motor_values.rudder_motors[1] = 120.f;
        motor_values.rudder_motors[2] = -120.f;
        for (int i = 0; i < 3; i++)
        {
            motor_values.vel_motors[i] = remote_input.move.angle * PARAM_ANGLE;
        }
    }
    break;

    case CTRL_TYPE_SWEEP:
    {
        motor_values.rudder_motors[0] = 90.f + remote_input.move.angle; // todo: fix function here
        motor_values.rudder_motors[1] = 90.f;
        motor_values.rudder_motors[2] = 90.f;
        motor_values.vel_motors[0] = remote_input.move.speed;
        motor_values.vel_motors[1] = 0.f;
        motor_values.vel_motors[2] = 0.f;
    }
    break;

    default:
        break;
    }

    temperBoard.info.val.a_e = remote_input.zhua.expand_angle;
    temperBoard.info.val.a_r = remote_input.zhua.rotate_angle;
    temperBoard.info.val.h1 = remote_input.zhua.height;
    temperBoard.info.val.h2 = remote_input.puller.height;
    temperBoard.info.val.pull_len = remote_input.puller.len;
    temperBoard.info.val.pull_state = remote_input.puller.pState;

    // TODO: sucker!
    if (remote_input.puller.isSuckerOn == 0)
    {
        relay_close(RELAY_INDEX_SUCK);
    }
    else
    {
        relay_open(RELAY_INDEX_SUCK);
    }
    // temperBoard.set_sucker(remote_input.puller.isSuckerOn == 0 ? false : true);
}

static void motorTimeupCallback(TIM_HandleTypeDef *htim)
{
    char descript[2] = "n";
    // if (tim_queue_enable){
    //   // ST_LOGD("T");
    xQueueSendFromISR(qMotorTimeupHandle, descript, NULL);
    // }
}

void motorRoutineTaskFunc(void const *argument)
{
    float temp_err = 0.f;
    char *__ptr;
    uint8_t cnt_12ms = 0;
    // motor init
    __last_state = remote_input.move.type;
    transfer_remote_input_data();

    motor_values.type = CTRL_TYPE_ANGLE;

    motor_values.rudder_motors[0] = 0.f;
    motor_values.rudder_motors[1] = 0.f;
    motor_values.rudder_motors[2] = 0.f;

    motor_values.vel_motors[0] = 0.f;
    motor_values.vel_motors[1] = 0.f;
    motor_values.vel_motors[2] = 0.f;

    // start tim
    HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, motorTimeupCallback);
    HAL_TIM_Base_Start_IT(&htim6);
    for (;;)
    {
        if (xQueueReceive(qMotorTimeupHandle, &__ptr, portMAX_DELAY) == pdTRUE)
        { // wait for timer semaphore
            // dji motor cal area
            if (is_init_ok == 2)
            {
                transfer_remote_input_data();
            }
            for (int i = 0; i < 3; i++)
            {
                djiMotorGroupLowerId.SetInput(i, motor_values.rudder_motors[i], MotorPID::PENG_CTRL_TYPE_POSITION);
            }
            djiMotorGroupLowerId.output();

            // odrive motor cal area
            for (int i = 0; i < 3; i++)
            {
                odrv_motors.askPos(i);
            }

            switch (remote_input.move.type)
            {
            case CTRL_TYPE_SPEED:
            {
                for (int i = 0; i < 3; i++)
                {
                    odrv_motors.setSpeed(i, motor_values.vel_motors[i]);
                }
            }
            break;

            case CTRL_TYPE_ANGLE:
            {
                temp_err = 0;
                for (int i = 0; i < 3; i++)
                {
                    temp_err += fabsf(djiMotorGroupLowerId.motor[i].angle - motor_values.rudder_motors[i]);
                }
                if (temp_err < 5.f)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        odrv_motors.setPos(i, motor_values.vel_motors[i] + __angle_mode_offset[i]);
                    }
                }
                else {
                for (int i = 0; i < 3; i++)
                {
                    __angle_mode_offset[i] = odrv_motors.T_motor[i].odrive_get_axis.encoder_pos_estimate;
                }
                //     ST_LOGD("%.1f (%.1f,%.1f,%.1f)", temp_err, __angle_mode_offset[0], __angle_mode_offset[1], __angle_mode_offset[2]);
                }
            }
            break;

            case CTRL_TYPE_SWEEP:
            {
                for (int i = 0; i < 3; i++)
                {
                    odrv_motors.setTorque(i, motor_values.vel_motors[i]);
                }
            }
            break;

            default:
                break;
            }

            cnt_12ms++;
            if (cnt_12ms >= 4)
            {
                cnt_12ms = 0;
                if (is_init_ok == 1)
                {
                    relay_open(RELAY_INDEX_ZHUAZHUA);
                    temperBoard.init();
                    is_init_ok = 2;
                    ST_LOGD("init temperboard");
                    continue;
                }
                else if (is_init_ok == 2)
                {
                    temperBoard.output();
                }
            }
        }
    }
}