 /*
 * Space_Packet_Protocol.c
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs
 */

#include "Space_Packet_Protocol.h"
#include <stdio.h>

uint8_t Space_Packet_Data_Buffer[1024];

#define HK_TEST_LEN 172
uint8_t HK_test_packet[HK_TEST_LEN] = { // Hard coded test response
	        	0x09, 0x41, 0xC0, 0x1B, 0x00, 0xA5, 0x10, 0x03, 0x19, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x0D, 0x59,
		        0xAA, 0x00, 0x00, 0x64, 0x80, 0x60, 0x01, 0x00, 0x00, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		        0x12, 0x23, 0x34, 0x45, 0x56, 0x78, 0x67, 0x89, 0x9A, 0xBC, 0xAB, 0x03, 0x00, 0x00, 0x00, 0x00,
		        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		        0x19, 0x41, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0xF4, 0xA6, 0x00, 0x00, 0x01, 0x00, 0x1B,
		        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x05, 0x0D, 0xC5, 0x10, 0x00,
		        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x0B, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
		        0x18, 0x0C, 0x00, 0x02, 0x00, 0x00, 0x01, 0x70, 0x00, 0x00, 0x0F, 0xAF, 0x00, 0x00, 0x2E, 0xED,
		        0x53, 0xC2, 0x61, 0x40, 0x00, 0x01, 0x00, 0x20, 0x00, 0x20, 0xFF, 0xFF, 0x60, 0x14, 0xD4, 0xC0,
		        0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0x55}; 


void SPP_Callback() {
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    SPP_primary_header_t primary_header;
    SPP_decode_primary_header(Space_Packet_Data_Buffer, &primary_header);

    HAL_UART_Transmit_DMA(&huart5, Space_Packet_Data_Buffer, SPP_PRIMARY_HEADER_LEN + primary_header.packet_data_length + 1);
    //memcpy(test_arr, &ht, 1);
    //memcpy(test_arr+1, &rt, 1);
}


void SPP_send_HK_test_packet() {
    HAL_UART_Transmit_DMA(&huart4, HK_test_packet, HK_TEST_LEN);
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
        return PACKET_CRC_MISMATCH;
    } else {
        return OK;
    }
}


// Takes pointer to SPP_PRIMARY_HEADER_LEN length buffer.
// Takes pointer to SPP_primary_header_t struct.
// Fills primary_header with corresponding data from raw_buffer
// Return negative error code if failed, or 0 if correct. (SPP_error)
// CHECK SIZE OF raw_header BEFORE PASSING TO THIS FUNCTION
SPP_error SPP_decode_primary_header(uint8_t* raw_header, SPP_primary_header_t* primary_header) {
	primary_header->packet_version_number	= (raw_header[0] & 0xE0) >> 5;
	primary_header->packet_type 			= (raw_header[0] & 0x10) >> 4;
	primary_header->secondary_header_flag	= (raw_header[0] & 0x08) >> 3;
	primary_header->application_process_id	=((raw_header[0] & 0x03) << 8) | (raw_header[1]);
	primary_header->sequence_flags			= (raw_header[2] & 0xC0) >> 6;
	primary_header->packet_sequence_count	=((raw_header[2] & 0x3F) << 8) | (raw_header[3]);
	primary_header->packet_data_length		= (raw_header[4] << 8) 		   | (raw_header[5]);
    return OK;
}


// Takes filled SPP primary header struct, encodes into SPP_PRIMARY_HEADER_LEN length buffer
// and copies results do given buffer pointer
// Return negative error code if failed, or 0 if correct. (SPP_error)
// CHECK SIZE OF result_buffer BEFORE PASSING TO THIS FUNCTION
SPP_error SPP_encode_primary_header(SPP_primary_header_t* primary_header, uint8_t* result_buffer) {
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
    return OK;
}


// Returns the length of the packet data, and writes a heap pointer to "data".
SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t* data, uint16_t* ret_data_len, SPP_primary_header_t* decoded_out_header) {
    uint8_t raw_header[6] = {0, 0, 0, 0, 0, 0};
	memcpy(&raw_header, packet, SPP_PRIMARY_HEADER_LEN);

    SPP_error decode_err = SPP_decode_primary_header(raw_header, decoded_out_header);
    if (decode_err != OK) {return decode_err;}

	uint16_t data_length = decoded_out_header->packet_data_length + 1; // The docs say that the packet_data_length is the number of bytes - 1, so no idea if thus should be +1 or is there just a sentinal value.

    packet = packet + SPP_PRIMARY_HEADER_LEN;
    
    memcpy(data, packet, data_length);
    *ret_data_len = data_length;
     
    return OK;
}

