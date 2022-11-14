#include<reg51.h>
#include "LCD1602.h"
#include "DS1302.h"

#define uchar unsigned char
#define uint unsigned int

sbit speaker=P2^4;
bit key_flag1=0,key_flag2=0;
  	
SYSTEMTIME adjusted;//????????

uchar sec_add=0,min_add=0,hou_add=0,day_add=0,mon_add=0,yea_add=0;
uchar data_alarm[7]={0};



/*********************************????****************************************************/

 
//??
void delayms(uint ms)
{
	uchar t;
	while(ms--)for(t=0;t<120;t++);
}

/************************************??????****************************************/

 
/************************************************??????*********************************/
void changing(void) interrupt 0 using 0	   	//?????????,??????
{
  if(key_flag1)
    key_flag1=0;	
  else 
    key_flag1=1;
}


//????
uchar key_scan()
{
	uchar tmp,keyno;
	//?4??1,??4?
	P1=0x0f;
	delayms(1);
	//???00001111???0000xxxx,x??1??0,????1
	//?????????3?1??0,???0??1
	tmp=P1^0x0f;//(^?C???????)
	//???????0~3??????
	switch(tmp)
	{
		case 1:keyno=0;break;
		case 2:keyno=1;break;
		case 4:keyno=2;break;
		case 8:keyno=3;break;
		default:keyno=16;//????
	}

	//?4??0,???
	P1=0xf0;
	delayms(1);

	//???11110000???xxxx0000,x??1??0,3???1
	//????????4????4?,???????0??1,???0
	 tmp=P1>>4^0x0f;
	 //?0-3???????
	 switch(tmp)
	 {
	 	case 1:keyno+=0;break;
		case 2:keyno+=4;break;
		case 4:keyno+=8;break;
		case 8:keyno+=12;
	 }
	 
	 return keyno;
}


/*********************************???*************************************************/   
main()
{
  uint i;
  uchar p[] = "MODE 1";
	uchar p1[8] = {'1'};
  SYSTEMTIME T;

  EA=1;
  EX0=1;
  IT0=1;
  EA=1;
  EX1=1;
  IT1=1;
  init1602();
  Initial_DS1302() ;

  while(1)
  {	/*
		write_com(0x80);
    write_string(p,6);
    //write_com(0xc0);
   // write_string(p2,2);
    DS1302_GetTime(&T) ;
    adjusted.Second=T.Second;
    adjusted.Minute=T.Minute;
		adjusted.Hour=T.Hour;
		adjusted.Week=T.Week;
		adjusted.Day=T.Day;
		adjusted.Month=T.Month;
		adjusted.Year=T.Year;
		
		for(i=0;i<9;i++)
	{
	  adjusted.DateString[i]=T.DateString[i];
	  adjusted.TimeString[i]=T.TimeString[i]; 
    }
		DateToStr(&adjusted);
	TimeToStr(&adjusted);
	
    write_com(0xc2);
    write_string(adjusted.TimeString,8);
	delay(10);
  }	 */
	p1[0] = key_scan();
			write_com(0xc2);
			write_string("16",1);
	

		delay(10);
}
	
}