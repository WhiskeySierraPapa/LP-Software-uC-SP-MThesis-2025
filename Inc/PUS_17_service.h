/*
 * PUS_17_service.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"


#ifndef PUS_17_SERVICE_H_
#define PUS_17_SERVICE_H_

/* PUS_17_service */
SPP_error SPP_handle_TEST_TC(SPP_header_t* req_SPP_header, PUS_TC_header_t* req_PUS_header);

#endif /* PUS_17_SERVICE_H_ */
