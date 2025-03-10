/*
 * PUS_8_servie.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"

#ifndef PUS_8_SERVIE_H_
#define PUS_8_SERVIE_H_

#define PUS_8_MAX_DATA_LEN 	20
#define TARGET_uC 			1
#define TARGET_FPGA 		0



// Function Management [8] subtype IDs
typedef enum {
    FM_PERFORM_FUNCTION                    = 1,  // TC
} PUS_FM_Subtype_ID;

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
	uint8_t data[PUS_8_MAX_DATA_LEN];
} PUS_8_msg;

typedef enum {
    GS_FPGA_TARGET = 0,
    GS_FRAM_TARGET = 1,
} GS_Target_t;

typedef struct {
	uint8_t  func_id;
	uint8_t  N_args;
    uint8_t  probe_ID;
    uint8_t  step_ID;
    uint16_t voltage_level;
    uint16_t N_skip;
    uint8_t  N_steps;
    uint16_t N_f; // Samples per points
    uint16_t N_points;
    GS_Target_t  target;
    uint16_t N_samples_per_step;
    uint8_t FRAM_Table_ID;
} PUS_8_msg_unpacked;

typedef enum {
    FPGA_EN_CB_MODE          		= 0xCA,
    FPGA_DIS_CB_MODE         		= 0xC0,

    FPGA_SET_CB_VOL_LVL      		= 0xCB,
    FPGA_GET_CB_VOL_LVL      		= 0xCC,

    FPGA_SWT_ACTIVATE_SWEEP         = 0xAA,
    FPGA_SET_SWT_VOL_LVL            = 0xAB,
    FPGA_SET_SWT_STEPS              = 0xAC,
    FPGA_SET_SWT_SAMPLES_PER_STEP   = 0xAD,
    FPGA_SET_SWT_SAMPLE_SKIP        = 0xAE,
    FPGA_SET_SWT_SAMPLES_PER_POINT  = 0xAF,
    FPGA_SET_SWT_NPOINTS            = 0xB0,

    FPGA_GET_SWT_SWEEP_CNT          = 0xA0,
    FPGA_GET_SWT_VOL_LVL            = 0xA1,
    FPGA_GET_SWT_STEPS              = 0xA2,
    FPGA_GET_SWT_SAMPLES_PER_STEP   = 0xA3,
    FPGA_GET_SWT_SAMPLE_SKIP        = 0xA4,
    FPGA_GET_SWT_SAMPLES_PER_POINT  = 0xA5,
    FPGA_GET_SWT_NPOINTS            = 0xA6,
	CPY_TABLE_FRAM_TO_FPGA	 		= 0xE0,

} PUS_8_Func_ID;

typedef enum {
    PROBE_ID_ARG_ID             = 0x01,
    STEP_ID_ARG_ID              = 0x02,
    VOL_LVL_ARG_ID              = 0x03,
    N_STEPS_ARG_ID              = 0x04,
    N_SKIP_ARG_ID               = 0x05,
    N_F_ARG_ID                  = 0x06,
    N_POINTS_ARG_ID             = 0x07,
    GS_TARGET_ARG_ID            = 0x08, // GS Target = Get Set Target
    FRAM_TABLE_ID_ARG_ID        = 0x09,
    N_SAMPLES_PER_STEP_ARG_ID   = 0x0A,
} FPGA_Arg_ID_t;

/* PUS_8_service */
bool PUS_8_check_FPGA_msg_format(uint8_t* msg, uint8_t msg_len);
void PUS_8_unpack_msg(uint8_t* data, PUS_8_msg_unpacked* pus8_msg_unpacked);
SPP_error PUS_8_perform_function(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_TC_h , PUS_8_msg_unpacked* pus8_msg_unpacked);
SPP_error PUS_8_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* secondary_header, uint8_t* data);

#endif /* PUS_8_SERVIE_H_ */
