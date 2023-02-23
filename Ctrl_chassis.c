#include "Ctrl_chassis.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "Configuration.h"
#include "PID.h"
#include "can1.h"
#include "can2.h"
#include <math.h>
#include <stdlib.h>
#include "Higher_Class.h"
#include "RemoteControl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "User_Api.h"
#include "Comm_umpire.h"
#include "Ctrl_gimbal.h"
#include "laser.h"
#include "math.h"
#include "Usart_SendData.h"
#define CHASSIS_MAX_SPEED ((LIM_3510_SPEED/60)*2*PI*WHEEL_R/MOTOR_P)//��������ٶȣ�mm/s
SpinTop_t SpinTop={0};
float SpinTop_Angle;
float W1_encode_angle,W2_encode_angle,W3_encode_angle,W4_encode_angle;
float Total_speed;
#define Chassis_DPI 0.0439453125f	//��ֵ�Ƕȷֱ��ʣ�360/8192
/*********************************************************************
 *ʵ�ֺ�����void virtual_encoders(int16_t aim[4])
 *��    �ܣ�������������
 *��    �룺������Ŀ��ֵ
 *˵    ����
 ********************************************************************/
int16_t Encoders[4];		//��������ֵ
void virtual_encoders(int16_t aim[4])
{
	uint8_t i;
	for(i=0; i<4; i++)
	{
		Encoders[i] += aim[i];
		if (Encoders[i]>8191) Encoders[i] -= 8192;
		else if (Encoders[i]<0) Encoders[i] += 8192;
	}
}
/*********************************************************************
 *ʵ�ֺ�����void Zero_handler(int16_t Angle_Val)
 *��    �ܣ�����ֵ��0����
 *��    �룺������ĽǶ�ֵ
 *˵    ����
 ********************************************************************/
void Zero_handler(int16_t Angle_Val)
{
	if(Angle_Val>180)
	{
		Angle_Val-=360;
	}
	else if(Angle_Val<-180)
	{
		Angle_Val+=360;
	}
	
}
/*********************************************************************
 *ʵ�ֺ�����void Chassis_Encoder_angle_Handle(void)
 *��    �ܣ���̨�����ת�ǶȺ���
 *��    �룺NONE
 *˵    ����
 ********************************************************************/
void Chassis_Encoder_angle_Handle(void)
{
	//can2��������ʵʱ����ֵ
	float W1_encode = motor2_data[1].NowAngle;
	float W2_encode = motor2_data[2].NowAngle;
	float W3_encode = motor2_data[3].NowAngle;
	float W4_encode = motor2_data[4].NowAngle;
	
	//	float W4_encode = motor_data[5].NowAngle;
	//���̻�������ĽǶ�ֵ
  W1_encode_angle=((W1_encode-W1_Mid_encode)*Chassis_DPI)*Wheel_Direction;
	W2_encode_angle=((W2_encode-W2_Mid_encode)*Chassis_DPI)*Wheel_Direction;
	W3_encode_angle=((W3_encode-W3_Mid_encode)*Chassis_DPI)*Wheel_Direction;
	W4_encode_angle=((W4_encode-W4_Mid_encode)*Chassis_DPI)*Wheel_Direction;
	//��0����
	Zero_handler(W1_encode_angle);
	Zero_handler(W2_encode_angle);
	Zero_handler(W3_encode_angle);
	Zero_handler(W4_encode_angle);

}


/*********************************************************************
 *ʵ�ֺ�����int encoders_err(int8_t i,int error)
 *��    �ܣ�����ʵ��Ŀ������ �� Ŀ���������̲� ֵ
 *��    �룺���ʶ��������ڲ�ֵ
 *��    �أ�ʵ��Ŀ������ �� Ŀ���������̲� ֵ
 *˵    ����
 ********************************************************************/
int encoders_err(int8_t i,int error)
{
	int temp;
	
	temp = motor_data[i].NowAngle + error;
	temp = temp%8192;	
	if (temp<0) temp += 8192;						//ʵ��Ŀ������ֵ
	temp = Encoders[i] - temp;
	if (temp<-3000) temp += 8192;
	else if (temp>3000) temp -= 8192;
	
	return temp;
}


