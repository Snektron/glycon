#ifndef _GLYCO_FLASH_H
#define _GLYCO_FLASH_H

#include <stdint.h>

// Write a single byte to flash memory at a particular address. The byte at the target
// address is AND-ed with this value, and so should be cleared to 0xFF before writing
// using either `flash_erase_sector` or `flash_erase_chip`.
// Requires bus acquired, see bus.h
void flash_byte_program(uint16_t address, uint8_t data);

// Read the flash chip's manufacterer- and device-id.
// Requires bus acquired, see bus.h
void flash_get_software_id(uint8_t* mfg, uint8_t* dev);

// Erase a particular flash sector, setting each byte in the target sector to 0xFF.
// Flash sectors are 0x4000 (16K) bytes in size. The sector in which `sector_address` lies
// is erased. Erasing a sector takes about 20ms.
// Requires bus acquired, see bus.h
void flash_erase_sector(uint16_t sector_address);

// Erase the entire flash chip, setting each byte to 0xFF. This operation is less
// efficient if only a few sectors need to be erased (this operation takes ~100ms).
// Requires bus acquired, see bus.h
void flash_erase_chip();

#endif
