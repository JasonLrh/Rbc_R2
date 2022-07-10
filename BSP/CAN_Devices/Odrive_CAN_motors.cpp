#include "Odrive_CAN_motors.h"
#include "bsp_can.h"
#include "string.h"


static bsp_can_rx_cb_ret_e __odrv_rx_process(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);

Odrive_CAN_motors odrv_motors(&hfdcan2);

Odrive_CAN_motors::Odrive_CAN_motors(FDCAN_HandleTypeDef *_hfdcan){
	can_odrv_devices.hfdcan = _hfdcan;
    can_odrv_devices.rx_cb = __odrv_rx_process;
    T_motor[0].odrive_set_axis.axis_node_id = (1 << 5);
    T_motor[1].odrive_set_axis.axis_node_id = (2 << 5);
    T_motor[2].odrive_set_axis.axis_node_id = (3 << 5);
    bsp_can_add_device(&can_odrv_devices);
}

typedef union _float_to_uint8_t {
	uint8_t raw[8];
	float value[2];
	uint32_t u32_data[2];
	int32_t int32_data[2];
} float_to_uint8_t;

typedef union
{
	uint8_t raw[8];
	struct __packed
	{
		uint32_t axis_error;
		uint8_t axis_current_state;
		uint8_t __reserved_1;
		uint8_t __reserved_2;
		uint8_t ctrl_status;
	} hb_info;
	struct __packed
	{
		float pos;
		float vel;
	} enc_info;
} odrv_heartbeat_data_t;

void Odrive_CAN_motors::setSpeed(uint8_t index, float speed){
	// if (T_motor[index].odrive_set_axis.input_mode != ODRV_CONTROL_CTRLMODE_VELOCITY){
	// 	T_motor[index].odrive_set_axis.control_mode = ODRV_CONTROL_INPUTMODE_VEL_RAMP;
	// 	send_msg(index, MSG_SET_CONTROLLER_MODES);
	// }
    T_motor[index].odrive_set_axis.input_vel = speed;
	send_msg(index, MSG_SET_INPUT_VEL);
}

void Odrive_CAN_motors::setTorque(uint8_t index, float torque){
	if (T_motor[index].odrive_set_axis.input_mode != ODRV_CONTROL_CTRLMODE_TORQUE){
		T_motor[index].odrive_set_axis.control_mode = ODRV_CONTROL_INPUTMODE_TORQUE_RAMP;
		send_msg(index, MSG_SET_CONTROLLER_MODES);
	}
    T_motor[index].odrive_set_axis.input_current = torque;
	send_msg(index, MSG_SET_INPUT_CURRENT);
}

// void Odrive_CAN_motors::Tmotor_set_mode(Odrive_Controller_inputMode input_mode, Odrive_Controller_ctrlMode ctrl_mode){
//     // Odrive_Controller_ctrlMode c_mode;
//     T_motor.odrive_set_axis.input_mode = (int32_t)input_mode;
//     switch(input_mode){
//         case ODRV_CONTROL_INPUTMODE_VEL_RAMP:
//             ctrl_mode = ODRV_CONTROL_CTRLMODE_VELOCITY;
//             break;
//         case ODRV_CONTROL_INPUTMODE_POS_FILTER:
//             ctrl_mode = ODRV_CONTROL_CTRLMODE_POSITION;
// 		    break;

//         case ODRV_CONTROL_INPUTMODE_TRAP_TRAJ:
//             ctrl_mode = ODRV_CONTROL_CTRLMODE_POSITION;
//             break;

//         case ODRV_CONTROL_INPUTMODE_TORQUE_RAMP:
//             ctrl_mode = ODRV_CONTROL_CTRLMODE_TORQUE;
//             break;
//         default:
//             break;
//     }
//     T_motor.odrive_set_axis.control_mode = ctrl_mode;
// }

