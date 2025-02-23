/*
 * General_Functions.c
 *
 *  Created on: Feb 11, 2025
 *      Author: sergi
 */

#include "General_Functions.h"
#include "Space_Packet_Protocol.h"
#include "PUS_1_service.h"
#include "PUS_3_service.h"
#include "PUS_8_service.h"
#include "PUS_17_service.h"
#include "cmsis_os.h"

volatile uint8_t uart_tx_OBC_done = 1;

extern uint16_t HK_SPP_APP_ID;
extern uint16_t HK_PUS_SOURCE_ID;

uint16_t SPP_SEQUENCE_COUNTER = 0;

QueueHandle_t UART_OBC_Out_Queue;

void Prepare_full_msg(SPP_header_t* resp_SPP_header,
						PUS_TM_header_t* resp_PUS_header,
						uint8_t* data,
						uint16_t data_len,
						uint8_t* OUT_full_msg,
						uint16_t* OUT_full_msg_len ) {

    uint8_t* current_pointer = OUT_full_msg;

    SPP_encode_header(resp_SPP_header, current_pointer);
    current_pointer += SPP_HEADER_LEN;

    if (resp_PUS_header != NULL) {
        PUS_encode_TM_header(resp_PUS_header, current_pointer);
        current_pointer += SPP_PUS_TM_HEADER_LEN_WO_SPARE;
    }

    if (data != NULL) {
        SPP_add_data_to_packet(data, data_len, current_pointer);
        current_pointer += data_len;
    }

    SPP_add_CRC_to_msg(OUT_full_msg, current_pointer - OUT_full_msg, current_pointer);
    current_pointer += CRC_BYTE_LEN;
    *OUT_full_msg_len = current_pointer - OUT_full_msg;
}

void Send_TM(SPP_header_t* resp_SPP_header,
				PUS_TM_header_t* resp_PUS_header,
				uint8_t* data,
				uint16_t data_len) {

    uint8_t response_TM_packet[SPP_MAX_PACKET_LEN] = {0};
    uint8_t response_TM_packet_COBS[SPP_MAX_PACKET_LEN] = {0};
    uint16_t packet_total_len = 0;

    Prepare_full_msg(resp_SPP_header,
						resp_PUS_header,
						data, data_len,
						response_TM_packet,
						&packet_total_len);

    uint16_t cobs_packet_total_len = COBS_encode(response_TM_packet,
												packet_total_len,
												response_TM_packet_COBS);

    // Wait until the previous DMA transfer has finished
    while (!uart_tx_OBC_done)
	{
		osDelay(1);
	}

    // Mark the DMA pipeline busy
    uart_tx_OBC_done = 0;

    memcpy(UART_TxBuffer, response_TM_packet_COBS, cobs_packet_total_len);
    UART_TxBuffer[cobs_packet_total_len] = 0x00; // Adding sentinel value.
    cobs_packet_total_len +=1;

	if (HAL_UART_Transmit_DMA(&DEBUG_UART, UART_TxBuffer, cobs_packet_total_len) != HAL_OK) {
		//TO DO: move system in a critical state taht would try to fix the problem
		HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED3_Pin, GPIO_PIN_SET);
		uart_tx_OBC_done = 1;  // Reset flag on failure
	}
}


void Add_SPP_PUS_and_send_TM(UART_OUT_msg* UART_OUT_msg_received) {

		if(SPP_SEQUENCE_COUNTER >= 65535)
			SPP_SEQUENCE_COUNTER = 0;
		else
			SPP_SEQUENCE_COUNTER++;

        SPP_header_t TM_SPP_header = SPP_make_header(
			SPP_VERSION,
			SPP_PACKET_TYPE_TM,
			UART_OUT_msg_received->PUS_HEADER_PRESENT,
			SPP_APP_ID,
			SPP_SEQUENCE_SEG_UNSEG,
			SPP_SEQUENCE_COUNTER,
			SPP_PUS_TM_HEADER_LEN_WO_SPARE + UART_OUT_msg_received->TM_data_len + CRC_BYTE_LEN - 1
        );

        if(UART_OUT_msg_received->PUS_HEADER_PRESENT == 1){
			PUS_TM_header_t TM_PUS_header  = PUS_make_TM_header(
				PUS_VERSION,
				0,
				UART_OUT_msg_received->SERVICE_ID,
				UART_OUT_msg_received->SUBTYPE_ID,
				0,
				UART_OUT_msg_received->PUS_SOURCE_ID,
				0
			);
			Send_TM(&TM_SPP_header, &TM_PUS_header, UART_OUT_msg_received->TM_data, UART_OUT_msg_received->TM_data_len);
        }
        else{
        	Send_TM(&TM_SPP_header, NULL, UART_OUT_msg_received->TM_data, UART_OUT_msg_received->TM_data_len);
        }
}


// Function that processes incoming TC
SPP_error Handle_incoming_TC() {

    // Decode COBS frame if valid
    if(!COBS_is_valid(UART_RxBuffer.RxBuffer, UART_RxBuffer.frame_size)){
		return UNDEFINED_ERROR;
    }
    uint8_t input_data[UART_RxBuffer.frame_size];
    COBS_decode(UART_RxBuffer.RxBuffer, UART_RxBuffer.frame_size, input_data);

    // Decode SPP header and verify its checksum
    SPP_header_t 	SPP_header;
    uint16_t  SPP_buffer_length;
    SPP_decode_header(input_data, &SPP_header);
    SPP_buffer_length = SPP_header.packet_data_length + SPP_HEADER_LEN + 1; // length = PUS + data + CRC
	if (SPP_validate_checksum(input_data, SPP_buffer_length) != SPP_OK) {
		return SPP_PACKET_CRC_MISMATCH;
	}

    // Decode PUS header if present
    if (SPP_header.secondary_header_flag) {
    	uint8_t secondary_header_buffer[PUS_TC_HEADER_LEN_WO_SPARE];
        memcpy(secondary_header_buffer, input_data + SPP_HEADER_LEN, PUS_TC_HEADER_LEN_WO_SPARE);

        PUS_TC_header_t PUS_TC_header;
        PUS_decode_TC_header(secondary_header_buffer, &PUS_TC_header);

		uint8_t* data = input_data + SPP_HEADER_LEN + PUS_TC_HEADER_LEN_WO_SPARE;

		if (PUS_TC_header.service_type_id == HOUSEKEEPING_SERVICE_ID) {
			PUS_3_handle_HK_TC(&SPP_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == FUNCTION_MANAGEMNET_ID) {
			PUS_8_handle_FM_TC(&SPP_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == TEST_SERVICE_ID) {
			PUS_17_handle_TEST_TC(&SPP_header, &PUS_TC_header);
		} else {
			PUS_1_send_fail_acc(&SPP_header, &PUS_TC_header);
			return SPP_UNHANDLED_PUS_ID;
		}
    }
    return SPP_OK;
}

