/*
 * PUS_1_service.h
 *
 *  Created on: Feb 11, 2025
 *      Author: sergi
 */

#ifndef PUS_1_SERVICE_H_
#define PUS_1_SERVICE_H_

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
} ACK_info_structure;

/* PUS_1_service */
void PUS_1_send_succ_acc  (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_fail_acc  (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_succ_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_fail_start(SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_succ_prog (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_fail_prog (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_succ_comp (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);
void PUS_1_send_fail_comp (SPP_header_t* SPP_h, PUS_TC_header_t* PUS_h);

#endif /* PUS_1_SERVICE_H_ */
