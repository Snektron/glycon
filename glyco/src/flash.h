#ifndef _GLYCO_FLASH_H
#define _GLYCO_FLASH_H

#include <stdint.h>

void flash_byte_program(uint16_t address, uint8_t data);

void flash_get_software_id(uint8_t* mfg, uint8_t* dev);

#endif
