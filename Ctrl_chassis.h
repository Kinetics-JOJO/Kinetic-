#ifndef CHASSIS_H
#define CHASSIS_H
#include "sys.h"


typedef struct Motor_Pid	//λ�û�pid���ݽṹ
{
	int old_aim;			//��Ŀ������ֵ
	int old_err;			//�ɲ�ֵ
	int16_t output;		//���
}M_pid;


typedef struct
{
	s16 RVx,Vx;//ʵ���ٶȣ��趨�ٶ�
	s16 RVy,Vy;
	s16 AngleCompensate;//��ǰ�ǶȲ���
	float Angle;//��ǰ�Ƕ��趨
	float SAngle,RAngle;//ң���������Ƕȣ�ʵ�ʽǶ�ֵ
}SpinTop_t;

extern SpinTop_t SpinTop;
static void motor_speed_ctrl(int16_t M1_speed,int16_t M2_speed,int16_t M3_speed,int16_t M4_speed);
static void Inverse_Kinematic_Ctrl(float Vx,float Vy,float Wz);
static void Chassis_AngleInverse_Ctrl(float Angle_Vx,float Angle_Vy,float Angle_Wz);
static void Chassis_6020Inverse_ctrl(int16_t M16020_angle,int16_t M26020_angle,int16_t M36020_angle,int16_t M46020_angle);
void FRT_Inverse_Kinematic_Ctrl(void *pvParameters);
void Chassis_Encoder_angle_Handle(void);//��̨�����ת�ǶȺ���
void Zero_handler(int16_t Angle_Val);
#endif
