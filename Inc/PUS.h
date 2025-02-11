/*
 * PUS.h
 *
 *  Created on: Feb 11, 2025
 *      Author: sergi
 */

#ifndef PUS_H_
#define PUS_H_


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


/* PUS */
PUS_TM_header_t PUS_make_TM_header(uint8_t PUS_version_number, uint8_t sc_time_ref_status, uint8_t service_type_id,
                                uint8_t message_subtype_id, uint16_t message_type_counter, uint16_t destination_id, uint16_t time);

SPP_error PUS_decode_TC_header(uint8_t* raw_header, PUS_TC_header_t* secondary_header);
SPP_error PUS_encode_TC_header(PUS_TC_header_t* secondary_header, uint8_t* result_buffer);
SPP_error PUS_decode_TM_header(uint8_t* raw_header, PUS_TM_header_t* secondary_header);
SPP_error PUS_encode_TM_header(PUS_TM_header_t* secondary_header, uint8_t* result_buffer);

#endif /* PUS_H_ */
