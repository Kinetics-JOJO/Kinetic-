///*************************************************************************
// *�߼�����ʵ��
// *���ƹ���
// *Ť��ģʽ
// *��̨����ģʽ
// *�Ӿ��Զ���׼ģʽ
// *
// *************************************************************************/
#include "Higher_Class.h"
#include "User_Api.h"
#include "Ctrl_gimbal.h"
#include "IMU.h"
#include "MPU6500.h"
#include "Configuration.h"
#include "led.h"
#include "buzzer.h"
#include "RemoteControl.h"
#include "can1.h"
#include "can2.h"
#include "Ctrl_chassis.h"
#include "Comm_umpire.h"
#include <math.h>
#include <stdlib.h>
#include "PID.h"
#include "Kalman.h"
#include "Ctrl_shoot.h"
#include "pwm.h"
#include "laser.h"
#include "FreeRTOS.h"
#include "task.h"

#define Chassis_MAX_speed_output 10000
#define Chassis        t

float TotalAverage_speed;
float imu_yaw1 = 0;//���ڵ����˶�����
float K_limit ;
int yawcnt = 0;//���ڵ����˶�����

float imu_last1;//������ת180�ȵı���
float Twist_buff;     //Ť����ű���
float SpinTop_buff;   //С���ݴ�ű���
float SpinTop_timecount=0; //С���������ֱ���ʱ��ֹ������

//static float Vx_Lpf=0,Vy_Lpf=0,Wz_Lpf=0;	//���Ƽ��ٶȺ���˶���
float speed_zoom_double = 0;
float forcast_data = 0;

int wait_time=0;//imu���ü���
int imu_reset_flag=3;	
int vision_yaw_time = 0;

//*************�������Ʊ�����*************
int16_t ACC_Offset = 0;//���ٶ�ƫ�ã����ڹ��ʿ���
int16_t Can_Msg_Power;
float Actual_Power ;
float Powerpid_out=1;
float speed_zoom_init=0.8;//35w��ʱ�������ʼ�ٶ�ϵ��Ϊ0.8
u8 PowerLimit_Flag =0;

//******************�Ӿ�����******************
float vision_yaw_angle = 0;
float vision_yaw_angle_last = 0;
float vision_yaw_angle_forcast = 0;
float vision_yaw_angle_forcast_last = 0;
float vision_yaw_angle_target = 0;
float vision_yaw_angle_target_last = 0;
float vision_yaw_angle_result = 0;
float vision_yaw_angle_real = 0;
float vision_yaw_angle_real_last = 0;
float vision_yaw_speed_result = 0;
float vision_yaw_speed_real = 0;
float vision_pitch_angle_target = 0;
float vision_pitch_angle_target_last = 0;
float vision_pitch_angle_result = 0;
float vision_pitch_angle_real = 0;
float vision_pitch_speed_result = 0;
float vision_pitch_speed_real = 0;
float vision_run_time = 0;
float vision_run_fps = 0;

int vision_yaw_angle_jscope = 0;
int vision_yaw_angle_forcast_jscope = 0;
int vision_yaw_angle_real_jscope = 0;
int vision_yaw_angle_target_jscope = 0;
int vision_yaw_speed_real_jscope = 0;


/**
  * @brief  ��ȡ��ֵ�ľ���ֵ
  * @param  value
  * @retval ����ֵ
  */
float Func_Abs(float value)
{
	if(value >= 0)
		return value;
	else 
		return -value;	
}


