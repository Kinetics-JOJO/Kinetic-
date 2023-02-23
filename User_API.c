#include "Kalman.h"
#include "User_Api.h"
#include "Ctrl_gimbal.h"
#include "Ctrl_chassis.h"
#include "IMU.h"
#include "MPU6500.h"
#include "Configuration.h"
#include "led.h"
#include "buzzer.h"
#include "RemoteControl.h"
#include "Comm_umpire.h"
#include "Higher_Class.h"
#include "can2.h"
#include "PID.h"
#include "can1.h"
#include "pwm.h"
#include "laser.h"
#include "Ctrl_shoot.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#define DITHERING_TIMNE 150 
#define Visual_TIME1 200
//״̬��
u8 SpinTop_Flag =0;     //С���ݱ�־
u8 Twist_Flag = 0;			//Ť����־
u8 Vision_Flag = 0;			//�Ӿ������־
u8 Shoot_Motor = 0;    //Ħ���ֱ�־
u8 Shoot_Long = 0;     //�����־
u8 Steering_gear_test = 0;//������ձ�־λ
u8 Gimbal_180_flag = 0; //180�ȱ�־
u8 close_combatflag;   //��ս��־
u8 Stronghold_flag = 0;//�������Ʊ�־
u8 Inverse_flag = 0;   //���ַ�ת
u8 chassis_power_level_1_Flag = 0;   //�������̹���1��
u8 chassis_power_level_2_Flag = 0;   //�������̹���2��
u8 chassis_power_level_3_Flag = 0;   //�������̹���3��
u8 chassis_power_level_1_once_Flag = 0;   //�������̹���1����һ��
u8 chassis_power_level_2_once_Flag = 0;   //�������̹���2����һ��
u8 chassis_power_level_3_once_Flag = 0;   //�������̹���3����һ��
u8 speed_17mm_level_Start_Flag = 0;  //��Ħ���֣�Ĭ���ٶ�
u8 speed_17mm_level_Low_Flag = 0;  //���ٵ�
u8 speed_17mm_level_High_Flag = 0;  //���ٵ�
u8 speed_17mm_level_Low_once_Flag = 0;  
u8 speed_17mm_level_High_once_Flag = 0;
u8 Usart_send_Flag = 0;//���ͱ�ʶ��
u8 TwistCnt,VisualCnt,ShootMotorCnt,speed_17mm_level_High_Cnt,speed_17mm_level_Start_Cnt,speed_17mm_level_Low_Cnt;
u8 chassis_power_level_3_Cnt,chassis_power_level_2_Cnt,chassis_power_level_1_Cnt,TriggerSingleMotorCnt,TriggerMotorCnt;
u8 ShootLongCnt,Steering_Gear_Cnt,Gimbal_180_Cnt,close_combatCnt,Stronghold_cnt,Inverse_cnt,SpinTop_cnt,Usart_send_Cnt;//������
u8 Cap_Safe_Flag =0;
float Gimbal_180_stop_flag_cnt = 0;
//////////////////////////////////////////////////
int32_t VerticalCnt=0,HorizontalCnt=0;
float Ramp_K = 1.0f;  //1.0f
//////////////////////////////////////////////////
int Heatmax = 0;//�������
int Shootnumber = 0;//�ɷ����ӵ�����
int SafeHeatflag = 1;//��ȫ������־
int Shootspeed = 0;//����
int Shootnumber_fired = 0;//�ѷ����ӵ���
int cycle_time = 0;//һ����������ʱ��

ControlDATA_TypeDef Control_data;//��������
int slowly_increasing = 1800;//�����𽥼���
int PC_SPEED_X;//�����ٶ�
float Wz_increasing = 0;

int Chassismode_flag = 0;//����ģʽ��־λ
int Gimbalmode_flag = 0;//��̨ģʽ��־λ


