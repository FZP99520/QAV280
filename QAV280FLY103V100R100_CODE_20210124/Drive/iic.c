#include "iic.h"
#include "systick.h"
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
/*******************************/
#define SWI2C_INUSE

void IIC_Init(void)
{			
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	//端口配置
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;	//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //10M
  GPIO_Init(GPIOB, &GPIO_InitStructure);					      //根据设定参数初始化GPIOB 
	IIC_SDA_H;
	IIC_SCL_H;
}
static void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	//端口配置
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //10M
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
static void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	//端口配置
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //10M
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
static void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA_H;
	IIC_SCL_H;
	iic_delay(5);
	IIC_SDA_L;
	iic_delay(10);
	IIC_SCL_L;
}
static void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL_L;
	IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
 	iic_delay(10);//Delay_us(1);
	IIC_SCL_H; 
	iic_delay(10);
	IIC_SDA_H;  //发送I2C总线结束信号						   	
}
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA_H;
	//iic_delay(10);//Delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>200)
		{
			IIC_Stop();
			return 0;
		}
	  iic_delay(2);//Delay_us(1);
	}
	IIC_SCL_H;
	iic_delay(6);
	IIC_SCL_L;//时钟输出0 	   
	return 1;  
} 
static void IIC_Ack(void)
{
	//IIC_SCL_L;
	SDA_OUT();
	IIC_SDA_L;
	iic_delay(5);//Delay_us(1);
	IIC_SCL_H;
	iic_delay(5);//Delay_us(1);
	IIC_SCL_L;
}   
static void IIC_NAck(void)
{
	SDA_OUT();
	IIC_SDA_H;
	iic_delay(5);//Delay_us(1);
	IIC_SCL_H;
	iic_delay(5);//Delay_us(1);
	IIC_SCL_L;
} 
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	  SDA_OUT();
    for(t=0;t<8;t++)
    {   
			if(txd&0x80)IIC_SDA_H;
			else IIC_SDA_L;
      txd<<=1;
		  iic_delay(5);
		  IIC_SCL_H;
		  iic_delay(5);
			IIC_SCL_L;	
			//iic_delay(2);	
    }	 
} 	 

u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
	IIC_SCL_L; 
  for(i=0;i<8;i++ )
	{
	   iic_delay(2);
		 IIC_SCL_H;
     receive<<=1;
		 iic_delay(1);
     if(READ_SDA)receive++; 
		 IIC_SCL_L; 
   }					 
    if (ack)
        IIC_Ack(); //发送ACK 
    else
        IIC_NAck();//发送nACK  
    return receive;
}
void iic_delay(u8 i)
{
	u8 a;
	for(a=0;a<i;a++)
	{
		__nop();
	}
}
void IIC_Write_CMD(u8 slave_addr,u8 cmd)
{
	IIC_Start();
	IIC_Send_Byte(slave_addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(cmd);
	IIC_Wait_Ack();
	IIC_Stop();
}
void IIC_Write(u8 slave_addr,u8 addr,u8 data)
{
	IIC_Start();
	IIC_Send_Byte(slave_addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(data);
	IIC_Wait_Ack();
	IIC_Stop();
}
u8 IIC_Read(u8 slave_addr,u8 addr)
{
	u8 temp;
	IIC_Start();
	IIC_Send_Byte(slave_addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();
	IIC_Start();
	IIC_Send_Byte(slave_addr+1);
	IIC_Wait_Ack();
	temp=IIC_Read_Byte(0);
	IIC_Stop();
	return temp;
}
void IIC_Write_Len(u8 slave_addr,u8 addr,u8* buff,u8 len)
{
	u8 i;
	IIC_Start();
	IIC_Send_Byte(slave_addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();
	for(i=0;i<len;i++)
	{
	  IIC_Send_Byte(buff[i]);
		IIC_Wait_Ack();
	}
	IIC_Stop();
}
void IIC_Read_Buff(u8 slave_addr,u8 addr,u8* buff,u8 len)
{
	u8 count;
	IIC_Start();
	IIC_Send_Byte(slave_addr);
	IIC_Wait_Ack();
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();
	IIC_Start();
	IIC_Send_Byte(slave_addr+1);
	IIC_Wait_Ack();
	for(count=0;count<len-1;count++)
	{
		buff[count]=IIC_Read_Byte(1);
	}
	buff[count]=IIC_Read_Byte(0);
	IIC_Stop();
}

int I2C_WriteBytes(u8 DeviceID,u8 RegAddr,u8 size,u8* buff)
{
    u8 i;
    int ret;
	IIC_Start();
	IIC_Send_Byte(DeviceID);
	ret = IIC_Wait_Ack();
    if(ret == 0) return -1;
	ret = IIC_Send_Byte(RegAddr);
    if(ret == 0) return -1;
	IIC_Wait_Ack();
	for(i=0;i<size;i++)
	{
	  IIC_Send_Byte(buff[i]);
	  IIC_Wait_Ack();
      if(ret == 0) return -1;
	}
	IIC_Stop();
    ret = 1;
    return ret;
    
}
