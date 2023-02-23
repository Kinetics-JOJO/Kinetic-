#include "Ctrl_shoot.h"
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
#include "laser.h"
#include "can1.h"
#define Butten_Trig_Pin GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)

int32_t trigger_moto_position_ref2;//Ŀ��Ƕ�
int16_t trigger_moto_speed_ref2;//Ŀ���ٶ�  //103��
int16_t trigger_moto_current;//���������ֵ

int32_t round_cnt = 0;//���ֵ����תȦ��

int block_flag = 0;//��ת��־
int block_time = 0;//��תʱ��
int blockruntime = 0;//��ת����

int block_flag1 = 0;//��ת��־
int block_time1 = 0;//��תʱ��
int blockruntime1 = 0;//��ת���� 

int time = 0;//��������
int time2 = 0;

int single_flag = 0;
int continue_flag = 0;

int32_t Magazine_motor_encode_target;
int32_t Magazine_motor_encode_target_buff = 0;
float degree_60_to_encode_ramp = 73728/128;//25941/1024;
float degree_60_to_encode = 0;//25941;//25941;
int degree_60_to_encode_cnt = 0;
int continue_Flag;
int continue_mouse_Flag;

int trigger_motor_key_time = 0;
int shoot_ready_flag = 0;
int Butten_Trig_Pin_flag = 0;


//���������ʼ����
#define FireMotor_Direction 1 //���䲦�̵�ת������
#define T_Dect 380 //��ʼ��ת��ʱ��  ������ת��Ƶ��  //ͬʱת�ؼ�������Ҳ�� ������ת��Ƶ��
#define Rev_T  100//��ת��ʱ��  ������ת�ķ���


void Encode_C(M_Data*ptr)
{
	if (abs(ptr->NowAngle - ptr->OldAngle) > 4095)
  {
    if(ptr->NowAngle < ptr->OldAngle)
		{
		   ptr->total_encode += 8191 - ptr->OldAngle + ptr->NowAngle;
		}
		else
		{
			ptr->total_encode -= 8191 - ptr->NowAngle + ptr->OldAngle;
		}
  }
  else 
	{
    ptr->total_encode += ptr->NowAngle - ptr->OldAngle;
	}		
}

float RAMP_float( float final, float now, float ramp )
{
	  float buffer = 0;
	
	
	  buffer = final - now;
	
		if (buffer > 0)
		{
				if (buffer > ramp)
				{  
						now += ramp;
				}   
				else
				{
						now += buffer;
				}
		}
		else
		{
				if (buffer < -ramp)
				{
						now += -ramp;
				}
				else
				{
						now += buffer;
				}
		}
		
		return now;
}

