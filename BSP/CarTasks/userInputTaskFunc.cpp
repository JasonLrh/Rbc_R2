#include "CarTasks.h"
#include "CAN_Devices/Odrive_CAN_motors.h"

#include "string.h"

const static float __car_rotate_angle[3] = {0, 60, 120};

extern remote_input_t remote_input;
extern remote_input_t serial_input;
extern Odrive_CAN_motors odrv_motors;

void userInputTaskFunc(void * argument){
    static uint8_t cnt = 0;

    ST_LOGI("User input task");

    memcpy(&serial_input, &remote_input, sizeof(remote_input_t));

    for (;;){
        memcpy(&remote_input, &serial_input, sizeof(remote_input_t));
        vTaskDelay(48);
        cnt ++;
        if (cnt > 20){
            cnt = 0;
            // uint8_t * a = (uint8_t *)&serial_input;
            // ST_LOGD("%02x %02x %02x %02x %02x %02x %02x %02x",  a[0],
            //                                                     a[1],
            //                                                     a[2],
            //                                                     a[3],
            //                                                     a[4],
            //                                                     a[5],
            //                                                     a[6],
            //                                                     a[7]);
        }
    }

}