 /*
 * Space_Packet_Protocol.c
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#include "Space_Packet_Protocol.h"
#include <stdio.h>

uint8_t DEBUG_Space_Packet_Data_Buffer[256];
uint8_t OBC_Space_Packet_Data_Buffer[1024];


UART_Rx_COBS_Frame DEBUGRxBuffer;
uint8_t SPP_DEBUG_recv_char = 0xff;
uint16_t SPP_DEBUG_recv_count = 0;
uint8_t DEBUGTxBuffer[COBS_FRAME_LEN];


UART_Rx_COBS_Frame OBCRxBuffer;
uint8_t SPP_OBC_recv_char = 0xff;
uint16_t SPP_OBC_recv_count = 0;
uint8_t OBCTxBuffer[COBS_FRAME_LEN];


//uint8_t OBCRxBuffer[COBS_FRAME_LEN];
//uint8_t OBCTxBuffer[COBS_FRAME_LEN];
//uint16_t SPP_OBC_recv_count = 0;
//uint8_t SPP_OBC_recv_char = 0xff;


// NONSTATIC FOR TESTING PURPOSES
/* static */ SPP_error SPP_UART_transmit(uint8_t* data, uint16_t data_len) {
	*(data + data_len) = 0x00; // Adding sentinel value.
	data_len++;
    HAL_UART_Transmit(&SPP_DEBUG_UART, data, data_len, 100);
    HAL_UART_Transmit(&SPP_OBC_UART, data, data_len, 100);
    return SPP_OK;
}


// CRC16-CCITT
static uint16_t SPP_CRC16_byte(uint16_t crcValue, uint8_t newByte) {
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)){
			crcValue = (crcValue << 1)  ^ 0x1021;
		}else{
			crcValue = (crcValue << 1);
		}
		newByte <<= 1;
	}
  
	return crcValue;
}

static uint16_t SPP_calc_CRC16(uint8_t* data, uint16_t length) {
    uint16_t CRCvalue = 0xFFFF;
    for(uint16_t i = 0; i < length; i++) {
        CRCvalue = SPP_CRC16_byte(CRCvalue, data[i]);
    }
   
    return CRCvalue;
}


// CRC is calculated over all of the packet. The last two bytes are the received CRC.
SPP_error SPP_validate_checksum(uint8_t* packet, uint16_t packet_length) {
    uint16_t received_CRC = 0x0000;
    received_CRC |= (packet[packet_length - 2] << 8) | packet[packet_length - 1];
    uint16_t calculated_CRC = SPP_calc_CRC16(packet, packet_length - 2);
    if (received_CRC != calculated_CRC) {
        return SPP_PACKET_CRC_MISMATCH;
    } else {
        return SPP_OK;
    }
}



SPP_error SPP_decode_header(uint8_t* raw_header, SPP_header_t* primary_header) {
	primary_header->packet_version_number	= (raw_header[0] & 0xE0) >> 5;
	primary_header->packet_type 			= (raw_header[0] & 0x10) >> 4;
	primary_header->secondary_header_flag	= (raw_header[0] & 0x08) >> 3;
	primary_header->application_process_id	=((raw_header[0] & 0x03) << 8) | (raw_header[1]);
	primary_header->sequence_flags			= (raw_header[2] & 0xC0) >> 6;
	primary_header->packet_sequence_count	=((raw_header[2] & 0x3F) << 8) | (raw_header[3]);
	primary_header->packet_data_length		= (raw_header[4] << 8) 		   | (raw_header[5]);
    return SPP_OK;
}



SPP_error SPP_encode_header(SPP_header_t* primary_header, uint8_t* result_buffer) {
    for(int i = 0; i < SPP_PRIMARY_HEADER_LEN; i++) {
        result_buffer[i] ^= result_buffer[i];    // Clear result buffer.
    }

    result_buffer[0] |=  primary_header->packet_version_number  << 5;
    result_buffer[0] |=  primary_header->packet_type            << 4;
    result_buffer[0] |=  primary_header->secondary_header_flag  << 3;
    result_buffer[0] |= (primary_header->application_process_id & 0x300) >> 8;
    result_buffer[1] |=  primary_header->application_process_id & 0x0FF;
    result_buffer[2] |=  primary_header->sequence_flags         << 6;
    result_buffer[2] |= (primary_header->packet_sequence_count  & 0x3F00) >> 8;
    result_buffer[3] |=  primary_header->packet_sequence_count  & 0x00FF;
    result_buffer[4] |= (primary_header->packet_data_length     & 0xFF00) >> 8;
    result_buffer[5] |=  primary_header->packet_data_length     & 0x00FF;
    return SPP_OK;
}