uint8_t Odrive_CAN_motors::send_msg(uint8_t index, Odrive_Command cmd){
    float_to_uint8_t pack;
	FDCAN_TxHeaderTypeDef odrive_tx_header;
	odrive_tx_header.Identifier = (T_motor[index].odrive_set_axis.axis_node_id) | (uint32_t)cmd;
	odrive_tx_header.IdType = FDCAN_STANDARD_ID;
	odrive_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
	odrive_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
	odrive_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	odrive_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	odrive_tx_header.MessageMarker = 0;

    OdriveAxisSetState_t * odrv_set = &(T_motor[index].odrive_set_axis);
    switch(cmd) {
		case MSG_ODRIVE_ESTOP:
			/* TODO: Implement */
			break;
		case MSG_GET_MOTOR_ERROR:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_GET_ENCODER_ERROR:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_GET_SENSORLESS_ERROR:
			/* TODO: Implement */
			break;
		case MSG_SET_AXIS_NODE_ID:
			/* TODO: Implement */
			break;
		case MSG_SET_AXIS_REQUESTED_STATE:
			memcpy(pack.raw, &(odrv_set->requested_state), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_SET_AXIS_STARTUP_CONFIG:
			/* TODO: Implement */
			break;
		case MSG_GET_ENCODER_ESTIMATES:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_GET_ENCODER_COUNT:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_SET_CONTROLLER_MODES:
			pack.raw[0] = odrv_set->control_mode;
			pack.raw[4] = odrv_set->input_mode;
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_8;
			break;
		case MSG_SET_INPUT_POS:
			memcpy(pack.raw, &(odrv_set->input_pos), 4);
			pack.raw[4] = odrv_set->vel_ff & 0x00FF;
			pack.raw[5] = odrv_set->vel_ff >> 8;
			pack.raw[6] = odrv_set->current_ff & 0x00FF;
			pack.raw[7] = odrv_set->current_ff >> 8;
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_8;
			break;
		case MSG_SET_INPUT_VEL:
			pack.value[0] = odrv_set->input_vel;	// odrive_set_axis0.input_vel;
			pack.value[1] = odrv_set->torque_vel;	 // odrive_set_axis0.torque_vel;
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_8;
			// ST_LOGI("%.2f, %.2f", pack.value[0], pack.value[1]);
			break;
		case MSG_SET_INPUT_CURRENT:
			memcpy(pack.raw, &(odrv_set->input_current), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_SET_VEL_LIMIT:
			memcpy(pack.raw, &(odrv_set->vel_limit), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_START_ANTICOGGING:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_SET_TRAJ_VEL_LIMIT:
			memcpy(pack.raw, &(odrv_set->traj_vel_limit), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_SET_TRAJ_ACCEL_LIMITS:
			memcpy(pack.raw    , &(odrv_set->traj_accel_limit), 4);
			memcpy(pack.raw + 4, &(odrv_set->traj_decel_limit), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_SET_TRAJ_A_PER_CSS:
			memcpy(pack.raw, &(odrv_set->traj_a_per_css), 4);
			odrive_tx_header.TxFrameType = FDCAN_DATA_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_4;
			break;
		case MSG_GET_IQ:
			/* TODO: Implement */
			break;
		case MSG_GET_SENSORLESS_ESTIMATES:
			/* TODO: Implement */
			break;
		case MSG_RESET_ODRIVE:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_GET_VBUS_VOLTAGE:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_CLEAR_ERRORS:
			odrive_tx_header.TxFrameType = FDCAN_REMOTE_FRAME;
			odrive_tx_header.DataLength = FDCAN_DLC_BYTES_0;
			break;
		case MSG_CO_HEARTBEAT_CMD:
			/* TODO: Implement */
			break;
		default:
			break;
	}
	bsp_can_send_message(&can_odrv_devices, &odrive_tx_header, (uint8_t *)pack.raw);
    return 0;
}

static bsp_can_rx_cb_ret_e __odrv_rx_process(FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData){
	// TODO need to be overwrite
	uint8_t odrv_id   = pRxHeader->Identifier & 0b11111100000;
	uint8_t odrv_type = pRxHeader->Identifier & 0b00000011111;

	Odrive_CAN_motors::Odrive_HeartBeat_ret_msg_t * msg = (Odrive_CAN_motors::Odrive_HeartBeat_ret_msg_t *)pRxData;
	Odrive_CAN_motors::Odrive_HeartBeat_enc_est_t * est = (Odrive_CAN_motors::Odrive_HeartBeat_enc_est_t *)pRxData;
	Odrive_CAN_motors::OdriveAxisGetState_t * get = NULL;

	for (int i = 0; i < 3; i++){
		if (odrv_motors.T_motor[i].odrive_set_axis.axis_node_id == odrv_id){
			get = &(odrv_motors.T_motor[i].odrive_get_axis);
		}
	}

	if (get == NULL){
		return BSP_CAN_RX_CB_VALUE_INVALID;
	}


	switch (odrv_type)
	{
	case 0x001:
		get->axis_error = msg->axis_err;
		get->axis_current_state = msg->axis_state;
		break;
	
	case 0x009:
		get->encoder_pos_estimate = est->pos;
		get->encoder_vel_estimate = est->vel;

	default:
		return BSP_CAN_RX_CB_VALUE_INVALID;
		break;
	}

    return BSP_CAN_RX_CB_VALUE_VALID;
}