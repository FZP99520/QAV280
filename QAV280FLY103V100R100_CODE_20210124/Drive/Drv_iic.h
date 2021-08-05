#ifndef __I2C_H
#define __I2C_H
#include "stm32f10x.h"
#include "systick.h"
#define IIC_GPIO GPIOB
#define IIC_SCL GPIO_Pin_10
#define IIC_SDA GPIO_Pin_11

//Software IIC Driver Function
//Add by Feng Zipeng 20210130

void SDA_OUT(void);
void SDA_IN(void);
//IO��������	 
#define IIC_SCL_H     GPIO_SetBits(IIC_GPIO,IIC_SCL)//SCL
#define IIC_SCL_L     GPIO_ResetBits(IIC_GPIO,IIC_SCL)//SCL

#define IIC_SDA_H     GPIO_SetBits(IIC_GPIO,IIC_SDA)//SDA
#define IIC_SDA_L     GPIO_ResetBits(IIC_GPIO,IIC_SDA)//SDA

#define READ_SDA   GPIO_ReadInputDataBit(IIC_GPIO,IIC_SDA)  //����SDA 

typedef struct
{
  u8 SlaveAddr;
  u8 Reg;
  u8* DataToWrite;
  u8* DataToRead;
  u8 W_length;
  u8 R_length;
  u8 cnt;
}IIC_TypeDef;


  //IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char data);
u8 IICwriteBytes(u8 dev, u8 reg, u8 length, u8* data);
u8 IICwriteBits(u8 dev,u8 reg,u8 bitStart,u8 length,u8 data);
u8 IICwriteBit(u8 dev,u8 reg,u8 bitNum,u8 data);
u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data);

//int i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
//int i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
void IIC_Write(u8 slave_addr,u8 addr,u8 data);
u8 IIC_Read(u8 slave_addr,u8 addr);
void IIC_Write_Len(u8 slave_addr,u8 addr,u8* buff,u8 len);
void IIC_Read_Buff(u8 slave_addr,u8 addr,u8* buff,u8 len);
void IIC_Write_CMD(u8 slave_addr,u8 cmd);
void iic_delay(u8 i);
#endif
