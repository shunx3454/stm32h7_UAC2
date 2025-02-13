#include "gt911.h"
#include "delay.h"
#include "periph.h"
#include "sw_i2c.h"

#define GT9XX_ADDR 0xBA
#define GT9XX_TOUCH_MAX 5
#define TOUCH_RST_Pin GPIO_PIN_8
#define TOUCH_RST_GPIO_Port GPIOI
#define TOUCH_INT_Pin GPIO_PIN_11
#define TOUCH_INT_GPIO_Port GPIOI

TouchPointInfo TouchPoints[GT9XX_TOUCH_MAX];
static int8_t gt911_i2c_bus_id = -1;

void gt911_i2c_io_init(void) {
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOH_CLK_ENABLE();

  gpio.Mode = GPIO_MODE_OUTPUT_OD;
  gpio.Pin = TOUCH_IIC_CLK_Pin;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(TOUCH_IIC_CLK_GPIO_Port, &gpio);
  gpio.Pin = TOUCH_IIC_SDA_Pin;
  HAL_GPIO_Init(TOUCH_IIC_SDA_GPIO_Port, &gpio);
}

void gt911_i2c_delay(void) {
  delay_us(2);
}

void gt911_i2c_sda_out(uint8_t var) {
  var ? HAL_GPIO_WritePin(TOUCH_IIC_SDA_GPIO_Port, TOUCH_IIC_SDA_Pin, GPIO_PIN_SET)
      : HAL_GPIO_WritePin(TOUCH_IIC_SDA_GPIO_Port, TOUCH_IIC_SDA_Pin, GPIO_PIN_RESET);
}
uint8_t gt911_i2c_sda_in(void) {
  return HAL_GPIO_ReadPin(TOUCH_IIC_SDA_GPIO_Port, TOUCH_IIC_SDA_Pin);
}

void gt911_i2c_clk(uint8_t var) {
  var ? HAL_GPIO_WritePin(TOUCH_IIC_CLK_GPIO_Port, TOUCH_IIC_CLK_Pin, GPIO_PIN_SET)
      : HAL_GPIO_WritePin(TOUCH_IIC_CLK_GPIO_Port, TOUCH_IIC_CLK_Pin, GPIO_PIN_RESET);
}

void gt9xx_send(uint16_t RegAddr, uint8_t *pData, uint16_t DataSize) {
  // HAL_I2C_Mem_Write(&g_i2c2, GT9XX_ADDR, RegAddr, I2C_MEMADD_SIZE_16BIT, pData, DataSize, 0x100);
  i2c_bus_send(gt911_i2c_bus_id, RegAddr, 1, pData, DataSize);
}

void gt9xx_receive(uint16_t RegAddr, uint8_t *pData, uint8_t DataSize) {
  // HAL_I2C_Mem_Read(&g_i2c2, GT9XX_ADDR, RegAddr, I2C_MEMADD_SIZE_16BIT, pData, DataSize, 0x100);
  i2c_bus_receive(gt911_i2c_bus_id, RegAddr, 1, pData, DataSize);
}

void touch_int_IN(void) {
  GPIO_InitTypeDef gpio_config = {0};
  __HAL_RCC_GPIOI_CLK_ENABLE();

  gpio_config.Pin = TOUCH_INT_Pin;
  gpio_config.Mode = GPIO_MODE_INPUT;
  gpio_config.Pull = GPIO_NOPULL;
  gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TOUCH_INT_GPIO_Port, &gpio_config);
}

void touch_int_OUT(void) {
  GPIO_InitTypeDef gpio_config = {0};
  __HAL_RCC_GPIOI_CLK_ENABLE();

  gpio_config.Pin = TOUCH_INT_Pin;
  gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull = GPIO_NOPULL;
  gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TOUCH_INT_GPIO_Port, &gpio_config);
}

void touch_rst_OUT(void) {
  GPIO_InitTypeDef gpio_config = {0};
  __HAL_RCC_GPIOI_CLK_ENABLE();

  gpio_config.Pin = TOUCH_RST_Pin;
  gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull = GPIO_NOPULL;
  gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TOUCH_RST_GPIO_Port, &gpio_config);
}

