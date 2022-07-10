#include "CarTasks.h"
#include "CAN_Devices/bsp_can.h"
#include "CAN_Devices/Dji_CAN_motors.h"
#include "CAN_Devices/Odrive_CAN_motors.h"
#include "main.h"

GPIO_TypeDef *HALL_Port[3]= {GPIOA,
                             GPIOA,
                             GPIOA
                            };      
enum _init_flags{
    HALL1_OK = 1 << 2ul,
    HALL2_OK = 1 << 1ul,
    HALL3_OK = 1 << 0ul,
}init_flags;
#define LOW_LEVEL 0
#define HALL_OK 1
#define ALL_OK 0b111

uint16_t HALL_PIN[3] = {HALL_INPUT_1_Pin, 
                        HALL_INPUT_2_Pin, 
                        HALL_INPUT_3_Pin
                        };
int init_status[3] = {1, 1, 1};

extern DjiMotorGroup  djiMotorGroupLowerId;

EventGroupHandle_t init_event_handle = xEventGroupCreate();
float offset[3] = {28.f, -143.f, 145.f};


void initMotorDirectionTaskFunc(void const * argument){
    EventBits_t ret_val;
    float base_angle = 0.0;
    for(int i = 0; i < 3; i++){
        motor_values.rudder_motors[i] = 0.0;
    }
    motor_values.type = CTRL_TYPE_ANGLE;

    ST_LOGI("init function start");

    for(;;){
        // for(int i = 0; i < 3; i++){
        //     init_status[i] = HAL_GPIO_ReadPin(HALL_Port[i], HALL_PIN[i]);
        //     if(init_status[i] == LOW_LEVEL){
        //         xEventGroupSetBits(init_event_handle, HALL_OK << i);
        //         // motor_values.rudder_motors[i] = 0; 
        //         djiMotorGroupLowerId.motor[i].base_angle = djiMotorGroupLowerId.motor[i].angle + offset[i];     
        //     }
        // }
        // ret_val = xEventGroupWaitBits(init_event_handle, HALL1_OK|HALL2_OK|HALL3_OK, pdFAIL, pdTRUE, 1000);
        // if(ret_val & ALL_OK){
        //     for(int i = 0; i < 3; i++){
        //         // motor_values.rudder_motors[i] = djiMotorGroupLowerId.motor[i].base_angle + 90;
        //         djiMotorGroupLowerId.SetInput(i, motor_values.rudder_motors[i], MotorPID::PENG_CTRL_TYPE_POSITION);
        //     }
        //     //TODO: angle control
        //     break;
        // }
        for(int i = 0; i < 3; i++){
            motor_values.rudder_motors[i] = base_angle;
        }
        motor_values.type = CTRL_TYPE_ANGLE;
        osDelay(5000);
        base_angle += 10.f;
        // ST_LOGI("init angle : %.1f", base_angle);
    }

    xTaskCreate(userInputTaskFunc, "UserInput", 1024, NULL, tskIDLE_PRIORITY, NULL);
    vTaskDelete(NULL);
}