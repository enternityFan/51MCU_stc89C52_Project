#ifndef __IIC_H__
#define __IIC_H__


#define uchar unsigned char 
#define uint unsigned int

sbit SCL=P2^4;
sbit SDA=P2^3;
// 从机位置为0xa0
void iic_write(uchar add,uchar date); //Ð´²Ù×÷
uchar iic_read(uchar add);//¶Á²Ù×÷


#endif