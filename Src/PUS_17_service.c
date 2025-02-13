/*
 * PUS_17_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"
#include "General_Functions.h"
#include "PUS_1_service.h"
#include "PUS_17_service.h"


SPP_error SPP_handle_TEST_TC(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (Current_Global_Device_State != NORMAL_MODE) {
        return UNDEFINED_ERROR;
    }
    if (SPP_h == NULL || PUS_h == NULL) {
        return UNDEFINED_ERROR;
    }

    if (PUS_h->message_subtype_id == T_ARE_YOU_ALIVE_TEST_ID) {

        PUS_1_send_succ_start(SPP_h, PUS_h);
        
        PUS_1_send_succ_prog(SPP_h, PUS_h);

		Add_SPP_PUS_and_send_TM(SPP_h->application_process_id,
								1,
								SPP_h->packet_sequence_count,
								PUS_h->source_id,
								TEST_SERVICE_ID,
								T_ARE_YOU_ALIVE_TEST_ID,
								NULL,
								0);

		PUS_1_send_succ_comp(SPP_h, PUS_h);
    }

    return SPP_OK;
}


