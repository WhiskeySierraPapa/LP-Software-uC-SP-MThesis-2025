/*
 * langmuir_probe_bias.c
 *
 *  Created on: 2024. gada 31. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#include "langmuir_probe_bias.h"
#include "FPGA_UART.h"

// Constant Bias mode
void enable_FPGA_CB_mode() {
    uint8_t en_CB_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_EN_CB_MODE, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(en_CB_msg, 4);
    SPP_UART_transmit_DMA(en_CB_msg, sizeof(en_CB_msg));
}

void send_FPGA_CB_voltage_level(uint8_t probe_id, uint8_t* voltage_level) {
    uint8_t set_CB_vl_msg[7] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_CB_VOL_LVL, probe_id, voltage_level[0], voltage_level[1], FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(set_CB_vl_msg, 5);
    writeFRAM(FRAM_CB_VOL_LVL, voltage_level, 2);
    SPP_UART_transmit_DMA(set_CB_vl_msg, sizeof(set_CB_vl_msg));
}

uint16_t get_FPGA_CB_voltage_level(uint8_t probe_id){
    uint8_t get_CB_vl_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_GET_CB_VOL_LVL, probe_id, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(set_CB_vl_msg, 4);
    SPP_UART_transmit_DMA(get_CB_vl_msg, sizeof(get_CB_vl_msg));
    //TODO: Add a way to receive response msg from FPGA.    
    return 0;
}


// Sweep Table mode
void enable_FPGA_SWT_mode() {
    uint8_t en_SWT_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_EN_SWT_MODE, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(en_SWT_msg, 4);
    SPP_UART_transmit_DMA(en_SWT_msg, sizeof(en_SWT_msg));
}

void send_FPGA_SWT_steps(uint8_t steps) {
    uint8_t set_SWT_steps_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_STEPS, steps , FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_SWT_steps_msg, 5);
    writeFRAM(FRAM_SWT_STEPS, &steps, 1);
    SPP_UART_transmit_DMA(set_SWT_steps_msg, sizeof(set_SWT_steps_msg));
}

void send_FPGA_SWT_sample_rate(uint8_t sample_rate) {
    uint8_t set_SWT_sample_rate_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_SMPL_RT, sample_rate, FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_SWT_sample_rate_msg, 5);
    writeFRAM(FRAM_SWT_SMPL_RT, &sample_rate, 1);
    SPP_UART_transmit_DMA(set_SWT_sample_rate_msg, sizeof(set_SWT_sample_rate_msg));
}

void send_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id, uint8_t* voltage_level) {
    uint8_t set_SWT_vl_msg[8] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_VOL_LVL, probe_id, step_id, voltage_level[0], voltage_level[1] , FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(set_CB_vl_msg, 5)
    if (probe_id == PROBE_ID_0) {
        writeFRAM(FRAM_SWEEP_TABLE_0_START + step_id * 2, voltage_level, 2);
    } else if (probe_id == PROBE_ID_1) {
        writeFRAM(FRAM_SWEEP_TABLE_1_START + step_id * 2, voltage_level, 2);
    }
    SPP_UART_transmit_DMA(set_SWT_vl_msg, sizeof(set_SWT_vl_msg));
}


uint8_t get_FPGA_SWT_steps() {
    uint8_t get_SWT_steps_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_GET_SWT_STEPS, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(get_SWT_steps_msg, sizeof(get_SWT_steps_msg));
    SPP_UART_transmit_DMA(get_SWT_steps_msg, sizeof(get_SWT_steps_msg));
    return 0;
}

uint8_t get_FPGA_SWT_sample_rate() {
    uint8_t get_SWT_smpl_rt_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_GET_SWT_SMPL_RT, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(get_SWT_smpl_rt_msg, sizeof(get_SWT_smpl_rt_msg));
    SPP_UART_transmit_DMA(get_SWT_smpl_rt_msg, sizeof(get_SWT_smpl_rt_msg));
    return 0;
}

uint16_t get_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id) {
    uint8_t get_SWT_vl_msg[6] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_GET_SWT_VOL_LVL, probe_id, step_id, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(get_SWT_vl_msg, sizeof(get_SWT_vl_msg));
    SPP_UART_transmit_DMA(get_SWT_vl_msg, sizeof(get_SWT_vl_msg));
    return 0;
}