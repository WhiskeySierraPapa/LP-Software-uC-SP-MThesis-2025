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

// Test (Ping) service [17] subtype IDS
typedef enum {
    T_ARE_YOU_ALIVE_TEST_ID              = 1, // TC
    T_ARE_YOU_ALIVE_TEST_REPORT_ID       = 2, // TM
    T_ON_BOARD_CONN_TEST_ID              = 3, // TC
    T_ON_BOARD_CONN_TEST_REPORT_ID       = 4, // TM
} PUS_T_Subtype_ID;

/* PUS_17_service */
SPP_error PUS_17_handle_TEST_TC(SPP_header_t* req_SPP_header, PUS_TC_header_t* req_PUS_header);

#endif /* PUS_17_SERVICE_H_ */
