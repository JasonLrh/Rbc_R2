#include "MotorPID.h"
#include "tim.h"

#include <string.h>

float MotorPID::CalcDceOutput(float _inputPos, float _inputVel)
{
    float errorPos = _inputPos - angle;
    float errorVel = _inputVel - velocity;
    float deltaPos = errorPos - dce.lastError;
    dce.lastError = errorPos;
    dce.integralPos += errorPos; // + deltaPos * dce.kd
    if (dce.integralPos > DCE_INTEGRAL_LIMIT) dce.integralPos = DCE_INTEGRAL_LIMIT;
    else if (dce.integralPos < -DCE_INTEGRAL_LIMIT) dce.integralPos = -DCE_INTEGRAL_LIMIT;
    dce.integralVel += errorVel;
    if (dce.integralVel > DCE_INTEGRAL_LIMIT) dce.integralVel = DCE_INTEGRAL_LIMIT;
    else if (dce.integralVel < -DCE_INTEGRAL_LIMIT) dce.integralVel = -DCE_INTEGRAL_LIMIT;

    // if (errorPos * dce.lastError < 0.f){
    //     dce.integralPos *= 0.1;
    //     dce.integralVel *= 0.1;
    // } else {
    //     dce.integralPos *= 0.999;
    //     dce.integralVel *= 0.999;
    // }

    dce.output = dce.kp * errorPos +
                 dce.ki * dce.integralPos +
                 dce.kd * deltaPos +
                 dce.kv * dce.integralVel;

    if (dce.output > limitTorque) dce.output = limitTorque;
    else if (dce.output < -limitTorque) dce.output = -limitTorque;

    static uint32_t cnt = 0;
    cnt ++;
	if (cnt > 500){
		cnt = 0;
		ST_LOGI("(%.1f\t%.1f\t%.1f)",  angle, 
								_inputPos,
								dce.output);
	}

    return dce.output;
}

void MotorPID::update_state(float _vel, float _pos){
    angle = _pos;
    velocity = _vel;
}

static float32_t arm_pid_f32(
    arm_pid_instance_f32 *S,
    float32_t in)
{
    float32_t out;

    /* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
    out = (S->A0 * in) +
          (S->A1 * S->state[0]) + (S->A2 * S->state[1]) + (S->state[2]);

    /* Update state */
    S->state[1] = S->state[0]; // last_err err
    S->state[0] = in;          // last_err
    S->state[2] = out;

    /* return to application */
    return (out);
}

void arm_pid_init_f32(
    arm_pid_instance_f32 *S,
    int32_t resetStateFlag)
{
    /* Derived coefficient A0 */
    S->A0 = S->Kp + S->Ki + S->Kd;

    /* Derived coefficient A1 */
    S->A1 = (-S->Kp) - ((float32_t)2.0 * S->Kd);

    /* Derived coefficient A2 */
    S->A2 = S->Kd;

    /* Check whether state needs reset or not */
    if (resetStateFlag)
    {
        /* Reset state to zero, The size will be always 3 samples */
        memset(S->state, 0, 3U * sizeof(float32_t));
    }
}

void MotorPID::SetTorqueLimit(float _percent)
{
    if (_percent > 1)
        _percent = 1;
    else if (_percent < 0)
        _percent = 0;

    limitTorque = _percent * 14000; // TODO : check 3508 & 2006 max value here
}

void MotorPID::SetSpeedLimit(float speed)
{
    if (speed > 10000)
        speed = 10000;
    else if (speed < 0)
        speed = 0;

    limitSpeed = speed; // TODO : check 3508 & 2006 max value here
}

float MotorPID::CalPeng(float _input, peng_ctrl_type_t type)
{
    float posErr, pos_out, velErr, vel_out;
    // if (last_ctrl_type != type) {
    //     pPos.last_err = 0.f;
    //     pPos.integral = 0.f;
    //     pVel.last_err = 0.f;
    //     pVel.integral = 0.f;
    //     last_ctrl_type = type;
    // }

    // if (type == PENG_CTRL_TYPE_POSITION){
    //     posErr = _input - *angle;
    //     pPos.integral += posErr;
    //     pos_out =   pPos.kp * posErr +
    //                 pPos.integral * pPos.ki +
    //                 (posErr - pPos.last_err) * pPos.kd;
    //     pPos.last_err = posErr;

    //     pos_out =   pos_out > limitSpeed ? limitSpeed :
    //                 pos_out < -limitSpeed ? -limitSpeed :
    //                 pos_out;
    // } else {
    //     pos_out = _input;
    //     pos_out =   pos_out > limitSpeed ? limitSpeed :
    //                 pos_out < -limitSpeed ? -limitSpeed :
    //                 pos_out;
    // }

    // velErr = pos_out - *velocity;
    // pVel.integral += velErr;
    // vel_out =   pVel.kp * velErr +
    //             pVel.integral * pVel.ki +
    //             (velErr - pVel.last_err) * pVel.kd;
    // pVel.last_err = velErr;

    // if (vel_out > limitTorque)
    //     vel_out = limitTorque;
    // else if (vel_out < -limitTorque)
    //     vel_out = -limitTorque;

    if (type == PENG_CTRL_TYPE_POSITION)
    {
        posErr = _input - angle;
        pos_out = arm_pid_f32(&pidPos, posErr);

        pos_out = pos_out > limitSpeed ? limitSpeed : pos_out < -limitSpeed ? -limitSpeed
                                                                            : pos_out;
    }
    else
    {
        pos_out = _input;
        pos_out = pos_out > limitSpeed ? limitSpeed : pos_out < -limitSpeed ? -limitSpeed
                                                                            : pos_out;
    }

    velErr = pos_out - velocity;
    vel_out = arm_pid_f32(&pidVel, velErr);

    if (vel_out > limitTorque)
        vel_out = limitTorque;
    else if (vel_out < -limitTorque)
        vel_out = -limitTorque;

    // static uint32_t cnt = 0;
    // cnt ++;
	// if (cnt > 500){
	// 	cnt = 0;
	// 	ST_LOGI("(%.1f\t%.1f\t%.1f\t%.1f)",  angle, 
	// 							_input,
	// 							velocity, pos_out);
	// }

    return vel_out;
}
