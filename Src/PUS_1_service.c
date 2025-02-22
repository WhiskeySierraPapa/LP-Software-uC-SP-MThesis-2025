/*
 * PUS_1_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"
#include "General_Functions.h"
#include "PUS_1_service.h"

extern QueueHandle_t UART_OBC_Out_Queue;

// Flags denoting if an ACK TM message is requested for
// Success of request acceptence, start , progress and completion of execution
static inline uint8_t succ_acceptence_req(PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x08;
}
static inline uint8_t succ_start_req     (PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x04;
}
static inline uint8_t succ_progress_req  (PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x02;
}
static inline uint8_t succ_completion_req(PUS_TC_header_t* secondary_header) {
    return secondary_header->ACK_flags & 0x01;
}

void PUS_1_send_succ_acc(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_acceptence_req(PUS_h)) {

    	UART_OUT_msg msg_to_send = {0};

		msg_to_send.PUS_HEADER_PRESENT	= 1;
    	msg_to_send.PUS_SOURCE_ID 		= PUS_h->source_id;
    	msg_to_send.SERVICE_ID			= REQUEST_VERIFICATION_SERVICE_ID;
    	msg_to_send.SUBTYPE_ID			= RV_SUCC_ACCEPTANCE_VERIFICATION_ID;
    	SPP_encode_header(SPP_h, msg_to_send.TM_data);
    	msg_to_send.TM_data_len			= SPP_HEADER_LEN;

    	xQueueSend(UART_OBC_Out_Queue, &msg_to_send, portMAX_DELAY);
    }
}
void PUS_1_send_fail_acc(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_acceptence_req(PUS_h)) {
//        SPP_send_req_ver(SPP_h, PUS_h, RV_FAIL_ACCEPTANCE_VERIFICATION_ID);
    }
}

void PUS_1_send_succ_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_start_req(PUS_h)) {

    	UART_OUT_msg msg_to_send = {0};

		msg_to_send.PUS_HEADER_PRESENT	= 1;
		msg_to_send.PUS_SOURCE_ID 		= PUS_h->source_id;
		msg_to_send.SERVICE_ID			= REQUEST_VERIFICATION_SERVICE_ID;
		msg_to_send.SUBTYPE_ID			= RV_SUCC_START_OF_EXEC_VERIFICATION_ID;
		SPP_encode_header(SPP_h, msg_to_send.TM_data);
		msg_to_send.TM_data_len			= SPP_HEADER_LEN;

		xQueueSend(UART_OBC_Out_Queue, &msg_to_send, portMAX_DELAY);
    }
}
void PUS_1_send_fail_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_start_req(PUS_h)) {
//        SPP_send_req_ver(SPP_h, PUS_h, RV_FAIL_START_OF_EXEC_VERIFICATION_ID);
    }
}

void PUS_1_send_succ_prog(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_progress_req(PUS_h)) {

    	UART_OUT_msg msg_to_send = {0};

		msg_to_send.PUS_HEADER_PRESENT	= 1;
		msg_to_send.PUS_SOURCE_ID 		= PUS_h->source_id;
		msg_to_send.SERVICE_ID			= REQUEST_VERIFICATION_SERVICE_ID;
		msg_to_send.SUBTYPE_ID			= RV_SUCC_PROG_OF_EXEC_VERIFICATION_ID;
		SPP_encode_header(SPP_h, msg_to_send.TM_data);
		msg_to_send.TM_data_len			= SPP_HEADER_LEN;

		xQueueSend(UART_OBC_Out_Queue, &msg_to_send, portMAX_DELAY);
    }
}
void PUS_1_send_fail_prog(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_progress_req(PUS_h)) {
//        SPP_send_req_ver(SPP_h, PUS_h, RV_FAIL_PROG_OF_EXEC_VERIFICATION_ID);
    }
}

void PUS_1_send_succ_comp(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_completion_req(PUS_h)) {

    	UART_OUT_msg msg_to_send = {0};

		msg_to_send.PUS_HEADER_PRESENT	= 1;
		msg_to_send.PUS_SOURCE_ID 		= PUS_h->source_id;
		msg_to_send.SERVICE_ID			= REQUEST_VERIFICATION_SERVICE_ID;
		msg_to_send.SUBTYPE_ID			= RV_SUCC_COMPL_OF_EXEC_VERIFICATION_ID;
		SPP_encode_header(SPP_h, msg_to_send.TM_data);
		msg_to_send.TM_data_len			= SPP_HEADER_LEN;

		xQueueSend(UART_OBC_Out_Queue, &msg_to_send, portMAX_DELAY);
    }
}
void PUS_1_send_fail_comp(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h) {
    if (succ_completion_req(PUS_h)) {
//        SPP_send_req_ver(SPP_h, PUS_h, RV_FAIL_COMPL_OF_EXEC_VERIFICATION_ID);
    }
}
