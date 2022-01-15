#ifndef _COMMON_GLYCON_H
#define _COMMON_GLYCON_H

#include <stdbool.h>
#include <stdint.h>

// Glycon common constants to prevent hardcoding stuff.

#define GLYCON_ADDRSPACE_SIZE (0x10000)

#define GLYCON_RAM_MASK (0x8000)

#define GLYCON_FLASH_START (0x0000)
#define GLYCON_FLASH_END (0x8000)
#define GLYCON_FLASH_SIZE (GLYCON_FLASH_END - GLYCON_FLASH_START)

#define GLYCON_RAM_START (GLYCON_FLASH_END)
#define GLYCON_RAM_END (GLYCON_ADDRSPACE_SIZE)
#define GLYCON_RAM_SIZE (GLYCON_RAM_END - GLYCON_RAM_START)

static inline bool glycon_is_ram_addr(uint16_t address) {
    return (address & GLYCON_RAM_MASK) != 0;
}

static inline bool glycon_is_flash_addr(uint16_t address) {
    return (address & GLYCON_RAM_MASK) == 0;
}

#endif
