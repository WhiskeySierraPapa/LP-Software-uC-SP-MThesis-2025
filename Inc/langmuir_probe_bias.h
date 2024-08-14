/*
 * langmuir_probe_bias.h
 *
 *  Created on: 2024. gada 31. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#ifndef LANGMUIR_PROBE_BIAS_H_
#define LANGMUIR_PROBE_BIAS_H_

#include "Space_Packet_Protocol.h"

typedef enum {
    PROBE_ID_0 = 0,
    PROBE_ID_1 = 1,
} Probe_ID_t;


typedef enum {
    SWEEP_PROBE_ARG_ID      = 0x01,
    SWEEP_STEPS_ARG_ID      = 0x02,
    SWEEP_VOL_LVL_ARG_ID    = 0x03,
} Sweep_VL_Arg_ID_t;


typedef enum {
    CONST_BIAS_PROBE_ARG_ID     = 0x01,
    CONST_BIAS_VOL_LVL_ARG_ID   = 0x02,
} CB_VL_Arg_ID_t;


typedef enum {
    //TODO: Check if these IDs are not already taken.
    FPGA_EN_CB_MODE          = 0xCA,
    FPGA_SET_CB_VOL_LVL      = 0xCB,
    FPGA_GET_CB_VOL_LVL      = 0xCC,

    FPGA_EN_SWT_MODE         = 0xAA,
    FPGA_SET_SWT_VOL_LVL     = 0xAB,
    FPGA_SET_SWT_STEPS       = 0xAC,
    FPGA_SET_SWT_SMPL_RT     = 0xAD,
    FPGA_GET_SWT_VOL_LVL     = 0xA0,
    FPGA_GET_SWT_STEPS       = 0xA1,
    FPGA_GET_SWT_SMPL_RT     = 0xA2,
} FPGA_cmd_ID_t;

typedef enum {
    FRAM_CB_VOL_LVL             = 0x0008,
    FRAM_SWT_STEPS              = 0x000A,
    FRAM_SWT_SMPL_RT            = 0x000B,
    FRAM_SWEEP_TABLE_0_START    = 0x000D,
    FRAM_SWEEP_TABLE_1_START    = 0x020D,
} FRAM_addr_t;

#define FPGA_MSG_PREMABLE_0     0xB5
#define FPGA_MSG_PREMABLE_1     0x43
#define FPGA_MSG_POSTAMBLE      0x0A


void enable_FPGA_CB_mode();
void send_FPGA_CB_voltage_level(uint8_t probe_id, uint8_t* voltage_level);
uint16_t get_FPGA_CB_voltage_level(uint8_t probe_id);

void enable_FPGA_SWT_mode();
void send_FPGA_SWT_steps(uint8_t steps);
void send_FPGA_SWT_sample_rate(uint8_t sample_rate);
void send_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id, uint8_t* voltage_level);
uint8_t get_FPGA_SWT_steps();
uint8_t get_FPGA_SWT_sample_rate();
uint16_t get_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id);

#endif /* LANGMUIR_PROBE_BIAS_H_ */
