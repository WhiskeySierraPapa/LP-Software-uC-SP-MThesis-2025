/*
 * PUS_8_service.c
 *
 *  Created on: 2024. gada 11. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "General_Functions.h"
#include "device_state.h"
#include "langmuir_probe_bias.h"
#include "PUS_1_service.h"
#include "PUS_8_service.h"

// This queue is used to receive info from the UART handler task
osMessageQId PUS_8_Queue;

void PUS_8_unpack_msg(uint8_t* data, PUS_8_msg_unpacked* pus8_msg_unpacked)
{
	uint8_t* 	data_interator = data;
	pus8_msg_unpacked->func_id = *data_interator++;
	pus8_msg_unpacked->N_args = *data_interator++;

	for(int i = 0; i < pus8_msg_unpacked->N_args; i++) {
		uint8_t arg_ID = *data_interator++;
		switch(arg_ID) {
			case PROBE_ID_ARG_ID:
				pus8_msg_unpacked->probe_ID = *data_interator++;
				break;
			case STEP_ID_ARG_ID:
				pus8_msg_unpacked->step_ID = *data_interator++;
				break;
			case VOL_LVL_ARG_ID:
				memcpy((uint8_t*)&pus8_msg_unpacked->voltage_level, data_interator, sizeof(pus8_msg_unpacked->voltage_level));
				data_interator += sizeof(pus8_msg_unpacked->voltage_level);
				break;
			case N_STEPS_ARG_ID:
				pus8_msg_unpacked->N_steps = *data_interator++;
				break;
			case N_SKIP_ARG_ID:
				memcpy((uint8_t*)&pus8_msg_unpacked->N_skip, data_interator, sizeof(pus8_msg_unpacked->N_skip));
				data_interator += sizeof(pus8_msg_unpacked->N_skip);
				break;
			case N_F_ARG_ID:
				memcpy((uint8_t*)&pus8_msg_unpacked->N_f, data_interator, sizeof(pus8_msg_unpacked->N_f));
				data_interator += sizeof(pus8_msg_unpacked->N_f);
				break;
			case N_POINTS_ARG_ID:
				memcpy((uint8_t*)&pus8_msg_unpacked->N_points, data_interator, sizeof(pus8_msg_unpacked->N_points));
				data_interator += sizeof(pus8_msg_unpacked->N_points);
				break;
			case GS_TARGET_ARG_ID:
				// "sizeof" cannot be used here as .target is an enum type whose length cannot specified(at least in <C23).
				// If "sizeof" is used, it returns 4, which is incorrect as the target is only a single byte value.
				// This bug does not cause problems if the target argument is the last one in the message but would
				// mess up the alignment if it is in the beginning or middle of the message.
				memcpy((uint8_t*)&pus8_msg_unpacked->target, data_interator, 1);
				data_interator += 1;
				break;
			case N_SAMPLES_PER_STEP_ARG_ID:
				memcpy((uint8_t*)&pus8_msg_unpacked->N_samples_per_step, data_interator, sizeof(pus8_msg_unpacked->N_samples_per_step));
				data_interator += sizeof(pus8_msg_unpacked->N_samples_per_step);
				break;
			default:
				break;
		}
	}
}


SPP_error PUS_8_perform_function(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_TC_h , PUS_8_msg_unpacked* pus8_msg_unpacked)
{
	switch(pus8_msg_unpacked->func_id)
	{
		case FPGA_SET_SWT_VOL_LVL:
			if(pus8_msg_unpacked->target == 1)
			{
				save_sweep_table_value_FRAM(pus8_msg_unpacked->probe_ID,
											pus8_msg_unpacked->step_ID,
											pus8_msg_unpacked->voltage_level);
			}
			break;
		case FPGA_GET_SWT_VOL_LVL:
			if(pus8_msg_unpacked->target == 1)
			{
				uint16_t step_voltage = read_sweep_table_value_FRAM(pus8_msg_unpacked->probe_ID,
																	pus8_msg_unpacked->step_ID);
				Add_SPP_PUS_and_send_TM(SPP_h->application_process_id,
										1,
										0,
										PUS_TC_h->source_id,
										FUNCTION_MANAGEMNET_ID,
										FM_PERFORM_FUNCTION,
										&step_voltage,
										2);

			}
			break;
		default:
			break;
	}

	return SPP_OK;
}

//SPP_error save_sweep_table_value_FRAM(uint8_t table_id, uint8_t step_id, uint16_t value) {
//    if (table_id > 7) { // Table IDs 0-7
//        // TODO Add error generation here. (PUS1)
//        return UNDEFINED_ERROR; // TODO Changes this to something unique.
//    }
//    uint16_t sweep_table_address = get_sweep_table_address(table_id);
//    uint16_t FRAM_address = sweep_table_address + (step_id * 2); // step ID is 0x00 to 0xFF, but each value is 16 bits.
//
//    writeFRAM(FRAM_address, (uint8_t*) &value, 2);
//    return SPP_OK;
//};


// Function Management PUS service 8
SPP_error PUS_8_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* PUS_TC_header, uint8_t* data) {

	if (Current_Global_Device_State != NORMAL_MODE) {
		return UNDEFINED_ERROR;
	}
	if (SPP_header == NULL || PUS_TC_header == NULL) {
		return UNDEFINED_ERROR;
	}

	switch (PUS_TC_header->message_subtype_id) {
		case FM_PERFORM_FUNCTION:
			PUS_1_send_succ_acc(SPP_header, PUS_TC_header);
			break;
		default:
			PUS_1_send_fail_acc(SPP_header, PUS_TC_header);
			return SPP_UNHANDLED_PUS_ID;  // Invalid message subtype
	}

	PUS_8_msg pus8_msg_to_send;
	pus8_msg_to_send.SPP_header = *SPP_header;
	pus8_msg_to_send.PUS_TC_header = *PUS_TC_header;
	memcpy(pus8_msg_to_send.data, data, PUS_8_MAX_DATA_LEN);

	if (xQueueSend(PUS_8_Queue, &pus8_msg_to_send, 0) != pdPASS) {
		PUS_1_send_fail_start(SPP_header, PUS_TC_header);
	}

    return SPP_OK;
}
