/*
 * Space_Packet_Protocol.h
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#ifndef SPACE_PACKET_PROTOCOL_H_
#define SPACE_PACKET_PROTOCOL_H_

#include <Device_State.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "COBS.h"
#include "main.h"

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart2;

extern uint8_t DEBUG_Space_Packet_Data_Buffer[256];
extern uint8_t OBC_Space_Packet_Data_Buffer[1024];


typedef struct {
	uint8_t RxBuffer[MAX_COBS_FRAME_LEN];  	// Pointer to the data buffer
    uint16_t frame_size; 			// Number of valid bytes in the buffer
} UART_Rx_OBC_Msg;


// Primary header is 6 bytes. From SPP standard.
#define SPP_HEADER_LEN            6
#define PUS_TC_HEADER_LEN_WO_SPARE    5
#define SPP_PUS_TM_HEADER_LEN_WO_SPARE    9
#define CRC_BYTE_LEN                      2
#define SPP_PUS_TC_MIN_LEN  SPP_PRIMARY_HEADER_LEN + SPP_PUS_TC_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN
#define SPP_PUS_TM_MIN_LEN  SPP_PRIMARY_HEADER_LEN + SPP_PUS_TM_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN
#define PUS_VERSION                       2 // Implementation is based on PUS-C

#define SPP_VERSION                       0

#define SPP_APP_ID 						22

#define SPP_MAX_PACKET_LEN              256

#define SPP_PACKET_TYPE_TM                0
#define SPP_PACKET_TYPE_TC                1

#define SPP_SEQUENCE_SEG_CONT             0
#define SPP_SEQUENCE_SEG_FIRST            1
#define SPP_SEQUENCE_SEG_LAST             2
#define SPP_SEQUENCE_SEG_UNSEG            3

#define DEBUG_UART                  huart4
#define OBC_UART					huart2



typedef enum {
    OBC_TC                            = 0,
    DEBUG_TC                          = 1,
} SPP_TC_source;


typedef enum {
    SPP_OK                                  = 0,
    SPP_DATA_EXTRACT_MALLOC_FAILED          = -1,
    SPP_ENCODE_RESULT_BUFFER_INCORRECT_LEN  = -2,
    SPP_DECODE_INPUT_BUFFER_INCORRECT_LEN   = -3,
    SPP_PACKET_CRC_MISMATCH                 = -4,
    SPP_UNHANDLED_PUS_ID                    = -5,
    SPP_PUS3_ERROR                          = -6,
    SPP_PUS8_ERROR                          = -7,
    SPP_PUS17_ERROR                         = -8,
    SPP_MISSING_PUS_HEADER                  = -9,
    UNDEFINED_ERROR                         = -127,
} SPP_error;


// Actual bit widths added as comments. Using bitmaps and just memcpy didnt work. Idk why.
typedef struct {
	uint8_t  packet_version_number; // 3
	uint8_t  packet_type;           // 1
	uint8_t  secondary_header_flag; // 1
	uint16_t application_process_id;// 11
	uint8_t  sequence_flags;        // 2
	uint16_t packet_sequence_count; // 14
	uint16_t packet_data_length;    // 16
} SPP_header_t;


/* SPP */
SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t* data, uint16_t* ret_data_len, SPP_header_t* decoded_out_header);
SPP_error SPP_encode_header(SPP_header_t* primary_header, uint8_t* result_buffer);
SPP_error SPP_decode_header(uint8_t* input_msg, uint8_t input_msg_size, SPP_header_t* primary_header);
SPP_error SPP_add_CRC_to_msg(uint8_t* packet, uint16_t length, uint8_t* output);
SPP_error SPP_add_data_to_packet(uint8_t* data, uint16_t data_len, uint8_t* packet);
SPP_error SPP_validate_checksum(uint8_t* packet, uint16_t packet_length);

void SPP_Callback();

SPP_header_t SPP_make_header(uint8_t packet_version_number, uint8_t packet_type, uint8_t secondary_header_flag, uint16_t application_process_id, uint8_t sequence_flags, uint16_t packet_sequence_count, uint16_t packet_data_length);

SPP_error SPP_DLog(char* data);

#endif /* SPACE_PACKET_PROTOCOL_H_ */
