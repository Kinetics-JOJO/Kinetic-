#include "PID.h"
#include "Configuration.h"
#include <math.h>
#include <stdlib.h>
#include <pwm.h>
/*
*PID���㡢PID���ݳ�ʼ��
*/

//******************������̨YAW�����******************
float VISION_PID_YAW_SPEED_KP = 150.0f;  //200.0f
float VISION_PID_YAW_SPEED_KI = 0;//0.2f;//2.0f;
float VISION_PID_YAW_SPEED_KD = 0;//200.0f;//100.0f;
float VISION_PID_YAW_ANGLE_KP = 6.0f;  //8.0f
float VISION_PID_YAW_ANGLE_KI = 0;//0.2f;//0.002f;
float VISION_PID_YAW_ANGLE_KD = 30.0f;  //20.0f


//*****************������̨PITCH�����*****************
float VISION_PID_PITCH_SPEED_KP = 30.0f;
float VISION_PID_PITCH_SPEED_KI = 0;
float VISION_PID_PITCH_SPEED_KD = 0;
float VISION_PID_PITCH_ANGLE_KP = 3.0f;
float VISION_PID_PITCH_ANGLE_KI = 0;
float VISION_PID_PITCH_ANGLE_KD = 0;


//********************��̨yaw�����********************
float PID_YAW_SPEED_KP = 150;	//300	//150
float PID_YAW_SPEED_KI = 0;  //1//0.1f
float PID_YAW_SPEED_KD = 0;//10;
float PID_YAW_ANGLE_KP = 30;	  //10  //5  //10
float PID_YAW_ANGLE_KI = 0;//0.0002;//0.002 //0.0002
float PID_YAW_ANGLE_KD = 0;


//*******************��̨pitch�����*******************
float PID_PITCH_SPEED_KP = 180.0f;//300.0f
float PID_PITCH_SPEED_KI = 0.2f;//1.8f;//0.00001f;//0.015f;//0.0001f;//.00000001f;  0.35f 0.2f
float PID_PITCH_SPEED_KD = 0;//0
float PID_PITCH_ANGLE_KP = 28;//28
float PID_PITCH_ANGLE_KI = 0.00001f;//0.001f;//0.001f;//0.002f;//0.1f;//.0000001f;//.0000001f;//.0001f;
float PID_PITCH_ANGLE_KD = 20;//0.0f;


PID_Controler vision_yaw_speed_pid = {0};  //yaw�������ٶȻ�pid
PID_Controler vision_yaw_angle_pid = {0};  //yaw������ǶȻ�pid
PID_Controler vision_pitch_speed_pid = {0};   //pitch�������ٶȻ�pid 
PID_Controler vision_pitch_angle_pid = {0};   //pitch������ǶȻ�pid 
PID_Controler motor_speed_pid[4];	  //���̵���ٶȻ�pid����
/***************************************************************************/
PID_Controler chassis_speed_pid[4];//���ֵ���6020����ٶȻ�pid����				 *	
PID_Controler chassis_angle_pid[4];//���ֵ���6020����ǶȻ�pid����         *
/***************************************************************************/

PID_Controler Yaw_speed_pid = {0};	 //��̨yaw����ٶ�
PID_Controler Yaw_pid = {0};
PID_Controler Pitch_speed_pid = {0};   //��̨pitch����ٶ�
PID_Controler Pitch_angle_pid = {0};   //��̨pitch��Ƕ�
PID_Controler Fire_speed_pid = {0};	  //���ֵ���ٶ�
PID_Controler Magazine_angle_pid = {0};	  //���ָǵ���Ƕ�
PID_Controler Fire_pid = {0};
PID_Controler pid_chassis_angle = {0};
PID_Controler Power_Limit_pid = {0};   //��������
PID_Controler Power_Predeal_pid = {0};   //���ʱջ�PID��ǰ����Ϊ��������ڼ䳬���ʵ�˳���ȣ�Ҳ��ֹ����ϵͳ����ʧ��
PID_Controler Spin_Top_pid = {0};


