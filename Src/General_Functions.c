/*
 * General_Functions.c
 *
 *  Created on: Feb 11, 2025
 *      Author: sergi
 */

#include "General_Functions.h"
#include "Space_Packet_Protocol.h"

extern uint16_t HK_SPP_APP_ID;
extern uint16_t HK_PUS_SOURCE_ID;
extern uint8_t OBCTxBuffer[COBS_FRAME_LEN];


SPP_error UART_transmit(uint8_t* data, uint16_t data_len) {
	*(data + data_len) = 0x00; // Adding sentinel value.
	data_len++;
    HAL_UART_Transmit(&SPP_DEBUG_UART, data, data_len, 100);
    HAL_UART_Transmit(&SPP_OBC_UART, data, data_len, 100);
    return SPP_OK;
}

SPP_error Send_TM(SPP_header_t* resp_SPP_header,
				PUS_TM_header_t* resp_PUS_header,
				uint8_t* data,
				uint16_t data_len) {

    uint8_t response_TM_packet[SPP_MAX_PACKET_LEN] = {0};
    uint8_t response_TM_packet_COBS[SPP_MAX_PACKET_LEN] = {0};
    uint16_t packet_total_len = 0;

    SPP_prepare_full_msg(resp_SPP_header,
						resp_PUS_header,
						data, data_len,
						response_TM_packet,
						&packet_total_len);

    uint16_t cobs_packet_total_len = COBS_encode(response_TM_packet,
												packet_total_len,
												response_TM_packet_COBS);

    memcpy(OBCTxBuffer, response_TM_packet_COBS, cobs_packet_total_len);
    UART_transmit(OBCTxBuffer, cobs_packet_total_len);
    return SPP_OK;
}


void Add_SPP_PUS_and_send_TM(uint16_t SPP_APP_ID,
								uint16_t PUS_SOURCE_ID,
								uint16_t SEQUENCE_COUNT,
								uint8_t* TM_data,
								uint16_t TM_data_len) {

        SPP_header_t TM_SPP_header = SPP_make_header(
            SPP_VERSION,
            SPP_PACKET_TYPE_TM,
            1,
            HK_SPP_APP_ID,
            SPP_SEQUENCE_SEG_UNSEG,
			SEQUENCE_COUNT,
            SPP_PUS_TM_HEADER_LEN_WO_SPARE + TM_data_len + CRC_BYTE_LEN - 1
        );
        PUS_TM_header_t TM_PUS_header  = PUS_make_TM_header(
            PUS_VERSION,
            0,
            HOUSEKEEPING_SERVICE_ID,
            HK_PARAMETER_REPORT,
            0,
            PUS_SOURCE_ID,
            0
        );
        Send_TM(&TM_SPP_header, &TM_PUS_header, TM_data, TM_data_len);
}
