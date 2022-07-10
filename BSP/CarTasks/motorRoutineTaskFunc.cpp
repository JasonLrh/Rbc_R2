#include "CarTasks.h"

#include "CAN_Devices/Dji_CAN_motors.h"
#include "CAN_Devices/Odrive_CAN_motors.h"
#include "tim.h"

extern DjiMotorGroup djiMotorGroupLowerId;
// extern DjiMotorGroup djiMotorGroupHigherId;
extern Odrive_CAN_motors odrv_motors;

extern osMessageQId qMotorTimeupHandle;


motors_output_t motor_values;


static void motorTimeupCallback(TIM_HandleTypeDef * htim){
    char descript[2] = "n";
    // if (tim_queue_enable){
    //   // ST_LOGD("T");
        xQueueSendFromISR(qMotorTimeupHandle, descript, NULL);
    // }

    
}
extern volatile float angle_test;
void motorRoutineTaskFunc(void const * argument)
{
    // TODO
    char * __ptr;
    uint32_t cnt;
    // motor init
    DjiCanMotorsInit();

    motor_values.type = CTRL_TYPE_ANGLE;

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
            for (int i = 0; i < 3; i++)
            {
                djiMotorGroupLowerId.SetInput(i, motor_values.rudder_motors[i],
                    motor_values.type == CTRL_TYPE_ANGLE ? 
                    MotorPID::PENG_CTRL_TYPE_POSITION : MotorPID::PENG_CTRL_TYPE_SPEED);
                // djiMotorGroupHigherId.SetInput(i, 0, 0);
            }
            djiMotorGroupLowerId.output();
            // djiMotorGroupLowerId.output();
            // djiMotorGroupHigherId->output();

            // odrive motor cal area
            for (int i = 0; i < 3; i++){
                odrv_motors.setSpeed(i, motor_values.vel_motors[i]);
            }


            // end process
            cnt ++;
            if (cnt > 200){
                // ST_LOGI("m1 out : (%.2f,%.2f)\t(%.2f,%.2f)\t(%.2f,%.2f)",   djiMotorGroupLowerId.motor[0].angle, djiMotorGroupLowerId.motor[0].output,
                //                                                             djiMotorGroupLowerId.motor[1].angle, djiMotorGroupLowerId.motor[1].output,
                //                                                             djiMotorGroupLowerId.motor[2].angle, djiMotorGroupLowerId.motor[2].output);
                cnt = 0;
            }
        }
    }
}