/***************************
 *���̵���ٶȿ���
 *���룺4�������Ŀ���ٶ�
 ***************************/
static void motor_speed_ctrl(int16_t M1_speed,int16_t M2_speed,int16_t M3_speed,int16_t M4_speed)
{
	u8 i;
	int16_t target_speed[4];
	
	target_speed[0]=M1_speed;	target_speed[1]=M2_speed;
	target_speed[2]=M3_speed;	target_speed[3]=M4_speed;
	
	for(i=0;i<4;i++)
		PID_Control(&motor_speed_pid[i],target_speed[i],motor_data[i].ActualSpeed);
}

/***************************
 *���ֻ������Ƕȿ��ƣ�˫����
 *���룺4�������Ŀ��Ƕ�
 ***************************/
static void Chassis_6020Inverse_ctrl(int16_t M16020_angle,int16_t M26020_angle,int16_t M36020_angle,int16_t M46020_angle)
{
	
	int16_t target_angle[4];

	target_angle[0]=M16020_angle;	target_angle[1]=M26020_angle;
	target_angle[2]=M36020_angle;	target_angle[3]=M46020_angle;
	

	PID_Control(&chassis_angle_pid[0],target_angle[0],W1_encode_angle);
	PID_Control(&chassis_speed_pid[0],chassis_angle_pid[0].Control_OutPut,motor2_data[1].ActualSpeed);
	
	PID_Control(&chassis_angle_pid[1],target_angle[1],W2_encode_angle);
	PID_Control(&chassis_speed_pid[1],chassis_angle_pid[1].Control_OutPut,motor2_data[2].ActualSpeed);
	
	PID_Control(&chassis_angle_pid[2],target_angle[2],W3_encode_angle);
	PID_Control(&chassis_speed_pid[2],chassis_angle_pid[2].Control_OutPut,motor2_data[3].ActualSpeed);
	
	
	
	PID_Control(&chassis_angle_pid[3],target_angle[3],W4_encode_angle);
	PID_Control(&chassis_speed_pid[3],chassis_angle_pid[3].Control_OutPut,motor2_data[4].ActualSpeed);

//Ԥ�����ID4 6020 �����can1���з�������
//	PID_Control(&chassis_angle_pid[3],target_angle[3],W4_encode_angle); 
//	PID_Control(&chassis_speed_pid[3],chassis_angle_pid[3].Control_OutPut,motor_data[5].ActualSpeed);
	

}

/***************************
 *���̵����ֵ���ƣ�λ�ñջ�v2.0
 *���룺4�������ÿ����Ŀ���ٶ�
 *�������̲�ֵ������Ŀ���ٶ��ϣ�����������̲�ֵ��pid�����ַ�����Ӧ���ܸ��ø�ƽ��
 ***************************/
