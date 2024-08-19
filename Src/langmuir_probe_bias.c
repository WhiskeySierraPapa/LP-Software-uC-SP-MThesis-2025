/*
 * langmuir_probe_bias.c
 *
 *  Created on: 2024. gada 31. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#include "langmuir_probe_bias.h"
#include "FPGA_UART.h"


void send_FPGA_langmuir_msg(uint8_t func_id, uint8_t N_args, FPGA_msg_arg_t* fpgama) {
    // TODO: Fill result and result_len fields in fpgama in GET function.
    uint8_t msg[256] = {0};
    uint8_t msg_cnt = 0;

    msg[msg_cnt++] = FPGA_MSG_PREMABLE_0;
    msg[msg_cnt++] = FPGA_MSG_PREMABLE_1;

    switch (func_id) {
        case FPGA_EN_CB_MODE:
            msg[msg_cnt++] = FPGA_EN_CB_MODE;
            break;
        case FPGA_SET_CB_VOL_LVL:
            msg[msg_cnt++] = FPGA_SET_CB_VOL_LVL;
            msg[msg_cnt++] = fpgama->probe_ID;
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[0];
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[1];
            break;
        case FPGA_GET_CB_MODE:
            msg[msg_cnt++] = FPGA_GET_CB_MODE;
            break;
        case FPGA_GET_CB_VOL_LVL:
            msg[msg_cnt++] = FPGA_GET_CB_VOL_LVL;
            msg[msg_cnt++] = fpgama->probe_ID;
            break;
        case FPGA_EN_SWT_MODE:
            msg[msg_cnt++] = FPGA_EN_SWT_MODE;
            break;
        case FPGA_SET_SWT_VOL_LVL:
            msg[msg_cnt++] = FPGA_SET_SWT_VOL_LVL;
            msg[msg_cnt++] = fpgama->probe_ID;
            msg[msg_cnt++] = fpgama->step_ID;
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[0];
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[1];
            break;
        case FPGA_SET_SWT_STEPS:
            msg[msg_cnt++] = FPGA_SET_SWT_STEPS;
            msg[msg_cnt++] = fpgama->N_steps;
            break;
        case FPGA_SET_SWT_SAMPLE_SKIP:
            msg[msg_cnt++] = FPGA_SET_SWT_SAMPLE_SKIP;
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_skip))[0];
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_skip))[1];
            break;
        case FPGA_SET_SWT_SAMPLES_PER_POINT:
            msg[msg_cnt++] = FPGA_SET_SWT_SAMPLES_PER_POINT;
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_f))[0];
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_f))[1];
            break;
        case FPGA_SET_SWT_NPOINTS:
            msg[msg_cnt++] = FPGA_SET_SWT_NPOINTS;
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_points))[0];
            msg[msg_cnt++] = ((uint8_t*)(&fpgama->N_points))[1];
            break;
        case FPGA_GET_SWT_MODE:
            msg[msg_cnt++] = FPGA_GET_SWT_MODE;
            break;
        case FPGA_GET_SWT_VOL_LVL:
            msg[msg_cnt++] = FPGA_GET_SWT_VOL_LVL;
            msg[msg_cnt++] = fpgama->probe_ID;
            msg[msg_cnt++] = fpgama->step_ID;
            break;
        case FPGA_GET_SWT_STEPS:
            msg[msg_cnt++] = FPGA_GET_SWT_STEPS;
            break;
        case FPGA_GET_SWT_SAMPLE_SKIP:
            msg[msg_cnt++] = FPGA_GET_SWT_SAMPLE_SKIP;
            break;
        case FPGA_GET_SWT_SAMPLES_PER_POINT:
            msg[msg_cnt++] = FPGA_GET_SWT_SAMPLES_PER_POINT;
            break;
        case FPGA_GET_SWT_NPOINTS:
            msg[msg_cnt++] = FPGA_GET_SWT_NPOINTS;
            break;
    }

    msg[msg_cnt++] = FPGA_MSG_POSTAMBLE;

    //FPGA_Transmit_Binary(msg, msg_cnt);
    SPP_UART_transmit_DMA(msg, msg_cnt);

};