#include "FRAM.h"
#include "Device_State.h"

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
	return HAL_I2C_Mem_Read(&hi2c4, FRAM_I2C_ADDR_READ, addr, 2, buf, size, 50);
}

static uint16_t get_sweep_table_address(uint8_t save_id) {
    uint16_t addr = 0x2000; // Invalid
    if (save_id == 0) {addr = FRAM_SWEEP_TABLE_0;}
    if (save_id == 1) {addr = FRAM_SWEEP_TABLE_1;}
    if (save_id == 2) {addr = FRAM_SWEEP_TABLE_2;}
    if (save_id == 3) {addr = FRAM_SWEEP_TABLE_3;}
    if (save_id == 4) {addr = FRAM_SWEEP_TABLE_4;}
    if (save_id == 5) {addr = FRAM_SWEEP_TABLE_5;}
    if (save_id == 6) {addr = FRAM_SWEEP_TABLE_6;}
    return addr;
}

SPP_error save_sweep_table_value_FRAM(uint8_t table_id, uint8_t step_id, uint16_t value) {
    if (table_id > 6) { // Table IDs 0-7
        // TODO Add error generation here. (PUS1)
        return UNDEFINED_ERROR; // TODO Changes this to something unique.
    }
    uint16_t sweep_table_address = get_sweep_table_address(table_id);
    uint16_t FRAM_address = sweep_table_address + (step_id * 2); // step ID is 0x00 to 0xFF, but each value is 16 bits.

    writeFRAM(FRAM_address, (uint8_t*) &value, 2);
    return SPP_OK;
}


uint16_t read_sweep_table_value_FRAM(uint8_t table_id, uint8_t step_id) {
    if (table_id > 6) { // Table IDs 0-7
        // TODO Add error generation here. (PUS1)
        return 0xFFFF;
    }
    uint16_t value = {0x0000};
    uint16_t sweep_table_address = get_sweep_table_address(table_id);

    uint16_t FRAM_address = sweep_table_address + (step_id * 2);
    readFRAM(FRAM_address, (uint8_t*) &value, 2);

    return value;
}

void unpack_metadata(Metadata_Struct* metadata, uint8_t* metadata_raw)
{
	metadata->crc = ((uint16_t)metadata_raw[0] << 8) | metadata_raw[1];
	metadata->new_metadata = metadata_raw[2];
	metadata->boot_feedback = metadata_raw[3];
	metadata->image_index = metadata_raw[4];
	metadata->boot_counter = metadata_raw[5];
	metadata->error_code = metadata_raw[6];
}

void pack_metadata(Metadata_Struct* metadata, uint8_t* metadata_raw)
{
    metadata_raw[2] = metadata->new_metadata;
    metadata_raw[3] = metadata->boot_feedback;
    metadata_raw[4] = metadata->image_index;
    metadata_raw[5] = metadata->boot_counter;
    metadata_raw[6] = metadata->error_code;
}
