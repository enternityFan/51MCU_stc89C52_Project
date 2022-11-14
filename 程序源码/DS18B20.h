#ifndef __DS18B20_H_
#define __DS18B20_H_


#include <reg52.h>
#include <intrins.h>
sbit IO_18B20 = P3 ^ 2; //DS18B20 通信引脚
/* 软件延时函数，延时时间(t*10)us */
void DelayX10us(unsigned char t)
{
    do {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    } while (--t);
}
/* 复位总线，获取存在脉冲，以启动一次读写操作 */
bit Get18B20Ack()
{
    bit ack;
    EA = 0; //禁止总中断
    IO_18B20 = 0; //产生 500us 复位脉冲
    DelayX10us(50);
    IO_18B20 = 1;
    DelayX10us(6); //延时 60us
    ack = IO_18B20; //读取存在脉冲
    while(!IO_18B20); //等待存在脉冲结束
    EA = 1; //重新使能总中断
    return ack;
}
/* 向 DS18B20 写入一个字节， dat-待写入字节 */
void Write18B20(unsigned char dat)
{
    unsigned char mask;
    EA = 0; //禁止总中断
    for (mask = 0x01; mask != 0; mask <<= 1) //低位在先，依次移出 8 个 bit
    {
        IO_18B20 = 0; //产生 2us 低电平脉冲
        _nop_();
        _nop_();
        if ((mask & dat) == 0) //输出该 bit 值
            IO_18B20 = 0;
        else
            IO_18B20 = 1;
        DelayX10us(6); //延时 60us
        IO_18B20 = 1; //拉高通信引脚
    }
    EA = 1; //重新使能总中断
}
/* 从 DS18B20 读取一个字节，返回值-读到的字节 */
unsigned char Read18B20()
{
    unsigned char dat;
    unsigned char mask;
    EA = 0; //禁止总中断
    for (mask = 0x01; mask != 0; mask <<= 1) //低位在先，依次采集 8 个 bit
    {
        IO_18B20 = 0; //产生 2us 低电平脉冲
        _nop_();
        _nop_();
        IO_18B20 = 1; //结束低电平脉冲，等待 18B20 输出数据
        _nop_(); //延时 2us
        _nop_();
        if (!IO_18B20) //读取通信引脚上的值
            dat &= ~mask;
        else
            dat |= mask;
        DelayX10us(6); //再延时 60us
    }
    EA = 1; //重新使能总中断
    return dat;
}
/* 启动一次 18B20 温度转换，返回值-表示是否启动成功 */
bit Start18B20()
{
    bit ack;
    ack = Get18B20Ack(); //执行总线复位，并获取 18B20 应答
    if (ack == 0) //如 18B20 正确应答，则启动一次转换
    {
        Write18B20(0xCC); //跳过 ROM 操作
        Write18B20(0x44); //启动一次温度转换
    }
    return ~ack; //ack==0 表示操作成功，所以返回值对其取反
}
/* 读取 DS18B20 转换的温度值，返回值-表示是否读取成功 */
bit Get18B20Temp(int *temp)
{
    bit ack;
    unsigned char LSB, MSB; //16bit 温度值的低字节和高字节
    ack = Get18B20Ack(); //执行总线复位，并获取 18B20 应答
    if (ack == 0) //如 18B20 正确应答，则读取温度值
    {
        Write18B20(0xCC); //跳过 ROM 操作
        Write18B20(0xBE); //发送读命令
        LSB = Read18B20(); //读温度值的低字节
        MSB = Read18B20(); //读温度值的高字节
        *temp = ((int)MSB << 8) + LSB; //合成为 16bit 整型数
    }
    return ~ack; //ack==0 表示操作应答，所以返回值为其取反值
}























#endif