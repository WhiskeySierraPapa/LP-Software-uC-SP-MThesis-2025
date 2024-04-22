#include "uC_Data_Saving.h"

FIL uCDataFile;

uint8_t ADCNewData = 0;
uint8_t ADCPacket[20];

uint8_t deWobble_new_packet = 0;
uint8_t dewobble_packet[20];
uint8_t dewobble_packet2[20];

extern uint8_t ffuID;
extern uint8_t unitID;

uint8_t uCFileOpen = 0;

FRESULT openUCDataFile() {
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
		sprintf(file_name, "/FFU%d_%s_UC_%d.bin", ffuID, unit_name, file_no);
		file_no++;
	} while (f_stat((const TCHAR*) file_name, (void*) 0) == FR_OK);

	printf(file_name);

	return f_open(&uCDataFile, (const TCHAR*) file_name, FA_OPEN_ALWAYS | FA_WRITE);
}
