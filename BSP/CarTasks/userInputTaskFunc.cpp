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
    temperBoard.set_angle_routate(remote_input.zhua.rotate_angle);
    temperBoard.set_angle_expand(remote_input.zhua.expand_angle);
    temperBoard.set_height_lower(remote_input.zhua.height);
    temperBoard.set_height_higher(remote_input.puller.height);
    temperBoard.set_sucker(remote_input.puller.isSuckerOn);
    if (remote_input.puller.pState == PULLER_STATE_POSITION){
        temperBoard.set_puller_position(remote_input.puller.len); 
    } else {
        temperBoard.set_puller(remote_input.puller.len == 0 ? false : true);
    }

    // TODO: check validate here

    // TODO: enable/disable fluent switch
}

void userInputTaskFunc(void * argument){

    for (;;){
        transfer_remote_input_data();
        temperBoard.output();
        vTaskDelay(12);
    }

}