void PID_Init(PID_Controler *Controler)
{
	
}


//pid��ʼ��
void Total_PID_Init(void)
{
	u8 i;	
	//********************����3508���********************
	for(i=0;i<4;i++)
	{
		motor_speed_pid[i].Kp = PID_MOTOR_SPEED_KP;
		motor_speed_pid[i].Ki = PID_MOTOR_SPEED_KI;
		motor_speed_pid[i].Kd = PID_MOTOR_SPEED_KD;
		motor_speed_pid[i].Integrate_Limit_Flag = 1;//�����޷�
		motor_speed_pid[i].Integrate_Separation_Flag = 1;//���ַ���
		motor_speed_pid[i].Integrate_Max = PID_MOTOR_SPEED_I_MAX;//���ַ�ֵ
		motor_speed_pid[i].Integrate_Separation_Err = PID_MOTOR_SPEED_I_Err;//���ַ��뷶Χ
		motor_speed_pid[i].Control_OutPut_Limit = 30000;//������
	}
	//********************����6020���********************
	for(i=0;i<4;i++)
	{
		chassis_speed_pid[i].Kp = PID_6020_SPEED_KP;
		chassis_speed_pid[i].Ki = PID_6020_SPEED_KI;
		chassis_speed_pid[i].Kd = PID_6020_SPEED_KD;
		chassis_speed_pid[i].Integrate_Limit_Flag = 1;//�����޷�
		chassis_speed_pid[i].Integrate_Separation_Flag = 1;//���ַ���
		chassis_speed_pid[i].Integrate_Max = PID_6020_SPEED_I_MAX;//���ַ�ֵ
		chassis_speed_pid[i].Integrate_Separation_Err = PID_6020_SPEED_I_Err;//���ַ��뷶Χ
		chassis_speed_pid[i].Control_OutPut_Limit = 30000;//������
		
		
		chassis_angle_pid[i].Kp = PID_6020_ANGLE_KP;
		chassis_angle_pid[i].Ki = PID_6020_ANGLE_KI;
		chassis_angle_pid[i].Kd = PID_6020_ANGLE_KD;
		chassis_angle_pid[i].Integrate_Limit_Flag = 1;//�����޷�
		chassis_angle_pid[i].Integrate_Separation_Flag = 1;//���ַ���
		chassis_angle_pid[i].Integrate_Max = PID_6020_ANGLE_I_MAX;//���ַ�ֵ
		chassis_angle_pid[i].Integrate_Separation_Err = PID_6020_ANGLE_I_Err;//���ַ��뷶Χ
		chassis_angle_pid[i].Control_OutPut_Limit = 30000;//������
	}
	
	//******************������̨yaw��******************
	vision_yaw_speed_pid.Integrate_Limit_Flag = 1;
	vision_yaw_speed_pid.Integrate_Separation_Flag = 1;//���ַ���
	vision_yaw_speed_pid.Integrate_Max = VISION_PID_YAW_SPEED_I_MAX;//���ַ�ֵ
	vision_yaw_speed_pid.Integrate_Separation_Err = VISION_PID_YAW_SPEED_I_Err;//���ַ��뷶Χ	
	vision_yaw_speed_pid.Kp = VISION_PID_YAW_SPEED_KP;
	vision_yaw_speed_pid.Ki = VISION_PID_YAW_SPEED_KI;
	vision_yaw_speed_pid.Kd = VISION_PID_YAW_SPEED_KD;
	vision_yaw_speed_pid.Control_OutPut_Limit = 30000;//������

	vision_yaw_angle_pid.Integrate_Limit_Flag = 1;
	vision_yaw_angle_pid.Integrate_Separation_Flag = 1;//���ַ���
	vision_yaw_angle_pid.Integrate_Max = VISION_PID_YAW_ANGLE_I_MAX;//���ַ�ֵ
	vision_yaw_angle_pid.Integrate_Separation_Err = VISION_PID_YAW_ANGLE_I_Err;//���ַ��뷶Χ	
	vision_yaw_angle_pid.Kp = VISION_PID_YAW_ANGLE_KP;
	vision_yaw_angle_pid.Ki = VISION_PID_YAW_ANGLE_KI;
	vision_yaw_angle_pid.Kd = VISION_PID_YAW_ANGLE_KD;
	vision_yaw_angle_pid.Control_OutPut_Limit = 30000;//������	
	

	//******************������̨pitch��******************
	vision_pitch_speed_pid.Integrate_Limit_Flag = 1;
	vision_pitch_speed_pid.Integrate_Separation_Flag = 1;//���ַ���
	vision_pitch_speed_pid.Integrate_Max = VISION_PID_PITCH_SPEED_I_MAX;//���ַ�ֵ
	vision_pitch_speed_pid.Integrate_Separation_Err = VISION_PID_PITCH_SPEED_I_Err;//���ַ��뷶Χ	
	vision_pitch_speed_pid.Kp = VISION_PID_PITCH_SPEED_KP;
	vision_pitch_speed_pid.Ki = VISION_PID_PITCH_SPEED_KI;
	vision_pitch_speed_pid.Kd = VISION_PID_PITCH_SPEED_KD;
	vision_pitch_speed_pid.Control_OutPut_Limit = 30000;//������

	vision_pitch_angle_pid.Integrate_Limit_Flag = 1;
	vision_pitch_angle_pid.Integrate_Separation_Flag = 1;//���ַ���
	vision_pitch_angle_pid.Integrate_Max = VISION_PID_PITCH_ANGLE_I_MAX;//���ַ�ֵ
	vision_pitch_angle_pid.Integrate_Separation_Err = VISION_PID_PITCH_ANGLE_I_Err;//���ַ��뷶Χ	
	vision_pitch_angle_pid.Kp = VISION_PID_PITCH_ANGLE_KP;
	vision_pitch_angle_pid.Ki = VISION_PID_PITCH_ANGLE_KI;
	vision_pitch_angle_pid.Kd = VISION_PID_PITCH_ANGLE_KD;
	vision_pitch_angle_pid.Control_OutPut_Limit = 30000;//������	


	//********************��̨pitch��********************
	Pitch_speed_pid.Integrate_Limit_Flag = 1;//�����޷�
	Pitch_speed_pid.Integrate_Separation_Flag = 1;//���ַ���
	Pitch_speed_pid.Integrate_Max = PID_PITCH_SPEED_I_MAX;//���ַ�ֵ
	Pitch_speed_pid.Integrate_Separation_Err = PID_PITCH_SPEED_I_Err;//���ַ��뷶Χ
	Pitch_speed_pid.Kp = PID_PITCH_SPEED_KP;
	Pitch_speed_pid.Ki = PID_PITCH_SPEED_KI;
	Pitch_speed_pid.Kd = PID_PITCH_SPEED_KD;
	Pitch_speed_pid.Control_OutPut_Limit = 30000;//������
	
	Pitch_angle_pid.Integrate_Limit_Flag = 1;//�����޷�
	Pitch_angle_pid.Integrate_Separation_Flag = 1;//���ַ���
	Pitch_angle_pid.Integrate_Max = PID_PITCH_ANGLE_I_MAX;//���ַ�ֵ
	Pitch_angle_pid.Integrate_Separation_Err = PID_PITCH_ANGLE_I_Err;//���ַ��뷶Χ
	Pitch_angle_pid.Kp = PID_PITCH_ANGLE_KP;
	Pitch_angle_pid.Ki = PID_PITCH_ANGLE_KI;
	Pitch_angle_pid.Kd = PID_PITCH_ANGLE_KD;
	Pitch_angle_pid.Control_OutPut_Limit = 40;//������
	
	
	//********************��̨yaw��********************
	Yaw_speed_pid.Integrate_Limit_Flag = 1;//�����޷�
	Yaw_speed_pid.Integrate_Separation_Flag = 1;//���ַ���
	Yaw_speed_pid.Integrate_Max = PID_YAW_SPEED_I_MAX;//���ַ�ֵ
	Yaw_speed_pid.Integrate_Separation_Err = PID_YAW_SPEED_I_Err;//���ַ��뷶Χ
	Yaw_speed_pid.Kp = PID_YAW_SPEED_KP;
	Yaw_speed_pid.Ki = PID_YAW_SPEED_KI;
	Yaw_speed_pid.Kd = PID_YAW_SPEED_KD;
	Yaw_speed_pid.Control_OutPut_Limit = 30000;//������
	
	Yaw_pid.Integrate_Limit_Flag = 1;
	Yaw_pid.Integrate_Separation_Flag = 1;
	Yaw_pid.Integrate_Max = PID_YAW_ANGLE_I_MAX;//���ַ�ֵ
	Yaw_speed_pid.Integrate_Separation_Err = PID_YAW_ANGLE_I_Err;
	Yaw_pid.Kp = PID_YAW_ANGLE_KP;
	Yaw_pid.Ki = PID_YAW_SPEED_KI;
	Yaw_pid.Kd = PID_YAW_SPEED_KD;
	Yaw_pid.Control_OutPut_Limit = 40;
	
	
	//**********************�������**********************
	Fire_speed_pid.Integrate_Limit_Flag = 1;//�����޷�
	Fire_speed_pid.Integrate_Max = PID_FIRE_SPEED_I_MAX;//���ַ�ֵ
	Fire_speed_pid.Kp = 10;  //3
	Fire_speed_pid.Ki = 0;
	Fire_speed_pid.Kd = 30;
	Fire_speed_pid.Control_OutPut_Limit = 8000;
	
	Fire_pid.Integrate_Limit_Flag = 1;//�����޷�
	Fire_pid.Integrate_Max = 2000;//���ַ�ֵ
	Fire_pid.Kp = 7;  //0.1
	Fire_pid.Ki = 0;
	Fire_pid.Kd = 22;
	Fire_pid.Control_OutPut_Limit = 6000;
	
	
	//**********************���ָǵ��**********************
	Magazine_angle_pid.Integrate_Limit_Flag = 1;//�����޷�
	Magazine_angle_pid.Integrate_Separation_Flag = 1;//���ַ���
	Magazine_angle_pid.Integrate_Max = PID_PITCH_ANGLE_I_MAX;//���ַ�ֵ
	Magazine_angle_pid.Integrate_Separation_Err = PID_PITCH_ANGLE_I_Err;//���ַ��뷶Χ
	Magazine_angle_pid.Kp = PID_MAGAZINE_ANGLE_KP;
	Magazine_angle_pid.Ki = PID_MAGAZINE_ANGLE_KI;
	Magazine_angle_pid.Kd = PID_MAGAZINE_ANGLE_KD;
	Magazine_angle_pid.Control_OutPut_Limit = 30000;//������

	
	//**********************���ʱջ�**********************
	Power_Limit_pid.Integrate_Limit_Flag =1;
	Power_Limit_pid.Integrate_Separation_Flag =1;
	Power_Limit_pid.Kp =0.02f ;  //0.092
	Power_Limit_pid.Ki = 0;//0.00002
	Power_Limit_pid.Kd = 0.002f;  //0.005
	Power_Limit_pid.Integrate_Max = 250;//���ַ�ֵ
	Power_Limit_pid.Control_OutPut_Limit =0.5f	;


//С����PID���� �����Է��ֲ���PIDЧ������
//	Spin_Top_pid.Integrate_Limit_Flag =1;
//	Spin_Top_pid.Integrate_Separation_Flag =1;
//	Spin_Top_pid.Kp =0.01f ;
//	Spin_Top_pid.Ki = 0.002f;
//	Spin_Top_pid.Kd = 0;
//	Spin_Top_pid.Integrate_Max = 90;//���ַ�ֵ
//	Spin_Top_pid.Control_OutPut_Limit =180.0f	;
//	Spin_Top_pid.Integrate_Separation_Err = 5;
}


