
#include "reg52.h"
#include "intrins.h"
#include "iic.h"

void delay7us()		//延时7us
{   _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

void start() //起始信号，这时数据线电平变化在时钟高电平时，下将沿有效
{   SCL = 1; //时钟线拉高，
    delay7us();
    SDA = 1; //数据线拉高
    delay7us(); //延时，延时应在有效范围内必须 >4.7us
    SDA = 0;	//拉低
    delay7us(); //延时，延时应在有效范围内必须 >4us
}

void stop()//终止信号	  这时数据线电平变化在时钟高电平时，，上升沿有效
{   //SDA=0;
    //delay7us();
    /// SCL=1;
    //delay7us();
    // SDA=1;
    //delay7us();
    SDA = 0; // 数据线先低
    delay7us();
    SCL = 1; //	时钟线再高
    delay7us();  //延时，延时应在有效范围内必须 >4us，SDA在高
    SDA = 1;	 //数据线拉高
    delay7us();//延时，延时应在有效范围内必须 >4.7us
}

void response()//应答信号，"0"
{   uchar i;
    SCL = 1;	 //时钟线拉高，
    delay7us(); ///延时，延时应在有效范围内必须 >4us
    while((SDA == 1) && (i < 200))i++; //等待应答，在 SCL>4us 内判断 SDA是否为“0”，为“0”表示应答，
    //在i<200内，未收到应答信号，就默认收到应答，并退出。
    SCL = 0; //时钟线拉低
    delay7us();
}

void noack() //非应答信号，“1”非应答也可不进行判断
{   uchar i;
    //SDA=1;
    // delay7us();
    SCL = 1;
    delay7us();//延时>4us,
    while((SDA != 1) && (i < 100))i++; //在 SCL>4us 内判断 SDA是否为“1”，为“1”表示非应答
    // 同上，
    SCL = 0;
    delay7us();
    //SDA=0;
}

void init_iic() //SCL,SDA初始化
{   SCL = 1;
    SDA = 1;
}

void write_byte(uchar date)	//写一字节
{   uchar i;
    /*      for(i=0;i<8;i++)
    {
    	date=date<<1;//移位，高位移入CY中
    	SCL=0;
        delay7us();
    	SDA=CY;
    	delay7us();
    	SCL=1;
    	delay7us();
      }

      SCL=0;
      delay7us();
      SDA=1;
      delay7us(); */

    SCL = 0;	//SCL拉低准备写入数据，SCL低电平时允许SDA数据变化（写入），SCL高电平时数据应保持稳定
    delay7us();	//延时
    for(i = 0; i < 8; i++)	 //循环8次写入一个字节
    {
        if(date & 0x80) //高位在前，先写高位
        {
            SDA = 1;   //写入 1
        }
        else
        {
            SDA = 0;   //写入 0
        }
        SCL = 1; //SCL拉高
        delay7us();	//延时
        date = date << 1; //移位，准备下一位数据
        SCL = 0; //SCL拉低，准备写下一位
        delay7us(); //
    }
    SDA = 1; //释放SDA数据线,以备接收应答信号“ 0 ”。
    delay7us();//
}

uchar read_byte() //读一个字节，
{   uchar i, k;
    SCL = 0;	 //
    delay7us();  //
    SDA = 1;	 //释放SDA，以备通信
    delay7us();  //
    for(i = 0; i < 8; i++) //循环8次读出一个字节
    {   SCL = 1; //SCL为1时，SDA数据应保持稳定
        delay7us();
        //k=(k<<1)|SDA;
        k = k << 1; //移位
        if(SDA) //检测SDA（某位数据）是否为1，先读高位
            k++;	  //如果为 1 就 k++写入1,相当以 k 的第0位加了一个 1，下次进入时，因为k先移了一位
        // 就移入了第1位，8次就将高位移入第8位，其余位同理。
        SCL = 0; //SCL低电平时允许SDA数据变化，即将数据读出
        delay7us();//延时
    }
    return k; //返回
}

void iic_write(uchar add, uchar date) //写操作
{   //init_iic();
    start(); //先发起始信号，操作之前必须
    write_byte(0xa0);//器件地址（1010 A0 A0 A0(000) R/W(0)）（访问该芯片的地址）
    //前4位为固定地址，出厂时给定。3位用户自定义地址，最后位"0"表示写入，
    response(); //给应答信号
    write_byte(add); //写入地址（将数据写入的地址，任意地址）
    response(); //给应答信号
    write_byte(date);//写入的数据
    response(); //给应答信号
    stop(); //发终止信号

}
uchar iic_read(uchar add) //读操作
{   uchar dat;
    start(); //先发起始信号，操作之前必须
    write_byte(0xa0);//访问该芯片的地址（器件地址）
    response(); //给应答信号
    write_byte(add);// 访问该芯片内部的数据地址（写入数据的地址）
    response();//给应答信号
    start();  //先发起始信号，开始读，
    write_byte(0xa1);//器件地址（1010 A0 A0 A0(000) R/W(1)）（访问该芯片的地址）
    //前4位为固定地址，出厂时给定。3位用户自定义地址，最后位"1"表示读，
    response();//给应答信号
    dat = read_byte(); //读
    noack();//非应答，读数据无需应答
    stop();//终止操作
    return dat;//返回数据
}