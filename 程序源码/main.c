#include <reg52.h>
#include <intrins.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ds1302.h"
#include "lcd1602.h"
#include "iic.h"
#include "DS18B20.h"


/************端口定义********************/
sbit CS = P3 ^ 5;  //ADC0832片选
sbit CLK = P3 ^ 6; //ADC0832时钟
sbit DIO = P3 ^ 7; //ADC0832输入输出
sbit k1 = P1 ^ 0;
sbit k2 = P1 ^ 1;
sbit k3 = P1 ^ 2;
sbit k4 = P1 ^ 3;
sbit k5 = P1 ^ 4;
sbit k6 = P1 ^ 5;
sbit k7 = P1 ^ 6;
sbit k8 = P1 ^ 7;
sbit led = P3 ^ 4;
sbit beep = P3 ^ 3;


/*****************变量定义***********************************/

uchar w1 = 0;
w2 = 0;
w3 = 0; //w是weight分解用的
unsigned long maxweight = 400; // 称的最大量程
unsigned long weight = 0; // 当前重量，由秤读出
uint skin = 0; // 存放去皮的皮重

uchar sum = 0; // 记录一共测试的数量
uchar bad = 0; // 记录了不符合要求的个数
uchar choice_error = 10;
uchar choice_weight = 100; // 用来选择数组error与steady的数组的单元的值的
uchar pro_month, pro_day, pro_hour, pro_minute;




/****************数组定义*******************************/

