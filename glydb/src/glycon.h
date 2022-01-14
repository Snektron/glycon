#ifndef _GLYDB_SRC_GLYCON_H
#define _GLYDB_SRC_GLYCON_H

// Glycon common constants to prevent hardcoding stuff.

#define GLYCON_ADDRSPACE_SIZE (0x10000)

#define GLYCON_RAM_MASK (0x8000)
#define glycon_is_ram_addr(x) (((x) & GLYCON_RAM_MASK) != 0)
#define glycon_is_flash_addr(x) (((x) & GLYCON_RAM_MASK) == 0)

#endif
