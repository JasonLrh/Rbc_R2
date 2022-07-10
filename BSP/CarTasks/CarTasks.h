#ifndef CARTASKS_H
#define CARTASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "cmsis_os.h"

#define CTRL_TYPE_ANGLE 0
#define CTRL_TYPE_SPEED 1


typedef struct _motors_output_t {
    float rudder_motors[3];
    float vel_motors[3];
    uint8_t type;
} motors_output_t;

typedef struct _remote_input_t
{
    struct {
        float angle;
        float speed;
        uint8_t type;
    } move;

    struct {
        float rotate_angle;
        float expand_angle;
        float height;
    } zhua;

    struct {
        float height;
        float len;
        uint8_t pState;
        bool isSuckerOn;
    } puller;


    


}remote_input_t;


extern motors_output_t motor_values;
extern remote_input_t remote_input;

void initMotorDirectionTaskFunc(void const * argument);
void motorRoutineTaskFunc(void const * argument);
void serialCmdProcTaskFunc(void const * argument);


void userInputTaskFunc(void * argument);

#ifdef __cplusplus
}
#endif

#endif
