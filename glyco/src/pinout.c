#include "pinout.h"

void pinout_set_data_ddr(enum pin_direction direction) {
    PINOUT_DATA_DDR = -(int) direction;
}

void pinout_set_addr_ddr(enum pin_direction direction) {
    PINOUT_ADDR_HI_DDR = -(int) direction;
    PINOUT_ADDR_LO_DDR = -(int) direction;
}

void pinout_write_data(uint8_t data) {
    PINOUT_DATA_PORT = data;
}

uint8_t pinout_read_data(void) {
    return PINOUT_DATA_PIN;
}

void pinout_write_addr(uint16_t addr) {
    PINOUT_ADDR_HI_PORT = addr >> 8;
    PINOUT_ADDR_LO_PORT = addr & 0xFF;
}

uint16_t pinout_read_addr(void) {
    return (PINOUT_ADDR_HI_PIN << 8) | PINOUT_ADDR_LO_PIN;
}
