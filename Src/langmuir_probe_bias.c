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

#define LANGMUIR_READBACK_MAX_SIZE      256
#define LANGMUIR_READBACK_TIMEOUT_MS    50
typedef enum {
    LANG_RB_PRE0,
    LANG_RB_PRE1,
    LANG_RB_ID,
    LANG_RB_DATA,
    LANG_RB_POST,
} Langmuir_Readback_State_t;

Langmuir_Readback_State_t LangmuirReadbackState = LANG_RB_PRE0;
uint8_t FPGA_byte_recv = 0xFF;
uint8_t FPGA_readback_msg[LANGMUIR_READBACK_MAX_SIZE];
uint16_t rb_seq_cnt = 0;
uint16_t rb_apid = 0xABBA;


static inline bool check_FPGA_msg_format(uint8_t len) {
    bool result = false;
    if (FPGA_readback_msg[0] == LANGMUIR_READBACK_PREMABLE_0) {
        if (FPGA_readback_msg[1] == LANGMUIR_READBACK_PREMABLE_1) {
            if (FPGA_readback_msg[(len - 1)] == LANGMUIR_READBACK_POSTAMBLE) {
                result = true;
            }
        }
    }
    return result;
}


static inline void clear_FPGA_rb_buf() {
    for (int i =  0; i < LANGMUIR_READBACK_MAX_SIZE; i++) {
        FPGA_readback_msg[i] = 0x00;
    }
}


static bool recieve_readback(uint8_t expected_data_len, uint8_t* output_buf) {
    clear_FPGA_rb_buf();
    uint8_t full_msg_len = 2 + expected_data_len + 1;  // Two preamble bytes and single postamble
    HAL_StatusTypeDef recv_res = HAL_UART_Receive(&huart5, FPGA_readback_msg, full_msg_len, 50);
    if (recv_res != HAL_OK) {
        return false;
    }
    if (!check_FPGA_msg_format(full_msg_len)) {
        return false;
    }
    memcpy(output_buf, FPGA_readback_msg + 2, expected_data_len); // Skip two preamble bytes // Skip two preamble bytes.
    return true;

}


static bool wait_for_readback(uint8_t* request_info, uint8_t request_info_len, uint8_t* output_buf, uint8_t expected_data_len) {
    memcpy(output_buf, request_info, request_info_len);
    bool res = recieve_readback(expected_data_len, output_buf + request_info_len);
    return res;
}



static void send_readback_ground(uint8_t* data, uint16_t data_len) {
    SPP_header_t SPP_header = SPP_make_header(
        SPP_VERSION,
        SPP_PACKET_TYPE_TM,
        0,
        rb_apid,
        SPP_SEQUENCE_SEG_UNSEG,
        rb_seq_cnt,
        data_len + CRC_BYTE_LEN - 1
    );
    rb_seq_cnt++;
    SPP_send_TM(&SPP_header, NULL, data, data_len);
};

static uint16_t get_sweep_table_address(uint8_t save_id) {
    uint16_t addr = 0x2000; // Invalid
    if (save_id == 0) {addr = FRAM_SWEEP_TABLE_0;}
    if (save_id == 1) {addr = FRAM_SWEEP_TABLE_1;}
    if (save_id == 2) {addr = FRAM_SWEEP_TABLE_2;}
    if (save_id == 3) {addr = FRAM_SWEEP_TABLE_3;}
    if (save_id == 4) {addr = FRAM_SWEEP_TABLE_4;}
    if (save_id == 5) {addr = FRAM_SWEEP_TABLE_5;}
    if (save_id == 6) {addr = FRAM_SWEEP_TABLE_6;}
    if (save_id == 7) {addr = FRAM_SWEEP_TABLE_7;}
    return addr;
}


SPP_error save_sweep_table_value_FRAM(uint8_t save_id, uint8_t step_id, uint16_t value) {
    volatile uint16_t sweep_table_address = get_sweep_table_address(save_id);
    if (sweep_table_address > FRAM_FINAL_ADDRESS) {
        return UNDEFINED_ERROR;
    }
    uint16_t FRAM_address = sweep_table_address + (step_id * 2); // step ID is 0x00 to 0xFF, but each value is 16 bits.

    volatile HAL_StatusTypeDef res = writeFRAM(FRAM_address, (uint8_t*) &value, 2);
    return SPP_OK;
};