/***********************
 *float PID_Control(PID_Controler *Controler,float target,float measure)
 *��ͨλ��ʽpid
 *���룺*Controler��pid���ݽṹ���ַ
 *       target��Ŀ��ֵ
 *       measure������ֵ
 ***********************/
float PID_Control(PID_Controler *Controler,float target,float measure)
{
	/*******ƫ�����*********************/
  Controler->Last_Err = Controler->Err;//�����ϴ�ƫ��
  Controler->Err = target - measure;//������ȥ�����õ�ƫ��
  if(Controler->Err_Limit_Flag == 1)//ƫ���޷��ȱ�־λ
  {
		if(Controler->Err >= Controler->Err_Max)   Controler->Err = Controler->Err_Max;
		if(Controler->Err <= -Controler->Err_Max)  Controler->Err = -Controler->Err_Max;
  }
	/*******���ּ���*********************/
  if(Controler->Integrate_Separation_Flag == 1)//���ַ����־λ
  {
    //if(fabs(Controler->Err) <= Controler->Integrate_Separation_Err)
			Controler->Integrate += Controler->Ki * Controler->Err;
  }
  else
    Controler->Integrate += Controler->Err;
	/*******�����޷�*********************/
 if(Controler->Integrate_Limit_Flag == 1)//�������Ʒ��ȱ�־
 {
  if(Controler->Integrate >= Controler->Integrate_Max)
    Controler->Integrate = Controler->Integrate_Max;
  if(Controler->Integrate <= -Controler->Integrate_Max) 
    Controler->Integrate = -Controler->Integrate_Max ;
 } 
	/*******���������*********************/
  Controler->Last_Control_OutPut = Controler->Control_OutPut;//���ֵ����
  Controler->Control_OutPut =  Controler->Kp * Controler->Err//����
															+ Controler->Integrate//����
															+Controler->Kd * (Controler->Err-Controler->Last_Err);//΢��
	/*******������޷�*********************/
  if(Controler->Control_OutPut >= Controler->Control_OutPut_Limit)
		Controler->Control_OutPut = Controler->Control_OutPut_Limit;
  if(Controler->Control_OutPut <= -Controler->Control_OutPut_Limit)
		Controler->Control_OutPut = -Controler->Control_OutPut_Limit;
	/*******���������*********************/
  return Controler->Control_OutPut;
}






