#include <FPGA_UART.h>

uint8_t FPGARxBuffer[FPGA_RX_BUFFER_SIZE];
uint8_t FPGATxBuffer[FPGA_TX_BUFFER_SIZE];
uint8_t FPGAMessage[FPGA_RX_BUFFER_SIZE];
uint8_t console_enabled = 0;
uint8_t FPGAReceivedMessage = 0;
char ConsoleCommand[CONSOLE_MAX_CMD_SIZE];
uint8_t ConsoleCommandReady = 0;
uint8_t cmd_cnt = 0;
uint8_t consoleSTX3SetupRequested = 0;
uint8_t FPGAFlightState = 0;
uint8_t FPGARxState = RX_STATE_PREAMBLE_1;

BYTE work[8192];		// Work buffer when formatting SD card

void FPGA_RX_CpltCallback() {
	if (console_enabled) {
		if (FPGARxBuffer[0] == 0x0D) {
			ConsoleCommand[cmd_cnt] = 0x00;
			ConsoleCommandReady = 1;
			cmd_cnt = 0;
		}
		else if (FPGARxBuffer[0] == 0x08) {
			cmd_cnt--;
			HAL_UART_Transmit_DMA(&huart5, FPGARxBuffer, 1);
			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
		}
		else {
			ConsoleCommand[cmd_cnt] = FPGARxBuffer[0];
			cmd_cnt++;
			HAL_UART_Transmit_DMA(&huart5, FPGARxBuffer, 1);
			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
		}

	}
	else {
		switch (FPGARxState) {
		case RX_STATE_PREAMBLE_1:
			if (FPGARxBuffer[0] == 0xB5)
				FPGARxState = RX_STATE_PREAMBLE_2;

			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
			break;
		case RX_STATE_PREAMBLE_2:
			if (FPGARxBuffer[0] == 0x43)
				FPGARxState = RX_STATE_MSGID;
			else
				FPGARxState = RX_STATE_PREAMBLE_1;

			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 2);
			break;
		case RX_STATE_MSGID:
			FPGAReceivedMessage = FPGARxBuffer[0];
			uint8_t L = FPGARxBuffer[1];

			if (L > 0) {
				HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, L);
				FPGARxState = RX_STATE_PAYLOAD;
			}
			else {
				HandleFPGAMessage();
				FPGARxState = RX_STATE_POSTAMBLE;
				HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
			}


			break;
		case RX_STATE_PAYLOAD:
			HandleFPGAMessage();
			FPGARxState = RX_STATE_POSTAMBLE;
			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
			break;
		case RX_STATE_POSTAMBLE:
			FPGARxState = RX_STATE_PREAMBLE_1;
			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
			break;
		default:
			FPGARxState = RX_STATE_PREAMBLE_1;
			HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
		}
	}
}

void FPGA_Transmit_DMA(const char* tx_string) {
	memcpy(FPGATxBuffer, tx_string, strlen(tx_string));
	HAL_UART_Transmit_DMA(&huart5, FPGATxBuffer, strlen(tx_string));
}

void FPGA_Transmit(const char* tx_string) {
	memcpy(FPGATxBuffer, tx_string, strlen(tx_string));
	HAL_UART_Transmit(&huart5, FPGATxBuffer, strlen(tx_string), 50);
}

void FPGA_Transmit_Binary(uint8_t* tx_data, size_t length) {
	memcpy(FPGATxBuffer, tx_data, length);
	HAL_UART_Transmit(&huart5, FPGATxBuffer, length, 50);
}

void FPGA_Transmit_Binary_DMA(uint8_t* tx_data, size_t length) {
	memcpy(FPGATxBuffer, tx_data, length);
	HAL_UART_Transmit_DMA(&huart5, FPGATxBuffer, length);
}

void HandleFPGAMessage() {
	  switch (FPGAReceivedMessage) {
	  case FPGA_MESSAGE_STATE:
		  memcpy(&FPGAFlightState, FPGARxBuffer, 1);
		  break;
	  case FPGA_MESSAGE_LED3:
		  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
		  break;
	  case FPGA_MESSAGE_LED4:
		  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
		  break;
	  case FPGA_MESSAGE_UC_CONSOLE_EN:
		  console_enabled = 1;
		  FPGA_Transmit_DMA("\n\ruC console mode activated.\n\r\n\r> ");
		  break;
	  case FPGA_MESSAGE_GYRO:
		  break;
	  case FPGA_MESSAGE_MOTOR_SPEED:
		  break;
	  case FPGA_MESSAGE_TELEMETRY:
		  SendTelemetry();
		  break;
	  }
}