static M_pid angle_pid[4];		//���pid�ṹ��
void motor_angle_ctrl(int16_t M1_speed,int16_t M2_speed,int16_t M3_speed,int16_t M4_speed)
{
	u8 i;
	float P,D;
	int error,temp;
	
	int16_t target_speed[4];
	int16_t target_angle[4];
	
	target_speed[0]=M1_speed;	target_speed[1]=M2_speed;
	target_speed[2]=M3_speed;	target_speed[3]=M4_speed;
	
	for(i=0;i<4;i++)//ת��Ϊÿ����Ŀ����ֵ
		target_angle[i] = (((float)target_speed[i]/ 60) * 8192)/PID_Hz;
	
	if(Encoders[0]==0)
	{
		if(Encoders[1]==0)
		{
			Encoders[0] = motor_data[0].NowAngle;
			Encoders[1] = motor_data[1].NowAngle;
			Encoders[2] = motor_data[2].NowAngle;
			Encoders[3] = motor_data[3].NowAngle;
		}
	}
	virtual_encoders(target_angle);	//������������ֵ
	
	for (i=0; i<4; i++)
	{
		P=0; D=0;									//�м�ֵ����
		error = angle_pid[i].old_aim - motor_data[i].D_Angle;	//��һ��δ��ɲ�ֵ
		error += encoders_err(i,error);					//�ۼ�ʵ������ֵ����������ֵ�Ĳ�
		angle_pid[i].old_aim = error + target_angle[i];					//���¾�Ŀ��ֵ
		
		//**********P��**************
		P = PID_MOTOR_ANGLE_KP * error;
		//**********D��**************
		D = (error - angle_pid[i].old_err) * PID_MOTOR_ANGLE_KD;
		angle_pid[i].old_err = error;
		temp = P - D;
		if (temp > LIM_3508_SPEED) angle_pid[i].output = LIM_3508_SPEED;			//Ŀ��ת�ٲ�������޷�
		else if(temp < -LIM_3508_SPEED) angle_pid[i].output = -LIM_3508_SPEED;
		else angle_pid[i].output = temp;
		if ((angle_pid[i].output<70) && (angle_pid[i].output>-70)) angle_pid[i].output = 0;
		
		//���뵽�ٶȻ�
		temp = angle_pid[i].output + target_speed[i];	//�������Ŀ���ٶ�
		PID_Control(&motor_speed_pid[i],temp,motor_data[i].ActualSpeed);
	}
	    

}


/***************************
 *���˶�ѧ����
 *�Ƶ��̵�ˮƽ�ٶȡ����ٶ�
 *����Ϊ���̵��˶�Ŀ�꣬�ֱ�ΪVx��Vy��Wv����λmm/s,rad/s,ǰ��ΪVx
 *������ǰ��Ϊw0����ʱ��˳��
 ***************************/
#ifdef CHASSIS_POSITION_CRTL
//#define ANGLE_CTRL		//ʹ��λ�û�
#endif

int16_t Chassis_mode=0;//����ģʽĬ��Ϊ0����Ϊ�����е�ֵ

int debug_testflag=0;
float chassis_theta_rad=0;
float Real_angle=0;
int nan_flag=0;

//����״̬��
#define RIGHT 2
#define LEFT  3
#define RIGHT_ROTATE 4
#define LEFT_ROTATE 5
#define RFLB_45 6
#define LFRB_45 7
#define SpinTop_On 1
#define STRAIGHT_BACK_WARD 0
#define STA_ZONE 40 //�������䷶Χ+-40%ң��
//@Input:Control_data.Vx_6020,Control_data.Vy_6020,Control_data.Chassis_Wz_6020
/*********************************************************************
 *ʵ�ֺ�����void Chassis_6020position_judge(float Vx_judge,float Vy_judge,float Wx_judge)
 *��    �ܣ��жϵ��̵�ǰ״̬��������debug�������Ե��̸���״̬�½��д���
 *��    �룺Control_data.Vx_6020,Control_data.Vy_6020,Control_data.Chassis_Wz_6020
 *��    �أ���״̬����ʶ��
 *˵    ����
 ********************************************************************/
void Chassis_6020position_judge(float Vx_judge,float Vy_judge,float Wx_judge)
{
	
	 if(SpinTop_Flag==1)  //��С����ģʽ������ĳ�ͻ����������������жϴ���
		Chassis_mode = SpinTop_On;
	else
	 {
	if((Real_angle>90)&&(Control_data.Chassis_Wz_6020==0))
	  Chassis_mode = RIGHT;
	else if ((Real_angle<-90)&&(Control_data.Chassis_Wz_6020==0))
		Chassis_mode = LEFT;
	else if((Control_data.Chassis_Wz_6020>0)&&(SpinTop_Flag==0))
	  Chassis_mode = RIGHT_ROTATE;
	else if ((Control_data.Chassis_Wz_6020<0)&&(SpinTop_Flag==0))
		Chassis_mode = LEFT_ROTATE;
	//��ǰ������ۺ�����д���Ƕȹ̶�����һ�£�ֻ��3508����ٶȻ������෴

	else if((Real_angle>0)&&(Real_angle<90)) 
		Chassis_mode = RFLB_45;
	else if ((Real_angle<0)&&(Real_angle>-90)) 
		Chassis_mode = LFRB_45;
	else
		Chassis_mode = STRAIGHT_BACK_WARD;
	}
}



