/*
 * PUS_8_service.c
 *
 *  Created on: 2024. gada 11. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "Space_Packet_Protocol.h"
#include "device_state.h"
#include "langmuir_probe_bias.h"




SPP_error perform_function(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_TC_h , uint8_t* data) {
    SPP_error err = SPP_OK;

    uint8_t  func_id = *data++;
    uint8_t  N_args = *data++;

    if (is_langmuir_func(func_id)) {
        FPGA_msg_arg_t fpgama =  {
            .probe_ID       = 0xFF,
            .step_ID        = 0xFF,
            .voltage_level  = 0x0000,
            .N_skip         = 0x0000,
            .N_steps        = 0x00,
            .N_f            = 0x0000, // Samples per points
            .N_points       = 0x0000,
            .result         = {0},
            .result_len     = 0,
        };

        for(int i = 0; i < N_args; i++) {
            uint8_t arg_ID = *data++;
            switch(arg_ID) {
                case PROBE_ID_ARG_ID:
                    fpgama.probe_ID = *data++;
                    break;
                case STEP_ID_ARG_ID:
                    fpgama.step_ID = *data++;
                    break;
                case VOL_LVL_ARG_ID:
                    memcpy((uint8_t*)&fpgama.voltage_level, data, sizeof(fpgama.voltage_level));
                    data += sizeof(fpgama.voltage_level);
                    break;
                case N_STEPS_ARG_ID:
                    fpgama.N_steps = *data++;
                    break;
                case N_SKIP_ARG_ID:
                    memcpy((uint8_t*)&fpgama.N_skip, data, sizeof(fpgama.N_skip));
                    data += sizeof(fpgama.N_skip);
                    break;
                case N_F_ARG_ID:
                    memcpy((uint8_t*)&fpgama.N_f, data, sizeof(fpgama.N_f));
                    data += sizeof(fpgama.N_f);
                    break;
                case N_POINTS_ARG_ID:
                    memcpy((uint8_t*)&fpgama.N_points, data, sizeof(fpgama.N_points));
                    data += sizeof(fpgama.N_points);
                    break;
                default:
                    break;
            }
        }
        send_FPGA_langmuir_msg(func_id, N_args, &fpgama);

    } else {
        switch (func_id) {
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
