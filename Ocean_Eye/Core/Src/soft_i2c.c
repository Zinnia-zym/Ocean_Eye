#include "soft_i2c.h"

static uint32_t SoftI2C_DelayTicks;

/* SCL/SDA 输出高电平：开漏模式下相当于释放总线 */
static void SoftI2C_SCL_High(void)
{
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
}

static void SoftI2C_SCL_Low(void)
{
    HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
}

static void SoftI2C_SDA_High(void)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
}

static void SoftI2C_SDA_Low(void)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
}

static void SoftI2C_Delay(void)
{
    uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < SoftI2C_DelayTicks)
    {
    }
}

void SoftI2C_Init(void)
{
    /* 启用 Cortex-M3 的微秒级计数器 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* 约 5 微秒延时，对应约 100 kHz I2C */
    SoftI2C_DelayTicks = SystemCoreClock / 200000U;

    SoftI2C_SCL_High();
    SoftI2C_SDA_High();
}

void SoftI2C_Start(void)
{
    SoftI2C_SDA_High();
    SoftI2C_SCL_High();
    SoftI2C_Delay();

    SoftI2C_SDA_Low();
    SoftI2C_Delay();

    SoftI2C_SCL_Low();
    SoftI2C_Delay();
}

void SoftI2C_Stop(void)
{
    SoftI2C_SDA_Low();
    SoftI2C_Delay();

    SoftI2C_SCL_High();
    SoftI2C_Delay();

    SoftI2C_SDA_High();
    SoftI2C_Delay();
}

uint8_t SoftI2C_WriteByte(uint8_t byte)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            SoftI2C_SDA_High();
        }
        else
        {
            SoftI2C_SDA_Low();
        }

        SoftI2C_Delay();
        SoftI2C_SCL_High();
        SoftI2C_Delay();
        SoftI2C_SCL_Low();
        SoftI2C_Delay();

        byte <<= 1;
    }

    /* 释放 SDA，由 OLED 输出 ACK（低电平） */
    SoftI2C_SDA_High();
    SoftI2C_Delay();

    SoftI2C_SCL_High();
    SoftI2C_Delay();

    ack = (HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin)
           == GPIO_PIN_RESET);

    SoftI2C_SCL_Low();
    SoftI2C_Delay();

    return ack;
}