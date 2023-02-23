#include "Ctrl_gimbal.h"
#include "MPU6500.h"
#include "IMU.h"
#include "PID.h"
#include "can1.h"
#include "Configuration.h"
#include "User_Api.h"
#include <math.h>
#include <stdlib.h> 
#include "RemoteControl.h"
#include "delay.h"
#include "pwm.h"
#include "Higher_Class.h"
#include "User_Api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Comm_umpire.h"
#include "laser.h"
#include "Ctrl_shoot.h"
#include "can2.h"
#include "Usart_SendData.h"

#define DPI 0.0439453125f	//��ֵ�Ƕȷֱ��ʣ�360/8192
#define PITCH_IMU_CTRL//pitch�ᷴ�����Ʒ�ʽ
float Yaw_Encode_Angle,Pitch_Encode_Angle;//Yaw��Pitch�Ƕ�

#define SHOOT_FRIC_PWM_ADD_VALUE 40//Ħ�����źŵ���ֵ
#define fri_min_value 1000//Ħ�����ź���С���ֵ
int fri_on;//Ħ���ֿ��ر�־
float fri_out = 0;//Ħ�����źŵ������ֵ
float fri_out1=0;//Ħ�����źŵݼ����ֵ
float fri_max_value = 1560;//Ħ�����ź�������ֵ
float speed_17mm_level = 0;
float speed_17mm_result = 0;


//����Yaw����Ƶı���
float error=0; 
float P1=0;
float I1=0;
float D1=0;
float angleYaw;
float E_yaw;//δУ׼��yaw�Ƕ�ֵ����Ҫ��������̨����ǰ��ֵE_yaw=imu.yaw
float E_yaw1 = 0;
float T_yaw;		//Ŀ��Ƕ�
float Yaw_AnglePid_i;//�ǶȻ�������
float angle_output1=0;
float Yaw_Vision_Speed_Target = 0;
float Yaw_Vision_Speed_Target_Mouse = 0;
float Yaw_Vision_Speed_Target1 = 0;
float Pitch_Vision_Speed_Target = 0;
float gryoYaw = 0;//���ٶ�
float gryoPITCH = 0;
float T_yawlast;
int Jscope_gryoYaw;
int Jscope_current;

int32_t Pitch_PID_target;
int32_t Pitch_PID_real;
int32_t Pitch_PID_current;
int16_t pitch_moto_current_final;
int16_t Yaw_PID_current;
int16_t yaw_moto_current;
int16_t yaw_moto_current_final;

static void Pitch_pid(float Target_angle)//pitch�ᴮ��PID����
{	
	if(Control_data.Pitch_angle < -10)  //20
	Control_data.Pitch_angle = -10;  //20
	if(Control_data.Pitch_angle > 26)  //25
	Control_data.Pitch_angle = 26;  //25
	
	gryoPITCH = 0.3f*(imu.gyro[0]-imu.gyroOffset[0])*57.2957795f;
  Target_angle = Control_data.Pitch_angle;
	PID_Control(&Pitch_angle_pid,Target_angle,-imu.roll);
  PID_Control(&Pitch_speed_pid,Pitch_angle_pid.Control_OutPut,-0.3f*(imu.gyro[0]-imu.gyroOffset[0])*57.2957795f);
	
	Pitch_PID_real = -0.4f*(imu.gyro[0]-imu.gyroOffset[0])*57.2957795f;//-imu.roll;
	Pitch_PID_target = Pitch_angle_pid.Control_OutPut;//Target_angle;
	Pitch_PID_current = Pitch_Direction*Pitch_speed_pid.Control_OutPut;
}



