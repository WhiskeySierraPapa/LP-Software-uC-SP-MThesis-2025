/*
 * Space_Packet_Protocol.h
 *
 *  Created on: 2024. gada 23. apr.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#ifndef SPACE_PACKET_PROTOCOL_H_
#define SPACE_PACKET_PROTOCOL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "FPGA_UART.h"
#include "COBS.h"
#include "main.h"
#include "device_state.h"

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart2;

extern uint8_t DEBUG_Space_Packet_Data_Buffer[256];
extern uint8_t OBC_Space_Packet_Data_Buffer[1024];

extern uint8_t DEBUGRxBuffer[COBS_FRAME_LEN];
extern uint8_t DEBUGTxBuffer[COBS_FRAME_LEN];
extern uint16_t SPP_DEBUG_recv_count;
extern uint8_t SPP_DEBUG_recv_char;

extern uint8_t OBCRxBuffer[COBS_FRAME_LEN];
extern uint8_t OBCTxBuffer[COBS_FRAME_LEN];
extern uint16_t SPP_OBC_recv_count;
extern uint8_t SPP_OBC_recv_char;


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
#define SPP_OBC_UART					huart2

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

typedef enum {
    REQUEST_VERIFICATION_SERVICE_ID      = 1,
    HOUSEKEEPING_SERVICE_ID              = 3,
    FUNCTION_MANAGEMNET_ID               = 8,
    TEST_SERVICE_ID                      = 17,
} PUS_Service_ID;

// Request Verification service [1] subtype IDs
typedef enum { // ALL TM
    RV_SUCC_ACCEPTANCE_VERIFICATION_ID     = 1,
    RV_FAIL_ACCEPTANCE_VERIFICATION_ID     = 2,
    RV_SUCC_START_OF_EXEC_VERIFICATION_ID  = 3,
    RV_FAIL_START_OF_EXEC_VERIFICATION_ID  = 4,
    RV_SUCC_PROG_OF_EXEC_VERIFICATION_ID   = 5,
    RV_FAIL_PROG_OF_EXEC_VERIFICATION_ID   = 6,
    RV_SUCC_COMPL_OF_EXEC_VERIFICATION_ID  = 7,
    RV_FAIL_COMPL_OF_EXEC_VERIFICATION_ID  = 8,
} PUS_RV_Subtype_ID;



// House Keeping service [3] subtype IDs
typedef enum {
    HK_CREATE_HK_PAR_REPORT_STRUCT         = 1,  // TC
    HK_DELETE_HK_PAR_REPORT_STRUCT         = 3,  // TC
    HK_EN_PERIODIC_REPORTS                 = 5,  // TC
    HK_DIS_PERIODIC_REPORTS                = 6,  // TC
    HK_REPORT_HK_PAR_REPORT_STRUCT         = 9,  // TC
    HK_REPORT_HK_PAR_REPORT_STRUCT_REPORT  = 10, // TM (response to 9)
    HK_PARAMETER_REPORT                    = 25, // TM
    HK_ONE_SHOT                            = 27, // TC
} PUS_HK_Subtype_ID;


// Function Management [8] subtype IDs
typedef enum {
    FM_PERFORM_FUNCTION                    = 1,  // TC
} PUS_FM_Subtype_ID;


// Test (Ping) service [17] subtype IDS
typedef enum {
    T_ARE_YOU_ALIVE_TEST_ID              = 1, // TC
    T_ARE_YOU_ALIVE_TEST_REPORT_ID       = 2, // TM
    T_ON_BOARD_CONN_TEST_ID              = 3, // TC
    T_ON_BOARD_CONN_TEST_REPORT_ID       = 4, // TM
} PUS_T_Subtype_ID;


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

typedef struct {
    uint8_t  PUS_version_number;    // 4
    uint8_t  ACK_flags;             // 4
    uint8_t  service_type_id;       // 8
    uint8_t  message_subtype_id;    // 8
    uint16_t source_id;             // 16
    uint32_t spare;                 
} PUS_TC_header_t;

typedef struct {
    uint8_t  PUS_version_number;    // 4
    uint8_t  sc_time_ref_status;    // 4
    uint8_t  service_type_id;       // 8
    uint8_t  message_subtype_id;    // 8
    uint16_t message_type_counter;  // 16
    uint16_t destination_id;        // 16
    uint16_t time;                  // 16
    uint32_t spare;
} PUS_TM_header_t;

/* SPP */
SPP_error SPP_extract_packet_data(uint8_t* packet, uint8_t* data, uint16_t* ret_data_len, SPP_header_t* decoded_out_header);
SPP_error SPP_encode_header(SPP_header_t* primary_header, uint8_t* result_buffer);
SPP_error SPP_decode_header(uint8_t* raw_header, SPP_header_t* primary_header);

SPP_error SPP_validate_checksum(uint8_t* packet, uint16_t packet_length);

SPP_error SPP_handle_incoming_TC(SPP_TC_source);
void SPP_Callback();

SPP_header_t SPP_make_header(uint8_t packet_version_number, uint8_t packet_type, uint8_t secondary_header_flag, uint16_t application_process_id, uint8_t sequence_flags, uint16_t packet_sequence_count, uint16_t packet_data_length);

SPP_error SPP_send_TM(SPP_header_t* resp_SPP_header, PUS_TM_header_t* response_secondary_header, uint8_t* data, uint16_t data_len);

SPP_error SPP_DLog(char* data);

/* PUS */
PUS_TM_header_t PUS_make_TM_header(uint8_t PUS_version_number, uint8_t sc_time_ref_status, uint8_t service_type_id,
                                uint8_t message_subtype_id, uint16_t message_type_counter, uint16_t destination_id, uint16_t time);

SPP_error PUS_decode_TC_header(uint8_t* raw_header, PUS_TC_header_t* secondary_header);
SPP_error PUS_encode_TC_header(PUS_TC_header_t* secondary_header, uint8_t* result_buffer);
SPP_error PUS_decode_TM_header(uint8_t* raw_header, PUS_TM_header_t* secondary_header);
SPP_error PUS_encode_TM_header(PUS_TM_header_t* secondary_header, uint8_t* result_buffer);


/* PUS_1_service */
void send_succ_acc  (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_fail_acc  (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_succ_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_fail_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_succ_prog (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_fail_prog (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_succ_comp (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void send_fail_comp (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);



/* PUS_3_service */
SPP_error SPP_handle_HK_TC(SPP_header_t* primary_header, PUS_TC_header_t* secondary_header, uint8_t* data);
void SPP_collect_HK_data(uint32_t current_ticks);
void SPP_periodic_HK_send();

/* PUS_8_service */
SPP_error SPP_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* secondary_header, uint8_t* data);

/* PUS_17_service */
SPP_error SPP_handle_TEST_TC(SPP_header_t* req_SPP_header, PUS_TC_header_t* req_PUS_header);




SPP_error SPP_UART_transmit_DMA(uint8_t* data, uint16_t data_len);
#endif /* SPACE_PACKET_PROTOCOL_H_ */