//uchar code Duan[21] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x90,0x88,0x83,0xA7,0xA1,0x86,0x8E,0xbf,0xff,0xbf,0x7f}; // 最后俩个是-和。
//uchar p[3][6] = {"MODE 1","MODE 2","MODE 3",};
char key_status[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 用来存放四个模式的按键的当前状态，默认s1为1，开机显示时间
//char v[5] = {0,0,0,0,0};
char code error[3] = {10, 20, 30}; // 误差带允许100g 200g 300g的误差，按照需要进行选择
uint code steady_weight[6] = {10, 50, 100, 150, 200, 250}; // 标准的重量

SYSTEMTIME adjusted;//此处为结构体定义

void delay_us(uint z); // us级别延迟
void delayms(uint ms); // ms级别延迟
void send(uchar dat); // 发送数据串口
void send_string(uchar *pp, uchar num); // 串口发送字符串
void send_informations(); // 串口发送消息的封装函数
void init(); // 初始化函数
void display_deal(); // 显示处理函数；控制LCD1602的显示
void time_deal(); // 时间处理函数
uchar weight_deal(uchar ch); // AD转换函数
void key_deal(); // 按键处理函数

void delay_us(uint z)
{
    while(z--);


}
void delayms(uint ms)
{
    char t;
    while(ms--)for(t = 0; t < 120; t++);
}


void send(uchar dat)
{
    SBUF = dat;
    while(TI == 0);
    ;
    TI = 0;

}

void send_string(uchar *pp, uchar num)
{
    uchar i = 0;
    for(i = 0; i < num; i++)
    {
        send(pp[i]);
    }
}


void send_informations()
{   uchar i = 0;
    ET1 = 1;
    TR1 = 1;
    ES = 1;
    // send_string("the sum :",8);
    // w1 = sum/10;
    // w2 = sum%10;
    // send(w1+'0');
    // send(w2+'0');
    //先发送别的数据
    /*******************************上次的时间****************************************/
    send_string("\r\n\r\n\r\n\r\n", 8);
    send_string("the last record time:\r\n", 23);
    w1 = pro_month / 10;
    w2 = pro_month % 10;
    send(w1 + '0');
    send(w2 + '0');
    send('-');

    w1 = pro_day / 10;
    w2 = pro_day % 10;
    send(w1 + '0');
    send(w2 + '0');
    send(' ');

    w1 = pro_hour / 10;
    w2 = pro_hour % 10;
    send(w1 + '0');
    send(w2 + '0');
    send(':');
    w1 = pro_minute / 10;
    w2 = pro_minute % 10;
    send(w1 + '0');
    send(w2 + '0');
    delay(500);
    send_string("\r\n\r\n\r\n\r\n", 8);
    /*******************************上次的标准值****************************************/
    send_string("the last record standard:\r\n", 27);
    w1 = choice_error / 10;
    w2 = choice_error % 10;
    send_string("the error:", 10);
    send(w1 + '0');
    send(w2 + '0');

    send_string("   the weight:", 14);
    w1 = choice_weight / 100;
    w2 = choice_weight % 100 / 10;
    w3 = choice_weight % 10;
    send(w1 + '0');
    send(w2 + '0');
    send(w3 + '0');
    send_string("kg", 2);
    send_string("\r\n\r\n\r\n\r\n", 8);
    delay(500);
    /*******************************上次的数据****************************************/
    send_string("the last record data:\r\n", 23);
    send_string("the sum:", 8);
    w1 = sum / 10;
    w2 = sum % 10;
    send(w1 + '0');
    send(w2 + '0');
    send_string("   the bad:", 11);
    w1 = bad / 10;
    w2 = bad % 10;
    send(w1 + '0');
    send(w2 + '0');
    send_string("\r\n\r\n\r\n\r\n", 8);
    delay(500);

    /*******************************上次的 每个 数据****************************************/
    send_string("the detail data:\r\n", 18);

    do
    {   i++;
        // 每次循环送出去一个
        w1 = i / 100;
        w2 = i % 100 / 10;
        w3 = i % 10;
        send_string("the times: ", 10);

        send(w1 + '0');
        send(w2 + '0');
        send(w3 + '0');
        weight = iic_read(0xff - i); // 读出存在这里的重量，正好对应
        w1 = weight / 100;
        w2 = weight % 100 / 10;
        w3 = weight % 10;
        send_string(" the weight: ", 14);
        send(w1 + '0');
        send('.');
        send(w2 + '0');
        send(w3 + '0');
        send_string("kg\r\n", 4);
        delayms(3000);
        led = ~led; // 状态提示
    } while(i <= sum);
    // 这个时候成功把数据全送出去了

    key_status[7] = 2; // 自动置2；
    ET1 = 0;
    TR1 = 0;
    ES = 0;
    led = 1;
}

void init()
{


    EA = 1;
    //IP=0x10;
    //定时器相关 ----------------------------------------------------------------------------------
    ET1 = 0;
    ET0 = 0;
    TMOD = 0x21;
    TR0 = 0;
    TR1 = 0;
    TH0 = (65536 - 50000) / 256;
    TL0 = (65536 - 50000) % 256;



    // 串口相关9600波特率--------------------------------------------------------
    TH1 = 0xfd;
    TL1 = 0xfd;
    ES = 0;
    SCON = 0x40;
    init1602();
    Initial_DS1302() ;
    led = 1; // 初始化熄灭状态


}

/***************************显示处理函数*******************************************************/
void display_deal()
{
    uint temp;
    uchar intT; // 温度使用的
    bit res;
    // 第一行显示当前的模式和其他消息
    // 时间模式
    if(key_status[1] == 0 && key_status[2] == 0 && key_status[3] == 0 && key_status[7] == 0)
    {
        write_com(0x80);
        write_string("MODE 1", 6);

        // 显示数字
        DateToStr(&adjusted);
        TimeToStr(&adjusted);
        write_com(0x88);
        write_string(adjusted.DateString, 8);
        write_com(0xc2);
        write_string(adjusted.TimeString, 8);
    }
    if(key_status[1] != 0)
    {
        if(weight - skin < 0)
            weight = 0;
        else
            weight = weight - skin; // 去皮后的值

        write_com(0x80);
        write_string("MODE 2", 6);

        //第一行显示最大的重量，第二行显示当前的重量
        write_com(0x88);
        write_string("MAX:", 4);
        w1 = maxweight % 1000 / 100;
        w2 = maxweight % 100 / 10;
        w3 = maxweight % 10;
        write_data(w1 + 0x30);
        write_data('.');
        write_data(w2 + 0x30);
        write_data(w3 + 0x30);
        write_com(0xc0);
        write_string("WEIGHT:", 7);
        w1 = weight % 1000 / 100;
        w2 = weight % 100 / 10;
        w3 = weight % 10;
        write_data(w1 + 0x30);
        write_data('.');
        write_data(w2 + 0x30);
        write_data(w3 + 0x30);
        write_string("KG", 2);
    }
    if(key_status[2] != 0)
    {

        write_com(0x80);
        write_string("MODE 3", 6);
        if(key_status[2] < 3)
        {
            write_com(0x88);
            write_string("ERROR:", 6);
            w1 = choice_error / 10;
            w2 = choice_error % 10;

            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_com(0xc0);
            write_string("WEIGHT:", 7);
            w1 = choice_weight / 100;
            w2 = choice_weight % 100 / 10;
            w3 = choice_weight % 10;
            write_data(w1 + 0x30);
            write_data('.');
            write_data(w2 + 0x30);
            write_data(w3 + 0x30);
            write_string("KG", 2);
        }
        else if(key_status[2] == 3)
        {
            // 这一行的sum主要是为了反映当前检测的数量
            write_com(0x88);
            write_string("SUM:", 4);
            w1 = sum / 10;
            w2 = sum % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_com(0xc0);
            write_string("WEIGHT:", 7);
            w1 = weight / 100;
            w2 = weight % 100 / 10;
            w3 = weight % 10;
            write_data(w1 + 0x30);
            write_data('.');
            write_data(w2 + 0x30);
            write_data(w3 + 0x30);
            write_string("KG", 2);

        }
        else
        {
            write_com(0x80);
            write_string("SAVING DATA...", 14);
            delay(3000); // 显示3s 只是为了增加人机交互友好度
            key_status[2] = 0;

        }

    }
    if(key_status[3] != 0)
    {
        if(key_status[3] == 1)
        {
            write_com(0x80);
            write_string("the last time:", 14);
            write_com(0xc0);
            //weight = iic_read(6); // 月份 这里用weight还是为了省变量
            w1 = pro_month / 10;
            w2 = pro_month % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_data('-');
            //weight = iic_read(5); // 日 这里用weight还是为了省变量
            w1 = pro_day / 10;
            w2 = pro_day % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_data(' ');
            //weight = iic_read(4); // 小时 这里用weight还是为了省变量
            w1 = pro_hour / 10;
            w2 = pro_hour % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_data(':');
            //weight = iic_read(3); // 分钟 这里用weight还是为了省变量
            w1 = pro_minute / 10;
            w2 = pro_minute % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);

        }
        else if(key_status[3] == 2)
        {
            // 这时显示当时的标准
            write_com(0x80);
            write_string("STD: ", 5);
            write_string(" ERROR:", 7);
            //weight = iic_read(8); // 误差
            w1 = choice_error / 10;
            w2 = choice_error % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_com(0xc0);
            write_string("WEIGHT:   ", 10);
            //weight = iic_read(7); // 选择的标准重量
            w1 = choice_weight / 100;
            w2 = choice_weight % 100 / 10;
            w3 = choice_weight % 10;

            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_data(w3 + 0x30);
            write_string("KG", 2);
        }
        else	if(key_status[3] == 3)
        {
            // 这时显示当时的数据
            write_com(0x80);
            write_string("DATA:", 5);
            write_string(" SUM:", 5);
            //weight = iic_read(1);
            w1 = sum / 10;
            w2 = sum % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
            write_com(0xc0);
            write_string("BAD:", 4);
            //weight = iic_read(2);
            w1 = bad / 10;
            w2 = bad % 10;
            write_data(w1 + 0x30);
            write_data(w2 + 0x30);
        }
        else
        {
            write_com(0x80);
            write_string("send data?", 10);
            write_com(0xc0);
            write_string("YES          NO", 15);
        }

    }

    if(key_status[7] != 0)
    {

        if(key_status[7] == 6)
        {
            write_com(0x80);
            write_string("temperature:", 12);
            write_com(0xc0);


            res = Get18B20Temp(&temp);
            if(res) // 读取成功
            {
                intT = temp >> 4; //分离出温度值整数部分

                w1 = intT / 100;
                w2 = intT % 100 / 10;
                w3 = intT % 10;
                write_data(w1 + 0x30);
                write_data(w2 + 0x30);
                write_data(w3 + 0x30);
                write_data(' ');
                write_data('C');

            }
        }
        else
        {
            if(key_status[7] == 1)
            {
                write_com(0x80);
                write_string("SENDING....", 11);
            }
            else
            {
                write_com(0x80);
                write_string("SEND SUCCEESS!", 13);
                write_com(0xc0);
                write_string("PRESS AND BACK..", 16);

            }
        }


    }

}