static void Yaw_angle_pid(float Targrt_d_angle)//yaw�ᴮ��PID����
{ 
	if(Gimbalmode_flag) //Gimbalmode_flag =1 ��̨���̲�����
	{
		error = 0;
		Yaw_AnglePid_i = 0;
		T_yaw = 0;
		P1 = 0;
		I1 = 0;
		D1 = 0;
		angle_output1 = 0;
		E_yaw = imu.yaw;
			
		PID_Control(&Yaw_pid,Targrt_d_angle*25,-Yaw_Encode_Angle);
		PID_Control(&Yaw_speed_pid,Yaw_pid.Control_OutPut,-motor_data[4].ActualSpeed);
	}
	 
	else
	{
		angleYaw = -(imu.yaw - E_yaw);//�����Ƕȣ������ϵ�ʱ�ĽǶ�У׼imu�����Ƕ����
		gryoYaw = (imu.gyro[2]-imu.gyroOffset[2])*57.2957795f;//���ٶ�  
		//Jscope_gryoYaw = (imu.gyro[2]-imu.gyroOffset[2])*100/(2*PI);//rpm
		//Jscope_current = (imu.gyro[2]-imu.gyroOffset[2])*7.4f*0.194f*0.194f*4*100/3;			
		
		T_yawlast = T_yaw;		
		T_yaw += Targrt_d_angle;		//����Ŀ��Ƕ�
		
		if (Vision_Flag)
			T_yaw = angleYaw;// + 0.2f;
		
		if (T_yaw > 180)
		{
			T_yaw = T_yaw - 360;
		}
		else if (T_yaw < -180)
		{
			T_yaw = T_yaw + 360;
		}		
		//************�⻷���ǶȻ�pi************
		error = T_yaw - angleYaw;
		if (error < -170)
		{
			error = error + 360;
		}
		else if (error > 170)
		{
			error = error - 360;
		}
		
		P1 = error * PID_YAW_ANGLE_KP;
		
		if((error<10)&&(error>-10))
			Yaw_AnglePid_i += error;
		//�����޷�
		if(Yaw_AnglePid_i > 50)	Yaw_AnglePid_i = 50;
		if(Yaw_AnglePid_i < -50)	Yaw_AnglePid_i = -50;
		if((error<0.2f)&&(error>-0.2f)) Yaw_AnglePid_i = 0;		//�ﵽĿ��ʱ���ֹ���
		I1 = Yaw_AnglePid_i * PID_YAW_ANGLE_KI;
		
		D1 = PID_YAW_ANGLE_KD * gryoYaw;
		
		angle_output1 = P1 + I1 - D1;	
		//************�ڻ������ٶȻ�pid*************
		PID_Control(&Yaw_speed_pid,angle_output1,-gryoYaw);
		Yaw_PID_current = Yaw_speed_pid.Control_OutPut;//motor_data[4].Intensity/10;//-(Yaw_Direction*Yaw_speed_pid.Control_OutPut)*100;	
		
		if(Gimbal_180_flag)
		{		
			if(imu_last1 <= 0)
			{
				if( fabs(imu.yaw - (180 + imu_last1)) < 5)
				{
					Gimbal_180_flag = 0;
				}
			}
			else if(imu_last1 > 0 )
			{
				if(fabs(imu.yaw - (imu_last1 - 180))< 5)
				{
					Gimbal_180_flag = 0;
				}				
			}	
		}
  }
}






void Encoder_angle_Handle(void)//��̨�����ת�ǶȺ���
{
	float Yaw_encode = motor_data[4].NowAngle;
	float Pitch_encode = motor2_data[0].NowAngle;

		
  Yaw_Encode_Angle = (((Yaw_encode-Yaw_Mid_encode)*DPI)*Yaw_Direction);
  Pitch_Encode_Angle =((Pitch_encode-Pitch_Mid_encode)*DPI)*Pitch_Direction;	
//YAW���0����
  if(Yaw_Encode_Angle>180)
	{
	Yaw_Encode_Angle -=360;
	}
	if(Yaw_Encode_Angle<-180)
	{
		Yaw_Encode_Angle +=360;
	}
}



