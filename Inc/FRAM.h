#define FRAM_I2C_ADDR 0xA0

#define FRAM_BOOT_CNT 0x0000
#define FRAM_FFU_ID 0x0002
#define FRAM_UNIT_ID 0x0003
#define FRAM_GS_ID_FPGA 0x0004
#define FRAM_GS_ID_UC 0x0005
#define FRAM_VHF_TIME_SLOT 0x0006


#include "main.h"

extern I2C_HandleTypeDef hi2c4;

HAL_StatusTypeDef writeFRAM_DMA(uint16_t addr, uint8_t* data, uint32_t size);
HAL_StatusTypeDef readFRAM_DMA(uint16_t addr, uint8_t* buf, uint32_t size);
HAL_StatusTypeDef writeFRAM(uint16_t addr, uint8_t* data, uint32_t size);
HAL_StatusTypeDef readFRAM(uint16_t addr, uint8_t* buf, uint32_t size);