SPP_error SPP_decode_secondary_header_PUS(uint8_t* raw_header, SPP_secondary_header_PUS_t* secondary_header) {
    secondary_header->PUS_version_number = (raw_header[0] & 0xF0) >> 4;
    secondary_header->ACK_flags          = (raw_header[0] & 0x0F);
    secondary_header->service_type_id    =  raw_header[1];
    secondary_header->message_subtype_id =  raw_header[2];
    secondary_header->source_id          = (raw_header[3] << 8) | raw_header[4];
    secondary_header->spare              = 0; // Based on PUS message type I guess? (Optional)
    return OK;
}

SPP_error SPP_encode_secondary_header_PUS(SPP_secondary_header_PUS_t* secondary_header, uint8_t* result_buffer) {
    for(int i = 0; i < SPP_SECONDARY_HEADER_LEN_WO_SPARE; i++) {
        result_buffer[i] ^= result_buffer[i];    // Clear result buffer.
    }

    result_buffer[0] |=  secondary_header->PUS_version_number << 4;
    result_buffer[0] |=  secondary_header->ACK_flags;
    result_buffer[1] |=  secondary_header->service_type_id;
    result_buffer[2] |=  secondary_header->message_subtype_id;
    result_buffer[3] |= (secondary_header->source_id & 0xFF00) >> 8;
    result_buffer[4] |= (secondary_header->source_id & 0x00FF) >> 8;

    return OK;
};

// Test function to check if decodeing and encoding and data seperation works correctly.
SPP_error SPP_handle_incoming_packet(uint8_t* space_packet, uint8_t* response) {
    SPP_primary_header_t primary_header;
    uint16_t* packet_data_len = 0;
    
    SPP_error data_extract_error = SPP_extract_packet_data(space_packet, Space_Packet_Data_Buffer, packet_data_len, &primary_header);
    if (data_extract_error) {
        return data_extract_error;
    }

    if (primary_header.secondary_header_flag) {
        uint8_t secondary_header_buffer[SPP_SECONDARY_HEADER_LEN_WO_SPARE];
        memcpy(secondary_header_buffer, Space_Packet_Data_Buffer, SPP_SECONDARY_HEADER_LEN_WO_SPARE);
        SPP_secondary_header_PUS_t secondary_header_PUS;
        SPP_error secondary_decode_err = SPP_decode_secondary_header_PUS(secondary_header_buffer, &secondary_header_PUS);
        if (secondary_decode_err) {
            return secondary_decode_err;
        }

        if (secondary_header_PUS.service_type_id    == HOUSEKEEPING_SERVICE_ID &&
            secondary_header_PUS.message_subtype_id == 25) {

           	uint8_t response_packet[172] = { // Hard coded test response
	        	0x09, 0x41, 0xC0, 0x1B, 0x00, 0xA5, 0x10, 0x03, 0x19, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x0D, 0x59,
		        0xAA, 0x00, 0x00, 0x64, 0x80, 0x60, 0x01, 0x00, 0x00, 0x12, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
		        0x12, 0x23, 0x34, 0x45, 0x56, 0x78, 0x67, 0x89, 0x9A, 0xBC, 0xAB, 0x03, 0x00, 0x00, 0x00, 0x00,
		        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		        0x19, 0x41, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0xF4, 0xA6, 0x00, 0x00, 0x01, 0x00, 0x1B,
		        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x05, 0x0D, 0xC5, 0x10, 0x00,
		        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x0B, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
		        0x18, 0x0C, 0x00, 0x02, 0x00, 0x00, 0x01, 0x70, 0x00, 0x00, 0x0F, 0xAF, 0x00, 0x00, 0x2E, 0xED,
		        0x53, 0xC2, 0x61, 0x40, 0x00, 0x01, 0x00, 0x20, 0x00, 0x20, 0xFF, 0xFF, 0x60, 0x14, 0xD4, 0xC0,
		        0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0x55}; 

            for(int i = 0; i < 172; i++) {
                char temp[2];
                sprintf(temp, "%02X", response_packet[i]);
                memcpy(response + 2*i, temp, 2);
            }
        }
    }
    return OK;
}



