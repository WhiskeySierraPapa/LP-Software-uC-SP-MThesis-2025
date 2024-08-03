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
    SPP_UART_transmit_DMA(en_CB_msg, 4);
}

void send_FPGA_CB_voltage_level(uint8_t voltage_level) {
    uint8_t set_CB_vl_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_CB_VOL_LVL, voltage_level , FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_CB_vl_msg, 5);
    SPP_UART_transmit_DMA(set_CB_vl_msg, 5);
}

uint8_t get_FPGA_CB_voltage_level(){
    uint8_t set_CB_vl_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_GET_CB_VOL_LVL, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(set_CB_vl_msg, 4);
    SPP_UART_transmit_DMA(set_CB_vl_msg, 4);
    //TODO: Add a way to receive response msg from FPGA.    
    return 0;
}


// Sweep Table mode
void enable_FPGA_SWT_mode() {
    uint8_t en_SWT_msg[4] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_EN_SWT_MODE, FPGA_MSG_POSTAMBLE};
    //FPGA_Transmit_Binary(en_SWT_msg, 4);
    SPP_UART_transmit_DMA(en_SWT_msg, 4);
}

void send_FPGA_SWT_steps(uint8_t steps) {
    uint8_t set_SWT_steps_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_STEPS, steps , FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_SWT_steps_msg, 5);
    SPP_UART_transmit_DMA(set_SWT_steps_msg, 5);
}

void send_FPGA_SWT_sample_rate(uint8_t sample_rate) {
    uint8_t set_SWT_sample_rate_msg[5] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_SMPL_RT, sample_rate, FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_SWT_sample_rate_msg, 5);
    SPP_UART_transmit_DMA(set_SWT_sample_rate_msg, 5);
}

void send_FPGA_SWT_voltage_level(uint8_t probe_id, uint8_t step_id, uint16_t voltage_level) {
    uint8_t set_CB_vl_msg[7] = {FPGA_MSG_PREMABLE_0, FPGA_MSG_PREMABLE_1, FPGA_SET_SWT_VOL_LVL, probe_id, step_id, voltage_level , FPGA_MSG_POSTAMBLE}; 
    //FPGA_Transmit_Binary(set_CB_vl_msg, 5);
    SPP_UART_transmit_DMA(set_CB_vl_msg, 5);
}