/* Butterworth�˲� */
float Control_Device_LPF(float curr_inputer,Butter_BufferData *Buffer,Butter_Parameter *Parameter)
{
	/* ��ȡ����x(n) */
        Buffer->Input_Butter[2]=curr_inputer;
	/* Butterworth�˲� */
        Buffer->Output_Butter[2]=
         Parameter->b[0] * Buffer->Input_Butter[2]
        +Parameter->b[1] * Buffer->Input_Butter[1]
				+Parameter->b[2] * Buffer->Input_Butter[0]
        -Parameter->a[1] * Buffer->Output_Butter[1]
        -Parameter->a[2] * Buffer->Output_Butter[0];
	/* x(n) ���б��� */
        Buffer->Input_Butter[0]=Buffer->Input_Butter[1];
        Buffer->Input_Butter[1]=Buffer->Input_Butter[2];
	/* y(n) ���б��� */
        Buffer->Output_Butter[0]=Buffer->Output_Butter[1];
        Buffer->Output_Butter[1]=Buffer->Output_Butter[2];
        return (Buffer->Output_Butter[2]);
}


Butter_Parameter Control_Device_Div_LPF_Parameter={
 //200---20hz
  1,    -1.14298050254,   0.4128015980962,
  0.06745527388907,   0.1349105477781,  0.06745527388907
};

