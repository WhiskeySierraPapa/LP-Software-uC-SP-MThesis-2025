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


SPP_error PUS_17_handle_TEST_TC(SPP_header_t* SPP_header, PUS_TC_header_t* PUS_TC_header) {

	if (Current_Global_Device_State != NORMAL_MODE) {
        return UNDEFINED_ERROR;
    }
    if (SPP_header == NULL || PUS_TC_header == NULL) {
        return UNDEFINED_ERROR;
    }

    if (PUS_TC_header->message_subtype_id == T_ARE_YOU_ALIVE_TEST_ID) {

    	PUS_1_send_succ_acc(SPP_header, PUS_TC_header);

        PUS_1_send_succ_start(SPP_header, PUS_TC_header);
        
        PUS_1_send_succ_prog(SPP_header, PUS_TC_header);

        uint8_t data = 0;

		Add_SPP_PUS_and_send_TM(SPP_header->application_process_id,
								1,
								SPP_header->packet_sequence_count,
								PUS_TC_header->source_id,
								TEST_SERVICE_ID,
								T_ARE_YOU_ALIVE_TEST_ID,
								&data,
								1);

		PUS_1_send_succ_comp(SPP_header, PUS_TC_header);
    }

    return SPP_OK;
}


