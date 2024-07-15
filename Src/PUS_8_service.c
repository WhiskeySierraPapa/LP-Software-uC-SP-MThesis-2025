/*
 * PUS_8_service.c
 *
 *  Created on: 2024. gada 11. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "Space_Packet_Protocol.h"
#include "device_state.h"

typedef enum {
    PROBE_SWEEP_BIAS            = 2,
    PROBE_CONST_BIAS            = 3,
    SET_DEV_STATE_NORMAL        = 4,
    SET_DEV_STATE_IDLE          = 5,
    SET_DEV_STATE_OFF           = 6,
    SET_DEV_STATE_REBOOT        = 7,
    SET_DEV_STATE_UPDATE        = 8,
    SET_DEV_STATE_SWAP_IMAGE    = 9,
} Function_ID;


void perform_function(uint8_t* data) {
    uint16_t func_id = 0;
    memcpy(&func_id, data, sizeof(func_id));
    data += sizeof(func_id);

    switch (func_id) {
        case PROBE_SWEEP_BIAS:
            break;
        case PROBE_CONST_BIAS:
            break;
        case SET_DEV_STATE_NORMAL:
        	set_device_state(NORMAL_MODE);
            break;
        case SET_DEV_STATE_IDLE:
        	set_device_state(IDLE_MODE);
            break;
        case SET_DEV_STATE_OFF:
        	set_device_state(OFF_MODE);
            break;
        case SET_DEV_STATE_REBOOT:
            break;
        case SET_DEV_STATE_UPDATE:
        	set_device_state(UPDATE_MODE);
            break;
        case SET_DEV_STATE_SWAP_IMAGE:
            break;
        default:
            break;
    }
}


// Function Management PUS service 8
SPP_error SPP_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* secondary_header, uint8_t* data) {
    if (secondary_header == NULL) {
        return UNDEFINED_ERROR;
    }

    if (secondary_header->message_subtype_id == FM_PERFORM_FUNCTION) {
        perform_function(data);
    }

    return SPP_OK;
}
