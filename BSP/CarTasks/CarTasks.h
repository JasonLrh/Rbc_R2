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
#define CTRL_TYPE_SWEEP 2


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
        int8_t rotate_angle;
        int8_t expand_angle;
        uint16_t height;
    } zhua;

    struct {
        uint16_t height;
        uint8_t len;
        uint8_t pState;
        uint8_t isSuckerOn;
    } puller;

}remote_input_t;




void initMotorDirectionTaskFunc(void const * argument);
void motorRoutineTaskFunc(void const * argument);
void serialCmdProcTaskFunc(void const * argument);


void userInputTaskFunc(void * argument);

#ifdef __cplusplus
}
#endif

#endif