void chassis_set_contorl(void)//���̿���������
{  
	if(SpinTop_Flag )
	{	
		SpinTop_timecount+=0.0016f;  //����С���ݻ���Ĺ���
		VAL_LIMIT(SpinTop_timecount,0,1); // ��������1  //����С����ֻ��Ҫ�����ֵ����������
		SpinTop_buff =  SpinTop_SPEED * SpinTop_timecount * SpinTop_Direction ;  //���Ի�һ��С���ݵķ��� ʵ�ڲ��а�С���ݵ����ȼ����ɲ������ȼ�
		Control_data.Chassis_Wz = SpinTop_buff ;
		speed_zoom_double = 2.0f;
	} 
	else 
	{ 
		 
		SpinTop_timecount =0;
    speed_zoom_double = 2.0f;
//	if(Twist_Flag)//Ť��
//  {
//	Encoder_angle_Handle();//���̽Ƕ�ת��
//	if(Yaw_Encode_Angle>=10)	//��̨�ڵ����ұ�
//	{
//		Twist_buff = -TWIST_SPEED;
//	}
//	else if(Yaw_Encode_Angle<=-10)	//��̨�ڵ������
//	{
//		Twist_buff = TWIST_SPEED;
//	}
//	Control_data.Chassis_Wz = Twist_buff;
//	}
			

	 
   if(Gimbal_180_flag)//һ��180
	 Control_data.Chassis_Wz = -8.3f;  //-8.3f

	if(Chassismode_flag)//������̨������ģʽ���̲���
	{
	if(imu.yaw > 170 | imu.yaw <-170)
	{ 
		yawcnt = 0;
		return;
	}
		if(!(RC_Ctl.rc.ch2-1024) && !RC_Ctl.mouse.x)	
	{
		if(!yawcnt)
		{
			imu_yaw1 =  imu.yaw;
			yawcnt++;
		}		
		//	Control_data.Chassis_Wz = Control_data.Chassis_Wz*0.8f + ( imu.yaw - imu_yaw1)*0.1f;  //0.2  0.1
	}
	else
	yawcnt = 0;
	
	}		
	
	else//��̨���̷���
	{
		if(RC_Ctl.rc.s2 == RC_SW_MID)//ң����ģʽ���̿�������
		{
			//Control_data.Chassis_Wz = Control_data.Chassis_Wz*0.1f + (-Yaw_Encode_Angle) * 0.15f;  //0.2  0.1   0.27   0.1
		}
	  if(RC_Ctl.rc.s2 == RC_SW_UP)//����ģʽ���̿�������
		{
	   // Control_data.Chassis_Wz = Control_data.Chassis_Wz*0.1f + (-Yaw_Encode_Angle) * 0.18f;   //������Ҫ�������Ե�ϵ������̨�ƶ�̫����ߵ��̲���̫���ᴥ�����̹��У����ڶ���8192
		}                                                    //0.1    //0.1     //0.6  0.1    //0.27
	}
}
	VAL_LIMIT(Control_data.Chassis_Wz,-Wz_MAX,Wz_MAX);

} 