/**************************************时间处理函数*************************************************************/
void time_deal()
{

    if(adjusted.Second > 59)
    {
        adjusted.Second = adjusted.Second % 60;
        adjusted.Minute++;
    }
    if(adjusted.Minute > 59)
    {
        adjusted.Minute = adjusted.Minute % 60;
        adjusted.Hour++;
    }
    if(adjusted.Hour > 23)
    {
        adjusted.Hour = adjusted.Hour % 24;
        adjusted.Day++;
    }
    if(adjusted.Day > 31)
        adjusted.Day = adjusted.Day % 31;
    if(adjusted.Month > 12)
        adjusted.Month = adjusted.Month % 12;
    if(adjusted.Year > 100)
        adjusted.Year = adjusted.Year % 100;

    // 吧处理后的时间写入DS1302
    DS1302_SetTime(DS1302_SECOND, adjusted.Second);
    DS1302_SetTime(DS1302_MINUTE, adjusted.Minute);
    DS1302_SetTime(DS1302_HOUR, adjusted.Hour);
    DS1302_SetTime(DS1302_DAY, adjusted.Day);
    DS1302_SetTime(DS1302_MONTH, adjusted.Month);
    DS1302_SetTime(DS1302_YEAR, adjusted.Year);
    delayms(1);
}


