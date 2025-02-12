/*
 * PUS_3_service.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "PUS_1_service.h"

#ifndef PUS_3_SERVICE_H_
#define PUS_3_SERVICE_H_


/* PUS_3_service */
SPP_error PUS_3_handle_HK_TC(SPP_header_t* primary_header, PUS_TC_header_t* secondary_header, uint8_t* data);
void PUS_3_collect_HK_data(uint32_t current_ticks);
void PUS_3_HK_send();

#endif /* PUS_3_SERVICE_H_ */