#define CAM_FOV 38.0f	//����ͷ�Ƕ�
void gimbal_set_contorl(void)
{
	vision_yaw_angle_real = T_yaw;
	vision_yaw_angle_real_last = T_yawlast;
	vision_yaw_speed_real = -gryoYaw;
	vision_yaw_angle_forcast_last = vision_yaw_angle_forcast;
	
	vision_pitch_angle_real = -imu.roll;
	vision_pitch_speed_real = -gryoPITCH;
	
	vision_yaw_angle = vision_yaw_angle_real + vision_yaw_angle_target;
	vision_yaw_angle_forcast = KalmanFilter(&Vision_kalman_x, vision_yaw_angle, -gryoYaw, vision_run_time, vision_run_fps);
	//vision_yaw_angle_forcast = KalmanFilter1(&Vision_kalman_x, vision_yaw_angle, vision_yaw_angle_last, vision_run_time, vision_run_fps);
	
	vision_yaw_angle_jscope = vision_yaw_angle;
	vision_yaw_angle_forcast_jscope = vision_yaw_angle_forcast;
	vision_yaw_angle_target_jscope = vision_yaw_angle_real;
	
	if (Vision_Flag)
	{
	  if (!Vision_Data.target_lose)  //yawʶ��
	  {
			vision_yaw_angle_result = PID_Control(&vision_yaw_angle_pid, vision_yaw_angle_forcast, vision_yaw_angle_real);
			vision_yaw_speed_result = PID_Control(&vision_yaw_speed_pid, vision_yaw_angle_result, vision_yaw_speed_real);				
			
			if ((vision_yaw_angle_forcast_last > 0 && vision_yaw_angle_target < 0) || (vision_yaw_angle_forcast_last < 0 && vision_yaw_angle_target > 0))
				vision_yaw_speed_result = vision_yaw_speed_result * fmax(vision_yaw_angle_forcast_last, vision_yaw_angle_target)/(fabs(vision_yaw_angle_forcast_last) + fabs(vision_yaw_angle_target));
			
			if (fabs(vision_yaw_angle_target) > 5)
				vision_yaw_speed_result = vision_yaw_speed_result * 5 / vision_yaw_angle_target;
			//vision_pitch_angle_result = PID_Control(&vision_pitch_angle_pid, (vision_pitch_angle_real + vision_pitch_angle_target), vision_pitch_angle_real);
			//vision_pitch_speed_result = PID_Control(&vision_pitch_speed_pid, vision_pitch_angle_result, vision_pitch_speed_real);	
			//pitch_moto_current_final = vision_pitch_speed_result;
		}	
	  else
	  {
      vision_yaw_angle = 0;
			vision_yaw_angle_forcast = 0;
			vision_yaw_angle_target = 0;
			vision_yaw_angle_result = 0;
			vision_yaw_speed_result = 0;
//      vision_pitch_angle = 0;
//			vision_yaw_angle_forcast = 0;
//			vision_yaw_angle_target = 0;
//			Kalman_Reset(&Vision_kalman_x);
//			Kalman_Reset(&Vision_kalman_y);
	  }	
	}

	if(Gimbal_180_flag)
	  Control_data.Gimbal_Wz = -0.45f;  //-0.25f

	else//���̸�����̨
	{
		if(RC_Ctl.rc.s2 == RC_SW_MID)//ң����ģʽ��̨��������
		{
			if((20 < Yaw_Encode_Angle && Control_data.Gimbal_Wz < 0)| (Yaw_Encode_Angle < -20 && Control_data.Gimbal_Wz >0))
				Control_data.Gimbal_Wz = Control_data.Gimbal_Wz*0.6f;  //0.1f  0.6
			else 
				Control_data.Gimbal_Wz = Control_data.Gimbal_Wz*0.6f;	   //2   0.7
		}			
	  else if(RC_Ctl.rc.s2 == RC_SW_UP&&SpinTop_Flag==0)//����ģʽ��̨��������  
		{
			if( (50 < Yaw_Encode_Angle && Control_data.Gimbal_Wz < 0)| (Yaw_Encode_Angle < -50 && Control_data.Gimbal_Wz >0))
			  Control_data.Gimbal_Wz = Control_data.Gimbal_Wz*0.7f;    //0.1f   //1.7    //1.2
			else 
			  Control_data.Gimbal_Wz = Control_data.Gimbal_Wz*0.7f;       //2    //1.2
    }		
		else
		  Control_data.Gimbal_Wz =0.015f*RC_Ctl.mouse.x;    //С���ݿ���ʱ����׼�ٶ�    //0.027
	}
	
	if(Control_data.Gimbal_Wz > 0.5f)//��̨�������޷�  //0.45f     0.6
		Control_data.Gimbal_Wz = 0.5f;                     //0.45f
	if(Control_data.Gimbal_Wz < -0.5f)                  //0.45f
		Control_data.Gimbal_Wz = -0.5f;                    //0.45f
}


void Power_off_function(void)//�ϵ籣��
{
			CAN1_SendCommand_chassis(0,0,0,0);
      CAN2_Send_Msg_chassis_turnover(0,0,0,0);
			CAN1_Send_Msg_gimbal(0,0,0);
			CAN2_Send_Msg_gimbal(0);
			fri_on = 0;
			speed_17mm_level_Start_Flag = 0;
			friction_wheel_ramp_function();
			Control_data.Vx = 0; 
			Control_data.Vy = 0; 
			Control_data.Chassis_Wz = 0;
			Control_data.Pitch_angle = 0;
			Control_data.Gimbal_Wz = 0;
			speed_17mm_level_Start_Flag = 0;//��־λ����
			//Steering_gear_test = 0;
			speed_17mm_level_High_Flag = 0;
			speed_17mm_level_Low_Flag = 0;
			Twist_Flag = 0;
			Vision_Flag = 0;
			Gimbal_180_flag = 0;
			SafeHeatflag = 1;
			block_flag = 0;
			Stronghold_flag = 0;
      offline_flag = 0;
      error = 0;
	    Yaw_AnglePid_i = 0;
	    T_yaw = 0;
      P1 = 0;
	    I1 = 0;
	    D1 = 0;
	    angle_output1 = 0;
			Yaw_Vision_Speed_Target = 0;
			Yaw_Vision_Speed_Target_Mouse = 0;
			Pitch_Vision_Speed_Target = 0;
	    Control_data.Gimbal_Wz = 0;
      E_yaw = imu.yaw;
      Chassismode_flag = 0;
      PWM_Write(PWM2_CH3,0);
      laser_off();		
    	
}





