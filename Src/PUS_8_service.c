/*
 * PUS_8_service.c
 *
 *  Created on: 2024. gada 11. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "Space_Packet_Protocol.h"
#include "device_state.h"
#include "langmuir_probe_bias.h"

typedef enum {
    EN_PROBE_CONST_BIAS_MODE                 = 0x0010,
    SET_CONSTANT_BIAS_VOLTAGE                = 0x0011,
    GET_CURRENT_CONSTANT_BIAS_VALUE          = 0x0012,

    EN_PROBE_SWEEP_BIAS_MODE                 = 0x0020,
    SET_NOF_STEPS_SWEEP_MODE                 = 0x0021,
    SET_SAMPLE_RATE_SWEEP_MODE               = 0x0022,
    SET_VOLTAGE_LEVEL_SWEEP_MODE             = 0x0023,
    GET_NOF_STEPS_SWEEP_MODE                 = 0x0024,
    GET_SAMPLE_RATE_SWEEP_MODE               = 0x0025,
    GET_VOLTAGE_LEVEL_SWEEP_MODE             = 0x0026,

    SET_DEV_STATE_NORMAL                     = 0x0100,
    SET_DEV_STATE_IDLE                       = 0x0101,
 //   SET_DEV_STATE_OFF                        = 0x0102,
    SET_DEV_STATE_REBOOT                     = 0x0103,
    SET_DEV_STATE_UPDATE                     = 0x0104,
    SET_DEV_STATE_SWAP_IMAGE                 = 0x0105,
} Function_ID_t;


SPP_error perform_function(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_TC_h , uint8_t* data) {
    SPP_error err = SPP_OK;

    uint16_t func_id = 0;
    memcpy(&func_id, data, sizeof(func_id));
    data += sizeof(func_id);


    switch (func_id) {
        case EN_PROBE_CONST_BIAS_MODE:
            enable_FPGA_CB_mode();
            break;

        case SET_CONSTANT_BIAS_VOLTAGE:
        {
            // Get value, then increment.
            uint8_t N = *data++;
            uint8_t probe_id = 0xFF;
            uint8_t voltage_level[2] = {0, 0};

            for(int i = 0; i < N; i++) {
                uint8_t arg_id = *data++;
                if (arg_id == CONST_BIAS_PROBE_ARG_ID) {
                    probe_id = *data++;
                } else if (arg_id == CONST_BIAS_VOL_LVL_ARG_ID) {
                    memcpy(&voltage_level, data, sizeof(voltage_level));
                    data += sizeof(voltage_level);
                } else {
                    data++;
                    err = SPP_PUS8_ERROR;
                    send_fail_prog(SPP_h, PUS_TC_h);
                }
            } 
            if ((probe_id == PROBE_ID_0 || probe_id == PROBE_ID_1) && (err == SPP_OK)) {
                send_succ_prog(SPP_h, PUS_TC_h);
                send_FPGA_CB_voltage_level(probe_id, voltage_level);
            }
            break;
        }

        case GET_CURRENT_CONSTANT_BIAS_VALUE:
        {
            uint8_t N = *data++;
            uint8_t arg_id = *data++;
            uint8_t probe_id = *data++;
            uint8_t current_CB_voltage = get_FPGA_CB_voltage_level(probe_id);
            break;
        }

        case EN_PROBE_SWEEP_BIAS_MODE:
            enable_FPGA_SWT_mode();
            break;

        case SET_NOF_STEPS_SWEEP_MODE:
        {
            uint8_t N = *data++;
            uint8_t arg_id = *data++;
            uint8_t steps = *data++;
            send_FPGA_SWT_steps(steps);
            break;
        }

        case SET_SAMPLE_RATE_SWEEP_MODE:
        {
            uint8_t N = *data++;
            uint8_t arg_id = *data++;
            uint8_t sample_rate = *data++;
            send_FPGA_SWT_sample_rate(sample_rate);
            break;
        }

        case SET_VOLTAGE_LEVEL_SWEEP_MODE:
        {
            uint8_t N = *data++;
            uint8_t arg_id = 0;
            uint8_t probe_id = 0xFF;
            uint8_t step_id  = 0xFF;
            uint8_t voltage_level[2] = {0,0};

            for(int i = 0; i < N; i++) {
                arg_id = *data++;
                if (arg_id == SWEEP_PROBE_ARG_ID) {
                    probe_id = *data++;
                } else if (arg_id == SWEEP_STEPS_ARG_ID) {
                    step_id = *data++;
                } else if (arg_id == SWEEP_VOL_LVL_ARG_ID) {
                    memcpy(voltage_level, data, sizeof(voltage_level));
                    data += sizeof(voltage_level);
                } else {
                    // Unknown argument id
                    data++;
                    err = SPP_PUS8_ERROR;
                    send_fail_prog(SPP_h, PUS_TC_h);
                }
            }
            if ((probe_id == PROBE_ID_0 || probe_id == PROBE_ID_1) && (err == SPP_OK)) {
                send_succ_prog(SPP_h, PUS_TC_h);
                send_FPGA_SWT_voltage_level(probe_id, step_id, voltage_level);
            }
            break;
        }
        
        case GET_NOF_STEPS_SWEEP_MODE:
            get_FPGA_SWT_steps();
            break;
        
        case GET_SAMPLE_RATE_SWEEP_MODE:
            get_FPGA_SWT_sample_rate();
            break;
        
        case GET_VOLTAGE_LEVEL_SWEEP_MODE:
        {
            uint8_t N = *data++;
            uint8_t arg_id = 0;
            uint8_t probe_id = 0xFF;
            uint8_t step_id  = 0xFF;

            for(int i = 0; i < N; i++) {
                arg_id = *data++;
                if (arg_id == SWEEP_PROBE_ARG_ID) {
                    probe_id = *data++;
                } else if (arg_id == SWEEP_STEPS_ARG_ID) {
                    step_id = *data++;
                } else {
                    // Unknown argument id
                    data++;
                    err = SPP_PUS8_ERROR;
                    send_fail_prog(SPP_h, PUS_TC_h);
                }
            }
            if ((probe_id == PROBE_ID_0 || probe_id == PROBE_ID_1) && (err == SPP_OK)) {
                send_succ_prog(SPP_h, PUS_TC_h);
                get_FPGA_SWT_voltage_level(probe_id, step_id);
            }
            break;
        }
            break;

        case SET_DEV_STATE_NORMAL:
        	set_device_state(NORMAL_MODE);
            break;

        case SET_DEV_STATE_IDLE:
        	set_device_state(IDLE_MODE);
            break;

        case SET_DEV_STATE_REBOOT:
            break;

        case SET_DEV_STATE_UPDATE:
        	set_device_state(UPDATE_MODE);
            break;

        case SET_DEV_STATE_SWAP_IMAGE:
            break;

        default:
            err = SPP_PUS8_ERROR;
            break;
    }
    return err;
}


// Function Management PUS service 8
SPP_error SPP_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* secondary_header, uint8_t* data) {
    SPP_error err = SPP_OK;
    if (secondary_header == NULL) {
        return SPP_MISSING_PUS_HEADER;
    }

    if (secondary_header->message_subtype_id == FM_PERFORM_FUNCTION) {
        send_succ_acc(SPP_header, secondary_header);
        err = perform_function(SPP_header, secondary_header, data);
    } else {
        send_fail_acc(SPP_header, secondary_header);
        err = SPP_UNHANDLED_PUS_ID;
    }

    return err;
}
