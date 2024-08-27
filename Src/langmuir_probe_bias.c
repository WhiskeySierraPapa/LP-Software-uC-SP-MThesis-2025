/*
 * langmuir_probe_bias.c
 *
 *  Created on: 2024. gada 31. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#include "langmuir_probe_bias.h"
#include "FPGA_UART.h"

// ADD FPGA Function ID TO BOTH THE ENUM AND ARRAY!
typedef enum {
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


const FPGA_Func_ID_t FPGA_supported_msg_IDs[] = {
    FPGA_EN_CB_MODE ,
    FPGA_SET_CB_VOL_LVL,
    FPGA_GET_CB_MODE,
    FPGA_GET_CB_VOL_LVL,
    FPGA_EN_SWT_MODE,
    FPGA_SET_SWT_VOL_LVL,
    FPGA_SET_SWT_STEPS,
    FPGA_SET_SWT_SAMPLE_SKIP,
    FPGA_SET_SWT_SAMPLES_PER_POINT,
    FPGA_SET_SWT_NPOINTS,
    FPGA_GET_SWT_MODE,
    FPGA_GET_SWT_VOL_LVL,
    FPGA_GET_SWT_STEPS,
    FPGA_GET_SWT_SAMPLE_SKIP,
    FPGA_GET_SWT_SAMPLES_PER_POINT,
    FPGA_GET_SWT_NPOINTS,
};

#define NOF_FPGA_FUNCS sizeof(FPGA_supported_msg_IDs) / sizeof(FPGA_supported_msg_IDs[0])

#define FPGA_MSG_PREMABLE_0     0xB5
#define FPGA_MSG_PREMABLE_1     0x43
#define FPGA_MSG_POSTAMBLE      0x0A

#define LANGMUIR_READBACK_PREMABLE_0    FPGA_MSG_PREMABLE_0
#define LANGMUIR_READBACK_PREMABLE_1    FPGA_MSG_PREMABLE_1
#define LANGMUIR_READBACK_POSTAMBLE     FPGA_MSG_POSTAMBLE

#define LANGMUIR_READBACK_MAX_SIZE      16

typedef enum {
    LANG_RB_PRE0,
    LANG_RB_PRE1,
    LANG_RB_ID,
    LANG_RB_DATA,
    LANG_RB_POST,
} Langmuir_Readback_State_t;

Langmuir_Readback_State_t LangmuirReadbackState = LANG_RB_PRE0;
uint8_t FPGA_byte_recv = 0xFF;
uint8_t FPGA_data_recv[LANGMUIR_READBACK_MAX_SIZE];
uint16_t rb_seq_cnt = 0;
uint16_t rb_apid = 0xABBA;

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

    FPGA_Transmit_Binary(msg, msg_cnt);
    //SPP_UART_transmit_DMA(msg, msg_cnt);

};

// Instead of receiving the data length in the readback message
// from the FPGA, we match against the received ID and find the
// predetermined size from this function. This is very ugly, probably
// should figure out a smarter way to do this.
static uint8_t get_readback_data_size(uint8_t rb_id) {
    uint8_t rb_size = 0;
    switch (rb_id) {
        // Each of these values represent the data length
        // of the readback value. For example, the mode enable
        // is represented with a single byte, while the voltage level
        // is represented with 2 bytes or a uint16_t.
        case FPGA_GET_CB_MODE:                  rb_size = 1; break;
        case FPGA_GET_CB_VOL_LVL:               rb_size = 2; break;
        case FPGA_GET_SWT_MODE:                 rb_size = 1; break;
        case FPGA_GET_SWT_VOL_LVL:              rb_size = 2; break;
        case FPGA_GET_SWT_STEPS:                rb_size = 1; break;
        case FPGA_GET_SWT_SAMPLE_SKIP:          rb_size = 2; break;
        case FPGA_GET_SWT_SAMPLES_PER_POINT:    rb_size = 2; break;
        case FPGA_GET_SWT_NPOINTS:              rb_size = 2; break;
        break;
    default:
        break;
    }
    return rb_size;
}

bool is_langmuir_func(uint8_t func_id) {
    bool result = false;
    for(uint8_t i = 0; i < NOF_FPGA_FUNCS; i++) {
        if (func_id == FPGA_supported_msg_IDs[i]) {
            result = true;
            break;
        }
    }
    return result;
}

static void send_readback_ground(uint8_t* data, uint16_t data_len) {
    SPP_header_t SPP_header = SPP_make_header(
        SPP_VERSION,
        SPP_PACKET_TYPE_TM,
        0,
        rb_apid,
        SPP_SEQUENCE_SEG_UNSEG,
        rb_seq_cnt,
        SPP_PUS_TM_HEADER_LEN_WO_SPARE + data_len + CRC_BYTE_LEN - 1
    );
    rb_seq_cnt++;
    SPP_send_TM(&SPP_header, NULL, data, data_len);
};

bool FPGA_rx_langmuir_readback(uint8_t recv_byte) {
    uint8_t data_len = 0;
    switch (LangmuirReadbackState) {
		case LANG_RB_PRE0:
			if (recv_byte == LANGMUIR_READBACK_PREMABLE_0)
				LangmuirReadbackState = LANG_RB_PRE1;

			HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
			break;
		case LANG_RB_PRE1:
			if (recv_byte == LANGMUIR_READBACK_PREMABLE_1)
				LangmuirReadbackState = LANG_RB_ID;
			else
				LangmuirReadbackState = LANG_RB_PRE0;

			HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
			break;
		case LANG_RB_ID:
            if (!is_langmuir_func(recv_byte)) {
                return false;
            }
            data_len = get_readback_data_size(recv_byte);
            if (data_len > 0) {
				LangmuirReadbackState = LANG_RB_DATA;
			    HAL_UART_Receive_DMA(&huart5, FPGA_data_recv, data_len);
            } else {
				LangmuirReadbackState = LANG_RB_PRE0;
			    HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
            }
			break;
		case LANG_RB_DATA:
            send_readback_ground(FPGA_data_recv, data_len);
            LangmuirReadbackState = LANG_RB_POST;
			HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
			break;
		case LANG_RB_POST:
			LangmuirReadbackState = LANG_RB_PRE0;
			HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
			break;
		default:
			LangmuirReadbackState = LANG_RB_PRE0;
			HAL_UART_Receive_DMA(&huart5, &FPGA_byte_recv, 1);
    }
    return true;
}

