#include "CarTasks.h"
#include "CAN_Devices/F4_CAN_TempBoard.h"


remote_input_t remote_input = {
    .move = {
        .angle = 90.f,
        .speed = 0.f,
        .type = CTRL_TYPE_SPEED
    },
    .zhua = {
        .rotate_angle = 0.f,
        .expand_angle = 0.f,
        .height = 0.f
    },
    .puller ={
        .height = 0.f,
        .len = 0,
        .pState = 0
    }

};

extern TemperBoard temperBoard;

const static float __car_rotate_angle[3] = {0, 60, 120};

void transfer_remote_input_data(void){
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
    set_angle_routate(&temperBoard, remote_input.zhua.rotate_angle);
    set_angle_expand(&temperBoard, remote_input.zhua.expand_angle);
    set_height_lower(&temperBoard, remote_input.zhua.height);
    set_height_higher(&temperBoard, remote_input.puller.height);
    set_sucker(&temperBoard, remote_input.puller.isSuckerOn == 0 ? false : true);
    if (remote_input.puller.pState == PULLER_STATE_POSITION){
        set_puller_position(&temperBoard, remote_input.puller.len); 
    } else {
        set_puller_force(&temperBoard, remote_input.puller.len);
    }

    // TODO: check validate here

    // TODO: enable/disable fluent switch
}

void userInputTaskFunc(void * argument){

    remote_input.zhua.height = 500;
    remote_input.puller.height = 500;
    remote_input.zhua.rotate_angle = 45;

    float def_angle = 45;

    static uint8_t cnt = 0;


    TemperBoard_init(&temperBoard, &hfdcan1);

    ST_LOGI("User input task");


    for (;;){
        remote_input.zhua.rotate_angle = def_angle;
        transfer_remote_input_data();

        TemperBoard_update(&temperBoard);
        vTaskDelay(12);

        def_angle += 0.001;
        cnt ++;
        if (cnt > 500){
            ST_LOGI("h");
            cnt = 0;
        }
    }

}