Butter_Parameter Control_Device_Err_LPF_Parameter={
  //200hz---2hz
  1,   -1.911197067426,   0.9149758348014,
  0.0009446918438402,  0.00188938368768,0.0009446918438402
};


/***********************
 *PID_Control_Div_LPF(PID_Controler *Controler,float target,float measure)
 *��΢�ֵ�ͨ�˲���λ��ʽpid
 *���룺*Controler��pid���ݽṹ���ַ
 *       target��Ŀ��ֵ
 *       measure������ֵ
 ***********************/
float PID_Control_Div_LPF(PID_Controler *Controler,float target,float measure)
{
  u16 i=0;
/*******ƫ�����*********************/
  Controler->Last_Err = Controler->Err;//�����ϴ�ƫ��
  Controler->Err = target - measure;//������ȥ�����õ�ƫ��
  Controler->Dis_Err = Controler->Err - Controler->Last_Err;//ԭʼ΢��
  for(i=4;i>0;i--)//���ֵ�ͨ��΢�����
		Controler->Dis_Error_History[i] = Controler->Dis_Error_History[i-1];
  Controler->Dis_Error_History[0] = Control_Device_LPF(Controler->Dis_Err,
                                  &Controler->Control_Device_LPF_Buffer,
                                  &Control_Device_Div_LPF_Parameter);//������˹��ͨ��õ���΢����,20hz

  if(Controler->Err_Limit_Flag == 1)//ƫ���޷��ȱ�־λ
  {
		if(Controler->Err >= Controler->Err_Max)   Controler->Err =  Controler->Err_Max;
		if(Controler->Err <= -Controler->Err_Max)  Controler->Err = -Controler->Err_Max;
  }
/*******���ּ���*********************/
  if(Controler->Integrate_Separation_Flag == 1)//���ַ����־λ
  {
    if(fabs(Controler->Err) <= Controler->Integrate_Separation_Err)
			Controler->Integrate += Controler->Ki * Controler->Err;
  }
  else
  {
    Controler->Integrate += Controler->Ki * Controler->Err;
  }
/*******�����޷�*********************/
 if(Controler->Integrate_Limit_Flag == 1)//�������Ʒ��ȱ�־
 {
  if(Controler->Integrate >= Controler->Integrate_Max)
    Controler->Integrate = Controler->Integrate_Max;
  if(Controler->Integrate <= -Controler->Integrate_Max)
    Controler->Integrate = -Controler->Integrate_Max ;
 }
/*******���������*********************/
  Controler->Last_Control_OutPut = Controler->Control_OutPut;//���ֵ����
  Controler->Control_OutPut = Controler->Kp * Controler->Err//����
                         +Controler->Integrate//����
                         //+Controler->Kd*Controler->Dis_Err;//΢��
                         +Controler->Kd * Controler->Dis_Error_History[0];//΢������Դ�ڰ�����˹��ͨ�˲���
/*******������޷�*********************/
  if(Controler->Control_OutPut >= Controler->Control_OutPut_Limit)
		Controler->Control_OutPut = Controler->Control_OutPut_Limit;
  if(Controler->Control_OutPut <= -Controler->Control_OutPut_Limit)
		Controler->Control_OutPut = -Controler->Control_OutPut_Limit;
/*******���������*********************/
  return Controler->Control_OutPut; 
}