/************************************* AD转换函数******************************************************/
uchar weight_deal(uchar ch) // 封装完好，同时用于单个检测和多个的检测
{
    uchar i, dat1 = 0, dat2 = 0;

    CS  = 0;
    _nop_();
    _nop_();         					//片选使能，低电平有效
    CLK = 0;
    _nop_();
    _nop_();          					//芯片时钟输入
    DIO = 1;
    _nop_();
    _nop_();
    CLK = 1;
    _nop_();
    _nop_();
    //第1个下降沿之前，设DI=1/0
    //选择单端/差分(SGL/DIF)模式中的单端输入模式
    CLK = 0;
    DIO = 1;
    _nop_();
    _nop_();
    CLK = 1;
    _nop_();
    _nop_();
    //第2个下降沿之前,设置DI=0/1,选择CHO/CH1

    CLK = 0;

    if(ch == 0)
        DIO = 0; 	//通道0 内部电压测试
    else DIO = 1;	//通道1

    _nop_();
    _nop_();

    CLK = 1;
    _nop_();
    _nop_();
    //第3个下降沿之前,设置DI=1
    CLK = 0;
    DIO = 1;
    _nop_();
    _nop_();
    //第4-11个下降沿读数据(MSB->LSB)
    for(i = 0; i < 8; i++)
    {
        CLK = 1;
        _nop_();
        _nop_();
        CLK = 0;
        _nop_();
        _nop_();
        dat1 = dat1 << 1 | DIO;
    }
    //第11-18个下降沿读数据(LSB->MSB)
    for(i = 0; i < 8; i++)
    {
        CLK = 1;
        _nop_();
        _nop_();
        CLK = 0;
        _nop_();
        _nop_();
        dat2 = dat2 << ((uchar)(DIO) << i);
    }
    CS = 1;//取消片选一个周期结束
    //如果MSB->LSB和LSB->MSB读取的结果相同,则返回读取的结果,否则返回0
    return dat1;
//	return (dat1 == dat2) ? dat1:0;//取消校验


}

