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


extern uint16_t HK_SPP_APP_ID;
extern uint16_t HK_PUS_SOURCE_ID;
extern uint8_t OBCTxBuffer[COBS_FRAME_LEN];


SPP_error UART_transmit(uint8_t* data, uint16_t data_len) {
	*(data + data_len) = 0x00; // Adding sentinel value.
	data_len++;
    HAL_UART_Transmit(&SPP_DEBUG_UART, data, data_len, 100);
    HAL_UART_Transmit(&SPP_OBC_UART, data, data_len, 100);
    return SPP_OK;
}

void Prepare_full_msg(SPP_header_t* resp_SPP_header,
						PUS_TM_header_t* resp_PUS_header,
						uint8_t* data,
						uint16_t data_len,
						uint8_t* OUT_full_msg,
						uint16_t* OUT_full_msg_len ) {

    uint8_t* current_pointer = OUT_full_msg;

    SPP_encode_header(resp_SPP_header, current_pointer);
    current_pointer += SPP_PRIMARY_HEADER_LEN;

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

SPP_error Send_TM(SPP_header_t* resp_SPP_header,
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

    memcpy(OBCTxBuffer, response_TM_packet_COBS, cobs_packet_total_len);
    UART_transmit(OBCTxBuffer, cobs_packet_total_len);
    return SPP_OK;
}


void Add_SPP_PUS_and_send_TM(uint16_t SPP_APP_ID,
								uint8_t PUS_HEADER_PRESENT,
								uint16_t SEQUENCE_COUNT,
								uint16_t PUS_SOURCE_ID,
								uint8_t SERVICE_ID,
								uint8_t SUBTYPE_ID,
								uint8_t* TM_data,
								uint16_t TM_data_len) {

        SPP_header_t TM_SPP_header = SPP_make_header(
            SPP_VERSION,
            SPP_PACKET_TYPE_TM,
			PUS_HEADER_PRESENT,
            SPP_APP_ID,
            SPP_SEQUENCE_SEG_UNSEG,
			SEQUENCE_COUNT,
            SPP_PUS_TM_HEADER_LEN_WO_SPARE + TM_data_len + CRC_BYTE_LEN - 1
        );

        if(PUS_HEADER_PRESENT == 1){
			PUS_TM_header_t TM_PUS_header  = PUS_make_TM_header(
				PUS_VERSION,
				0,
				SERVICE_ID,
				SUBTYPE_ID,
				0,
				PUS_SOURCE_ID,
				0
			);
			Send_TM(&TM_SPP_header, &TM_PUS_header, TM_data, TM_data_len);
        }
        else{
        	Send_TM(&TM_SPP_header, NULL, TM_data, TM_data_len);
        }
}


// Function that processes incoming TC
SPP_error Handle_incoming_TC(SPP_TC_source source) {
	uint8_t* 		COBS_buffer;
	size_t 			COBS_buffer_length;
	uint8_t* 		SPP_buffer;


    // Select the correct buffer based on the source
	switch (source) {
		case OBC_TC:
			COBS_buffer = OBCRxBuffer.RxBuffer;
			COBS_buffer_length = OBCRxBuffer.COBS_frame_size;
			SPP_buffer = OBC_Space_Packet_Data_Buffer;
			break;
		case DEBUG_TC:
			COBS_buffer = DEBUGRxBuffer.RxBuffer;
			COBS_buffer_length = DEBUGRxBuffer.COBS_frame_size;
			SPP_buffer = DEBUG_Space_Packet_Data_Buffer;
			break;
		default:
			return UNDEFINED_ERROR;
	}


    // Decode COBS frame if valid
    if(!COBS_is_valid(COBS_buffer, COBS_buffer_length)){
		return UNDEFINED_ERROR;
    }
    COBS_decode(COBS_buffer, COBS_FRAME_LEN, SPP_buffer);

    // Decode SPP header and verify its checksum
    SPP_header_t 	SPP_primary_header;
    SPP_decode_header(SPP_buffer, &SPP_primary_header);
    uint16_t  SPP_buffer_length = SPP_primary_header.packet_data_length + SPP_PRIMARY_HEADER_LEN + 1;
	if (SPP_validate_checksum(SPP_buffer, SPP_buffer_length) != SPP_OK) {
		return SPP_PACKET_CRC_MISMATCH;
	}

    // Decode PUS header if present
    if (SPP_primary_header.secondary_header_flag) {
    	uint8_t secondary_header_buffer[SPP_PUS_TC_HEADER_LEN_WO_SPARE];
        memcpy(secondary_header_buffer, SPP_buffer + SPP_PRIMARY_HEADER_LEN, SPP_PUS_TC_HEADER_LEN_WO_SPARE);

        PUS_TC_header_t PUS_TC_header;
        PUS_decode_TC_header(secondary_header_buffer, &PUS_TC_header);

		uint8_t* data = SPP_buffer + SPP_PRIMARY_HEADER_LEN + SPP_PUS_TC_HEADER_LEN_WO_SPARE;

		if (PUS_TC_header.service_type_id == HOUSEKEEPING_SERVICE_ID) {
			PUS_1_send_succ_acc(&SPP_primary_header, &PUS_TC_header);
			PUS_3_handle_HK_TC(&SPP_primary_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == FUNCTION_MANAGEMNET_ID) {
			PUS_1_send_succ_acc(&SPP_primary_header, &PUS_TC_header);
			SPP_handle_FM_TC(&SPP_primary_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == TEST_SERVICE_ID) {
			PUS_1_send_succ_acc(&SPP_primary_header, &PUS_TC_header);
			SPP_handle_TEST_TC(&SPP_primary_header, &PUS_TC_header);
		} else {
			PUS_1_send_fail_acc(&SPP_primary_header, &PUS_TC_header);
			return SPP_UNHANDLED_PUS_ID;
		}
    }
    return SPP_OK;
}