SPP_header_t SPP_make_header(uint8_t packet_version_number, uint8_t packet_type, uint8_t secondary_header_flag, uint16_t application_process_id, uint8_t sequence_flags, uint16_t packet_sequence_count, uint16_t packet_data_length) {
    SPP_header_t primary_header;
    primary_header.packet_version_number    = packet_version_number;
    primary_header.packet_type              = packet_type;
    primary_header.secondary_header_flag    = secondary_header_flag;
    primary_header.application_process_id   = application_process_id;
    primary_header.sequence_flags           = sequence_flags;
    primary_header.packet_sequence_count    = packet_sequence_count;
    primary_header.packet_data_length       = packet_data_length;
    return primary_header;
}



static inline uint16_t byte_swap16(uint16_t value) {
    return ((value << 8) | (value >> 8)) & 0xFFFF;
}


static SPP_error SPP_add_CRC_to_msg(uint8_t* packet, uint16_t length, uint8_t* output) {
    uint16_t calculated_CRC = SPP_calc_CRC16(packet, length);
    uint16_t bs_CRC = byte_swap16(calculated_CRC); // (byteswapped CRC) - memcpy copies starting from LSB thus we swap.
    memcpy(output, &bs_CRC, CRC_BYTE_LEN);
    return SPP_OK;
}

static SPP_error SPP_add_data_to_packet(uint8_t* data, uint16_t data_len, uint8_t* packet) {
    memcpy(packet, data, data_len);
    return SPP_OK;
}

// TODO: Test if this works.
// This function combines the SPP, PUS headers, data and appends a calculated CRC
void SPP_prepare_full_msg(SPP_header_t* resp_SPP_header, PUS_TM_header_t* 
resp_PUS_header, uint8_t* data, uint16_t data_len, uint8_t* OUT_full_msg,
 uint16_t* OUT_full_msg_len ) {

    uint8_t* current_pointer = OUT_full_msg;
    uint16_t packet_total_len = current_pointer - OUT_full_msg;

    SPP_encode_header(resp_SPP_header, current_pointer);
    current_pointer += SPP_PRIMARY_HEADER_LEN;
    packet_total_len = current_pointer - OUT_full_msg;


    if (resp_PUS_header != NULL) {
        PUS_encode_TM_header(resp_PUS_header, current_pointer);
        current_pointer += SPP_PUS_TM_HEADER_LEN_WO_SPARE;
        packet_total_len = current_pointer - OUT_full_msg;
    }

    if (data != NULL) {
        SPP_add_data_to_packet(data, data_len, current_pointer);
        current_pointer += data_len;
        packet_total_len = current_pointer - OUT_full_msg;
    }

    SPP_add_CRC_to_msg(OUT_full_msg, packet_total_len, current_pointer);
    current_pointer += CRC_BYTE_LEN;
    packet_total_len = current_pointer - OUT_full_msg;

    *OUT_full_msg_len = packet_total_len;
}


//SPP_error SPP_send_TM(SPP_header_t* resp_SPP_header, PUS_TM_header_t* resp_PUS_header, uint8_t* data, uint16_t data_len) {
//    uint8_t response_TM_packet[SPP_MAX_PACKET_LEN];
//    uint8_t response_TM_packet_COBS[SPP_MAX_PACKET_LEN];
//    for(int i = 0; i < SPP_MAX_PACKET_LEN; i++) {
//        response_TM_packet[i] = 0x00;
//        response_TM_packet_COBS[i] = 0x00;
//    }
//    uint16_t packet_total_len = 0;
//
//    SPP_prepare_full_msg(resp_SPP_header, resp_PUS_header, data, data_len, response_TM_packet, &packet_total_len);
//
//    uint16_t cobs_packet_total_len = COBS_encode(response_TM_packet, packet_total_len, response_TM_packet_COBS);
//
//    memcpy(OBCTxBuffer, response_TM_packet_COBS, cobs_packet_total_len);
//    SPP_UART_transmit(OBCTxBuffer, cobs_packet_total_len);
//    return SPP_OK;
//}



// Function that processes incoming TC
SPP_error SPP_handle_incoming_TC(SPP_TC_source source) {
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
			send_succ_acc(&SPP_primary_header, &PUS_TC_header);
//			uint8_t data_aux = 23;
//			HAL_UART_Transmit(&SPP_OBC_UART, &data_aux, 1, 100);
			PUS_3_handle_HK_TC(&SPP_primary_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == FUNCTION_MANAGEMNET_ID) {
//			send_succ_acc(&SPP_primary_header, &PUS_TC_header);
			SPP_handle_FM_TC(&SPP_primary_header, &PUS_TC_header, data);
		}
		else if (PUS_TC_header.service_type_id == TEST_SERVICE_ID) {
//			send_succ_acc(&SPP_primary_header, &PUS_TC_header);
			SPP_handle_TEST_TC(&SPP_primary_header, &PUS_TC_header);
		} else {
			send_fail_acc(&SPP_primary_header, &PUS_TC_header);
			return SPP_UNHANDLED_PUS_ID;
		}
    }
    return SPP_OK;
}



