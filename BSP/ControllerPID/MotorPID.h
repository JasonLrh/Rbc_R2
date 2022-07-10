#ifndef MOTORPID_H
#define MOTORPID_H

#include <cstdint>

// #include "arm_math.h"
typedef float float32_t;

typedef struct
{
        float32_t A0;          /**< The derived gain, A0 = Kp + Ki + Kd . */
        float32_t A1;          /**< The derived gain, A1 = -Kp - 2Kd. */
        float32_t A2;          /**< The derived gain, A2 = Kd . */
        float32_t state[3];    /**< The state array of length 3. */
        float32_t Kp;          /**< The proportional gain. */
        float32_t Ki;          /**< The integral gain. */
        float32_t Kd;          /**< The derivative gain. */
} arm_pid_instance_f32;

class MotorPID
{
public:
    MotorPID()
    {}

    const float DCE_INTEGRAL_LIMIT = 4000;

    struct DCE_t
    {
        float kp;
        float kv;
        float ki;
        float kd;
        // float setPointPos;
        // float setPointVel;
        float integralPos = 0.f;
        float integralVel = 0.f;
        float lastError;
        float output;
    };
    DCE_t dce;

    struct pid_t
    {
        float kp;
        float ki;
        float kd;
        float last_err;
        float integral;
    };

    pid_t pVel, pPos;

    arm_pid_instance_f32 pidVel, pidPos;

    enum peng_ctrl_type_t {
        PENG_CTRL_TYPE_SPEED,
        PENG_CTRL_TYPE_POSITION
    };
    


    void SetTorqueLimit(float _percent);
    void SetSpeedLimit(float speed);
    float CalcDceOutput(float _inputPos, float _inputVel);
    float CalPeng(float _inputPos, peng_ctrl_type_t type);
    void update_state(float _vel, float _pos);

private:
    float limitTorque;
    float limitSpeed; 
    peng_ctrl_type_t last_ctrl_type;

    float  angle;
    float  velocity;
};

void arm_pid_init_f32(arm_pid_instance_f32 *S, int32_t resetStateFlag);


#endif
