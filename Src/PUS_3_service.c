/*
 * PUS_3_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "General_Functions.h"
#include "Space_Packet_Protocol.h"
#include "PUS_1_service.h"
#include "PUS_3_service.h"
#include "Device_State.h"


extern uint16_t HK_SPP_APP_ID;
extern uint16_t HK_PUS_SOURCE_ID;
extern QueueHandle_t UART_OBC_Out_Queue;

extern uint8_t UART_FPGA_Rx_Buffer[100];
extern UART_HandleTypeDef huart5;

// This queue is used to receive info from the UART handler task
QueueHandle_t PUS_3_Queue; 


TM_Err_Codes PUS_3_set_report_frequency(uint8_t* data, PUS_3_msg* pus3_msg_received) {
    uint8_t* data_iterator = data;
    uint16_t SID_num = 0;
    uint16_t SID = 0;
    memcpy(&SID_num, data_iterator, sizeof(SID_num));

    data_iterator += sizeof(SID_num);

    for (int i = 0; i < SID_num; i++) {
        if (data_iterator > data + pus3_msg_received->data_size - 2) {
            return NOT_ENOUGH_DATA_ERROR;
        }

    	SID = 0;
        memcpy(&SID, data_iterator, sizeof(SID));
        data_iterator += sizeof(SID);

        if (SID == HK_ID_ACCELEROMETER ||
            SID == HK_ID_MAGNETOMETER  ||
            SID == HK_ID_GYRO          ||
            SID == HK_ID_PRESSURE       )
        {
            switch (pus3_msg_received->new_report_frequency) {
                case 1: // 3.27 --> ONE SHOT
                {
                    uint8_t msg[64] = {0};
                    uint8_t msg_cnt = 0;

                    msg[msg_cnt++] = FPGA_MSG_PREMABLE_0;
                    msg[msg_cnt++] = FPGA_MSG_PREMABLE_1;
                    msg[msg_cnt++] = FPGA_GET_SENSOR_DATA; // F9
                    msg[msg_cnt++] = (uint8_t)(SID & 0x00FF); // HK_ID
                    msg[msg_cnt++] = FPGA_MSG_POSTAMBLE;

                    memset(UART_FPGA_Rx_Buffer, 0, sizeof(UART_FPGA_Rx_Buffer));

                    if (HAL_UART_Transmit(&huart5, msg, msg_cnt, 100) != HAL_OK) {
                        HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED3_Pin, GPIO_PIN_SET);
                        return FPGA_MESSAGE_ERROR;
                    }
                    break;
                }

                case 2: // 3.31 --> SET PERIOD
                {
                    if (data_iterator >= data + pus3_msg_received->data_size) {
                        return NOT_ENOUGH_DATA_ERROR;
                    }

                    uint8_t T = *data_iterator;
                    data_iterator++;

                    uint8_t msg[64] = {0};
                    uint8_t msg_cnt = 0;

                    msg[msg_cnt++] = FPGA_MSG_PREMABLE_0;
                    msg[msg_cnt++] = FPGA_MSG_PREMABLE_1;
                    msg[msg_cnt++] = FPGA_SET_PERIOD; // F2
                    msg[msg_cnt++] = (uint8_t)(SID & 0x00FF); // HK_ID
                    msg[msg_cnt++] = T;
                    msg[msg_cnt++] = FPGA_MSG_POSTAMBLE;

                    memset(UART_FPGA_Rx_Buffer, 0, sizeof(UART_FPGA_Rx_Buffer));

                    if (HAL_UART_Transmit(&huart5, msg, msg_cnt, 100) != HAL_OK) {
                        HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED3_Pin, GPIO_PIN_SET);
                        return FPGA_MESSAGE_ERROR;
                    }
                    break;
                }

                default:
                    return UNSUPPORTED_ARGUMENT_ERROR;
            }
        }
        else
        {
            return UNSUPPORTED_ARGUMENT_ERROR;
        }
    }

    return NO_ERROR;
}


// HK - Housekeeping PUS service 3
TM_Err_Codes PUS_3_handle_HK_TC(SPP_header_t* SPP_header , PUS_TC_header_t* PUS_TC_header, uint8_t* data, uint8_t data_size)
{
    if(data_size < 4)
	{
		return NOT_ENOUGH_DATA_ERROR;
	}
	if (Current_Global_Device_State != NORMAL_MODE) {
        return WRONG_SYSTEM_STATE_ERROR;
    }
    if (SPP_header == NULL || PUS_TC_header == NULL) {
        return NULL_POINTER_DEREFERENCING_ERROR;
    }

    // Define report frequency and handle different message subtypes
    uint8_t report_frequency = 0;

    switch (PUS_TC_header->message_subtype_id) {
        case HK_ONE_SHOT:
            report_frequency = 1;
            break;
        case HK_SET_PERIOD:
            report_frequency = 2;
            break;
        default:
            return UNSUPPORTED_SUBSERVICE_ID_ERROR;  // Invalid message subtype
    }

    PUS_1_send_succ_acc(SPP_header, PUS_TC_header);

	PUS_3_msg pus3_msg_to_send;
	pus3_msg_to_send.SPP_header = *SPP_header;
	pus3_msg_to_send.PUS_TC_header = *PUS_TC_header;
	memcpy(pus3_msg_to_send.data, data, data_size);
	pus3_msg_to_send.data_size = data_size;
	pus3_msg_to_send.new_report_frequency = report_frequency;

    if (xQueueSend(PUS_3_Queue, &pus3_msg_to_send, 0) != pdPASS) {
    	PUS_1_send_fail_comp(SPP_header, PUS_TC_header, PUS_PROCESS_BUSY_ERROR);
    }

    return NO_ERROR;
}
