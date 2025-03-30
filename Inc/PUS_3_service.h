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

#define PUS_3_MAX_DATA_LEN	6
#define MAX_PAR_COUNT       16
#define MAX_STRUCT_COUNT    16
#define MAX_TM_DATA_LEN     (MAX_PAR_COUNT * 4) + 2 // Each parameter is potentialy 4 bytes and struct id is 2 bytes.
#define DEF_COL_INTV        500
#define DEF_UC_N1           3
#define DEF_UC_PS           0

#define DEF_FPGA_N1         2
#define DEF_FPGA_PS         0


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
    uint16_t SID;
    uint16_t collection_interval;
    uint16_t N1;
    uint32_t parameters[MAX_PAR_COUNT]; // 32 bit here for future proofing. All params currently are 16-bit.
    uint8_t  periodic_send; // 0 - do not send, 1 - send one time, 2 - send periodically
    uint32_t last_collect_tick;
    uint32_t seq_count;
} HK_par_report_structure_t;

typedef enum {
    UC_SID            = 0xAAAA,
    FPGA_SID          = 0x5555,
} HK_SID;

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
	uint8_t data[PUS_3_MAX_DATA_LEN];
	uint8_t data_size;
	uint8_t new_report_frequency;
} PUS_3_msg;

/* PUS_3_service */
void PUS_3_set_report_frequency(uint8_t* data, PUS_3_msg* pus3_msg_received);
SPP_error PUS_3_handle_HK_TC(SPP_header_t* primary_header, PUS_TC_header_t* secondary_header, uint8_t* data, uint8_t data_size);
void PUS_3_collect_HK_data(uint32_t current_ticks);
void PUS_3_HK_send(PUS_3_msg* pus3_msg_received);

#endif /* PUS_3_SERVICE_H_ */
