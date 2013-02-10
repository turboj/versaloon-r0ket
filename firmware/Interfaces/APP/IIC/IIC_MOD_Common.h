#ifndef __IIC_MOD_COMMON_INCLUDED__
#define __IIC_MOD_COMMON_INCLUDED__

typedef enum IIC_MOD_RESULT
{
    IIC_MOD_ACK = 0,
    IIC_MOD_NACK,
    IIC_MOD_TO,
    IIC_MOD_ADDR_NO_RESPONSE
}IIC_MOD_RESULT_t;

typedef enum IIC_STOP
{
    IIC_NOSTOP = 0,
    IIC_FORCESTOP
}IIC_STOP_t;

#endif	/* __IIC_MOD_COMMON_INCLUDED__ */