/*******************************************按键检测处理函数**************************************************/
void key_deal()
{
    static uchar tmp1 = 0;
    static uchar tmp2 = 0;
    char i = 0;
    ET0 = 1;
    TR0 = 1;
    if(k1 == 0)
    {
        delayms(10);
        if(k1 == 0 && key_status[1] == 0 && key_status[2] == 0 && key_status[3] == 0 && key_status[7] == 0)
        {
            //		只在时间模式下进行温度采集 和温度显示模式下

            key_status[0]++;
            if(key_status[0] == 7) {
                key_status[0] = 0;   // 最多按7次可以修改年的大小时间
            }
            write_com(0x01); // 清屏 防止切换模式后花屏
        }

        while(!k1);
    }

    if(k2 == 0)
    {
        delayms(10);
        if(k2 == 0 && key_status[0] == 0 && key_status[2] == 0 && key_status[3] == 0 && key_status[7] == 0)
        {
            ET0 = 0;
            TR0 = 0;
            key_status[1]++;
            if(key_status[1] == 2)
            {
                // 初始化  同时初始化指示灯和蜂鸣器 防止bug
                key_status[1] = 0;
                led = 1;
                beep = 1;
                skin = 0;
            }
            write_com(0x01); // 清屏 防止切换模式后花屏
        }

        while(!k2);
    }
    if(k3 == 0)
    {
        delayms(10);
        if(k3 == 0 && key_status[0] == 0 && key_status[1] == 0 && key_status[3] == 0 && key_status[7] == 0)
        {

            key_status[2]++;
            if(key_status[2] == 3)
            {
                ET0 = 1;
                TR0 = 1;
                // 打开串口，每次成功记录后，就把数据发送到PC端

                skin = 0;
                sum = 0;
                bad = 0;


            }
            if(key_status[2] == 4)
            {

                ET0 = 0;
                TR0 = 0;
                led = 1; // 结束后熄灭led灯
                /*ET1 = 0;
                TR1 = 0;
                ES = 0;*/
                skin = 0;
                // 吧数字存储进去
                iic_write(1, sum);
                delayms(200);
                iic_write(2, bad);
                delayms(200);
                //从3-6存放结束时的时间。
                iic_write(3, adjusted.Minute);
                delayms(200);
                iic_write(4, adjusted.Hour);
                delayms(200);
                iic_write(5, adjusted.Day);
                delayms(200);
                iic_write(6, adjusted.Month);
                delayms(200);
                iic_write(7, choice_weight);
                delayms(200);
                iic_write(8, choice_error);
                // 由显示屏的相关状态置0，


            }


            write_com(0x01); // 清屏 防止切换模式后花屏
        }

        while(!k3);
    }


    if(k4 == 0)
    {
        delayms(10);
        if(k4 == 0 && key_status[0] == 0 && key_status[1] == 0 && key_status[2] == 0 && key_status[7] == 0) // 判断如果现在是显示时间的模式的话，那么我就显示
        {
            ET0 = 0;
            TR0 = 0;
            key_status[3]++;
            switch(key_status[3])
            {
            case 1: {
                pro_month = iic_read(6);
                delayms(3);
                pro_day = iic_read(5);
                delayms(3);
                pro_hour = iic_read(4);
                delayms(3);
                pro_minute = iic_read(3);
                delayms(3);
                break;
            }
            case 2: {
                choice_error = iic_read(8);
                delayms(3);
                choice_weight = iic_read(7);
                delayms(3);
                break;
            }
            case 3: {
                sum = iic_read(1);
                delayms(3);
                bad = iic_read(2);
                break;
            }

            default:
                break;
            }
            if(key_status[3] == 5)key_status[3] = 0; // 四个模式 显示当时的时间，然后再显示当时的测量的标准（误差标准及标准重量),再显示总数据;接着再按则询问是否要详细的数据，若按确定按钮，则串口发送
            write_com(0x01); // 清屏 防止切换模式后花屏
        }
    }



    if(k5 == 0) // 增加被按下
    {
        delay(10);
        if(k5 == 0 & (key_status[0] != 0 | key_status[1] != 0 | key_status[2] != 0))
        {
            ET0 = 0;
            TR0 = 0;
            // 一种一种情况来判断
            if(key_status[0] != 0)
            {
                switch(key_status[0])
                {
                case 1:
                    adjusted.Second++;
                    break;
                case 2:
                    adjusted.Minute++;
                    break;
                case 3:
                    adjusted.Hour++;
                    break;
                case 4:
                    adjusted.Day++;
                    break;
                case 5:
                    adjusted.Month++;
                    break;
                case 6:
                    adjusted.Year++;
                    break;
                default:
                    break;
                }
            }


            if(key_status[1] != 0)
            {
                maxweight++;
            }

            if(key_status[2] != 0)
            {
                switch(key_status[2])
                {
                case 1: {
                    if(tmp1 != 5) {
                        tmp1++;
                    }
                    choice_weight = steady_weight[tmp1];
                    break;
                }
                case 2: {
                    if(tmp2 != 2) {
                        tmp2++;
                    }
                    choice_error = error[tmp2];
                    break;
                }
                default:
                    break;
                }
            }






            while(!k5);
        }
    }

    if(k6 == 0)
    {
        delayms(10);

        if(k6 == 0 && (key_status[0] != 0 | key_status[1] != 0 | key_status[2] != 0))
        {   ET0 = 0;
            TR0 = 0;
            if(key_status[0] != 0)
            {
                switch(key_status[0])
                {
                case 1:
                    adjusted.Second--;
                    break;
                case 2:
                    adjusted.Minute--;
                    break;
                case 3:
                    adjusted.Hour--;
                    break;
                case 4:
                    adjusted.Day--;
                    break;
                case 5:
                    adjusted.Month--;
                    break;
                case 6:
                    adjusted.Year--;
                    break;
                default:
                    break;
                }
            }


            if(key_status[1] != 0)
            {
                maxweight--;
            }

            if(key_status[2] != 0)
            {
                switch(key_status[2])
                {
                case 1: {
                    if(tmp1 != 0)tmp1--;
                    choice_weight = steady_weight[tmp1];
                    break;
                }
                case 2: {
                    if(tmp2 != 0)tmp2--;
                    choice_error = error[tmp2];
                    break;
                }
                default:
                    break;
                }

            }
            while(!k6);
        }
    }

    if(k7 == 0) // 去皮
    {
        delayms(10);
        if(k7 == 0 && key_status[1] == 1) // 只有在单次称重下去皮模式才起作用
        {
            ET0 = 0;
            TR0 = 0;
            skin = weight; // 保存皮的重量
            weight = 0; //皮给去了嘛
            key_status[6]++;
            if(key_status[6] == 2)key_status[6] = 0;

            while(!k7);
        }
    }
    if(k8 == 0)
    {
        delayms(10);
        if(k8 == 0 && (key_status[3] == 4 | key_status[7] == 2))
        {
            key_status[3] = 0;
            key_status[7]++;

            if(key_status[7] == 3) {
                key_status[7] = 0;
            }

            write_com(0x01); // 清屏 防止切换模式后花屏
            while(!k8);
        }
        else if(k8 == 0 && key_status[0] == 0 && key_status[1] == 0 && key_status[2] == 0 && key_status[3] == 0) // k8的第二种模式，只有在按下的时候才能显示温度
        {   // 为了单独区分出来一种转态
            if(key_status[7] == 0)key_status[7] = 6;
            else key_status[7] = 0;
            while(!k8); // 直到被按下
        }

        write_com(0x01);
    }
}



