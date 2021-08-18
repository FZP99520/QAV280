#include "IncludeAll.h"
#include "stdlib.h"
u16 BuffWrite[10]={1,2,3,4,5,6,7,8,9,10};
u16 BuffRead[10];
int main()
{
	u8 flag_esc_cal=0;
	u16 del_cnt=0;
	u8 r;
	u8 dir=0;
	Clock_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //·Ö×é
	IIC_Init();
	Led_Init();
	Adc_Init();
	PWM_Init();  
	Delay_ms(500);
	Delay_ms(500);
	
	MPU6050_Init();
	MS5611_Init();
	MAG_Init();
	
	Flash_Read_Sensor_Data(&IMU_Data,&MAG_Data,&MS5611);
	
	if(IMU_Data.AccelOffsetFinished==0)
		     IMU_Data.AccelOffsetReq=1;
	if(IMU_Data.GyroOffsetFinished==0)
		     IMU_Data.GyroOffsetReq=1;
	if(MS5611.OffsetFinished == 0)
		     MS5611.OffsetReq=1;
	RC_flag.send_accel_cal_result=1;
	RC_flag.send_gyro_cal_result=1;
	RC_flag.send_mag_cal_result=1;
	RC_flag.send_baro_cal_result=1;
	
	NRF24L01_Init();
  if(NRF24L01_Check()==1)
	{
		LED_Red_ON;
		EXTI12_Init();
		NRF24L01_RX_Mode();
	}
	 PID_STA = Flash_Read_PID();
	 GPS_Position_PID_Para_Init();
	
//  USART1_Init(9600);
	USART2_Init(115200);
	TIMER4_Init();
	#define ESC_CAL 0
		#if ESC_CAL
	  while(1)
	  {
		  PWM1=RC_ctrl.height;
	    PWM2=RC_ctrl.height;
  	  PWM3=RC_ctrl.height;
	    PWM4=RC_ctrl.height;
			Delay_ms(20);
	  }
		#endif
	
	TIMER2_Init();//open timer2 interrupt
	
	while(1)
	{

		//add test 0
		//Branch 2021
//
//		FlashErase(FlashDataBase,1);
//		FlashWrite(FlashDataBase,BuffWrite,10);
//		Delay_ms(500);
//		FlashRead(FlashDataBase,BuffRead,10);
//		Delay_ms(500);
//		
//		BuffWrite[0]=10;
//		BuffWrite[1]=11;
//		BuffWrite[2]=12;
//		BuffWrite[3]=13;
//		BuffWrite[4]=14;
//		BuffWrite[5]=15;
//		BuffWrite[6]=16;
//		BuffWrite[7]=17;
//		BuffWrite[8]=18;
//		BuffWrite[0]=19;
//		FlashErase(FlashDataBase,1);
//		FlashWrite(FlashDataBase,BuffWrite,10);
//		Delay_ms(500);
//		FlashRead(FlashDataBase,BuffRead,10);
//		Delay_ms(500);
//		Delay_ms(500);
//		Delay_ms(500);
  //  Kalman_High((float)ms5611.alt,(float)gps_data.altitude,(float)mpu.az,&Alt.now);
    //MPU6050_Update(&mpu);
	}
}