float delta_speed =4.5f; //б������
float PC_Speed_Vx =0;
float PC_Speed_Vy =0;
int Wheel_position =1;
float speed_zoom = 1.0f ; //ң�������Ƶ��ٶ�
int16_t Target_chassis_power = 0;


//���̹������Ʊ�����
//�ٶ������ʼ���ֵ�����޸ģ�
int32_t Vertical_Initspeed=3000;
int32_t Horizontal_Initspeed=2000;
//�ٶ�����������ֵ��������
int32_t Vertical_Maxspeed;
int32_t Horizontal_Maxspeed;


void chassis_control_acquisition(void)
{
		if (RC_Ctl.rc.s2 == RC_SW_MID)//ң�ؿ���
		{
			Control_data.Vx = (RC_Ctl.rc.ch1-1024)*6.2;
			Control_data.Vy = (RC_Ctl.rc.ch0-1024)*6.2;
			Control_data.Chassis_Wz = (RC_Ctl.rc.ch2-1024)*0.035f;    //0.32f
			
			//������Χ��-100��100 ����debug������ֵ(100/660 = 0.15151515)
			Control_data.Vx_6020 = (RC_Ctl.rc.ch1-1024)*0.1515f;
			Control_data.Vy_6020 = (RC_Ctl.rc.ch0-1024)*0.1515f;
			Control_data.Chassis_Wz_6020 = (RC_Ctl.rc.ch2-1024)*0.1515f;    //0.32f
			
			
			Chassismode_flag = 0;
			offline_flag++;//����ϵͳ���߼���
	    if(offline_flag >= 2000)
		    offline_flag = 2000;
		}
		
		//�������µ�ʱ���ٶȴ�С������
		else //���Կ���
		{  
			//Chassis_Power_Limit();  //���ʱջ�����
			//����ֵ����б��
			if(RC_Ctl.key.Vertical)
			{ 
			  VerticalCnt++;
				PC_Speed_Vx = Ramp_K*sqrt(5000*VerticalCnt);  //�������������ٶ��𽥼�С�ļ���б��
			}  
			else 
			{
				PC_Speed_Vx =0;
				VerticalCnt=0;
			}
			if (RC_Ctl.key.Horizontal)
			{   
				 HorizontalCnt++;
         PC_Speed_Vy = Ramp_K*sqrt(4000*HorizontalCnt);//б�������й�  //3000
			}	 
			else 
		  {
				 PC_Speed_Vy=0;
				 HorizontalCnt=0;
			} 

		  Control_data.Vx =  RC_Ctl.key.Vertical*PC_Speed_Vx;
		  Control_data.Vy =  RC_Ctl.key.Horizontal*PC_Speed_Vy;
			 

			 //������ٶ��޷�
			 VAL_LIMIT(Control_data.Vx ,-Vx_MAX,Vx_MAX);  
			 VAL_LIMIT(Control_data.Vy ,-Vy_MAX,Vy_MAX);
			 
			 //��תб��
			 if(RC_Ctl.mouse.x != 0) 
			   Wz_increasing = 0.015f;    //0.053   //0.049      //Wz_increasing += 0.02f
       else
         Wz_increasing = 0;
			 
			 if(Wz_increasing >= 2.3f)   //2.3f    4.5f
			   Wz_increasing = 2.3f;        //2.3f    4.5f
			 Control_data.Chassis_Wz = 	Wz_increasing*RC_Ctl.mouse.x;

			if(RC_Ctl.rc.s2 == RC_SW_UP && RC_Ctl.rc.s1 == RC_SW_UP)//��̨���̷���Ͳ�����ģʽ�л�
			{
				Chassismode_flag = 1;//��̨���̲�����ģʽ	
			}
			else
			{
				yawcnt = 0;
				Chassismode_flag = 0;//��̨���̷���ģʽ
			}
			 
	  }
}


