/*
 * PUS_1_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"

// Flags denoting if an ACK TM message is requested for
// Success of request acceptence, start , progress and completion of execution
uint8_t succ_acceptence_req(PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x08;
}
uint8_t succ_start_req     (PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x04;
}
uint8_t succ_progress_req  (PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x02;
}
uint8_t succ_completion_req(PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x01;
}


SPP_error SPP_send_req_ver(SPP_header_t* req_SPP_header, PUS_TC_header_t* req_PUS_header, PUS_RV_Subtype_ID requested_ACK) {

    SPP_header_t resp_SPP_header;
    PUS_TM_header_t resp_PUS_TM_header;
    uint8_t data[SPP_PRIMARY_HEADER_LEN];

    resp_SPP_header = SPP_make_header(
        SPP_VERSION,
        SPP_PACKET_TYPE_TM,
        req_SPP_header->secondary_header_flag,
        req_SPP_header->application_process_id,
        SPP_SEQUENCE_SEG_UNSEG,
        req_SPP_header->packet_sequence_count,
        SPP_PUS_TM_HEADER_LEN_WO_SPARE + SPP_PRIMARY_HEADER_LEN + CRC_BYTE_LEN - 1
    );
    // Create response PUS TM header with 1,requested_ACK
    resp_PUS_TM_header = PUS_make_TM_header(
        PUS_VERSION,
        0,
        REQUEST_VERIFICATION_SERVICE_ID,
        requested_ACK,
        0,
        req_PUS_header->source_id,
        0
    );

    // Data sent in request verification is the request primary header itself.
    // Thus we need to copy it into the data field of the response.
    SPP_encode_header(req_SPP_header, data);

    SPP_send_TM(&resp_SPP_header, &resp_PUS_TM_header, data, SPP_PRIMARY_HEADER_LEN);
    return SPP_OK;
}