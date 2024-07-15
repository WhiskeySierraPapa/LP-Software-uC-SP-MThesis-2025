/*
 * device_state.c
 *
 *  Created on: 2024. gada 13. jūl.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */


#include "Space_Packet_Protocol.h"
#include "device_state.h"

DeviceState Current_Global_Device_State = NORMAL_MODE;

void set_device_state(DeviceState state) {
	Current_Global_Device_State = state;
}
