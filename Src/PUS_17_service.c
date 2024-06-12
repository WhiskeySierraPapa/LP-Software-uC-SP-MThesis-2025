/*
 * PUS_17_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"



SPP_error SPP_handle_TEST_TC(SPP_primary_header_t* request_primary_header, SPP_PUS_TC_header_t* request_secondary_header) {
    if (request_primary_header == NULL || request_secondary_header == NULL) {
        return UNDEFINED_ERROR;
    }

    SPP_primary_header_t response_primary_header;
    SPP_PUS_TM_header_t response_PUS_TM_header;

    if (request_secondary_header->message_subtype_id == T_ARE_YOU_ALIVE_TEST_ID) {
        if (succ_acceptence_req(request_secondary_header)) {
            SPP_send_request_verification(request_primary_header, request_secondary_header, RV_SUCC_ACCEPTANCE_VERIFICATION_ID);
        }
        if (succ_start_req(request_secondary_header)) {
            SPP_send_request_verification(request_primary_header, request_secondary_header, RV_SUCC_START_OF_EXEC_VERIFICATION_ID);
        }

        response_primary_header = SPP_make_new_primary_header(SPP_VERSION, SPP_PACKET_TYPE_TM, request_primary_header->secondary_header_flag,
            request_primary_header->application_process_id, SPP_SEQUENCE_SEG_UNSEG, request_primary_header->packet_sequence_count,
            SPP_PUS_TM_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN - 1
        );

        if (succ_progress_req(request_secondary_header)) {
            SPP_send_request_verification(request_primary_header, request_secondary_header, RV_SUCC_PROG_OF_EXEC_VERIFICATION_ID);
        }

        // Create response PUS TM header with 17,2
        response_PUS_TM_header = SPP_make_new_PUS_TM_header(PUS_VERSION, 0, TEST_SERVICE_ID, T_ARE_YOU_ALIVE_TEST_REPORT_ID,
            0, request_secondary_header->source_id, 0
        );
    }

    SPP_send_TM(&response_primary_header, &response_PUS_TM_header, NULL, 0);

    if (succ_completion_req(request_secondary_header)) {
        SPP_send_request_verification(request_primary_header, request_secondary_header, RV_SUCC_COMPL_OF_EXEC_VERIFICATION_ID);
    }

    return SPP_OK;
}


