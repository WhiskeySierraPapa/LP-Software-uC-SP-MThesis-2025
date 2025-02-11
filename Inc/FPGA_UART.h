#ifndef __FPGA_RX_H
#define __FPGA_RX_H

#include "main.h"
#include "cmsis_os.h"
#include "FRAM.h"
#include "uC_Data_Saving.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fatfs.h>

#define RX_STATE_PREAMBLE_1 1
#define RX_STATE_PREAMBLE_2 2
#define RX_STATE_MSGID 3
#define RX_STATE_LENGTH 4
#define RX_STATE_PAYLOAD 5
#define RX_STATE_POSTAMBLE 6

#define FPGA_RX_BUFFER_SIZE 64
#define FPGA_TX_BUFFER_SIZE 2048
#define FPGA_MESSAGE_LED3 0x41
#define FPGA_MESSAGE_LED4 0x42
#define FPGA_MESSAGE_STATE 0x53
#define FPGA_MESSAGE_UC_CONSOLE_EN 0x43
#define FPGA_MESSAGE_GYRO 0x47
#define FPGA_MESSAGE_MOTOR_SPEED 0x4D
#define FPGA_MESSAGE_TELEMETRY 0xA4
#define CONSOLE_MAX_CMD_SIZE 256

#define STATE_BOOT 1
#define STATE_IDLE 2
#define STATE_INSIDE_ROCKET 3
#define STATE_FREEFALL 4
#define STATE_CUTTER 5
#define STATE_PARACHUTE 6
#define STATE_LANDED 7
#define STATE_POWER_SAVE 8
#define STATE_DEBUG 9

extern UART_HandleTypeDef huart5;

extern uint8_t FPGAFlightState;

extern uint8_t FPGARxBuffer[FPGA_RX_BUFFER_SIZE];
extern uint8_t FPGATxBuffer[FPGA_TX_BUFFER_SIZE];
extern uint8_t FPGAMessage[FPGA_RX_BUFFER_SIZE];
extern uint8_t console_enabled;
extern uint8_t FPGAReceivedMessage;
extern char ConsoleCommand[CONSOLE_MAX_CMD_SIZE];
extern uint8_t ConsoleCommandReady;
extern uint8_t cmd_cnt;
extern uint8_t consoleSTX3SetupRequested;

extern float temperature;
extern float uc3v;
extern float fpga3v;
extern float fpga1p5v;
extern float vbat;

void FPGA_RX_CpltCallback();
void FPGA_Transmit_DMA(const char* tx_string);
void FPGA_Transmit(const char* tx_string);
void FPGA_Transmit_Binary(uint8_t* tx_data, size_t length);
void FPGA_Transmit_Binary_DMA(uint8_t* tx_data, size_t length);
void HandleFPGAMessage();
void HandleConsole();

#endif
