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
#include "COBS.h"
#include "main.h"

extern UART_HandleTypeDef huart4;
extern uint8_t Space_Packet_Data_Buffer[1024];
extern uint8_t OBCRxBuffer[COBS_FRAME_LEN];
extern uint8_t OBCTxBuffer[COBS_FRAME_LEN];

extern uint8_t SPP_message_received;
extern uint8_t SPP_recv_char;
extern uint32_t SPP_recv_count;

// Primary header is 6 bytes. From SPP standard.
#define SPP_PRIMARY_HEADER_LEN            6
#define SPP_PUS_TC_HEADER_LEN_WO_SPARE    5
#define SPP_PUS_TM_HEADER_LEN_WO_SPARE    9
#define CRC_BYTE_LEN                      2
#define SPP_PUS_TC_MIN_LEN  SPP_PRIMARY_HEADER_LEN + SPP_PUS_TC_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN
#define SPP_PUS_TM_MIN_LEN  SPP_PRIMARY_HEADER_LEN + SPP_PUS_TM_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN
#define PUS_VERSION                       2 // Implementation is based on PUS-C

#define SPP_VERSION                       0

#define SPP_MAX_PACKET_LEN              256

#define SPP_PACKET_TYPE_TM                0
#define SPP_PACKET_TYPE_TC                1

#define SPP_SEQUENCE_SEG_CONT             0
#define SPP_SEQUENCE_SEG_FIRST            1
#define SPP_SEQUENCE_SEG_LAST             2
#define SPP_SEQUENCE_SEG_UNSEG            3

#define SPP_DEBUG_UART                  huart4

typedef enum {
    SPP_OK                                  = 0,
    SPP_DATA_EXTRACT_MALLOC_FAILED          = -1,
    SPP_ENCODE_RESULT_BUFFER_INCORRECT_LEN  = -2,
    SPP_DECODE_INPUT_BUFFER_INCORRECT_LEN   = -3,
    SPP_PACKET_CRC_MISMATCH                 = -4,
    SPP_INCORRECT_ID                        = -5,
    UNDEFINED_ERROR                         = -127,
} SPP_error;

typedef enum {
    REQUEST_VERIFICATION_SERVICE_ID      = 1,
    HOUSEKEEPING_SERVICE_ID              = 3,
    TEST_SERVICE_ID                      = 17,
} PUS_Service_ID;

// Request Verification service [1] subtype IDs
typedef enum { // ALL TM
    SUCC_ACCEPTANCE_VERIFICATION_ID     = 1,
    FAIL_ACCEPTANCE_VERIFICATION_ID     = 2,
    SUCC_START_OF_EXEC_VERIFICATION_ID  = 3,
    FAIL_START_OF_EXEC_VERIFICATION_ID  = 4,
    SUCC_PROG_OF_EXEC_VERIFICATION_ID   = 5,
    FAIL_PROG_OF_EXEC_VERIFICATION_ID   = 6,
    SUCC_COMPL_OF_EXEC_VERIFICATION_ID  = 7,
    FAIL_COMPL_OF_EXEC_VERIFICATION_ID  = 8,
} PUS_RV_Subtype_ID;

// Test service [17] subtype IDS
typedef enum {
    R_U_ALIVE_TEST_ID                  = 1, // TC
    R_U_ALIVE_TEST_REPORT_ID           = 2, // TM
    ON_BOARD_CONN_TEST_ID              = 3, // TC
    ON_BOARD_CONN_TEST_REPORT_ID       = 4, // TM
} PUS_T_Subtype_ID;


// Actual bit widths added as comments
typedef struct {
	uint8_t  packet_version_number; // 3
	uint8_t  packet_type;           // 1
	uint8_t  secondary_header_flag; // 1
	uint16_t application_process_id;// 11
	uint8_t  sequence_flags;        // 2
	uint16_t packet_sequence_count; // 14
	uint16_t packet_data_length;    // 16
} SPP_primary_header_t;

typedef struct {
    uint8_t  PUS_version_number;    // 4
    uint8_t  ACK_flags;             // 4
    uint8_t  service_type_id;       // 8
    uint8_t  message_subtype_id;    // 8
    uint16_t source_id;             // 16
    uint32_t spare;                 
} SPP_PUS_TC_header_t;

typedef struct {
    uint8_t  PUS_version_number;    // 4
    uint8_t  sc_time_ref_status;    // 4
    uint8_t  service_type_id;       // 8
    uint8_t  message_subtype_id;    // 8
    uint16_t message_type_counter;  // 16
    uint16_t destination_id;        // 16
    uint16_t time;                  // 16
    uint32_t spare;
} SPP_PUS_TM_header_t;

SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t* data, uint16_t* ret_data_len, SPP_primary_header_t* decoded_out_header);
SPP_error SPP_encode_primary_header(SPP_primary_header_t* primary_header, uint8_t* result_buffer);
SPP_error SPP_decode_primary_header(uint8_t* raw_header, SPP_primary_header_t* primary_header);

SPP_error SPP_decode_PUS_TC_header(uint8_t* raw_header, SPP_PUS_TC_header_t* secondary_header);
SPP_error SPP_encode_PUS_TC_header(SPP_PUS_TC_header_t* secondary_header, uint8_t* result_buffer);

SPP_error SPP_validate_checksum(uint8_t* packet, uint16_t packet_length);

SPP_error SPP_handle_incoming_TC();
void SPP_Callback();
void SPP_send_HK_test_packet();
#endif /* SPACE_PACKET_PROTOCOL_H_ */