static void ramp_calc(void)//Ħ����pwmֵ����
{
	fri_out += (SHOOT_FRIC_PWM_ADD_VALUE*0.01);
	if(fri_out > speed_17mm_level)
	{
		fri_out = speed_17mm_level;
	}
	else if(fri_out < fri_min_value)
	{
		fri_out = fri_min_value;
	}
}	


static void ramp_calc1(void)//Ħ����pwmֵ�ݼ�
{ 
	
	fri_out1 = fri_out-1;
	fri_out--;
	
	if(fri_out1 > speed_17mm_level)
	{
		fri_out1 = speed_17mm_level;
	}
	else if(fri_out1 < fri_min_value)
	{
		fri_out1 = fri_min_value;
	}
}

void friction_wheel_ramp_function(void)//Ħ���ֿ��ƺ���
{
	if(speed_17mm_level_Start_Flag && !speed_17mm_level_High_Flag)
	{
		speed_17mm_level = 1547; 
		//��Ħ���֣�Ĭ������ٶ�//1558     1650��14.5��   1640��14.0��
		ramp_calc();
		PWM_Write(PWM2_CH1,fri_out);
    PWM_Write(PWM2_CH2,fri_out);
	}
	if(!speed_17mm_level_Start_Flag)
	{
		ramp_calc1();
		PWM_Write(PWM2_CH1,fri_out1);
    PWM_Write(PWM2_CH2,fri_out1);
	}
	if(speed_17mm_level_High_Flag)
	{
		speed_17mm_level = 1620;	//���ٵ�	1620
    ramp_calc();		
		PWM_Write(PWM2_CH1,fri_out);
    PWM_Write(PWM2_CH2,fri_out);
	}
}

void Control_on_off_friction_wheel(void)//Ħ���ֿ��غ���
{
	if (RC_Ctl.rc.s1 == RC_SW_UP && RC_Ctl.rc.s1_last != RC_SW_UP)
	{
//		fri_on = !fri_on;
		speed_17mm_level_Start_Flag = !speed_17mm_level_Start_Flag;
	}
}



//��̨�������
void Gimbal_Ctrl(float pitch,float yaw_rate)
{
  Encoder_angle_Handle();//��̨�����ת�ǶȺ���
	#ifdef PITCH_IMU_CTRL	
	/*if(imu.roll > 0 && imu.roll > 150)
		imu.roll =(imu.roll - 179)-179;
	if(imu.roll < 0)
		imu.roll += 179;
	if(imu.roll > 120)
		imu.roll = -25;
	if(imu.roll >15)
		imu.roll = 15;*/
	Pitch_pid(pitch);//pitch�ᴮ��PID����
	#else	
	Pitch_pid(pitch);
	#endif
	Yaw_angle_pid(yaw_rate);//yaw�ᴮ��PID����
	
	yaw_moto_current = Yaw_PID_current;
	if (Vision_Flag)
		 yaw_moto_current = vision_yaw_speed_result;
	yaw_moto_current_final = -yaw_moto_current;
	pitch_moto_current_final = Pitch_Direction*Pitch_speed_pid.Control_OutPut;
  CAN1_Send_Msg_gimbal(yaw_moto_current_final,200,trigger_moto_current);//can������̨������
	CAN2_Send_Msg_gimbal(pitch_moto_current_final);
}


 

void FRT_Gimbal_Ctrl(void *pvParameters)//��̨��������
{
	vTaskDelay(201);
	while(1)
	{
		if (RC_Ctl.rc.s2 == RC_SW_DOWN)
			Power_off_function();
		else
		{	
			imu_reset_flag = 0;
			gimbal_control_acquisition();//��̨��������ȡ��״̬������
			gimbal_set_contorl();//��̨����������
			Gimbal_Ctrl(Control_data.Pitch_angle,Control_data.Gimbal_Wz);//��̨������Ŀ��ƽӿ�	
		}
	  vTaskDelay(1);//������������
	}
}

