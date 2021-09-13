#include "mpu6050.h"
#include "systick.h"
#include "printf.h"

#include "math.h"
#include "IMU.h"
#include "qmc5883.h"
#include "flash.h"
#include "RC.h"
#include "log.h"

#define Ka 0.08f
IMU_Data_TypeDef IMU_Data;
#define  GyroOffsetNum   200
#define  AccelOffsetNum  200
//define Accelerate move average struct
MoveAvarageFilter_TypeDef Filter_Acc_X={8,0,0,{0}};
MoveAvarageFilter_TypeDef Filter_Acc_Y={8,0,0,{0}};
MoveAvarageFilter_TypeDef Filter_Acc_Z={8,0,0,{0}};
//define gyro move average struct
MoveAvarageFilter_TypeDef Filter_Gyro_X={4,0,0,{0}};
MoveAvarageFilter_TypeDef Filter_Gyro_Y={4,0,0,{0}};
MoveAvarageFilter_TypeDef Filter_Gyro_Z={4,0,0,{0}};

float Pitch,Roll,Yaw; 

static int MPU6050_RD_Buff(u8 addr,u8 size,u8* pBuff);
static void MPU_RD_ACCEL_GYRO(IMU_Data_TypeDef* imu_data);
static u8 MPU_GET_ACCEL_OFFSET(IMU_Data_TypeDef* imu_data);
static u8 MPU_GET_GYRO_OFFSET(IMU_Data_TypeDef* imu_data);
static int MPU6050_Write_Data(u8 addr,u8 data);
static int MPU6050_Read_Data(u8 addr,u8* pData);

