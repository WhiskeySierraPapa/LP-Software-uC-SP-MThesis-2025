/*
 * Space_Packet_Protocol.h
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs
 */

#ifndef SPACE_PACKET_PROTOCOL_H_
#define SPACE_PACKET_PROTOCOL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "FPGA_UART.h"
#include "main.h"

extern UART_HandleTypeDef huart4;

typedef enum {
    OK                                  = 0,
    DATA_EXTRACT_MALLOC_FAILED          = -1,
    ENCODE_RESULT_BUFFER_INCORRECT_LEN  = -2,
    DECODE_INPUT_BUFFER_INCORRECT_LEN   = -3,
    PACKET_CRC_MISMATCH                 = -4,
} SPP_error;

typedef enum {
    HOUSEKEEPING_SERVICE_ID              = 3,
} PUS_Service_ID;

// Primary header is 6 bytes. From SPP standard.
#define SPP_PRIMARY_HEADER_LEN            6
#define SPP_SECONDARY_HEADER_LEN_WO_SPARE 5

extern uint8_t Space_Packet_Data_Buffer[1024];


typedef struct {
	uint8_t  packet_version_number;
	uint8_t  packet_type;
	uint8_t  secondary_header_flag;
	uint16_t application_process_id;
	uint8_t  sequence_flags;
	uint16_t packet_sequence_count;
	uint16_t packet_data_length;
} SPP_primary_header_t;

typedef struct {
    uint8_t  PUS_version_number;
    uint8_t  ACK_flags;
    uint8_t  service_type_id;
    uint8_t  message_subtype_id;
    uint16_t source_id;
    uint32_t spare;
} SPP_secondary_header_PUS_t;


SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t* data, uint16_t* ret_data_len, SPP_primary_header_t* decoded_out_header);
SPP_error SPP_encode_primary_header(SPP_primary_header_t* primary_header, uint8_t* result_buffer);
SPP_error SPP_decode_primary_header(uint8_t* raw_header, SPP_primary_header_t* primary_header);

SPP_error SPP_decode_secondary_header_PUS(uint8_t* raw_header, SPP_secondary_header_PUS_t* secondary_header);
SPP_error SPP_encode_secondary_header_PUS(SPP_secondary_header_PUS_t* secondary_header, uint8_t* result_buffer);

SPP_error SPP_validate_checksum(uint8_t* packet, uint16_t packet_length);

SPP_error SPP_handle_incoming_packet(uint8_t* space_packet, uint8_t* response);
void SPP_Callback();
void SPP_send_HK_test_packet();
#endif /* SPACE_PACKET_PROTOCOL_H_ */