/***********************
 *PID_Control_Err_LPF(PID_Controler *Controler,float target,float measure)
 *��ƫ���ͨ�˲���λ��ʽpid
 *���룺*Controler��pid���ݽṹ���ַ
 *       target��Ŀ��ֵ
 *       measure������ֵ
 ***********************/
float PID_Control_Err_LPF(PID_Controler *Controler,float target,float measure)
{
/*******ƫ�����*********************/
  Controler->Last_Err = Controler->Err;//�����ϴ�ƫ��
  Controler->Err = target - measure;//������ȥ�����õ�ƫ��
  Controler->Dis_Err = Controler->Err - Controler->Last_Err;//ԭʼ΢��

  Controler->Last_Err_LPF = Controler->Err_LPF;
  Controler->Err_LPF = Control_Device_LPF(Controler->Err,
                                  &Controler->Control_Device_LPF_Buffer,
                                  &Control_Device_Err_LPF_Parameter);//������˹��ͨ��õ���΢����,20hz

  Controler->Dis_Err_LPF = Controler->Err_LPF - Controler->Last_Err_LPF;//ƫ�����ͨ���΢����

  if(Controler->Err_Limit_Flag == 1)//ƫ���޷��ȱ�־λ
  {
  if(Controler->Err_LPF >= Controler->Err_Max)   Controler->Err_LPF = Controler->Err_Max;
  if(Controler->Err_LPF <= -Controler->Err_Max)  Controler->Err_LPF =-Controler->Err_Max;
  }
/*******���ּ���*********************/
  if(Controler->Integrate_Separation_Flag == 1)//���ַ����־λ
  {
    if(fabs(Controler->Err_LPF) <= Controler->Integrate_Separation_Err)
    Controler->Integrate += Controler->Ki * Controler->Err_LPF;
  }
  else
  {
    Controler->Integrate += Controler->Ki * Controler->Err_LPF;
  }
/*******�����޷�*********************/
 if(Controler->Integrate_Limit_Flag == 1)//�������Ʒ��ȱ�־
 {
  if(Controler->Integrate >= Controler->Integrate_Max)
    Controler->Integrate = Controler->Integrate_Max;
  if(Controler->Integrate <= -Controler->Integrate_Max)
    Controler->Integrate = -Controler->Integrate_Max ;
 }
/*******���������*********************/
  Controler->Last_Control_OutPut = Controler->Control_OutPut;//���ֵ����
  Controler->Control_OutPut = Controler->Kp * Controler->Err_LPF//����
                         +Controler->Integrate//����
                          +Controler->Kd * Controler->Dis_Err_LPF;//�Ѷ�ƫ���ͨ���˴����ٶ�΢�������ͨ
/*******������޷�*********************/
  if(Controler->Control_OutPut >= Controler->Control_OutPut_Limit)
  Controler->Control_OutPut = Controler->Control_OutPut_Limit;
  if(Controler->Control_OutPut <= -Controler->Control_OutPut_Limit)
  Controler->Control_OutPut = -Controler->Control_OutPut_Limit;
/*******���������*********************/
  return Controler->Control_OutPut;
}


