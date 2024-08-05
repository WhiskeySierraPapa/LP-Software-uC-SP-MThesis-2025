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
} Sweep_VL_Set_Arg_ID_t;


typedef enum {
    //TODO: Check if these IDs are not already taken.
    FPGA_EN_CB_MODE          = 0xCA,
    FPGA_SET_CB_VOL_LVL      = 0xCB,
    FPGA_GET_CB_VOL_LVL      = 0xCC,

    FPGA_EN_SWT_MODE         = 0xAA,
    FPGA_SET_SWT_VOL_LVL     = 0xAB,
    FPGA_SET_SWT_STEPS       = 0xAC,
    FPGA_SET_SWT_SMPL_RT     = 0xAD,
} FPGA_cmd_ID_t;


#define FPGA_MSG_PREMABLE_0     0xB5
#define FPGA_MSG_PREMABLE_1     0x43
#define FPGA_MSG_POSTAMBLE      0x0A


void enable_FPGA_CB_mode();
void send_FPGA_CB_voltage_level(uint8_t* voltage_level);
uint16_t get_FPGA_CB_voltage_level();

void enable_FPGA_SWT_mode();
void send_FPGA_SWT_steps(uint8_t steps);
void send_FPGA_SWT_sample_rate(uint8_t sample_rate);
void send_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id, uint8_t* voltage_level);

#endif /* LANGMUIR_PROBE_BIAS_H_ */
