#ifndef __USERAPI_H
#define __USERAPI_H
#include "sys.h"


typedef struct
{
	float Vx;			//ǰ���ٶ�
	float Vy;			//�����ٶ�
	float Chassis_Wz;			//��ת�ٶ�
	float Vx_6020;
	float Vy_6020;
	float Chassis_Wz_6020;
	float Gimbal_Wz;
	float Gimbal_Wzlast;
	float Pitch_angle;	//pitch��Ƕ�
	
}ControlDATA_TypeDef;

//�������
typedef struct
{
	u16 Frequency;	//��Ƶ
	float Speed;		//����
	
}ShootProject_TypeDef;

extern ControlDATA_TypeDef Control_data;//��������
extern u8 Twist_Flag;				//Ť��
extern u8 Vision_Flag;			//�Ӿ�����
extern u8 Shoot_Flag;				//���
extern u8 Shoot_Motor;			//Ħ����
extern u8 chassis_power_level_1_Flag;   //�������̹���1��
extern u8 chassis_power_level_2_Flag;   //�������̹���2��
extern u8 chassis_power_level_3_Flag;   //�������̹���3��
extern u8 chassis_power_level_1_once_Flag;   //�������̹���1����һ��
extern u8 chassis_power_level_2_once_Flag;   //�������̹���2����һ��
extern u8 chassis_power_level_3_once_Flag;   //�������̹���3����һ��
extern u8 speed_17mm_level_Start_Flag;
extern u8 speed_17mm_level_Low_Flag;
extern u8 speed_17mm_level_High_Flag;
extern u8 speed_17mm_level_Low_once_Flag;
extern u8 speed_17mm_level_High_once_Flag;
extern u8 Gimbal_180_flag;
extern u8 close_combatflag;
extern u8 Steering_gear_test;
extern u8 Shoot_Long;
extern u8 SpinTop_Flag;     //С���ݱ�־
extern int SafeHeatflag;
extern u8 Stronghold_flag;
extern int Chassismode_flag;
extern int Gimbalmode_flag;
extern int Shootnumber;
extern int Shootspeed; 
extern int Shootnumber_fired;
extern int Wheel_position;
extern float speed_zoom;
extern u8 Cap_Safe_Flag;
extern float Ramp_K;
extern int32_t VerticalCnt;
extern int32_t HorizontalCnt;
extern u8 Usart_send_Flag;

	//���̹������Ʋ�����
extern int32_t Horizontal_Initspeed;
extern int32_t Vertical_Initspeed;
extern int32_t Horizontal_Maxspeed;
extern int32_t Vertical_Maxspeed;
extern float speed_zoom;
void User_Api(void);
void computer_ctrl(void);
void computer_handle(void);
void FRT_computer_ctrl(void *pvParameters);
static void ShooterHeat_Ctrl(void);
void chassis_control_acquisition(void);
void gimbal_control_acquisition(void);
/* ��ֵ���ƺ����궨�� */

#define VAL_LIMIT(val,  min, max)\
if((val) <= (min))\
{\
  (val) = (min);\
}\
else if((val) >= (max))\
{\
  (val) = (max);\
}\




#endif