//pid���ָ�λ
void  PID_Integrate_Reset(PID_Controler *Controler)  {Controler->Integrate=0.0f;}








/*�˲���������
Butter_Parameter 80HZ_Parameter={
  //200hz---80hz
1,     1.14298050254,   0.4128015980962,
0.638945525159,    1.277891050318,    0.638945525159
};

Butter_Parameter 60HZ_Parameter={
  //200hz---60hz
1,   0.3695273773512,   0.1958157126558,
0.3913357725018,   0.7826715450035,   0.3913357725018
};

Butter_Parameter 51HZ_Parameter={
  //200hz---51hz
1,  0.03680751639284,   0.1718123812701,
0.3021549744157,   0.6043099488315,   0.3021549744157,
};

Butter_Parameter 30HZ_Parameter={
  //200hz---30hz
1,  -0.7477891782585,    0.272214937925,
0.1311064399166,   0.2622128798333,   0.1311064399166
};
Butter_Parameter 20HZ_Parameter={
  //200hz---20hz
  1,    -1.14298050254,   0.4128015980962,
  0.06745527388907,   0.1349105477781,  0.06745527388907
};
Butter_Parameter 15HZ_Parameter={
  //200hz---15hz
  1,   -1.348967745253,   0.5139818942197,
  0.04125353724172,  0.08250707448344,  0.04125353724172
};

Butter_Parameter 10HZ_Parameter={
  //200hz---10hz
  1,   -1.561018075801,   0.6413515380576,
  0.02008336556421,  0.04016673112842,  0.02008336556421};
Butter_Parameter 5HZ_Parameter={
  //200hz---5hz
  1,   -1.778631777825,   0.8008026466657,
  0.005542717210281,  0.01108543442056, 0.005542717210281
};

Butter_Parameter 2HZ_Parameter={
  //200hz---2hz
  1,   -1.911197067426,   0.9149758348014,
  0.0009446918438402,  0.00188938368768,0.0009446918438402
};
*/