void shoot_ready_control(void)
{
	//block_flag = 0;
	block_time = 0;
	blockruntime = 0;
	//continue_flag = 0;
	RC_Ctl.rc.s1_last = RC_Ctl.rc.s1;
	RC_Ctl.mouse.press_l_last = RC_Ctl.mouse.press_l;
	if(Butten_Trig_Pin)////�ж��ӵ�����΢�����ش�   //!Butten_Trig_Pin����΢������ 
	{
		trigger_moto_current = PID_Control(&Fire_speed_pid, 0 ,motor_data[6].ActualSpeed);//	ע������trigger_moto_current����ֱ�Ӹ�0��Ȼ�ᵼ�º��淢���ʱ���̶�����;
		trigger_motor_key_time = 0;
		Butten_Trig_Pin_flag = 1;
	}

	else if(!Butten_Trig_Pin && trigger_motor_key_time < 1000)//�ж����ӵ�һ��ʱ��  //���������뵥��ģʽ�µĽ��
	{  
		Butten_Trig_Pin_flag = 2;
		if(single_flag)//����ģʽת����Լ�����Ƕ�ͣת
		{
	//		 trigger_moto_speed_ref2 = PID_Control(&Fire_pid, trigger_moto_position_ref2, motor_data[6].total_ecd);
	//     trigger_moto_current = PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2 ,motor_data[6].ActualSpeed);		
			if(trigger_motor_key_time > 20)//����ģʽ��ǿ�Ƽ���ֹͣ//��Ȼ��֪���������
			trigger_moto_current = PID_Control(&Fire_speed_pid, 0 ,motor_data[6].ActualSpeed);//PID�ٶȻ�//	ע������trigger_moto_current����ֱ�Ӹ�0
		}
		if(continue_flag)//����ģʽֱ��ͣת//����������������������������л�Ч�����л��м�ͣ��һ��
		trigger_moto_current = PID_Control(&Fire_speed_pid, 0 ,motor_data[6].ActualSpeed);//PID�ٶȻ�	
		trigger_motor_key_time++;
	}

	else if(!Butten_Trig_Pin && trigger_motor_key_time == 1000)//΢������һ��ʱ��û���ӵ������벦��
	{
		//Fire_PID_Init();
		Butten_Trig_Pin_flag = 2;
		if(!block_flag1)
		{
		  trigger_moto_speed_ref2 = 900*FireMotor_Direction;//΢������û��⵽�ӵ��������Զ�ת
		  trigger_moto_current = PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//΢�����������еĿ�����ת����
		//���ǵ��������������ڼ䲦��ת������ʹ�ü���ٶȶ����Ǽ��ת��
		//if(!motor_data[6].ActualSpeed)//������ת����
		if(motor_data[6].ActualSpeed < 20)//������ת����	
		  blockruntime1++;
		else if(motor_data[6].ActualSpeed < 0)//debug����һ��
		  blockruntime1 = 0;	
		if(blockruntime1 > 100) //��ʼ��ת��ʱ�� ������ת��Ƶ��
		{
		  trigger_moto_speed_ref2 = -FireMotor_Direction*5000;	//������ת������
		  block_flag1 = 1;
		}
		if(block_flag1)
		{
			block_time1++;
			if(block_time1 > 40)//��ת��ʱ�� ������ת�ķ���
			{
				block_flag1 = 0;
				block_time1 = 0;
				blockruntime1 = 0;
			}	
		}
	//΢�����������еĿ�����ת����
	//���ת��
	//	if(motor_data[6].Intensity>3000)
	//	blockruntime1++;
	//	if(blockruntime1 > 30)
	//	  {
	//		 trigger_moto_speed_ref2 = -FireMotor_Direction*4000;	
	//	   block_flag1 = 1;
	//		}
	//		if(block_flag1)
	//		{
	//			block_time1++;	
	//			if(block_time1 > 50)
	//			{
	//				block_flag1 = 0;
	//				block_time1 = 0;
	//				blockruntime1 = 0;
	//			}
	//		}
			
		if(block_flag1)
		  trigger_moto_current = PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));	
	}
}


void shoot_task(void)//ң����ģʽ���Ʋ���
{
	block_flag1 = 0;
  blockruntime1 = 0;
	trigger_motor_key_time = 0;
	Encode_C(&motor_data[6]);
	if(!block_flag)
	{
	if(RC_Ctl.rc.s1 == RC_SW_DOWN && RC_Ctl.rc.s1_last != RC_SW_DOWN)
	{
	  time = xTaskGetTickCount();
		single_flag = 1;
    continue_flag = 0;			
		trigger_moto_position_ref2 = motor_data[6].total_encode + FireMotor_Direction*DEGREE_60_TO_ENCODER;
	}
		
	trigger_moto_speed_ref2 = PID_Control(&Fire_pid, trigger_moto_position_ref2, motor_data[6].total_encode);
	trigger_moto_speed_ref2 = -trigger_moto_speed_ref2;
	
  if(RC_Ctl.rc.s1 == RC_SW_DOWN && xTaskGetTickCount() - time > 500)//�󲦸�һֱ��������ģʽ  �����ӵ���
	{
    trigger_moto_speed_ref2 = FireMotor_Direction*5000;
		single_flag = 0;
    continue_flag = 1;		
	}

	PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));	

	trigger_moto_current = Fire_speed_pid.Control_OutPut;
	
	RC_Ctl.rc.s1_last = RC_Ctl.rc.s1;
  }
  
	block_bullet_handle();
	//����ֵǿ������
	if((RC_Ctl.rc.s1 != RC_SW_DOWN))
	{
	  trigger_moto_current = 0;
    blockruntime = 0;	
	}
	
}