//�ú���������ѹ���ʵ����ó�ʼ���ٶ�ϵ����������Ч��ֹ�������͵���ǰ�ڵ��ݵ����ĵ�̫��
//���ͬʱ��ֹcanͨ���жϵ��µ��̲��������ƶ������ݰ���MCUͨ��ʧ��ǿ�������ٶ�
void Powerlimit_Decision(void)
{
//	if(PowerData[3] == 44) //Ĭ�ϸ���50w��ѹ
//	{		
//		if(SpinTop_Flag)
//			speed_zoom_init = 0.65f;
//		else
//			speed_zoom_init = 0.75f;
//	}
	if(PowerData[3] == 44)//�����ӵ�45w��ѹ
	{
		if(SpinTop_Flag)
			speed_zoom_init = 0.45f;
		else
		  speed_zoom_init = 0.65f;
	}
	else if(PowerData[3] == 49)//�����ӵ�50w��ѹ
	{
		if(SpinTop_Flag)
			speed_zoom_init = 0.55f;
		else
		  speed_zoom_init = 0.75f;
	}
	else if(PowerData[3] == 54)//55w����ͳһ��ʼϵ��1��������Ҫ���Ϳ����ټӸ��ж�
	{
		if(SpinTop_Flag)
			speed_zoom_init = 0.65f;
		else
		  speed_zoom_init = 0.85f;
	}
	else//canͨ��ʧ��ǿ���޳�ʼ�ٶ�,�����ǿ���ͨ��shift���ټ�������ջ�
		//speed_zoom_init =0.75f;
	  speed_zoom_init = 0.45f;
}
//���ʱջ�v2.0
//ͨ��PID�����������ֵ������ϵ������һ�����Թ�ϵ
//��ÿ�����ӵ�PID���ֵ��������ϵ�����ﵽ�������ٵ�Ч��
//��ȡ����ϵͳʵʱ����
//ͨ������ϵ������������
//
//

void Chassis_Power_Limit(void)
{
    Powerlimit_Decision(); //���޸�Ramp_K�������������ٶȣ���б��б��
	  PID_Control(&Power_Limit_pid, 20, PowerData[1]);  //�Ե��ݵ�ѹֵ���������ϵ��
    if(PowerLimit_Flag==1) //���ʱջ��ֶ������
		{  
			if(PowerData[1] <= 16)
				speed_zoom = 0.65f;
			else
				speed_zoom = 2.0f;//shift�����٣�ʵ���Ǹ����ٶ��޷����ֵ
		}
		else 
		{
		  speed_zoom =speed_zoom_init-Power_Limit_pid.Control_OutPut*speed_zoom_double;  //����޷�Ϊ5	 
   	  //VAL_LIMIT(speed_zoom,0.25f,speed_zoom_init);
			VAL_LIMIT(speed_zoom,0.5f,speed_zoom_init);
	  }
}


//��ϳ������ݰ�
//���룺�趨��ѹ���ʣ�35w����130w��
//�����ú���ʱ���ݰ��ϵ�����Ĭ��35w
void Powerlimit_Ctrl(int16_t Target_power)
{
	Can_Msg_Power = Target_power*100;
	CAN1_SendCommand_Powerlimit(Can_Msg_Power);
}

void imu_reset(void)//imuУ׼
{
	if(imu_reset_flag==0)
	{
		wait_time = xTaskGetTickCount();
		imu_reset_flag=1;
	}
	if(imu_reset_flag==1 && xTaskGetTickCount() - wait_time > 1500 && xTaskGetTickCount() - wait_time < 4500)
	{
		buzzer_on(95, 10000);
		imu.ready = 0;
		imu_reset_flag=2;
	}
	else if(xTaskGetTickCount() - wait_time >= 4500)
	{
		buzzer_off();
	}
}