void HandleConsole() {
	  char* saveptr;
	  char* cmd = strtok_r(ConsoleCommand, " ", &saveptr);
	  char* arg1 = strtok_r(NULL, " ", &saveptr);
	  char* arg2 = strtok_r(NULL, " ", &saveptr);
	  char* arg3 = strtok_r(NULL, " ", &saveptr);
	  char* arg4 = strtok_r(NULL, " ", &saveptr);
	  char* arg5 = strtok_r(NULL, " ", &saveptr);

	  if (strcmp(cmd, "help") == 0) {
		  FPGA_Transmit("\n\r\n\rThis is a help string.\n\r\n\r> ");
		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "format") == 0) {
		  FPGA_Transmit("\n\r\n\rClosing FPGA & uC data files...\n\r");
//		  FPGAFileOpen = 0;
		  uCFileOpen = 0;
//		  f_close(&FPGADataFile);
		  f_close(&uCDataFile);
		  FPGA_Transmit("Formatting SD card...\n\r");

		  FRESULT result = f_mkfs("", FM_FAT32, 32768, work, 8192);

		  if (result == FR_OK)
			  FPGA_Transmit("Success.\n\r");
		  else {
			  char str[256];
			  sprintf(str, "Failure: error %d while trying to format SD card.\n\r", result);
			  FPGA_Transmit(str);
		  }
		  FPGA_Transmit("Opening new FPGA & uC data files...\n\r\n\r> ");
		  openUCDataFile();
		  openFPGADataFile();
		  uCFileOpen = 1;
//		  FPGAFileOpen = 1;

		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "ls") == 0) {
		  FPGA_Transmit("\n\r\n\rListing files on SD card:\n\r\n\r");

		  FRESULT fr;     /* Return value */
		  DIR dj;         /* Directory search object */
		  FILINFO fno;    /* File information */
		  char str[256];

		  fr = f_findfirst(&dj, &fno, "", "*");  /* Start to search for photo files */

		  if (fr != FR_OK) {
			  sprintf(str, "Error %d reading SD card!", fr);
			  FPGA_Transmit(str);
		  }
		  else {
			  if (fno.fname[0] == 0) {
				  FPGA_Transmit("SD card is empty.");
			  }
			  else {
				  FPGA_Transmit("Size\t\tFile name\n\r");

				  while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
					  sprintf(str, "%ld\t\t%s\n\r", fno.fsize, fno.fname);                /* Display the object name */
					  FPGA_Transmit(str);
					  fr = f_findnext(&dj, &fno);               /* Search for next item */
				  }
			  }
		  }

		  f_closedir(&dj);

		  FPGA_Transmit("\n\r\n\r> ");

		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "fram") == 0) {
		  if (arg1 == 0) {
			  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
		  }
		  else if (strcmp(arg1, "get") == 0) {
			  if (arg2 == 0) {
				  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
			  }
			  else if (strcmp(arg2, "BOOT_CNT") == 0) {
				  char str[256];
				  uint16_t bootcnt = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_BOOT_CNT, (uint8_t*) &bootcnt, 2);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_BOOT_CNT) = %d\n\r\n\r>", FRAM_BOOT_CNT, bootcnt);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else if (strcmp(arg2, "FFU_ID") == 0) {
				  char str[256];
				  uint8_t val = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_FFU_ID, (uint8_t*) &val, 1);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_FFU_ID) = %d\n\r\n\r>", FRAM_FFU_ID, val);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else if (strcmp(arg2, "UNIT_ID") == 0) {
				  char str[256];
				  uint8_t val = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_UNIT_ID, (uint8_t*) &val, 1);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_UNIT_ID) = %d\n\r\n\r>", FRAM_UNIT_ID, val);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else if (strcmp(arg2, "GS_ID_FPGA") == 0) {
				  char str[256];
				  uint8_t val = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_GS_ID_FPGA, (uint8_t*) &val, 1);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_GS_ID_FPGA) = %d\n\r\n\r>", FRAM_GS_ID_FPGA, val);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else if (strcmp(arg2, "GS_ID_UC") == 0) {
				  char str[256];
				  uint8_t val = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_GS_ID_UC, (uint8_t*) &val, 1);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_GS_ID_UC) = %d\n\r\n\r>", FRAM_GS_ID_UC, val);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else if (strcmp(arg2, "VHF_TIME_SLOT") == 0) {
				  char str[256];
				  uint8_t val = 0;
				  uint8_t framresult;

				  framresult = readFRAM(FRAM_VHF_TIME_SLOT, (uint8_t*) &val, 1);

				  if (framresult == HAL_OK)
					  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_VHF_TIME_SLOT) = %d\n\r\n\r>", FRAM_VHF_TIME_SLOT, val);
				  else
					  sprintf(str, "\n\r\n\rHAL Error %d while trying to read FRAM.\n\r\n\r>", framresult);

				  FPGA_Transmit(str);
			  }
			  else {
				  FPGA_Transmit("\n\r\n\rUnknown FRAM register.\n\r\n\r> ");
			  }
		  }
		  else if (strcmp(arg1, "set") == 0) {
			  if (arg2 == 0) {
				  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
			  }
			  else if (strcmp(arg2, "BOOT_CNT") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint16_t bootcnt = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_BOOT_CNT, (uint8_t*) &bootcnt, 2);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_BOOT_CNT, (uint8_t*) &bootcnt, 2);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_BOOT_CNT) = %d\n\r\n\r>", FRAM_BOOT_CNT, bootcnt);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }

			  }
			  else if (strcmp(arg2, "FFU_ID") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint8_t val = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_FFU_ID, (uint8_t*) &val, 1);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_FFU_ID, (uint8_t*) &val, 1);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_FFU_ID) = %d\n\r\n\r>", FRAM_FFU_ID, val);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }
			  }
			  else if (strcmp(arg2, "UNIT_ID") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint8_t val = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_UNIT_ID, (uint8_t*) &val, 1);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_UNIT_ID, (uint8_t*) &val, 1);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_UNIT_ID) = %d\n\r\n\r>", FRAM_UNIT_ID, val);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }
			  }
			  else if (strcmp(arg2, "GS_ID_FPGA") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint8_t val = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_GS_ID_FPGA, (uint8_t*) &val, 1);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_GS_ID_FPGA, (uint8_t*) &val, 1);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_GS_ID_FPGA) = %d\n\r\n\r>", FRAM_GS_ID_FPGA, val);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }
			  }
			  else if (strcmp(arg2, "GS_ID_UC") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint8_t val = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_GS_ID_UC, (uint8_t*) &val, 1);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_GS_ID_UC, (uint8_t*) &val, 1);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_GS_ID_UC) = %d\n\r\n\r>", FRAM_GS_ID_UC, val);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }
			  }
			  else if (strcmp(arg2, "VHF_TIME_SLOT") == 0) {
				  if (arg3 == 0) {
					  FPGA_Transmit("\n\r\n\rWrite / read from FRAM.\n\r\n\rUsage: fram <get/set> <name> [value]\n\r\n\r> ");
				  }
				  else {
					  char str[256];
					  uint8_t val = atol(arg3);

					  uint8_t framresult;

					  framresult = writeFRAM(FRAM_VHF_TIME_SLOT, (uint8_t*) &val, 1);

					  if (framresult == HAL_OK) {
						  framresult = readFRAM(FRAM_VHF_TIME_SLOT, (uint8_t*) &val, 1);

						  if (framresult == HAL_OK)
							  sprintf(str, "\n\r\n\rAddress 0x%04X (FRAM_VHF_TIME_SLOT) = %d\n\r\n\r>", FRAM_VHF_TIME_SLOT, val);
						  else
							  sprintf(str, "\n\r\n\rWrite success but HAL Error %d while trying to read back data.\n\r\n\r>", framresult);
					  }
					  else
						  sprintf(str, "\n\r\n\rHAL Error %d while trying to write to FRAM.\n\r\n\r>", framresult);

					  FPGA_Transmit(str);
				  }
			  }
			  else {
				  FPGA_Transmit("\n\r\n\rUnknown FRAM register.\n\r\n\r> ");
			  }
		  }
		  else if (strcmp(arg1, "list") == 0) {
			  char str[256];

			  uint16_t bootcnt = 0;
			  uint8_t ffuid = 0;
			  uint8_t unitid = 0;
			  uint8_t gsidfpga = 0;
			  uint8_t gsiduc = 0;
			  uint8_t vhf = 0;

			  readFRAM(FRAM_BOOT_CNT, (uint8_t*) &bootcnt, 2);
			  readFRAM(FRAM_FFU_ID, (uint8_t*) &ffuid, 1);
			  readFRAM(FRAM_UNIT_ID, (uint8_t*) &unitid, 1);
			  readFRAM(FRAM_GS_ID_FPGA, (uint8_t*) &gsidfpga, 1);
			  readFRAM(FRAM_GS_ID_UC, (uint8_t*) &gsiduc, 1);
			  readFRAM(FRAM_VHF_TIME_SLOT, (uint8_t*) &vhf, 1);

			  sprintf(str, "\n\r\n\rFRAM registers:\r\n\r\n"
					       "0x%04X\tBOOT_CNT\t%d\tuint16_t\n\r"
					  	   "0x%04X\tFFU_ID\t\t%d\tuint8_t\n\r"
					  	   "0x%04X\tUNIT_ID\t\t%d\tuint8_t\n\r"
					   	   "0x%04X\tGS_ID_FPGA\t%d\tuint8_t\n\r"
					   	   "0x%04X\tGS_ID_UC\t%d\tuint8_t\n\r"
					  	   "0x%04X\tVHF_TIME_SLOT\t%d\tuint8_t\n\r"
					       "\n\r\n\r>",
						   FRAM_BOOT_CNT, bootcnt,
						   FRAM_FFU_ID, ffuid,
						   FRAM_UNIT_ID, unitid,
						   FRAM_GS_ID_FPGA, gsidfpga,
						   FRAM_GS_ID_UC, gsiduc,
						   FRAM_VHF_TIME_SLOT, vhf);

			  FPGA_Transmit(str);
		  }

		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "status") == 0) {
		  char str[256];

		  float s_vbat = vbat;
		  float s_temp = temperature;
		  float s_fpga3v = fpga3v;
		  float s_fpga1p5v = fpga1p5v;
		  float s_uc3v = uc3v;

		  uint16_t vbat_int = floor(s_vbat);
		  uint16_t temperature_int = floor(s_temp);
		  uint16_t fpga3v_int = floor(s_fpga3v);
		  uint16_t fpga1p5v_int = floor(s_fpga1p5v);
		  uint16_t uc3v_int = floor(s_uc3v);

		  uint16_t vbat_dec = (s_vbat - vbat_int) * 1000;
		  uint16_t temperature_dec = (s_temp - temperature_int) * 10;
		  uint16_t fpga3v_dec = (s_fpga3v - fpga3v_int) * 1000;
		  uint16_t fpga1p5v_dec = (s_fpga1p5v - fpga1p5v_int) * 1000;
		  uint16_t uc3v_dec = (s_uc3v - uc3v_int) * 1000;

		  sprintf(str, "\n\r\n\rTemperature:\t\t%d.%d deg C\n\rBattery voltage:\t%d.%03d V"
				  	   "\n\rFPGA IO voltage:\t%d.%03d V\n\rFPGA core voltage:\t%d.%03d V\n\ruC & sensor voltage:\t%d.%03d V\n\r\n\r>",
					   temperature_int, temperature_dec, vbat_int, vbat_dec, fpga3v_int, fpga3v_dec, fpga1p5v_int, fpga1p5v_dec,
					   uc3v_int, uc3v_dec);

		  FPGA_Transmit(str);
		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "spp_message") == 0 ) {
		  if (arg1 == 0) {
			  FPGA_Transmit("\n\r\n\rMissing message\n\r\n\r>");
		  } else {
		  	  // This will be used to simulate sending spp message to datahub
			  // spp <HEX String of message> <Hex String message length (number of letters NOT BYTES)>
			  //FPGA_Transmit("\n\r\n\rMessage received!\n\r\n\r>");
		  	  uint32_t hex_str_len = strlen(arg1); 
		  	  uint8_t data[256];
		  	  //uint8_t response[1024];
		  	  char hex[2];
		  	  for(int i = 0; i < hex_str_len; i += 2) {
			  	  memcpy(hex, arg1 + i, 2); // copy two letters to hex char array
			  	  data[i/2] = (uint8_t)strtol(hex, NULL, 16); // convert hexcstring to number
		  	  }
		  

		  	  //SPP_handle_incoming_packet(data, response);
			  //response[1023] = 0x00;
		  	  FPGA_Transmit("\n\r\n\r");
			  FPGA_Transmit(arg1);
		  	  FPGA_Transmit("\n\r\n\r");
		  }
		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }
	  else if (strcmp(cmd, "exit") == 0) {
		  FPGA_Transmit("\n\r\n\rGood bye.\n\r\n\r ");
		  osDelay(3);
		  console_enabled = 0;
		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 4);
	  }
	  else {
		  FPGA_Transmit("\n\r\n\rUnknown command.\n\r\n\r> ");
		  HAL_UART_Receive_DMA(&huart5, FPGARxBuffer, 1);
	  }

	  ConsoleCommandReady = 0;
}