/*********************************************************************
 *ʵ�ֺ�����void Chassis_position_Ctrl(void)
 *��    �ܣ�״̬���µĵ��λ�û�����
 *��    �룺NONE
 *��    �أ�chassis_theta_rad�������Ƶ�λ����Real_angle�������ң��ת����ʵ�ʽǶ�ֵ��
 *˵    ����ң�п���״̬����ֵΪnan��atan����ֵ��Ϊ-pi/2 ~ pi/2 
 ********************************************************************/
void Chassis_position_Ctrl(void)
{
	
	chassis_theta_rad=atan(Control_data.Vy_6020/Control_data.Vx_6020);    //atan����-pi/2 ~ pi/2 
	Real_angle=chassis_theta_rad*180/PI;
//ң�п���״̬����ֵΪnan������������
	if(Real_angle!=Real_angle)
	{
		Real_angle=0;
		nan_flag=1;
	}else nan_flag=0;


	if((Chassis_mode==4)||(Chassis_mode==5)||(Chassis_mode==1))//LEFT_ROTATE mode=5 RIGHT 6   Spin_Top_ON 1
	Chassis_6020Inverse_ctrl(-45,45,-45,45);
	

	else if((Chassis_mode==6)||(Chassis_mode==7))//STRAIGHT_BACK_WARD mode=0
	Chassis_6020Inverse_ctrl(Real_angle,Real_angle,Real_angle,Real_angle);
	else//����ƽ��ʱ
	Chassis_6020Inverse_ctrl(Real_angle,Real_angle,Real_angle,Real_angle);
}


//@Input:Control_data.Vx_6020,Control_data.Vy_6020,Control_data.Chassis_Wz_6020
static void Chassis_AngleInverse_Ctrl(float Angle_Vx,float Angle_Vy,float Angle_Wz)
{
	
	Chassis_Encoder_angle_Handle();//������̽ǶȻ��㴦��
	Chassis_6020position_judge(Angle_Vx,Angle_Vy,Angle_Wz);
  Chassis_position_Ctrl();
  CAN2_Send_Msg_chassis_turnover(chassis_speed_pid[0].Control_OutPut,chassis_speed_pid[1].Control_OutPut,chassis_speed_pid[2].Control_OutPut,chassis_speed_pid[3].Control_OutPut);
	
	
	//CAN1_Send_Msg_chassis6020(chassis_speed_pid[3].Control_OutPut);
	//CAN2_Send_Msg_chassis_turnover(0,0,0,0);
}



