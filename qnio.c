#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "qndriver.h"
#include "qnio.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SPRINTF(a)

#ifndef I2C_FILE_NAME
#define I2C_FILE_NAME "/dev/i2c-0"
#endif

extern  int i2c_file;

static int I2C_Read_1byte(unsigned char address, unsigned char regaddr) 
{
    unsigned char inoutbuf[4];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    inoutbuf[0] = regaddr;
    messages[0].addr = 0x42;
    messages[0].flags = 0;
    messages[0].len = 1;
    messages[0].buf = inoutbuf;

    messages[1].addr = 0x42;
    messages[1].flags = I2C_M_RD;
    messages[1].len = 1;
    messages[1].buf = &inoutbuf[1];

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(i2c_file, I2C_RDWR, &packets) < 0) {
        perror("Unable to write/read data");
        return -1;
    }
    return inoutbuf[1];
}

static int I2C_Write_nbyte(unsigned char address, unsigned char reg, unsigned char *outbuf, int length) 
{
    char buf[length+1];

    if(!i2c_file)
        i2c_file = open(I2C_FILE_NAME, O_RDWR);

    // Begin by setting the accelerometer's current register to X.
    if (ioctl(i2c_file, I2C_SLAVE, address) < 0) {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        perror("Unable to assign slave address");
        return 0;
    }

    // Write to register 0x07, the MODE register.
    buf[0] = reg;
    memcpy(&(buf[1]), outbuf, length);
    if(write(i2c_file, buf, length+1) != length+1) {
        perror("Unable to write value");
        return -1;
    }
    return 1;
}

static int I2C_Write_1byte(unsigned char address, unsigned char reg, unsigned char data) 
{
    return I2C_Write_nbyte(address, reg, &data, 1);
}

static UINT8 QND_I2C_READ(UINT8 Regis_Addr)
{
    int ret;
    UINT8 tryCount = I2C_TIMEOUT_COUNT;

    while(--tryCount) {
	ret = I2C_Read_1byte(I2C_DEV0_ADDRESS, Regis_Addr);
	if(ret != -1)
	        break;
    }
    if(!tryCount) {
	printf(("QND_I2C_READ error!\n"));
    }
	return ret;
}

static UINT8 QND_I2C_WRITE(UINT8 Regis_Addr,UINT8 Data)
{
    UINT8 ret;
    UINT8 tryCount = I2C_TIMEOUT_COUNT;

    while(--tryCount) {
        ret = I2C_Write_1byte(I2C_DEV0_ADDRESS, Regis_Addr, Data);
        if(ret) 
		break;
    }
    if(!tryCount) {
	SPRINTF(("QND_I2C_WRITE error!\n"));
		return 0;
    }
	return 1;
}

UINT8 QND_WriteReg(UINT8 Regis_Addr,UINT8 Data)
{
	return QND_I2C_WRITE(Regis_Addr,Data);
}

UINT8 QND_ReadReg(UINT8 Regis_Addr)
{
    UINT8 Data;
    Data = QND_I2C_READ(Regis_Addr);
    return Data;
}
#ifdef  __cplusplus
}
#endif
