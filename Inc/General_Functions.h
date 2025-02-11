/*
 * General_Functions.h
 *
 *  Created on: Feb 11, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"

#ifndef GENERAL_FUNCTIONS_H_
#define GENERAL_FUNCTIONS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"

void Add_SPP_PUS_and_send_TM(uint16_t SPP_APP_ID,
							uint16_t PUS_SOURCE_ID,
							uint16_t SEQUENCE_COUNT,
							uint8_t* TM_data,
							uint16_t TM_data_len);

SPP_error Send_TM(SPP_header_t* resp_SPP_header,
				PUS_TM_header_t* resp_PUS_header,
				uint8_t* data,
				uint16_t data_len);

SPP_error UART_transmit(uint8_t* data, uint16_t data_len);

#endif /* GENERAL_FUNCTIONS_H_ */