u8 temp;
u8 MPU6050_Init(void) //��ʼ��
{
	u8 dev_id;
	MPU6050_Read_Data(WHO_AM_I,&dev_id);
	if(dev_id != 0x68 )
    {
      DebugLog("[ERROR]MPU6050 Init Fail:Device ID != 0x68\n");
      return 0;
    }
	MPU6050_Write_Data(PWR_MGMT_1, 0x80);//��λ
	Delay_ms(100);
	MPU6050_Write_Data(PWR_MGMT_1, 0x0b);//ʹ��Z gyro����Ϊ�ο�ʱ��
	Delay_ms(100);
	MPU6050_Write_Data(SMPLRT_DIV, 0x01);//Sample Rate=1Khz/(1+2) =500Hz T = 2ms output rate
	
	MPU6050_Write_Data(CONFIG, 0x02); //DLPF Accel:Bandwidth 94Hz delay3ms Gyro:Bandwidth 98Hz delay 2.8ms
	
	MPU6050_Write_Data(GYRO_CONFIG, 0x10);//��������+-1000deg/s
	
	MPU6050_Write_Data(ACCEL_CONFIG, 0x08);//+-4g
	MPU6050_Read_Data(ACCEL_CONFIG,&temp);
	
	MPU6050_Write_Data(FIFO_EN,0x00);//Disable fifo
	MPU6050_Write_Data(INT_EN,0x00);//Disable interrupt
	
	MPU6050_Write_Data(INT_EN,0x00);  //�����ж�,closed
	DebugLog("[OK]MPU6050 Init OK~\n");
	return 1;
}
void IMU_Data_Update(void)
{
  MPU_RD_ACCEL_GYRO(&IMU_Data); //Read data from mpu6050 first
	if(IMU_Data.AccelOffsetReq == 1)//��ҪУ׼ʱ������Ԫ������
	{
		   if( MPU_GET_ACCEL_OFFSET(&IMU_Data)==1)
			 {
         IMU.q0 = 1.0f;
				 IMU.q1 = 0;
				 IMU.q2 = 0;
				 IMU.q3 = 0;
			 }
			 return;
	}
	if(IMU_Data.GyroOffsetReq == 1)//��ҪУ׼ʱ������Ԫ������
	{
		   if(  MPU_GET_GYRO_OFFSET(&IMU_Data)==1)
			 {
				 IMU.q0 = 1.0f;
				 IMU.q1 = 0;
				 IMU.q2 = 0;
				 IMU.q3 = 0;
			 }
		     return;
	}
		IMU_Data.ACCEL_X = IMU_Data.ACCEL_X - IMU_Data.os_accel_x;
		IMU_Data.ACCEL_Y = IMU_Data.ACCEL_Y - IMU_Data.os_accel_y;
		IMU_Data.ACCEL_Z = IMU_Data.ACCEL_Z - IMU_Data.os_accel_z;	
		IMU_Data.GYRO_X  = IMU_Data.GYRO_X - IMU_Data.os_gyro_x;
		IMU_Data.GYRO_Y  = IMU_Data.GYRO_Y - IMU_Data.os_gyro_y;
		IMU_Data.GYRO_Z  = IMU_Data.GYRO_Z - IMU_Data.os_gyro_z;
	
	IMU_Data.ACCEL_X = MoveAvarageFilter(&Filter_Acc_X,IMU_Data.ACCEL_X);//����ƽ���˲�
	IMU_Data.ACCEL_Y = MoveAvarageFilter(&Filter_Acc_Y,IMU_Data.ACCEL_Y);//����ƽ���˲�
	IMU_Data.ACCEL_Z = MoveAvarageFilter(&Filter_Acc_Z,IMU_Data.ACCEL_Z);//����ƽ���˲�
	IMU_Data.GYRO_X = MoveAvarageFilter(&Filter_Gyro_X,IMU_Data.GYRO_X);//����ƽ���˲�
	IMU_Data.GYRO_Y = MoveAvarageFilter(&Filter_Gyro_Y,IMU_Data.GYRO_Y);//����ƽ���˲�
	IMU_Data.GYRO_Z = MoveAvarageFilter(&Filter_Gyro_Z,IMU_Data.GYRO_Z);//����ƽ���˲�

	IMU_Data.gx = IMU_Data.GYRO_X*GyroCoefficient;
	IMU_Data.gy = IMU_Data.GYRO_Y*GyroCoefficient;
	IMU_Data.gz = IMU_Data.GYRO_Z*GyroCoefficient;
	
	
	IMU_Update(IMU_Data,MAG_Data.yaw,1);
}
/*
read mpu6050 data from registers
*/
static void MPU_RD_ACCEL_GYRO(IMU_Data_TypeDef* mpu)
{
	  u8 accel_temp_gyro[14];
	  MPU6050_RD_Buff(ACCEL_XOUT_H,14,accel_temp_gyro);
	  mpu->ACCEL_X = ((s16)(accel_temp_gyro[0])<<8 |accel_temp_gyro[1]) ;
	  mpu->ACCEL_Y = ((s16)(accel_temp_gyro[2])<<8 |accel_temp_gyro[3]) ;
	  mpu->ACCEL_Z = ((s16)(accel_temp_gyro[4])<<8 |accel_temp_gyro[5]) ;
	  mpu->TEMP =    ((s16)(accel_temp_gyro[6])<<8 |accel_temp_gyro[7]);
	  mpu->GYRO_X =  ((s16)(accel_temp_gyro[8])<<8 |accel_temp_gyro[9]);
  	  mpu->GYRO_Y =  ((s16)(accel_temp_gyro[10])<<8 |accel_temp_gyro[11]);
	  mpu->GYRO_Z =  ((s16)(accel_temp_gyro[12])<<8 |accel_temp_gyro[13]);
}
/*************************/
static u8 MPU_GET_ACCEL_OFFSET(IMU_Data_TypeDef* imu)
{
	u8 i;
	u16 buff[3];
	static s32 Accel[3]={0,0,0};
	static u16 count=0;
	if(imu->AccelOffsetReq ==1 )
	{
		imu->AccelOffsetFinished = 0;
		if(count >= AccelOffsetNum)
		{
			imu->AccelOffsetFinished = 1;
			imu->AccelOffsetReq = 0;
			imu->os_accel_x = Accel[0]/count;
			imu->os_accel_y = Accel[1]/count;
			imu->os_accel_z = Accel[2]/count;
			/*********Save Data to Flash**************/
			buff[0] = (u16)imu->os_accel_x;
			buff[1] = (u16)imu->os_accel_y;
			buff[2] = (u16)imu->os_accel_z;
			FlashErase(MPUAccelOffsetAddr,1);
			i = FlashWrite(MPUAccelOffsetAddr,buff,3); //Save Offset Data
			/*************************************/
			if(i == 3)
			    RC_flag.send_accel_cal_result=1;
			Accel[0] = 0;
			Accel[1] = 0;
			Accel[2] = 0;
			count=0;
			return 1;
		}
		else
		{
			Accel[0] += imu->ACCEL_X;
			Accel[1] += imu->ACCEL_Y;
			Accel[2] += imu->ACCEL_Z-Range_Acc_Setting;
			count++;
			return 0;
		}
	}
	else 
		return 0;
}
static u8 MPU_GET_GYRO_OFFSET(IMU_Data_TypeDef* imu)
{
	  u8 i;
	  u16 buff[3];
	  static s32 Gyro[3]={0,0,0};
		static u16 count=0;
		if(imu->GyroOffsetReq == 1)
		{
			imu->GyroOffsetFinished = 0;
			if(count > GyroOffsetNum)
			{
				imu->GyroOffsetFinished = 1;
				imu->GyroOffsetReq = 0;
				imu->os_gyro_x = Gyro[0]/count;
			  imu->os_gyro_y = Gyro[1]/count;
			  imu->os_gyro_z = Gyro[2]/count;
				//Save Data to flash
				buff[0] = (u16)imu->os_gyro_x;
				buff[1] = (u16)imu->os_gyro_y;
				buff[2] = (u16)imu->os_gyro_z;
				FlashErase(MPUGyroOffsetAddr,1);
        i = FlashWrite(MPUGyroOffsetAddr,buff,3);
				//*************************************//
				if(i == 3) 
					  RC_flag.send_gyro_cal_result=1;
				count = 0;
				Gyro[0] = 0;
			  Gyro[1] = 0;
			  Gyro[2] = 0;
				return 1;
			}
			else
		  {
	     Gyro[0] += imu->GYRO_X;
			 Gyro[1] += imu->GYRO_Y;
			 Gyro[2] += imu->GYRO_Z;
			 count++;
			 return 0;
		  }
		}
		else 
			return 0;
}



/*************************/
static int MPU6050_Write_Data(u8 addr,u8 data) //д����
{
    int res;
    u8* pBuff;
    *pBuff = data;
	res = Api_IIC_WriteBytes(MPU_SlaveAddress,addr,1,pBuff);
    return res;
}
static int MPU6050_Read_Data(u8 addr,u8* pData) //��ȡ����
{
    int res;
    res = Api_IIC_ReadBytes(MPU_SlaveAddress,addr,1,pData);
	return res;
}

static int MPU6050_RD_Buff(u8 addr,u8 size,u8* pBuff)
{
    int res;
    res = Api_IIC_ReadBytes(MPU_SlaveAddress,addr,size,pBuff);
    return res;
}