static void Inverse_Kinematic_Ctrl(float Vx,float Vy,float Wz)
{
	float w[4];		//�ĸ����ӵ�ʵ��ת��rad/s
	int16_t n[4];	//ת��Ϊ�����ٶȵ��ĸ������ת��
	uint8_t i=0;
	

	SpinTop.Angle  = (motor_data[4].NowAngle - Yaw_Mid_encode + 8192) % 8192 / 22.7555556f;
	if(SpinTop.Angle > 360)
		SpinTop.Angle = (SpinTop.Angle - 360) * 0.0174532f;
	else
		SpinTop.Angle *= 0.0174532f;

  SpinTop_Angle = SpinTop.Angle * (180 / PI);
	
	SpinTop.Vx = Control_data.Vx * cos(SpinTop.Angle) + Control_data.Vy * sin(SpinTop.Angle);
	SpinTop.Vy = -Control_data.Vx * sin(SpinTop.Angle) + Control_data.Vy * cos(SpinTop.Angle);
		
	
	
	Total_speed=Vy + Vx;
	
//���ֻ���3508�ٶȻ�

//	w[0] = (-SpinTop.Vy - SpinTop.Vx +CHASSIS_K*Wz)/WHEEL_R*speed_zoom;    //speed_zoom ��������ϵ��  ��ǰ��
//	w[1] = (SpinTop.Vy + SpinTop.Vx + CHASSIS_K*Wz)/WHEEL_R*speed_zoom;   //��ǰ
//	w[2] = (SpinTop.Vy + SpinTop.Vx + CHASSIS_K*Wz)/WHEEL_R*speed_zoom;  //���
//	w[3] = (-SpinTop.Vy - SpinTop.Vx + CHASSIS_K*Wz)/WHEEL_R*speed_zoom;  //�Һ�
//		
 if(Chassis_mode==2)
	 
 {
	 
	w[0] = (Vy + Vx -CHASSIS_K*Wz)/WHEEL_R;
	w[1] = (-Vy - Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[2] = (-Vy - Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[3] = (Vy + Vx - CHASSIS_K*Wz)/WHEEL_R;

	 
	 
	 
 }else if(Chassis_mode==6)
 
 {
	 
	w[0] = (   Vx -CHASSIS_K*Wz)/WHEEL_R;
	w[1] = (   -Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[2] = (   -Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[3] = (   Vx - CHASSIS_K*Wz)/WHEEL_R;
	 
 }
 
 else
 
{
	w[0] = (-Vy + Vx -CHASSIS_K*Wz)/WHEEL_R;
	w[1] = (Vy - Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[2] = (Vy - Vx - CHASSIS_K*Wz)/WHEEL_R;
	w[3] = (-Vy + Vx - CHASSIS_K*Wz)/WHEEL_R;

}
  //}


	for(i=0;i<4;i++)
	 n[i] = ((float)w[i]*MOTOR_P / (2*PI)) * 60;	//ת��Ϊ��������ٶ�

	//����б���ߵ��ٶ�
	for(i=0;i<4;i++)
	{
		if(n[i] > LIM_3508_SPEED)
		{
			uint8_t j=0;
			float temp = (float)n[i] / LIM_3508_SPEED;	//����
			
			for(j=0;j<4;j++)	n[j] = (float)n[j]/temp;		//�ȱ�����С
		}
		else if(n[i] < -LIM_3508_SPEED) 
		{
			uint8_t j=0;
			float temp = -(float)n[i] / LIM_3508_SPEED;	//����
			
			for(j=0;j<4;j++)	n[j] = (float)n[j]/temp;		//�ȱ�����С
		}
	}
//	#ifdef ANGLE_CTRL	//λ�ñջ�
//	motor_angle_ctrl(n[0],n[1],n[2],n[3]);
//	#else	//�ٶȱջ�
	motor_speed_ctrl(n[0],n[1],n[2],n[3]);
//	#endif
	CAN1_SendCommand_chassis(motor_speed_pid[0].Control_OutPut,motor_speed_pid[1].Control_OutPut,
														motor_speed_pid[2].Control_OutPut,motor_speed_pid[3].Control_OutPut);

}


void FRT_Inverse_Kinematic_Ctrl(void *pvParameters)//���̿�������
{
	vTaskDelay(357);
	while(1)
	{ 
		if ( RC_Ctl.rc.s2 == RC_SW_DOWN)
		{
	    Power_off_function();		
		  //laser_off();		
		}
		else
		{
		  //number_t(2);	
		  //laser_on();
		  //Chassis_power_level();
		  Chassis_Power_Limit(); //ͨ���޸�б������б�ʣ��Լ���������ϵ�����ﵽЧ��
	    chassis_control_acquisition();//���̿�������ȡ��״̬������
		  chassis_set_contorl();//���̿���������
				
			Chassis_AngleInverse_Ctrl(Control_data.Vx_6020,Control_data.Vy_6020,Control_data.Chassis_Wz_6020);//���ֲ������̽Ƕ��ܿ���
			Inverse_Kinematic_Ctrl(Control_data.Vx,Control_data.Vy,Control_data.Chassis_Wz);//���̿��ƽӿ�
		}
	  vTaskDelay(2);
	}
}

