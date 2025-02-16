///*
// * PUS_8_service_old.c
// *
// *  Created on: Feb 16, 2025
// *      Author: sergi
// */
//
//
///*
// * PUS_8_service.c
// *
// *  Created on: 2024. gada 11. jūl.
// *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
// */
//
//
//#include "Space_Packet_Protocol.h"
//#include "PUS.h"
//#include "device_state.h"
//#include "langmuir_probe_bias.h"
//#include "PUS_1_service.h"
//#include "PUS_8_service.h"
//
//typedef enum {
//    CPY_TABLE_FRAM_TO_FPGA = 0xE0,
//} Aux_Func_ID_t;
//
//
//SPP_error perform_function(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_TC_h , uint8_t* data) {
//    SPP_error err = SPP_OK;
//
//    uint8_t  func_id = *data++;
//    uint8_t  N_args = *data++;
//
//    if (is_langmuir_func(func_id)) {
//        FPGA_msg_arg_t fpgama =  {
//            .probe_ID       = 0xFF,
//            .step_ID        = 0xFF,
//            .voltage_level  = 0x0000,
//            .N_steps        = 0x00,
//            .N_skip         = 0x0000,
//            .N_f            = 0x0000, // Samples per points
//            .N_points       = 0x0000,
//            .target     = 0xFF,
//            .N_samples_per_step = 0x0000,
//            // .result         = {0},
//            // .result_len     = 0,
//        };
//
//        for(int i = 0; i < N_args; i++) {
//            uint8_t arg_ID = *data++;
//            switch(arg_ID) {
//                case PROBE_ID_ARG_ID:
//                    fpgama.probe_ID = *data++;
//                    break;
//                case STEP_ID_ARG_ID:
//                    fpgama.step_ID = *data++;
//                    break;
//                case VOL_LVL_ARG_ID:
//                    memcpy((uint8_t*)&fpgama.voltage_level, data, sizeof(fpgama.voltage_level));
//                    data += sizeof(fpgama.voltage_level);
//                    break;
//                case N_STEPS_ARG_ID:
//                    fpgama.N_steps = *data++;
//                    break;
//                case N_SKIP_ARG_ID:
//                    memcpy((uint8_t*)&fpgama.N_skip, data, sizeof(fpgama.N_skip));
//                    data += sizeof(fpgama.N_skip);
//                    break;
//                case N_F_ARG_ID:
//                    memcpy((uint8_t*)&fpgama.N_f, data, sizeof(fpgama.N_f));
//                    data += sizeof(fpgama.N_f);
//                    break;
//                case N_POINTS_ARG_ID:
//                    memcpy((uint8_t*)&fpgama.N_points, data, sizeof(fpgama.N_points));
//                    data += sizeof(fpgama.N_points);
//                    break;
//                case GS_TARGET_ARG_ID:
//                    // "sizeof" cannot be used here as .target is an enum type whose length cannot specified(at least in <C23).
//                    // If "sizeof" is used, it returns 4, which is incorrect as the target is only a single byte value.
//                    // This bug does not cause problems if the target argument is the last one in the message but would
//                    // mess up the alignment if it is in the beginning or middle of the message.
//                    memcpy((uint8_t*)&fpgama.target, data, 1);
//                    data += 1;
//                    break;
//                case N_SAMPLES_PER_STEP_ARG_ID:
//                    memcpy((uint8_t*)&fpgama.N_samples_per_step, data, sizeof(fpgama.N_samples_per_step));
//                    data += sizeof(fpgama.N_samples_per_step);
//                    break;
//                default:
//                    break;
//            }
//        }
//        send_FPGA_langmuir_msg(func_id, &fpgama);
//        //send_succ_comp(SPP_h, PUS_TC;);
//
//    } else {
//        switch (func_id) {
//            case CPY_TABLE_FRAM_TO_FPGA:
//            {
//                uint8_t FPGA_table_id = 0xFF;
//                uint8_t FRAM_table_id = 0xFF;
//
//                for(int i = 0; i < N_args; i++) {
//                    uint8_t arg_ID = *data++;
//
//                    switch(arg_ID) {
//                        case PROBE_ID_ARG_ID:
//                            FPGA_table_id = *data++;
//                            break;
//                        case FRAM_TABLE_ID_ARG_ID:
//                            FRAM_table_id = *data++;
//                            break;
//                    }
//                }
//
//                if (FRAM_table_id != 0xFF && FPGA_table_id != 0xFF) {
//                    copy_full_sweep_table_FRAM_to_FPGA(FRAM_table_id, FPGA_table_id);
//                }
//               break;
//            }
//            case SET_DEV_STATE_NORMAL:
//            	set_device_state(NORMAL_MODE);
//                break;
//
//            case SET_DEV_STATE_IDLE:
//            	set_device_state(IDLE_MODE);
//                break;
//
//            case SET_DEV_STATE_REBOOT:
//                break;
//
//            case SET_DEV_STATE_UPDATE:
//            	set_device_state(UPDATE_MODE);
//                break;
//
//            case SET_DEV_STATE_SWAP_IMAGE:
//                break;
//
//            default:
//                err = SPP_PUS8_ERROR;
//                break;
//        }
//    }
//    return err;
//}
//
//
//// Function Management PUS service 8
//SPP_error PUS_8_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* PUS_TC_header, uint8_t* data) {
//
//	if (Current_Global_Device_State != NORMAL_MODE) {
//		return UNDEFINED_ERROR;
//	}
//	if (SPP_header == NULL || PUS_TC_header == NULL) {
//		return UNDEFINED_ERROR;
//	}
//
//	switch (PUS_TC_header->message_subtype_id) {
//		case FM_PERFORM_FUNCTION:
//			PUS_1_send_succ_acc(SPP_header, PUS_TC_header);
//			break;
//		default:
//			PUS_1_send_fail_acc(SPP_header, PUS_TC_header);
//			return SPP_UNHANDLED_PUS_ID;  // Invalid message subtype
//	}
//
//	PUS_8_msg pus8_msg_to_send;
//	pus8_msg_to_send.SPP_header = *SPP_header;
//	pus8_msg_to_send.PUS_TC_header = *PUS_TC_header;
//	memcpy(pus8_msg_to_send.data, data, PUS_8_MAX_DATA_LEN);
//
//	if (xQueueSend(PUS_8_Queue, &pus8_msg_to_send, 0) != pdPASS) {
//		PUS_1_send_fail_start(SPP_header, PUS_TC_header);
//	}
//
//    return SPP_OK;
//}
