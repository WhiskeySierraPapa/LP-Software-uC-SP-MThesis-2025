/*
 * PUS_3_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"


// HK - Housekeeping PUS service 3
SPP_error SPP_handle_HK_TC(SPP_PUS_TC_header_t* secondary_header) {
    if (secondary_header == NULL) {
        return UNDEFINED_ERROR;
    }


    if (secondary_header->message_subtype_id == 128) {
        
    }
    return SPP_OK;
}
