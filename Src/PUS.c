/*
 * PUS.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"

SPP_error SPP_decode_PUS_TC_header(uint8_t* raw_header, SPP_PUS_TC_header_t* secondary_header) {
    secondary_header->PUS_version_number = (raw_header[0] & 0xF0) >> 4;
    secondary_header->ACK_flags          = (raw_header[0] & 0x0F);
    secondary_header->service_type_id    =  raw_header[1];
    secondary_header->message_subtype_id =  raw_header[2];
    secondary_header->source_id          = (raw_header[3] << 8) | raw_header[4];
    secondary_header->spare              = 0; // Based on PUS message type I guess? (Optional)
    return SPP_OK;
}

// Technically this is not needed since, Langmuir Probe Payload will not send TCs.
SPP_error SPP_encode_PUS_TC_header(SPP_PUS_TC_header_t* secondary_header, uint8_t* result_buffer) {
    for(int i = 0; i < SPP_PUS_TC_HEADER_LEN_WO_SPARE; i++) {
        result_buffer[i] ^= result_buffer[i];    // Clear result buffer.
    }

    result_buffer[0] |=  secondary_header->PUS_version_number << 4;
    result_buffer[0] |=  secondary_header->ACK_flags;
    result_buffer[1] |=  secondary_header->service_type_id;
    result_buffer[2] |=  secondary_header->message_subtype_id;
    result_buffer[3] |= (secondary_header->source_id & 0xFF00) >> 8;
    result_buffer[4] |= (secondary_header->source_id & 0x00FF);

    return SPP_OK;
};

// Technically this is not needed since, Langmuir Probe Payload will only send TMs.
SPP_error SPP_decode_PUS_TM_header(uint8_t* raw_header, SPP_PUS_TM_header_t* secondary_header) {
    secondary_header->PUS_version_number    = (raw_header[0] & 0xF0) >> 4;
    secondary_header->sc_time_ref_status    = (raw_header[0] & 0x0F);
    secondary_header->service_type_id       =  raw_header[1];
    secondary_header->message_subtype_id    =  raw_header[2];
    secondary_header->message_type_counter  = (raw_header[3] << 8) | raw_header[4];
    secondary_header->destination_id        = (raw_header[5] << 8) | raw_header[6];
    secondary_header->time                  = (raw_header[7] << 8) | raw_header[8];
    secondary_header->spare                 = 0; // Based on PUS message type I guess? (Optional)
    return SPP_OK;
}

SPP_error SPP_encode_PUS_TM_header(SPP_PUS_TM_header_t* secondary_header, uint8_t* result_buffer) {
    for(int i = 0; i < SPP_PUS_TM_HEADER_LEN_WO_SPARE; i++) {
        result_buffer[i] ^= result_buffer[i];    // Clear result buffer.
    }

    result_buffer[0] |=  secondary_header->PUS_version_number << 4;
    result_buffer[0] |=  secondary_header->sc_time_ref_status;
    result_buffer[1] |=  secondary_header->service_type_id;
    result_buffer[2] |=  secondary_header->message_subtype_id;
    result_buffer[3] |= (secondary_header->message_type_counter & 0xFF00) >> 8;
    result_buffer[4] |= (secondary_header->message_type_counter & 0x00FF);
    result_buffer[5] |= (secondary_header->destination_id & 0xFF00) >> 8;
    result_buffer[6] |= (secondary_header->destination_id & 0x00FF);
    result_buffer[7] |= (secondary_header->time & 0xFF00) >> 8;
    result_buffer[8] |= (secondary_header->time & 0x00FF);
    
    return SPP_OK;
};




SPP_PUS_TM_header_t SPP_make_new_PUS_TM_header(uint8_t PUS_version_number, uint8_t sc_time_ref_status, uint8_t service_type_id,
                                uint8_t message_subtype_id, uint16_t message_type_counter, uint16_t destination_id, uint16_t time) {
    SPP_PUS_TM_header_t PUS_TM_header;
    PUS_TM_header.PUS_version_number      =  PUS_version_number;
    PUS_TM_header.sc_time_ref_status      =  sc_time_ref_status;
    PUS_TM_header.service_type_id         =  service_type_id;
    PUS_TM_header.message_subtype_id      =  message_subtype_id;
    PUS_TM_header.message_type_counter    =  message_type_counter;
    PUS_TM_header.destination_id          =  destination_id;
    PUS_TM_header.time                    =  time;
    return PUS_TM_header;
}