/*****************************************************主函数*****************************************************/
void main()
{
    uchar i = 0;

    SYSTEMTIME T;
    init();

    /*
    i = iic_read(1);

    i++;
    iic_write(1, i);
    w1 = i/10;
    w2 = i%10;
    write_com(0x80);
    write_string("HELLO :",6);
    write_data(w1+0x30);
    write_data(w2+0x30);*/
//		ES = 1;
//	TR1 = 1;
//	ET1 = 1;
    while(1)
    {


        //write_com(0xc0);
        // write_string(p2,2);
        DS1302_GetTime(&T) ;
        adjusted.Second = T.Second;
        adjusted.Minute = T.Minute;
        adjusted.Hour = T.Hour;
        adjusted.Week = T.Week;
        adjusted.Day = T.Day;
        adjusted.Month = T.Month;
        adjusted.Year = T.Year;

        for(i = 0; i < 9; i++)
        {
            adjusted.DateString[i] = T.DateString[i];
            adjusted.TimeString[i] = T.TimeString[i];
        }
        time_deal();
        key_deal();
        // 增加这个判断条件 减少CPU的使用
        if(key_status[1] != 0 | key_status[2] == 3) {
            weight = (unsigned long)(weight_deal(0) * 1.9608 - 1);

//		send_string("the times: ",10);
//		send(sum);
//		send_string("the weight: ",13);
//		send(weight);

        }
        display_deal(); //显示处理


//
//	if(send_flag==1)
//	{
//		send_informations();
//		send_flag=0;
//	}
        if(key_status[7] == 1)
        {
            send_informations();
        }
        if(key_status[1] != 0)
        {
            // 判断一下是否超过最大量程，若超过，那就蜂鸣器响
            if((int)weight > maxweight)
            {
                led = 0; // 指示灯亮
                beep = 0;
            }
            else
            {
                led = 1;
                beep = 1;
            }

        }
        delayms(10);





    }




    return;
}

/************************************************定时器函数*******************************************************/
void t1() interrupt 1
{
    static char i = 0;
    TH0 = (65536 - 50000) / 256;
    TL0 = (65536 - 50000) % 256;
    i++;

    if(i == 20)
    {
        i = 0; // 一秒了
        if(key_status[2] == 3) {
            if(weight == skin)
            {
                // 这个时候就可以确定本次计数了
                sum++;
                if(weight > choice_weight)
                {
                    if(weight - choice_weight > choice_error)bad++;
                }
                else
                {
                    if(choice_weight - weight > choice_error)bad++;
                }
                iic_write(0xff - sum, weight); // 把这个质量记录倒着记录进24C02中
                led = ~led; // 反转指示


            }
            skin = weight; // 这里skin变量被利用来存储1s前的重量
        }
        else
        {
            Start18B20();
        }
    }

}
