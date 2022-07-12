#include "CarTasks.h"
#include "CAN_Devices/F4_CAN_TempBoard.h"




const static float __car_rotate_angle[3] = {0, 60, 120};

extern remote_input_t remote_input;

void userInputTaskFunc(void * argument){
    static uint8_t cnt = 0;

    ST_LOGI("User input task");

    for (;;){
        // remote_input.zhua.rotate_angle = def_angle;
        
        vTaskDelay(12);
        cnt ++;
        if (cnt > 500){
            cnt = 0;
        }
    }

}