#include "CarTasks.h"

#include "CAN_Devices/Dji_CAN_motors.h"
#include "CAN_Devices/Odrive_CAN_motors.h"
#include "CAN_Devices/F4_CAN_TempBoard.h"
#include "tim.h"

extern DjiMotorGroup djiMotorGroupLowerId;
// extern DjiMotorGroup djiMotorGroupHigherId;
extern Odrive_CAN_motors odrv_motors;
extern TemperBoard temperBoard;

extern osMessageQId qMotorTimeupHandle;
// extern TemperBoard temperBoard;


motors_output_t motor_values;
remote_input_t remote_input = {
    .move = {
        .angle = 90.f,
        .speed = 0.f,
        .type = CTRL_TYPE_SPEED
    },
    .zhua = {
        .rotate_angle = 0.f,
        .expand_angle = 180.f,
        .height = 0.f
    },
    .puller ={
        .height = 0.f,
        .len = 0,
        .pState = 0
    }
};

static void transfer_remote_input_data(void){
    if (remote_input.move.type == CTRL_TYPE_SPEED){
        for (int i = 0; i < 3; i++){
            motor_values.rudder_motors[i] = remote_input.move.angle;
            motor_values.vel_motors[i] = remote_input.move.speed; 
        }
    } else {
        for (int i = 0; i < 3; i++){
            motor_values.rudder_motors[i] = 60.f * i; // TODO: check rotate angle and direction here
            motor_values.vel_motors[i] = remote_input.move.speed; 
        }
    }
    temperBoard.set_angle_routate(remote_input.zhua.rotate_angle);
    temperBoard.set_angle_expand(remote_input.zhua.expand_angle);
    temperBoard.set_height_lower(remote_input.zhua.height);
    temperBoard.set_height_higher(remote_input.puller.height);
    temperBoard.set_sucker(remote_input.puller.isSuckerOn == 0 ? false : true);
    if (remote_input.puller.pState == PULLER_STATE_POSITION){
        temperBoard.set_puller_position(remote_input.puller.len); 
    } else {
        temperBoard.set_puller_force(remote_input.puller.len);
    }

    // TODO: check validate here

    // TODO: enable/disable fluent switch
}


static void motorTimeupCallback(TIM_HandleTypeDef * htim) {
    char descript[2] = "n";
    // if (tim_queue_enable){
    //   // ST_LOGD("T");
        xQueueSendFromISR(qMotorTimeupHandle, descript, NULL);
    // }

    
}
extern volatile float angle_test;
extern uint8_t is_init_ok;

void motorRoutineTaskFunc(void const * argument) {
    char * __ptr;
    uint8_t cnt_12ms = 0;
    // motor init
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
    for (;;) {
        if (xQueueReceive(qMotorTimeupHandle, &__ptr, portMAX_DELAY) == pdTRUE)
        { // wait for timer semaphore
            // dji motor cal area
            for (int i = 0; i < 3; i++) {
                djiMotorGroupLowerId.SetInput(i, motor_values.rudder_motors[i],
                    motor_values.type == CTRL_TYPE_ANGLE ? 
                    MotorPID::PENG_CTRL_TYPE_POSITION : MotorPID::PENG_CTRL_TYPE_SPEED);
            }
            djiMotorGroupLowerId.output();

            // odrive motor cal area
            for (int i = 0; i < 3; i++){
                odrv_motors.setSpeed(i ,motor_values.vel_motors[i]);
            }

            // end process
            cnt_12ms ++;
            if (cnt_12ms >= 4){
                cnt_12ms = 0;
                if (is_init_ok == 1){
                    temperBoard.init();
                    is_init_ok = 2;
                    continue;
                } else if (is_init_ok == 2) {
                    transfer_remote_input_data();
                }
                temperBoard.output();
            }
        }
    }
}