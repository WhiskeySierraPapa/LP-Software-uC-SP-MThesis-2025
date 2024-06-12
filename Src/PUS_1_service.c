/*
 * PUS_1_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"

// Flags denoting if an ACK TM message is requested for
// Success of request acceptence, start , progress and completion of execution
uint8_t succ_acceptence_req(SPP_PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x08;
}
uint8_t succ_start_req     (SPP_PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x04;
}
uint8_t succ_progress_req  (SPP_PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x02;
}
uint8_t succ_completion_req(SPP_PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x01;
}


SPP_error SPP_send_request_verification(SPP_primary_header_t* request_primary_header, SPP_PUS_TC_header_t* request_secondary_header, PUS_RV_Subtype_ID requested_ACK) {

    SPP_primary_header_t response_primary_header;
    SPP_PUS_TM_header_t response_PUS_TM_header;
    uint8_t data[SPP_PRIMARY_HEADER_LEN];

    response_primary_header = SPP_make_new_primary_header(SPP_VERSION, SPP_PACKET_TYPE_TM, request_primary_header->secondary_header_flag,
        request_primary_header->application_process_id, SPP_SEQUENCE_SEG_UNSEG, request_primary_header->packet_sequence_count,
        SPP_PUS_TM_HEADER_LEN_WO_SPARE + SPP_PRIMARY_HEADER_LEN + CRC_BYTE_LEN - 1
    );
    // Create response PUS TM header with 1,requested_ACK
    response_PUS_TM_header = SPP_make_new_PUS_TM_header(PUS_VERSION, 0, REQUEST_VERIFICATION_SERVICE_ID, requested_ACK,
        0, request_secondary_header->source_id, 0
    );

    // Data sent in request verification is the request primary header itself.
    // Thus we need to copy it into the data field of the response.
    SPP_encode_primary_header(request_primary_header, data);

    SPP_send_TM(&response_primary_header, &response_PUS_TM_header, data, SPP_PRIMARY_HEADER_LEN);
    return SPP_OK;
}