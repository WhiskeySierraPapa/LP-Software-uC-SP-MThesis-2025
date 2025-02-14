/*
 * PUS_3_service.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "PUS_1_service.h"

#ifndef PUS_3_SERVICE_H_
#define PUS_3_SERVICE_H_

// House Keeping service [3] subtype IDs
typedef enum {
    HK_CREATE_HK_PAR_REPORT_STRUCT         = 1,  // TC
    HK_DELETE_HK_PAR_REPORT_STRUCT         = 3,  // TC
    HK_EN_PERIODIC_REPORTS                 = 5,  // TC
    HK_DIS_PERIODIC_REPORTS                = 6,  // TC
    HK_REPORT_HK_PAR_REPORT_STRUCT         = 9,  // TC
    HK_REPORT_HK_PAR_REPORT_STRUCT_REPORT  = 10, // TM (response to 9)
    HK_PARAMETER_REPORT                    = 25, // TM
    HK_ONE_SHOT                            = 27, // TC
} PUS_HK_Subtype_ID;

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
	uint8_t uC_report_frequency;
	uint8_t new_uC_report_frequency;
	uint8_t FPGA_report_frequency;
	uint8_t new_FPGA_report_frequency; // 1 - uC, 2 - FPGA, 3 - uC & FPGA
} PUS_3_msg;

/* PUS_3_service */
SPP_error PUS_3_handle_HK_TC(SPP_header_t* primary_header, PUS_TC_header_t* secondary_header, uint8_t* data);
void PUS_3_collect_HK_data(uint32_t current_ticks);
void PUS_3_HK_send(PUS_3_msg* pus3_msg_received);

#endif /* PUS_3_SERVICE_H_ */
