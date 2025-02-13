#include "sw_i2c.h"

#define SW_I2C_MAX_BUS 5

static uint8_t bus_num = 0;
static sw_i2c_config_t i2c_config[SW_I2C_MAX_BUS];

static void sw_i2c_start(int8_t bus_id);
static void sw_i2c_stop(int8_t bus_id);
static uint8_t sw_i2c_wait_ack(int8_t bus_id);
static void sw_i2c_ack(int8_t bus_id);
static void sw_i2c_nack(int8_t bus_id);
static void sw_i2c_send(int8_t bus_id, uint8_t byte);
static uint8_t sw_i2c_receive(int8_t bus_id, uint8_t ack);

int8_t sw_i2c_init(sw_i2c_config_t *config)
{
    uint8_t bus_id = -1;
    if (config->io_init && config->io_clk && config->io_sda_in && config->io_sda_out && config->io_delay && bus_num <= SW_I2C_MAX_BUS)
    {
        i2c_config[bus_num].DevAddr = config->DevAddr;
        i2c_config[bus_num].io_init = config->io_init;
        i2c_config[bus_num].io_clk = config->io_clk;
        i2c_config[bus_num].io_sda_in = config->io_sda_in;
        i2c_config[bus_num].io_sda_out = config->io_sda_out;
        i2c_config[bus_num].io_delay = config->io_delay;
        bus_id = bus_num;
        bus_num++;

        i2c_config[bus_id].io_init();
        sw_i2c_stop(bus_id);
        i2c_config[bus_id].io_delay();
        return bus_id;
    }
    return -1;
}

static void sw_i2c_start(int8_t bus_id)
{
    if (bus_id < 0)
        return;
    i2c_config[bus_id].io_clk(1);
    i2c_config[bus_id].io_sda_out(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_sda_out(0);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(0);
    i2c_config[bus_id].io_delay();
}

static void sw_i2c_stop(int8_t bus_id)
{
    if (bus_id < 0)
        return;

    i2c_config[bus_id].io_sda_out(0);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_sda_out(1);
    i2c_config[bus_id].io_delay();
}

static void sw_i2c_send(int8_t bus_id, uint8_t byte)
{
    if (bus_id < 0)
        return;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            i2c_config[bus_id].io_sda_out(1);
        }
        else
        {
            i2c_config[bus_id].io_sda_out(0);
        }
        i2c_config[bus_id].io_delay();
        i2c_config[bus_id].io_clk(1);
        i2c_config[bus_id].io_delay();
        i2c_config[bus_id].io_clk(0);
        i2c_config[bus_id].io_delay();
        byte <<= 1;
    }
    i2c_config[bus_id].io_sda_out(1);
}

static uint8_t sw_i2c_wait_ack(int8_t bus_id)
{
    uint8_t ack = 0;
    uint8_t try = 0;
    if (bus_id < 0)
        return 1;

    i2c_config[bus_id].io_sda_out(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(1);
    i2c_config[bus_id].io_delay();

    while (i2c_config[bus_id].io_sda_in() != 0)
    {
        try++;
        if (try > 200)
        {
            ack = 1; /* No slave ack */
            break;
        }
    }
    i2c_config[bus_id].io_clk(0);
    i2c_config[bus_id].io_delay();
    return ack;
}

static void sw_i2c_ack(int8_t bus_id)
{
    if (bus_id < 0)
        return;

    i2c_config[bus_id].io_sda_out(0);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(0);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_sda_out(1);
}

static void sw_i2c_nack(int8_t bus_id)
{
    if (bus_id < 0)
        return;

    i2c_config[bus_id].io_sda_out(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(1);
    i2c_config[bus_id].io_delay();
    i2c_config[bus_id].io_clk(0);
    i2c_config[bus_id].io_delay();
}

static uint8_t sw_i2c_receive(int8_t bus_id, uint8_t ack)
{
    uint8_t byte = 0;
    uint8_t i = 0;
    if (bus_id < 0)
        return 0;

    for (; i < 8; i++)
    {
        byte <<= 1;
        i2c_config[bus_id].io_clk(1);
        i2c_config[bus_id].io_delay();
        if (i2c_config[bus_id].io_sda_in())
        {
            byte++;
        }
        i2c_config[bus_id].io_clk(0);
        i2c_config[bus_id].io_delay();
    }
    if (ack)
    {
        sw_i2c_ack(bus_id);
    }
    else
    {
        sw_i2c_nack(bus_id);
    }
    return byte;
}

void i2c_bus_send(int8_t bus_id, uint16_t reg_addr, uint16_t reg_addr_type, uint8_t *data, uint16_t size)
{
    if (bus_id < 0)
        return;
    sw_i2c_start(bus_id);
    sw_i2c_send(bus_id, i2c_config[bus_id].DevAddr);
    if (sw_i2c_wait_ack(bus_id) != 0)
    {
        sw_i2c_stop(bus_id);
        return;
    }

    if (reg_addr_type == 0)
    {
        sw_i2c_send(bus_id, (uint8_t)reg_addr);
        sw_i2c_wait_ack(bus_id);
    }
    else
    {
        sw_i2c_send(bus_id, (reg_addr >> 8));
        sw_i2c_wait_ack(bus_id);
        sw_i2c_send(bus_id, (reg_addr & 0xff));
        sw_i2c_wait_ack(bus_id);
    }
    for (uint16_t i = 0; i < size; i++)
    {
        sw_i2c_send(bus_id, data[i]);
        sw_i2c_wait_ack(bus_id);
    }
    sw_i2c_stop(bus_id);
}

void i2c_bus_receive(int8_t bus_id, uint16_t reg_addr, uint16_t reg_addr_type, uint8_t *data, uint16_t size)
{
    if (bus_id < 0)
        return;
    sw_i2c_start(bus_id);
    sw_i2c_send(bus_id, i2c_config[bus_id].DevAddr);
    if (sw_i2c_wait_ack(bus_id) != 0)
    {
        sw_i2c_stop(bus_id);
        return;
    }

    if (reg_addr_type == 0)
    {
        sw_i2c_send(bus_id, (uint8_t)reg_addr);
        sw_i2c_wait_ack(bus_id);
    }
    else
    {
        sw_i2c_send(bus_id, (reg_addr >> 8));
        sw_i2c_wait_ack(bus_id);
        sw_i2c_send(bus_id, (reg_addr & 0xff));
        sw_i2c_wait_ack(bus_id);
    }
    sw_i2c_stop(bus_id);

    sw_i2c_start(bus_id);
    sw_i2c_send(bus_id, i2c_config[bus_id].DevAddr + 1);
    sw_i2c_wait_ack(bus_id);

    for (uint16_t i = 0; i < size - 1; i++)
    {
        data[i] = sw_i2c_receive(bus_id, 1);
    }
    data[size - 1] = sw_i2c_receive(bus_id, 0);
    sw_i2c_stop(bus_id);
}