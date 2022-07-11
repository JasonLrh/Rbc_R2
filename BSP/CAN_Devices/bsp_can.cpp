#include "bsp_can.h"

#define BSP_CAN_LOG_Error(format)  LOG_COLOR_E "[bsp_can.c]" LOG_RESET_COLOR ":\t" format "\n"
#define BSP_CAN_LOGE(format, ...) uart_printf(BSP_CAN_LOG_Error(format), ##__VA_ARGS__)

static CanDevice * root_dev = NULL;

static inline void __bsp_fdcan_send(FDCAN_HandleTypeDef *hfdcan, FDCAN_TxHeaderTypeDef * head, uint8_t *pTxData){
	uint32_t freeLevel;
	
	if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, head, pTxData) != HAL_OK){
		ST_LOGE("Ret Error");
		Error_Handler();
	}
	
	freeLevel = HAL_FDCAN_GetTxFifoFreeLevel(hfdcan);
	if (freeLevel == 0){
		BSP_CAN_LOGE("hfdcan%d no free fifo", hfdcan == &hfdcan1 ? 1 : 2);
		Error_Handler();
	}
}

static inline void __bsp_fdcan_send8(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *pTxData){
	static FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = id;
	txHeader.IdType = FDCAN_STANDARD_ID;
	txHeader.TxFrameType = FDCAN_DATA_FRAME;
	txHeader.DataLength = FDCAN_DLC_BYTES_8;
	txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	txHeader.MessageMarker = 0;
	
	__bsp_fdcan_send(hfdcan, &txHeader, pTxData);
}


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef rx_header;
	CanDevice * dev = root_dev;
	uint8_t __aligned(4) rx_data[8];


	while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0){
		HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data);
		dev = root_dev;
		while (dev != NULL){
			if (dev->rx_cb(&rx_header, rx_data) == CanDevice::BSP_CAN_RX_CB_VALUE_VALID){
				break;
			}
			dev = dev->next;
		}
	}

	assert_param(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) == HAL_OK);
}
/*
C++ area
*/


CanDevice::CanDevice(FDCAN_HandleTypeDef * _hfdcan){
	hfdcan = _hfdcan;

	// add to bus
	CanDevice * p = root_dev;

	this->next = NULL;

	if (root_dev == NULL){
		root_dev = this;
	} else {
		while (p->next != NULL) {
			p = p->next;
		}
		p->next = this;
	}
}

CanDevice::~CanDevice(){
	CanDevice * p = root_dev;

	if (root_dev == this){
		root_dev = root_dev->next;
	} else {
		while (p != NULL){
			if (p->next == this) {
				p = p->next->next;
			}
			p = p->next;
		}
	}
}

void CanDevice::send_msg8(uint32_t id, uint8_t *data){
	__bsp_fdcan_send8(hfdcan, id, data);
}

void CanDevice::send_msg_raw(FDCAN_TxHeaderTypeDef * head, uint8_t *data){
	__bsp_fdcan_send(hfdcan, head, data);
}