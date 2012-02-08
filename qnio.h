#ifndef _QNIO_H__
#define _QNIO_H__

/* modify this according to I2C device address when you use standard I2C function like I2C_XXXX except AI2C_XXXX*/
#define I2C_DEV0_ADDRESS 0x42
#define I2C_TIMEOUT_COUNT    8
/************end*********************/
extern UINT8 QND_ReadReg(UINT8 adr);
extern UINT8 QND_WriteReg(UINT8 adr, UINT8 value);
#endif

