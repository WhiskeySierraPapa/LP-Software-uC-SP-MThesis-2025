 /*
 * Space_Packet_Protocol.c
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs
 */

#include "Space_Packet_Protocol.h"
#include <stdio.h>

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
// to be freed using free()!!!
SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t** data, uint16_t* ret_data_len) {
    uint8_t raw_header[6] = {0, 0, 0, 0, 0, 0};
	memcpy(&raw_header, packet, SPP_PRIMARY_HEADER_LEN);

	SPP_primary_header_t primary_header;
    SPP_error decode_err = SPP_decode_primary_header(raw_header, &primary_header);
    if (decode_err != OK) {return decode_err;}

	uint16_t data_length = primary_header.packet_data_length + 1; // The docs say that the packet_data_length is the number of bytes - 1, so no idea if thus should be +1 or is there just a sentinal value.

    packet = packet + SPP_PRIMARY_HEADER_LEN;
    
    *data = (uint8_t *)malloc(data_length); 
    if (*data == NULL) {return DATA_EXTRACT_MALLOC_FAILED;}   // If malloc fails return -1.

    memcpy(*data, packet, data_length);
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
