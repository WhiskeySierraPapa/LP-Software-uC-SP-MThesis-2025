/*
 * PUS_8_servie.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"

#ifndef PUS_8_SERVIE_H_
#define PUS_8_SERVIE_H_

#define PUS_8_MAX_DATA_LEN 20

// Function Management [8] subtype IDs
typedef enum {
    FM_PERFORM_FUNCTION                    = 1,  // TC
} PUS_FM_Subtype_ID;

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
	uint8_t data[PUS_8_MAX_DATA_LEN];
} PUS_8_msg;

/* PUS_8_service */
SPP_error PUS_8_handle_FM_TC(SPP_header_t* SPP_header , PUS_TC_header_t* secondary_header, uint8_t* data);

#endif /* PUS_8_SERVIE_H_ */
