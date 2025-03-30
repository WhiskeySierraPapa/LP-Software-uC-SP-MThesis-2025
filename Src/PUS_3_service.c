/*
 * PUS_3_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "General_Functions.h"
#include "Space_Packet_Protocol.h"
#include "PUS_1_service.h"
#include "PUS_3_service.h"
#include "Device_State.h"

extern uint16_t temperature_i;
extern uint16_t uc3v_i;
extern uint16_t fpga3v_i;
extern uint16_t fpga1p5v_i;
extern uint16_t vbat_i;
extern uint16_t HK_SPP_APP_ID;
extern uint16_t HK_PUS_SOURCE_ID;
extern QueueHandle_t UART_OBC_Out_Queue;

uint8_t current_uC_report_frequency = 0;
uint8_t current_FPGA_report_frequency = 0;

HK_par_report_structure_t HKPRS_uc = {
    .SID                    = UC_SID,
    .collection_interval    = DEF_COL_INTV,
    .N1                     = DEF_UC_N1,
    .parameters             = {0},
    .periodic_send          = DEF_UC_PS,
    .last_collect_tick      = 0,
    .seq_count              = 0,
};

HK_par_report_structure_t HKPRS_fpga = {
    .SID                    = FPGA_SID,
    .collection_interval    = DEF_COL_INTV,
    .N1                     = DEF_FPGA_N1,
    .parameters             = {0},
    .periodic_send          = DEF_FPGA_PS,
    .last_collect_tick      = 0,
    .seq_count              = 0,
};

HK_par_report_structure_t HKPRS_err = {
    .SID                    = 0,
    .collection_interval    = 0,
    .N1                     = 0,
    .parameters             = {0},
    .periodic_send          = 0,
    .last_collect_tick      = 0,
    .seq_count              = 0,
};


HK_par_report_structure_t* HKPRS;
uint8_t HK_TM_data[MAX_TM_DATA_LEN];
uint16_t HK_TM_data_len;

// This queue is used to receive info from the UART handler task
QueueHandle_t PUS_3_Queue;

static void fill_report_struct(uint16_t SID) {
    uint16_t s_vbat = vbat_i;
    uint16_t s_temp = temperature_i;
    uint16_t s_fpga3v = fpga3v_i;
    uint16_t s_fpga1p5v = fpga1p5v_i;
    uint16_t s_uc3v = uc3v_i;

    uint32_t uc_pars[DEF_UC_N1] = {s_vbat, s_temp, s_uc3v};
    uint32_t fpga_pars[DEF_FPGA_N1] = {s_fpga1p5v, s_fpga3v};

    switch(SID) {
        case UC_SID:
            for(int i = 0; i < HKPRS_uc.N1; i++) {
            	HKPRS_uc.parameters[i] = uc_pars[i];
            }
            break;
        case FPGA_SID:
            for(int i = 0; i < HKPRS_fpga.N1; i++) {
            	HKPRS_fpga.parameters[i] = fpga_pars[i];
            }
            break;
    }
}

void PUS_3_collect_HK_data(uint32_t current_ticks) {
    if ((current_ticks - HKPRS_uc.last_collect_tick) > HKPRS_uc.collection_interval) {
        fill_report_struct(HKPRS_uc.SID);
        HKPRS_uc.last_collect_tick = current_ticks;
    }
    if ((current_ticks - HKPRS_fpga.last_collect_tick) > HKPRS_fpga.collection_interval) {
        fill_report_struct(HKPRS_fpga.SID);
        HKPRS_fpga.last_collect_tick = current_ticks;
    }
}


static uint16_t encode_HK_struct(HK_par_report_structure_t* HKPRS, uint8_t* out_buffer) {
    uint8_t* iterator_pointer = out_buffer;
    memcpy(iterator_pointer, &(HKPRS->SID), sizeof(HKPRS->SID));
    iterator_pointer += sizeof(HKPRS->SID);

    for (int i = 0; i < HKPRS->N1; i++) {
        memcpy(iterator_pointer, &(HKPRS->parameters[i]), sizeof(HKPRS->parameters[i]));
        iterator_pointer += sizeof(HKPRS->parameters[i]);
    }
    return iterator_pointer - out_buffer;
}


void PUS_3_HK_send(PUS_3_msg* pus3_msg_received) {
	if (current_uC_report_frequency >= 1) {
		if (current_uC_report_frequency == 1) {
			current_uC_report_frequency = 0;
		}

		UART_OUT_OBC_msg msg_to_send_uC = {0};

		msg_to_send_uC.PUS_HEADER_PRESENT	= 1;
		msg_to_send_uC.PUS_SOURCE_ID 		= pus3_msg_received->PUS_TC_header.source_id;
		msg_to_send_uC.SERVICE_ID			= HOUSEKEEPING_SERVICE_ID;
		msg_to_send_uC.SUBTYPE_ID			= HK_PARAMETER_REPORT;
		uint16_t tm_data_len = encode_HK_struct(&HKPRS_uc, msg_to_send_uC.TM_data);
		msg_to_send_uC.TM_data_len			= tm_data_len;

		xQueueSend(UART_OBC_Out_Queue, &msg_to_send_uC, portMAX_DELAY);
	}

	if (current_FPGA_report_frequency >= 1) {
		if (current_FPGA_report_frequency == 1) {
			current_FPGA_report_frequency = 0;
		}

		UART_OUT_OBC_msg msg_to_send_FPGA = {0};

		msg_to_send_FPGA.PUS_HEADER_PRESENT	= 1;
		msg_to_send_FPGA.PUS_SOURCE_ID 		= pus3_msg_received->PUS_TC_header.source_id;
		msg_to_send_FPGA.SERVICE_ID			= HOUSEKEEPING_SERVICE_ID;
		msg_to_send_FPGA.SUBTYPE_ID			= HK_PARAMETER_REPORT;
		uint16_t tm_data_len = encode_HK_struct(&HKPRS_fpga, msg_to_send_FPGA.TM_data);
		msg_to_send_FPGA.TM_data_len			= tm_data_len;

		xQueueSend(UART_OBC_Out_Queue, &msg_to_send_FPGA, portMAX_DELAY);
	}
}


void PUS_3_set_report_frequency(uint8_t* data, PUS_3_msg* pus3_msg_received) {
    uint8_t* data_iterator = data;
	uint16_t SID_num = 0;
    memcpy(&SID_num, data_iterator, sizeof(SID_num));

    data_iterator += sizeof(SID_num);

    for(int i = 0; i < SID_num && data_iterator < data + (pus3_msg_received->data_size * sizeof(uint8_t)); i++) {
        uint16_t SID = 0;
        memcpy(&SID, data_iterator, sizeof(SID));
        data_iterator += sizeof(SID);

        switch(SID) {
            case UC_SID:
            	// update the report frequency for microcontroller
            	current_uC_report_frequency = pus3_msg_received->new_report_frequency;
                break;
            case FPGA_SID:
            	// update the report frequency for FPGA
            	current_FPGA_report_frequency = pus3_msg_received->new_report_frequency;
                break;
        }
    }
}


// HK - Housekeeping PUS service 3
SPP_error PUS_3_handle_HK_TC(SPP_header_t* SPP_header , PUS_TC_header_t* PUS_TC_header, uint8_t* data, uint8_t data_size)
{

	if (Current_Global_Device_State != NORMAL_MODE) {
        return UNDEFINED_ERROR;
    }
    if (SPP_header == NULL || PUS_TC_header == NULL || data_size < 4) {
        return UNDEFINED_ERROR;
    }

    // Define report frequency and handle different message subtypes
    uint8_t report_frequency = 0;

    switch (PUS_TC_header->message_subtype_id) {
        case HK_ONE_SHOT:
            report_frequency = 1;
            break;
        case HK_EN_PERIODIC_REPORTS:
            report_frequency = 2;
            break;
        case HK_DIS_PERIODIC_REPORTS:
            report_frequency = 0;
            break;
        default:
        	PUS_1_send_fail_acc(SPP_header, PUS_TC_header);
            return UNDEFINED_ERROR;  // Invalid message subtype
    }

    PUS_1_send_succ_acc(SPP_header, PUS_TC_header);

	PUS_3_msg pus3_msg_to_send;
	pus3_msg_to_send.SPP_header = *SPP_header;
	pus3_msg_to_send.PUS_TC_header = *PUS_TC_header;
	memcpy(pus3_msg_to_send.data, data, data_size);
	pus3_msg_to_send.data_size = data_size;
	pus3_msg_to_send.new_report_frequency = report_frequency;

    if (xQueueSend(PUS_3_Queue, &pus3_msg_to_send, 0) != pdPASS) {
    	PUS_1_send_fail_start(SPP_header, PUS_TC_header);
    }
    return SPP_OK;
}
