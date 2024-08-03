/*
 * device_state.h
 *
 *  Created on: 2024. gada 13. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */

#ifndef DEVICE_STATE_H_
#define DEVICE_STATE_H_


typedef enum {
    NORMAL_MODE = 1,
    IDLE_MODE   = 2,
    OFF_MODE    = 3,
    UPDATE_MODE = 4,
} DeviceState;

extern DeviceState Current_Global_Device_State;

void set_device_state(DeviceState state);

#endif /* DEVICE_STATE_H_ */
