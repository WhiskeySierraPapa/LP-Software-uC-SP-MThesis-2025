/*
 * PUS_17_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"



SPP_error SPP_handle_TEST_TC(SPP_header_t* req_SPP_header, PUS_TC_header_t* req_PUS_header) {
    if (Current_Global_Device_State != NORMAL_MODE) {
        return UNDEFINED_ERROR;
    }
    if (req_SPP_header == NULL || req_PUS_header == NULL) {
        return UNDEFINED_ERROR;
    }

    SPP_header_t resp_SPP_header;
    PUS_TM_header_t resp_PUS_TM_header;

    if (req_PUS_header->message_subtype_id == T_ARE_YOU_ALIVE_TEST_ID) {
        send_succ_acc(req_SPP_header, req_PUS_header);
        
        send_succ_start(req_SPP_header, req_PUS_header);

        resp_SPP_header = SPP_make_header(
            SPP_VERSION,
            SPP_PACKET_TYPE_TM,
            req_SPP_header->secondary_header_flag,
            req_SPP_header->application_process_id,
            SPP_SEQUENCE_SEG_UNSEG,
            req_SPP_header->packet_sequence_count,
            SPP_PUS_TM_HEADER_LEN_WO_SPARE + CRC_BYTE_LEN - 1
        );

        send_succ_prog(req_SPP_header, req_PUS_header);

        // Create response PUS TM header with 17,2
        resp_PUS_TM_header = PUS_make_TM_header(
            PUS_VERSION,
            0,
            TEST_SERVICE_ID,
            T_ARE_YOU_ALIVE_TEST_REPORT_ID,
            0,
            req_PUS_header->source_id,
            0
        );
    }

    SPP_send_TM(&resp_SPP_header, &resp_PUS_TM_header, NULL, 0);

    send_succ_comp(req_SPP_header, req_PUS_header);

    return SPP_OK;
}


