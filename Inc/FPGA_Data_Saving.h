#include "main.h"
#include <fatfs.h>
#include <stdio.h>
#include <string.h>

#define SD_BUFFERS 6
#define SD_BUFFER_SIZE 65536
#define FPGA_BUFFER_SIZE 2048
#define WRITE_QUEUE_MAX_SIZE SD_BUFFERS
#define BUFFERS_BEFORE_FLUSH 1

extern SRAM_HandleTypeDef hsram1;

extern uint8_t TRANSFERS_BEFORE_SWITCH;
extern uint8_t SD_buffer_1[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_2[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_3[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_4[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_5[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_6[SD_BUFFER_SIZE];
extern uint8_t SD_buffer_selection;
extern uint8_t SD_buffer_counter;
extern uint8_t bufferToWrite;
extern uint32_t fileWrites;

extern uint8_t writeQueue[WRITE_QUEUE_MAX_SIZE];
extern uint8_t writeQueueSize;
extern int8_t writeQueueFirst;
extern int8_t writeQueueLast;

extern uint16_t currentDataRate;

extern FIL FPGADataFile;
extern uint8_t FPGAFileOpen;

FRESULT openFPGADataFile();
void FPGADMATransferCplt();
void FPGAStartDMATransfer();
uint8_t writeBuffer(uint8_t bufferNumber);
void writeQueueEnqueue(uint8_t bufferno);
uint8_t writeQueueDequeue(void);
uint8_t writeQueuePeek(void);
void HandleFPGAStream();
