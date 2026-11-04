#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "main.h"

void SoftI2C_Init(void);
void SoftI2C_Start(void);
void SoftI2C_Stop(void);
uint8_t SoftI2C_WriteByte(uint8_t byte);

#endif