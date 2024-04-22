#include "FPGA_Data_Saving.h"

uint8_t TRANSFERS_BEFORE_SWITCH = SD_BUFFER_SIZE / FPGA_BUFFER_SIZE;
uint8_t SD_buffer_1[SD_BUFFER_SIZE];
uint8_t SD_buffer_2[SD_BUFFER_SIZE];
uint8_t SD_buffer_3[SD_BUFFER_SIZE];
uint8_t SD_buffer_4[SD_BUFFER_SIZE];
uint8_t SD_buffer_5[SD_BUFFER_SIZE];
uint8_t SD_buffer_6[SD_BUFFER_SIZE];
uint8_t SD_buffer_selection = 1;
uint8_t SD_buffer_counter = 0;
uint8_t bufferToWrite = 0;
uint32_t fileWrites = 0;

uint8_t writeQueue[WRITE_QUEUE_MAX_SIZE];
uint8_t writeQueueSize = 0;
int8_t writeQueueFirst = 0;
int8_t writeQueueLast = -1;

FIL FPGADataFile;
uint8_t FPGAFileOpen = 0;

uint32_t timeLastEnqueue = 0;
uint16_t currentDataRate = 0;

extern uint8_t ffuID;
extern uint8_t unitID;

FRESULT openFPGADataFile() {
	char unit_name[16];

	switch (unitID) {
	case 0x1A:
		memcpy(unit_name, "CU", 3);
		break;
	case 0x1B:
		memcpy(unit_name, "EMU_CONTROL", 12);
		break;
	case 0x1C:
		memcpy(unit_name, "EMU_SCIENCE", 12);
		break;
	case 0x1D:
		memcpy(unit_name, "SMILE", 6);
		break;
	default:
		memcpy(unit_name, "UNKNOWN", 8);
		break;
	}

	char file_name[64];
	uint8_t file_no = 0;

	do {
		sprintf(file_name, "/FFU%d_%s_FPGA_%d.bin", ffuID, unit_name, file_no);
		file_no++;
	} while (f_stat((const TCHAR*) file_name, (void*) 0) == FR_OK);

	printf(file_name);

	return f_open(&FPGADataFile, (const TCHAR*) file_name, FA_OPEN_ALWAYS | FA_WRITE);
}

void FPGADMATransferCplt() {
	SD_buffer_counter++;

	if (SD_buffer_counter == TRANSFERS_BEFORE_SWITCH) {
		switch (SD_buffer_selection) {
		case 1:
			SD_buffer_selection = 2;
			writeQueueEnqueue(1);
			break;
		case 2:
			SD_buffer_selection = 3;
			writeQueueEnqueue(2);
			break;
		case 3:
			SD_buffer_selection = 4;
			writeQueueEnqueue(3);
			break;
		case 4:
			SD_buffer_selection = 5;
			writeQueueEnqueue(4);
			break;
		case 5:
			SD_buffer_selection = 6;
			writeQueueEnqueue(5);
			break;
		case 6:
			SD_buffer_selection = 1;
			writeQueueEnqueue(6);
			break;
		}

		SD_buffer_counter = 0;
	}
}

void FPGAStartDMATransfer() {
	switch (SD_buffer_selection) {
	case 1:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_1 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	case 2:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_2 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	case 3:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_3 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	case 4:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_4 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	case 5:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_5 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	case 6:
		HAL_SRAM_Read_DMA(&hsram1, (uint32_t *)0x60000000, (uint32_t *)(SD_buffer_6 + SD_buffer_counter*FPGA_BUFFER_SIZE), FPGA_BUFFER_SIZE);
		break;
	}
}

uint8_t writeBuffer(uint8_t bufferNumber) {
	UINT bytesWritten = 0;

	uint8_t* chosenBuffer = 0;
	switch(bufferNumber) {
	case 1:
		chosenBuffer = SD_buffer_1;
		break;
	case 2:
		chosenBuffer = SD_buffer_2;
		break;
	case 3:
		chosenBuffer = SD_buffer_3;
		break;
	case 4:
		chosenBuffer = SD_buffer_4;
		break;
	case 5:
		chosenBuffer = SD_buffer_5;
		break;
	case 6:
		chosenBuffer = SD_buffer_6;
		break;
	}

	FRESULT result = f_write(&FPGADataFile, chosenBuffer, SD_BUFFER_SIZE, &bytesWritten);

	return result;
}

void writeQueueEnqueue(uint8_t bufferno) {
	// Measures the time it takes for a SD write buffer to fill up and calculates the data rate - should probably moved somewhere else.
	uint32_t timeThisEnqueue = xTaskGetTickCount();
	currentDataRate = SD_BUFFER_SIZE / (timeThisEnqueue - timeLastEnqueue);
	timeLastEnqueue = timeThisEnqueue;

	// Actual write queue stuff
	if (writeQueueSize < WRITE_QUEUE_MAX_SIZE) {
		if (writeQueueLast == WRITE_QUEUE_MAX_SIZE - 1)
			writeQueueLast = -1;

		writeQueue[++writeQueueLast] = bufferno;
		writeQueueSize++;
	}
}

uint8_t writeQueueDequeue(void) {
	uint8_t value;

	if (writeQueueSize != 0) {
		value = writeQueue[writeQueueFirst++];
		writeQueueSize--;
	}
	else
		value = 0;

	if (writeQueueFirst == WRITE_QUEUE_MAX_SIZE)
		writeQueueFirst = 0;

	return value;
}

uint8_t writeQueuePeek(void) {
	return writeQueue[writeQueueFirst];
}

void HandleFPGAStream() {
	  if (FPGAFileOpen) {
		  if (writeQueueSize) {
			  bufferToWrite = writeQueueDequeue();

			  uint8_t attemptsRemaining = 2;
			  FRESULT writeResult = 1;

			  while (attemptsRemaining && writeResult != FR_OK) {

				  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
				  writeResult = writeBuffer(bufferToWrite);
				  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

				  switch (writeResult) {
				  case FR_OK:
						fileWrites++;

						// Decides how often to flush data to the SD card. Important in case of for example a power loss.
						// Value is how many buffers before a flush, multiply with buffer size to get flush size.
						if (fileWrites >= BUFFERS_BEFORE_FLUSH) {
							f_sync(&FPGADataFile);
							fileWrites = 0;
						}
						break;
				  default:
					  f_close(&FPGADataFile);
					  BSP_SD_Init();

					  if (f_open(&FPGADataFile, "/FPGA.bin", FA_OPEN_APPEND | FA_WRITE) != FR_OK)
						  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);

					  attemptsRemaining--;
					  break;
				  }
			  }
		  }
	  }
}
