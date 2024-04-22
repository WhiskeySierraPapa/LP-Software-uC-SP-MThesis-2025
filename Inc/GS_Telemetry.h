#include "main.h"

#include "FPGA_UART.h"
#include "FPGA_Data_saving.h"

extern uint8_t uC_GS_ID;

void bytesToASCII(uint8_t* data, uint8_t* ascii_data, size_t data_length);	// ascii_data must be length*2
void SendTelemetry();