//����ģʽ���Ʋ���
void shoot_task1()//����ģʽ���Ʋ���
{
	block_flag1 = 0;
  blockruntime1 = 0;
	trigger_motor_key_time = 0;
	Encode_C(&motor_data[6]);
	if(!block_flag)
	{
		if(RC_Ctl.mouse.press_l && RC_Ctl.mouse.press_l != RC_Ctl.mouse.press_l_last)
		{
			time = xTaskGetTickCount();
			single_flag = 1;
			continue_flag = 0;			
			trigger_moto_position_ref2 = motor_data[6].total_encode + FireMotor_Direction*DEGREE_60_TO_ENCODER;
		}
		
		trigger_moto_speed_ref2 = PID_Control(&Fire_pid, trigger_moto_position_ref2, motor_data[6].total_encode);
	  trigger_moto_speed_ref2 = -trigger_moto_speed_ref2;
		
		if(RC_Ctl.mouse.press_l && xTaskGetTickCount() - time > 500)//��������������ģʽ
		{
			single_flag = 0;
			continue_flag = 1;		
			if(Stronghold_flag)
			{
				trigger_moto_speed_ref2 = 5000;  
			}		
			else
			{
				//trigger_moto_speed_ref2 = TRIGGER_MOTOR_SPEED*Shootnumber;
				trigger_moto_speed_ref2=6000;
//				if(trigger_moto_speed_ref2 >= 5000) //500
//					 trigger_moto_speed_ref2 = 5000; //1000
			}

		}	
		
		PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));	

		trigger_moto_current = Fire_speed_pid.Control_OutPut;

		RC_Ctl.mouse.press_l_last = RC_Ctl.mouse.press_l;//��¼�ϴ����������
  }
	
  block_bullet_handle();
	
  if(!RC_Ctl.mouse.press_l)//�������ɿ�
	{
    blockruntime = 0;	
	}		
}



//�����Զ���ת����
//�˴���ת�봥�����ش�����ͻ��������������ķ�ת������ķ�ת�����Ƕ�����
void block_bullet_handle()//�����Զ���ת60��
{
	if((motor_data[6].ActualSpeed < 20 && RC_Ctl.rc.s1 == RC_SW_DOWN) | (motor_data[6].ActualSpeed < 20 && RC_Ctl.mouse.press_l))//����ģʽ������ת//400 400
	  blockruntime++;
	else if((motor_data[6].ActualSpeed < 0 && RC_Ctl.rc.s1 == RC_SW_DOWN)| (motor_data[6].ActualSpeed < 0 && RC_Ctl.mouse.press_l))
	  blockruntime = 0;	
	

	if(blockruntime > 45) //100
	{
	  trigger_moto_speed_ref2 = -7500;  //8500
	  block_flag = 1;
	}
	
	if(block_flag)
	{
		block_time++;
		if(block_time > 55)  //150
		{
			blockruntime = 0;
			block_flag  = 0;
			block_time = 0; 		
		}	
  }
	if(block_flag)
    trigger_moto_current = PID_Control(&Fire_speed_pid, trigger_moto_speed_ref2, (motor_data[6].ActualSpeed));
}

////Canͨ��Ħ����
//void Can_Fri_Ctrl(int16_t Fri_LeftTarget,int16_t Fri_RightTarget)
//{
//	//���ٶȻ����ٴ���
//	//����ǰ�����PID����
//	PID_Control(&Fir_Left_pid,Fri_LeftTarget,motor_data[7].ActualSpeed);
//	PID_Control(&Fir_Right_pid,Fri_RightTarget,motor_data[8].ActualSpeed);
//	
//}


