#include "FRAM.h"

HAL_StatusTypeDef writeFRAM_DMA(uint16_t addr, uint8_t* data, uint32_t size) {
	return HAL_I2C_Mem_Write_DMA(&hi2c4, FRAM_I2C_ADDR, addr, 2, data, size);
}

HAL_StatusTypeDef readFRAM_DMA(uint16_t addr, uint8_t* buf, uint32_t size) {
	return HAL_I2C_Mem_Read_DMA(&hi2c4, FRAM_I2C_ADDR, addr, 2, buf, size);
}

HAL_StatusTypeDef writeFRAM(uint16_t addr, uint8_t* data, uint32_t size) {
	return HAL_I2C_Mem_Write(&hi2c4, FRAM_I2C_ADDR, addr, 2, data, size, 50);
}

HAL_StatusTypeDef readFRAM(uint16_t addr, uint8_t* buf, uint32_t size) {
	return HAL_I2C_Mem_Read(&hi2c4, FRAM_I2C_ADDR, addr, 2, buf, size, 50);
}
