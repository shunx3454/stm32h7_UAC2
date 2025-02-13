#ifndef __BSP_SW_I2C_H_
#define __BSP_SW_I2C_H_

#include <stdint.h>

typedef struct _sw_i2c_config_t
{
    uint16_t DevAddr;
    void (*io_init)(void);
    void (*io_clk)(uint8_t);
    void (*io_sda_out)(uint8_t);
    uint8_t (*io_sda_in)(void);
    void (*io_delay)(void);
}sw_i2c_config_t;

int8_t sw_i2c_init(sw_i2c_config_t *config);
void i2c_bus_send(int8_t bus_id, uint16_t reg_addr, uint16_t reg_addr_type, uint8_t *data, uint16_t size);
void i2c_bus_receive(int8_t bus_id, uint16_t reg_addr, uint16_t reg_addr_type, uint8_t *data, uint16_t size);

#endif