void touch_int_write(uint8_t sta) {
  sta ? HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_INT_Pin, GPIO_PIN_SET)
      : HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_INT_Pin, GPIO_PIN_RESET);
}
void touch_rst_write(uint8_t sta) {
  sta ? HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_SET)
      : HAL_GPIO_WritePin(TOUCH_INT_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_RESET);
}

void touch_init(void) {
  uint8_t temp;
  uint8_t GT911_Info[11];

  // i2c_init();  // 硬件i2c
  sw_i2c_config_t i2c_config = {
      .DevAddr = GT9XX_ADDR,
      .io_init = gt911_i2c_io_init,
      .io_delay = gt911_i2c_delay,
      .io_sda_out = gt911_i2c_sda_out,
      .io_sda_in = gt911_i2c_sda_in,
      .io_clk = gt911_i2c_clk,
  };
  gt911_i2c_bus_id = sw_i2c_init(&i2c_config);
  if (gt911_i2c_bus_id < 0) {
    printf("gt911 i2c bus init error\r\n");
    return;
  }

  touch_int_OUT();
  touch_rst_OUT();

  /* 配置GT911地址为0XBA */
  touch_int_write(0);
  touch_rst_write(1);
  HAL_Delay(5);

  touch_rst_write(0);
  HAL_Delay(10);
  touch_rst_write(1);
  HAL_Delay(10);
  touch_int_IN();
  HAL_Delay(10);

  /* 软复位 */
  temp = 0x01;
  gt9xx_send(GT9XX_COMMAND, &temp, 1);
  HAL_Delay(5);
  gt9xx_receive(GT9XX_CONFIGVERSION, &temp, 1);
  printf("\r\nGT911 Version: %c\r\n", temp);

  gt9xx_receive(GT9XX_PID1, GT911_Info, 11);
  printf("GT911 Product ID: %c%c%c%c\r\n", GT911_Info[0], GT911_Info[1], GT911_Info[2], GT911_Info[3]);
  printf("Fireware Version: %#04x\r\n", GT911_Info[4]);
  printf("x-y coordinate resolution: %ux%u\r\n", *(uint16_t *)&GT911_Info[6], *(uint16_t *)&GT911_Info[8]);
  printf("Vender ID:%d\r\n", GT911_Info[10]);
}

// result is points, =0 no pressed
uint8_t touch_is_pressed(void) {
    uint8_t dat = 0;
    uint8_t temp = 0;
    gt9xx_receive(GT9XX_STATUS, &dat, 1);
    gt9xx_send(GT9XX_STATUS, &temp, 1);
    return dat & 0x0f;
}

// always read from first points
void touch_read(uint8_t num) {
//  uint8_t *pData = (uint8_t *)&TouchPoints;
//  gt9xx_receive(GT9XX_STATUS, &status, 1);
//  if ((status & 0x0f) > 0) {
//    gt9xx_receive(GT9XX_TRACKID1, pData, sizeof(TouchPoints));
//  } else if ((status & 0x0f) == 0) {
//    for (uint32_t i = 0; i < sizeof(TouchPoints); i++) {
//      pData[i] = 0;
//    }
//  }
//  status = 0;
//  gt9xx_send(GT9XX_STATUS, &status, 1);
    gt9xx_receive(GT9XX_TRACKID1, (uint8_t *)&TouchPoints, sizeof(TouchPointInfo) * num);

  // printf("<%u-%u-%u> <%u-%u-%u> <%u-%u-%u> <%u-%u-%u> <%u-%u-%u>\r\n",
  //        TouchPoints[0].px, TouchPoints[0].py, TouchPoints[0].pSize,
  //        TouchPoints[1].px, TouchPoints[1].py, TouchPoints[1].pSize,
  //        TouchPoints[2].px, TouchPoints[2].py, TouchPoints[2].pSize,
  //        TouchPoints[3].px, TouchPoints[3].py, TouchPoints[3].pSize,
  //        TouchPoints[4].px, TouchPoints[4].py, TouchPoints[4].pSize);
}