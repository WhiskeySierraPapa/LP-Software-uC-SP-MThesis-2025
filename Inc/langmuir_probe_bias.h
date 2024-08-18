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
    PROBE_ID_ARG_ID     = 0x01,
    STEP_ID_ARG_ID      = 0x02,
    VOL_LVL_ARG_ID      = 0x03,
    N_STEPS_ARG_ID      = 0x04,
    N_SKIP_ARG_ID       = 0x05,
    N_F_ARG_ID          = 0x06,
    N_POINTS_ARG_ID     = 0x07,
} FPGA_Arg_ID_t;




typedef enum {
    //TODO: Check if these IDs are not already taken.
    FPGA_EN_CB_MODE          = 0xCA,

    FPGA_SET_CB_VOL_LVL      = 0xCB,

    FPGA_GET_CB_MODE         = 0xC0,
    FPGA_GET_CB_VOL_LVL      = 0xCC,

    FPGA_EN_SWT_MODE                = 0xAA,
    FPGA_SET_SWT_VOL_LVL            = 0xAB,
    FPGA_SET_SWT_STEPS              = 0xAC,
    FPGA_SET_SWT_SAMPLE_SKIP        = 0xAD,
    FPGA_SET_SWT_SAMPLES_PER_POINT  = 0xAE,
    FPGA_SET_SWT_NPOINTS            = 0xAF,

    FPGA_GET_SWT_MODE               = 0xA0,
    FPGA_GET_SWT_VOL_LVL            = 0xA1,
    FPGA_GET_SWT_STEPS              = 0xA2,
    FPGA_GET_SWT_SAMPLE_SKIP        = 0xA3,
    FPGA_GET_SWT_SAMPLES_PER_POINT  = 0xA4,
    FPGA_GET_SWT_NPOINTS            = 0xA5,

} FPGA_Func_ID_t;

typedef enum {
    FRAM_CB_VOL_LVL             = 0x0008,
    FRAM_SWT_STEPS              = 0x000A,
    FRAM_SWT_SMPL_RT            = 0x000B,
    FRAM_SWEEP_TABLE_0_START    = 0x000D,
    FRAM_SWEEP_TABLE_1_START    = 0x020D,
} FRAM_addr_t;

typedef struct FPGA_msg_arg {
    uint8_t  probe_ID;
    uint8_t  step_ID;
    uint16_t voltage_level;
    uint16_t N_skip;
    uint8_t  N_steps;
    uint16_t N_f; // Samples per points
    uint16_t N_points;
    uint8_t result[32];       // A small array here is sufficient for most get function from the FPGA, but If we want to 
                                    // Get the whole sweep table in a single buffer to send to ground. Then this needs to be 2 * 256 bytes.
    uint16_t result_len;
} FPGA_msg_arg_t;

#define FPGA_MSG_PREMABLE_0     0xB5
#define FPGA_MSG_PREMABLE_1     0x43
#define FPGA_MSG_POSTAMBLE      0x0A


void send_FPGA_langmuir_msg(uint8_t func_id, uint8_t N_args, FPGA_msg_arg_t* fpgama);
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
