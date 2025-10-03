/*
 * PUS_3_service.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sergi
 */
#include "Space_Packet_Protocol.h"
#include "PUS.h"
#include "General_Functions.h"
#include "PUS_1_service.h"

#ifndef PUS_3_SERVICE_H_
#define PUS_3_SERVICE_H_

#define PUS_3_MAX_DATA_LEN	6
#define MAX_PAR_COUNT       16
#define MAX_STRUCT_COUNT    16
#define MAX_TM_DATA_LEN     (MAX_PAR_COUNT * 4) + 2 // Each parameter is potentialy 4 bytes and struct id is 2 bytes.
#define DEF_COL_INTV        500
#define DEF_UC_N1           3
#define DEF_UC_PS           0

#define DEF_FPGA_N1         2
#define DEF_FPGA_PS         0

#define FPGA_MSG_PREMABLE_0     0xB5
#define FPGA_MSG_PREMABLE_1     0x43
#define FPGA_MSG_POSTAMBLE      0x0A


typedef enum {
    HK_ID_ACCELEROMETER   = 0x0000,
    HK_ID_MAGNETOMETER    = 0x0001,
    HK_ID_GYRO            = 0x0002,
    HK_ID_PRESSURE        = 0x0003
} HK_SID;

typedef enum {
	FPGA_GET_SENSOR_DATA			= 0xF9, 
    FPGA_SET_PERIOD                 = 0xF2,
} PUS_3_Func_ID;

typedef struct {
	SPP_header_t 	SPP_header;
	PUS_TC_header_t PUS_TC_header;
	uint8_t data[PUS_3_MAX_DATA_LEN];
	uint8_t data_size;
	uint8_t new_report_frequency;
} PUS_3_msg;

/* PUS_3_service */
TM_Err_Codes PUS_3_set_report_frequency(uint8_t* data, PUS_3_msg* pus3_msg_received);
TM_Err_Codes PUS_3_handle_HK_TC(SPP_header_t* primary_header, PUS_TC_header_t* secondary_header, uint8_t* data, uint8_t data_size);

#endif /* PUS_3_SERVICE_H_ */
