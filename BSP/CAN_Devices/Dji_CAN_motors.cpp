#include "Dji_CAN_motors.h"

/* 
********************************
defines
********************************
*/
#define get_motor_measure(ptr, data)                                   \
	{                                                                  \
		(ptr)->last_ecd = (ptr)->ecd;                                  \
		(ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);           \
		(ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);     \
		(ptr)->given_current = (uint16_t)((data)[4] << 8 | (data)[5]); \
		(ptr)->temperate = (data)[6];                                  \
	}

/* 
********************************
enum & struct
********************************
*/
typedef enum
{
	CAN_DJI_L4ALL_ID = 0x200,
	CAN_DJI_H4ALL_ID = 0x1FF,
	CAN_DJI_M1_ID = 0x201,
	CAN_DJI_M2_ID = 0x202,
	CAN_DJI_M3_ID = 0x203,
	CAN_DJI_M4_ID = 0x204,
	CAN_DJI_M5_ID = 0x205,
	CAN_DJI_M6_ID = 0x206,
	CAN_DJI_M7_ID = 0x207,
	CAN_DJI_M8_ID = 0x208,
} dji_motors_can_id_e;



/* 
********************************
c function part
********************************
*/

// rx process
CanDevice::bsp_can_rx_cb_ret_e DjiMotorGroup::rx_cb(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData){

	int32_t id = pRxHeader->Identifier - CAN_DJI_M1_ID;
	// DjiMotorGroup * gp;

	if (id > 7 || id < 0)
	{
		return BSP_CAN_RX_CB_VALUE_INVALID;
	}

	// gp = id > 3 ? &djiMotorGroupHigherId : &djiMotorGroupLowerId;

	id = id > 3 ? id - 4 : id;

	DjiMotor::motor_measure_t * state = &(motor[id].currentState);

	get_motor_measure(state, pRxData);

	if(state->ecd - state->last_ecd > 4096)
		state->circle--;
	else if(state->ecd - state->last_ecd < -4096)
		state->circle++;
	
	motor[id].pid.update_state(state->speed_rpm * 1.f, motor[id].get_angle(true));
	
	return BSP_CAN_RX_CB_VALUE_VALID;
}

/* 
********************************
c++ class: motor group part
********************************
*/

DjiMotorGroup::DjiMotorGroup(FDCAN_HandleTypeDef *_hfdcan, bool _isLowerIdentityGroup):CanDevice(_hfdcan)
{
	ID_tx = _isLowerIdentityGroup == true ? CAN_DJI_L4ALL_ID : CAN_DJI_H4ALL_ID;
}

void DjiMotorGroup::SetInput(uint8_t id, float _input, MotorPID::peng_ctrl_type_t _type){
	if (id > 3){
		return;
	}
	motor[id].update(_input, _type);
}

void DjiMotorGroup::output(void){

	int16_t val[4];
	for (int i = 0; i < 4; i++){
		val[i] = motor[i].is_update == true ? (int32_t)(motor[i].output) : 0;
		motor[i].is_update = false;
	}
	setCurrent(val);

	// static uint32_t cnt = 0;
	// cnt ++;
	// if (cnt > 500){
	// 	cnt = 0;
	// 	ST_LOGI("(%d,%d,%d)",   motor[0].currentState.ecd, 
	// 							motor[1].currentState.ecd,
	// 							motor[2].currentState.ecd);
	// 	ST_LOGI("(%d,%d,%d)",val[0]);
	// }


}
  
void DjiMotorGroup::setCurrent(int16_t val[4]){
	static uint8_t chassis_can_send_data[8];

	chassis_can_send_data[0] = val[0] >> 8;
	chassis_can_send_data[1] = val[0];
	chassis_can_send_data[2] = val[1] >> 8;
	chassis_can_send_data[3] = val[1];
	chassis_can_send_data[4] = val[2] >> 8;
	chassis_can_send_data[5] = val[2];
	chassis_can_send_data[6] = val[3] >> 8;
	chassis_can_send_data[7] = val[3];

	send_msg8(ID_tx, chassis_can_send_data);
}

void DjiMotorGroup::stop(void){
	int16_t val[4] = {0,0,0,0};
	is_force_stop = true;
	setCurrent(val);
}


/* 
********************************
c++ class: motor single part
********************************
*/

DjiMotor::DjiMotor(){
	base_angle = 0.f;
	ratio = (36 * 90 / 17);

	pid.SetTorqueLimit(0.80);
	pid.SetSpeedLimit(8000);

	// pid.dce.kp = 60.f;
	// pid.dce.ki = 1.2f;
	// pid.dce.kd = 14;
	// pid.dce.kv = 0.5f;


	pid.pPos.kp = 10.f * ratio;
	pid.pPos.ki = 0.0001f * ratio;
	pid.pPos.kd = 10.f * ratio;

	pid.pVel.kp = 10.f;
	pid.pVel.ki = 0.001f;
	pid.pVel.kd = 0.5f;

	pid.pidPos = {
		.Kp = 10.f * ratio ,
		.Ki = 0.001f * ratio,
		.Kd = 3.f * ratio
	};

	pid.pidVel = {
		.Kp = 10.f,
		.Ki = 0.01f,
		.Kd = 0.5
	};

	arm_pid_init_f32(&(pid.pidPos), 1);
	arm_pid_init_f32(&(pid.pidVel), 1);

	// pid.angle = &angle;
	// pid.velocity = &speed;

}

float DjiMotor::get_angle(bool if_cal_circle){
	float phrase = currentState.ecd * 360 / 8192.0;
	if (if_cal_circle == true){
		return (currentState.circle * 360.f + phrase) / ratio - base_angle;
	} else {
		return phrase;
	}
}

float DjiMotor::update(float _input, MotorPID::peng_ctrl_type_t _type){
	is_update = true;
	output = pid.CalPeng(_input, _type);
	// static float debug_input = -1;
	// if (debug_input != _input){
	// 	debug_input = _input;
	// 	ST_LOGD("%.1f,%.0f", debug_input, output);
	// }
	// output = pid.CalcDceOutput(_input, 0);
	return output;
}