uint16_t read_sweep_table_value_FRAM(uint8_t save_id, uint8_t step_id) {
    uint16_t value = {0};
    uint16_t sweep_table_address = get_sweep_table_address(save_id);

    if (sweep_table_address < FRAM_FINAL_ADDRESS) {
        uint16_t FRAM_address = sweep_table_address + (step_id * 2);
        HAL_Delay(10);
        volatile HAL_StatusTypeDef res = readFRAM(FRAM_address, (uint8_t*) &value, 2);
    }
    return value;
};


void send_FPGA_langmuir_msg(uint8_t func_id, uint8_t N_args, FPGA_msg_arg_t* fpgama) {
    uint8_t msg[256] = {0};
    uint8_t msg_cnt = 0;

    bool is_FPGA_readback_reqeust = false;
    uint8_t request_info[16] = {0};
    uint8_t request_info_len = 0;
    uint8_t readback_data[256] = {0};
    uint8_t readback_len = 0;

    bool save_to_FRAM = false;
    bool read_from_FRAM = false;

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
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_CB_MODE;
            is_FPGA_readback_reqeust = true;
            readback_len = 1;
            break;
        case FPGA_GET_CB_VOL_LVL:
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_CB_VOL_LVL;
            request_info[request_info_len++] = msg[msg_cnt++] = fpgama->probe_ID;
            is_FPGA_readback_reqeust = true;
            readback_len = sizeof(fpgama->voltage_level);
            break;
        case FPGA_EN_SWT_MODE:
            msg[msg_cnt++] = FPGA_EN_SWT_MODE;
            break;
        case FPGA_SET_SWT_VOL_LVL:
            if (fpgama->target == GS_FPGA_TARGET) {
                msg[msg_cnt++] = FPGA_SET_SWT_VOL_LVL;
                msg[msg_cnt++] = fpgama->probe_ID;
                msg[msg_cnt++] = fpgama->step_ID;
                msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[0];
                msg[msg_cnt++] = ((uint8_t*)(&fpgama->voltage_level))[1];
            } else if (fpgama->target == GS_FRAM_TARGET) {
                save_to_FRAM = true;
            }
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
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_MODE;
            is_FPGA_readback_reqeust = true;
            readback_len = 1;
            break;
        case FPGA_GET_SWT_VOL_LVL:
                request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_VOL_LVL;
                request_info[request_info_len++] = msg[msg_cnt++] = fpgama->probe_ID;
                request_info[request_info_len++] = msg[msg_cnt++] = fpgama->step_ID;
                readback_len = sizeof(fpgama->voltage_level);
            if (fpgama->target == GS_FPGA_TARGET) {
                is_FPGA_readback_reqeust = true;
            } else if (fpgama->target == GS_FRAM_TARGET) {
                read_from_FRAM = true;
            }
            break;
        case FPGA_GET_SWT_STEPS:
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_STEPS;
            is_FPGA_readback_reqeust = true;
            readback_len = sizeof(fpgama->N_steps);
            break;
        case FPGA_GET_SWT_SAMPLE_SKIP:
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_SAMPLE_SKIP;
            is_FPGA_readback_reqeust = true;
            readback_len = sizeof(fpgama->N_skip);
            break;
        case FPGA_GET_SWT_SAMPLES_PER_POINT:
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_SAMPLES_PER_POINT;
            is_FPGA_readback_reqeust = true;
            readback_len = sizeof(fpgama->N_f);
            break;
        case FPGA_GET_SWT_NPOINTS:
            request_info[request_info_len++] = msg[msg_cnt++] = FPGA_GET_SWT_NPOINTS;
            is_FPGA_readback_reqeust = true;
            readback_len = sizeof(fpgama->N_points);
            break;
    }

    msg[msg_cnt++] = FPGA_MSG_POSTAMBLE;

    //SPP_UART_transmit_DMA(msg, msg_cnt); return;


    if (save_to_FRAM) {
        save_sweep_table_value_FRAM(fpgama->probe_ID, fpgama->step_ID, fpgama->voltage_level);
    } else if (read_from_FRAM) {
        uint16_t value = read_sweep_table_value_FRAM(fpgama->probe_ID, fpgama->step_ID);
        memcpy(readback_data, request_info, request_info_len);
        memcpy(readback_data + request_info_len, (uint8_t*) &value, sizeof(value));
        send_readback_ground(readback_data, readback_len + request_info_len);
    } else {
        FPGA_Transmit_Binary(msg, msg_cnt);
    }


    if (is_FPGA_readback_reqeust) {
        bool success = wait_for_readback(request_info, request_info_len, readback_data, readback_len);
        if (success) {
            send_readback_ground(readback_data, request_info_len + readback_len);
        }
    }

};

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
