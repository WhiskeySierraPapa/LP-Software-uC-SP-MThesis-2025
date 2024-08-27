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

extern uint8_t FPGA_byte_recv;


void send_FPGA_langmuir_msg(uint8_t func_id, uint8_t N_args, FPGA_msg_arg_t* fpgama);
bool is_langmuir_func(uint8_t func_id);
bool FPGA_rx_langmuir_readback(uint8_t recv_byte);
#endif /* LANGMUIR_PROBE_BIAS_H_ */