void gimbal_control_acquisition(void)
{
	if (RC_Ctl.rc.s2 == RC_SW_MID)//ң�ؿ���
	{
		Control_data.Gimbal_Wz = (RC_Ctl.rc.ch2-1024)*0.001f;  //0.006    0.003  0.005
		Control_data.Pitch_angle += (RC_Ctl.rc.ch3-1024)*0.0003f;//ң����ģʽ�µ��̣���̨������   0.0003
	
		Control_on_off_friction_wheel();//Ħ���ֿ��غ���
		friction_wheel_ramp_function();//Ħ���ֿ��ƺ���
	
		if(RC_Ctl.rc.ch4 !=1024)
		{
			SpinTop_Flag =1;
			Control_data.Gimbal_Wz = 	(RC_Ctl.rc.ch2-1024)*0.001f;
		}else SpinTop_Flag =0;		
		
		if(RC_Ctl.rc.s1 != RC_SW_DOWN)
			shoot_ready_control();
		else
			shoot_task();
		
	 if(RC_Ctl.rc.s1 == RC_SW_UP)
	 {
			PWM_Write(PWM2_CH3,0);
	 } 
	 else
	 {
		 PWM_Write(PWM2_CH3,82);
	 }
	
	 Gimbalmode_flag = 0;
	 
	}
		
		
	else//���Կ���
	{
		Control_data.Gimbal_Wz = 0.008f*RC_Ctl.mouse.x;  //0.012     //0.035   //0.018
		
		VAL_LIMIT(Control_data.Gimbal_Wz,-0.696f,0.696f);		//-0.396f  0.396f      //-0.496    0.496
		Control_data.Pitch_angle = Control_data.Pitch_angle -RC_Ctl.mouse.y*0.005f;  //0.003
			

	  if(RC_Ctl.mouse.press_r)//����
		{
		  Vision_Flag = 1;
		  VisualCnt = 0;
		}	
		else
		{
		  Vision_Flag = 0;
		}
		
	
		//һ��˳ʱ��180�ȱ�־λ
		if(Gimbal_180_Cnt > DITHERING_TIMNE)
		{
			if(RC_Ctl.key.Z && !RC_Ctl.key.Ctrl && !Gimbal_180_flag && SpinTop_Flag==0)
			{
				Gimbal_180_flag = 1;
				imu_last1 = imu.yaw;
				Gimbal_180_Cnt = 0;
			}

		}else Gimbal_180_Cnt++;	
	
	
		//С����
		if(SpinTop_cnt>DITHERING_TIMNE)
		{
			if(RC_Ctl.key.G && Gimbal_180_flag==0)
			{
				SpinTop_Flag = !SpinTop_Flag;
				SpinTop_cnt = 0;
			}
		}
		else SpinTop_cnt++;
	
		//�������̹��ʵȼ�1~3��(��ʼ��0��)
		if(chassis_power_level_1_Cnt > DITHERING_TIMNE)//1��
		{
			if(RC_Ctl.key.X && RC_Ctl.key.Ctrl && chassis_power_level_2_Flag == 0 && chassis_power_level_3_Flag == 0)
			{
				chassis_power_level_1_Flag = 1;
				chassis_power_level_1_once_Flag = 1;
				chassis_power_level_1_Cnt = 0;
			}
		}chassis_power_level_1_Cnt++;
		if(chassis_power_level_2_Cnt > DITHERING_TIMNE)//2��
		{
			if(RC_Ctl.key.C && RC_Ctl.key.Ctrl && chassis_power_level_1_Flag && chassis_power_level_3_Flag == 0)
			{
				chassis_power_level_1_Flag = 0;
				chassis_power_level_2_Flag = 1;
				chassis_power_level_2_once_Flag = 1;
				chassis_power_level_2_Cnt = 0;
			}
		}chassis_power_level_2_Cnt++; 
		if(chassis_power_level_3_Cnt > DITHERING_TIMNE)//3��
		{
			if(RC_Ctl.key.V && RC_Ctl.key.Ctrl && chassis_power_level_2_Flag)
			{
				chassis_power_level_2_Flag = 0;
				chassis_power_level_3_Flag = 1;
				chassis_power_level_3_once_Flag = 1;
				chassis_power_level_3_Cnt = 0;
			}
		}chassis_power_level_3_Cnt++;
	
	
		//17mm���ٵ��ٵ����ٵ��л�(��Ħ����Ĭ������ٶ�)
		if(speed_17mm_level_Start_Cnt > DITHERING_TIMNE)
		{
			if(RC_Ctl.key.E)//��Ħ���֣�Ĭ������ٶ�
			{
				speed_17mm_level_Start_Flag = 1;
				speed_17mm_level_Start_Cnt = 0;
				speed_17mm_level_Low_Flag = 1;
				speed_17mm_level_Low_once_Flag = 1;
				speed_17mm_level_High_Flag = 0;
				speed_17mm_level_High_once_Flag = 0;
			}
		}speed_17mm_level_Start_Cnt++;
		if(speed_17mm_level_Low_Cnt > DITHERING_TIMNE)//���ٵ�
		{
	//		if(RC_Ctl.key.E && speed_42mm_level_Start_Flag && speed_42mm_level_High_Flag == 0 && speed_42mm_level_Low_once_Flag == 0)
			if(RC_Ctl.key.F && speed_17mm_level_Start_Flag && speed_17mm_level_Low_once_Flag)			
			{
				speed_17mm_level_High_Flag = 1;																					//////////////////			speed_17mm_level_Low_Flag = 1;
				speed_17mm_level_High_once_Flag = 1;                                    //////////////////			speed_17mm_level_Low_once_Flag = 1;
				speed_17mm_level_Low_Flag= 0;                                           //////////////////			speed_17mm_level_High_Flag = 0;
				speed_17mm_level_Low_once_Flag = 0;                                     //////////////////			speed_17mm_level_High_once_Flag = 0;
				speed_17mm_level_Low_Cnt = 0;                                          //////////////////			speed_17mm_level_Low_Cnt = 0;
			}
		}speed_17mm_level_Low_Cnt++;
		if(speed_17mm_level_High_Cnt > DITHERING_TIMNE)//��Ħ����
		{
			if(RC_Ctl.key.B && speed_17mm_level_Start_Flag && (speed_17mm_level_High_once_Flag||speed_17mm_level_Low_once_Flag))
			{
				speed_17mm_level_Start_Flag = !speed_17mm_level_Start_Flag;  			////////////////////			speed_17mm_level_High_Cnt = 0;
				speed_17mm_level_High_Cnt = 0;
				speed_17mm_level_Low_Flag = 0;
				speed_17mm_level_Low_once_Flag = 0;
				speed_17mm_level_High_Flag = 0;
				speed_17mm_level_High_once_Flag = 0;
			}
		}speed_17mm_level_High_Cnt++;
	


	  if(Usart_send_Cnt > DITHERING_TIMNE)
	  {
		  if(RC_Ctl.key.Q && RC_Ctl.key.Ctrl &&  Usart_send_Flag == 0)
		  {
			  Usart_send_Cnt = 0;
			  Usart_send_Flag =! Usart_send_Flag;
		  }
	  }Usart_send_Cnt++;
	 	
	
	  if(chassis_power_level_1_Flag)
	  {
      Powerlimit_Ctrl(49);  //�������̹��ʶ��� 50w
	  }
	  else if(chassis_power_level_2_Flag && chassis_power_level_1_once_Flag)
	  {
      Powerlimit_Ctrl(54);  //�������̹������� 55w
	  }
	  else if(chassis_power_level_3_Flag && chassis_power_level_2_once_Flag)
	  {
		  Powerlimit_Ctrl(54);  //�������̹������� 55w
	  }
	  //Powerlimit_Ctrl(Target_chassis_power);

	

		if(RC_Ctl.key.Shift)
		//if(RC_Ctl.key.Shift && PowerData[1]>15.0f)
		{
			PowerLimit_Flag = 1; //������ʱջ����ĵ��ݵĵ�
		}else PowerLimit_Flag =0;
	

	

//		if(RC_Ctl.key.G == 1)//���������־λ
//		{
//			Stronghold_flag = 1;//����ϵͳ���߽�������ջ���Ȼ���䲻���ӵ�
//		}
//		else
//			Stronghold_flag = 0;
//		
		
//	if(Inverse_cnt > DITHERING_TIMNE)//�����ֶ���ת
//	{
//		if(RC_Ctl.key.X == 1)
//		{
//			Inverse_flag = 1;
//			Inverse_cnt = 0;
//		}
//		else
//			Inverse_flag = 0;
//	}else Inverse_cnt++;

	
		offline_flag++;//����ϵͳ���߼���
		if(offline_flag >= 2000)
		{
			offline_flag = 2000;
			Stronghold_flag = 1; //����ϵͳ���߽�������ջ���Ȼ���䲻���ӵ�
		}
	
	
	  friction_wheel_ramp_function();//Ħ���ֿ���
	

		if(!RC_Ctl.mouse.press_l)//�������
		{
			cycle_time++;
			if(cycle_time >= 3000)
			{
				cycle_time = 3000;
				Shootnumber_fired = 0;
			}
			if(speed_17mm_level_Start_Flag)
				laser_on();
			if(!speed_17mm_level_Start_Flag)
				laser_off();
			shoot_ready_control();
		}
		else
		{
			cycle_time = 0;
			ShooterHeat_Ctrl();				
		}
		//shoot_task1();
	
		if(Inverse_flag)//�����ֶ���ת
		{
			trigger_moto_speed_ref2 = 5000;
			trigger_moto_current = PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));
		}				

		
		if(Steering_gear_test)//���ص���
			PWM_Write(PWM2_CH3,0);
		else
			PWM_Write(PWM2_CH3,82);
		
		
		if(RC_Ctl.rc.s2 == RC_SW_UP && RC_Ctl.rc.s1 == RC_SW_UP && !Vision_Flag  && !Gimbal_180_flag)//��̨���̷���򲻷���ģʽ�л�
			Gimbalmode_flag = 1; //��̨���̲�����
		else
			Gimbalmode_flag = 0;  //��̨���̷���
		
  }
		
}



//��������
static void ShooterHeat_Ctrl(void)
{		
	
	if(Game_robot_state.robot_level == 1)//�����˵ȼ���Ӧ����������
		Heatmax = 150;
	if(Game_robot_state.robot_level == 2)
		Heatmax = 280;
	if(Game_robot_state.robot_level == 3)
		Heatmax = 400;
	Shootnumber = (Heatmax - Umpire_PowerHeat.shooter_id1_17mm_cooling_heat)/10;//�ɷ����ӵ�����


	if( Shootnumber<=5)//�ѷ����ӵ������ڵ��ڿɷ����ӵ���ʱ����ͣת
	{  
	  SafeHeatflag = 0;
	  trigger_moto_current = 0;
	  Shootnumber_fired = 0;
	}
	else
	  SafeHeatflag	= 1;

	if(Stronghold_flag)//����ϵͳ���߽�������ջ���Ȼ���䲻���ӵ�
	  SafeHeatflag	= 1;

	 if(speed_17mm_level_Start_Flag && SafeHeatflag)//Ħ���ִ�ʱ���ֲ�������
	//if(speed_17mm_level_Start_Flag)//Ħ���ִ�ʱ���ֲ�������	 
	{
		 laser_on();//�������
		shoot_task1();//���ֿ���			
	}	 
	if(!speed_17mm_level_Start_Flag)
	{
		laser_off();//�رռ���
		trigger_moto_current = 0;
	}
}



