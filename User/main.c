//版本标识：mode1
//mode2成功
//mode35成功
//mode4成功
#include "stm32f10x.h" 
#include "Delay.h"
#include "LED.h"
#include "Key.h"
#include "OLED.h"
#include "Motor.h"
#include "Enconder.h"
#include "MPU.h"
#include "Track.h"
#include "Buzzer.h"
#include <stdio.h> 

int BaseSpeed = 90;      
int Track_Error = 0;      
int Prev_Track_Error = 0; 
int Last_Track_Error = 0; 

float Kp_Track = 1.0;     
float Kd_Track = 1.4;     

int16_t SpeedL = 90, SpeedR = 90;
int16_t EspeedL, EspeedR;
float pitch, roll, yaw;

// 连续偏航角
volatile float Total_Yaw = 0.0f;  /* written in main loop, read in TIM1 ISR */

uint8_t R3, R2, R1, M, L3, L2, L1;

//角度偏差计算
float Get_Yaw_Error(float current, float target) {
    return current - target;
}

//菜单管理
uint8_t KeyNum;
uint8_t Press_Count = 0;     
volatile uint8_t Menu_Select = 1;
volatile int Target_Laps = 1;
volatile int Run_State = 0;

volatile uint16_t AutoStart_Timer = 300;
volatile uint8_t Node_Count = 0;
int Corner_Cooldown = 0;         
uint8_t Node_Active = 0;         
uint8_t Last_Line_State = 0;     
uint8_t Start_Node_Released = 0; 
uint8_t Start_Clear_Count = 0;   

//任务
uint16_t Beep_Time = 0;      
float Target_Yaw = 0;        
int Mode4_State = 0;         

int main(void)
{
    OLED_Init();
    LED_Init();
    Delay_ms(200);
    Motor_Init();
    Encoder_Init();
    MPU6050_Init();
    MPU6050_Calibration();
    Key_Init();
    Track_Init();
    Buzzer_Init();
    
    GPIO_ResetBits(GPIOB, GPIO_Pin_15); 
    LED1_OFF(); LED2_OFF();

    while(1)
    {
        /* NOTE: dt = 0.01f is a CALIBRATED constant, not the measured loop
           period. All angle constants in Mode 3/4/5 (-19.0f per corner,
           the Target_Angle table, +47.5f etc.) are tuned against the
           current loop timing (software I2C + OLED refresh). Do NOT change
           I2C delays, OLED refresh, or MPU read pattern without
           recalibrating those angles. */
        GetAngles(&pitch, &roll, &yaw, 0.01f);
        
        // 解包算法
        static float prev_yaw = 0;
        static int yaw_init = 0;
        if (yaw_init == 0) {
            prev_yaw = yaw;
            Total_Yaw = yaw;
            yaw_init = 1;
        } else {
            float delta = yaw - prev_yaw;
            if (delta < -180.0f) delta += 360.0f;
            else if (delta > 180.0f) delta -= 360.0f;
            Total_Yaw += delta;
            prev_yaw = yaw;
        }
       
        //
        if (Run_State == 0)
        {
            KeyNum = key_GetNum(); 
            if (KeyNum == 1)
            {
                Press_Count++;
                if (Press_Count > 9) Press_Count = 1;
                
                if (Press_Count == 0 || Press_Count == 1) { Menu_Select=1; Target_Laps=1; AutoStart_Timer=300; }
                else if (Press_Count == 2) { Menu_Select=1; Target_Laps=2; AutoStart_Timer=300; }
                else if (Press_Count == 3) { Menu_Select=1; Target_Laps=3; AutoStart_Timer=300; }
                else if (Press_Count == 4) { Menu_Select=1; Target_Laps=4; AutoStart_Timer=300; }
                else if (Press_Count == 5) { Menu_Select=1; Target_Laps=5; AutoStart_Timer=300; }
                else if (Press_Count == 6) { Menu_Select=2; AutoStart_Timer=200; } 
                else if (Press_Count == 7) { Menu_Select=3; AutoStart_Timer=200; } 
                else if (Press_Count == 8) { Menu_Select=4; AutoStart_Timer=200; } 
                else if (Press_Count == 9) { Menu_Select=5; AutoStart_Timer=200; } 
            }
        }

        char str[16];
        if (Menu_Select == 1) sprintf(str, "Mode1_R%d       ", Target_Laps); 
        else if (Menu_Select == 2) sprintf(str, "Mode2           ");
        else if (Menu_Select == 3) sprintf(str, "Mode3           ");
        else if (Menu_Select == 4) sprintf(str, "Mode4           ");
        else if (Menu_Select == 5) sprintf(str, "Mode5           ");
        OLED_ShowString(0, 0, str, OLED_8X16); 
        
        if (Run_State == 0) {
            OLED_ShowString(0, 16, "State: Wait ", OLED_8X16);
            OLED_ShowNum(96, 16, AutoStart_Timer / 100, 1, OLED_8X16); 
        } else if (Run_State == 1) {
            OLED_ShowString(0, 16, "State: Run  ", OLED_8X16);
        } else {
            OLED_ShowString(0, 16, "State: Stop ", OLED_8X16);
        }
        
        OLED_ShowString(0, 32, "Nodes: ", OLED_8X16);
        OLED_ShowNum(56, 32, Node_Count, 2, OLED_8X16);
        OLED_ShowString(0, 48, "Yaw: ", OLED_8X16);
        OLED_ShowFloatNum(40, 48, Total_Yaw, 4, 2, OLED_8X16);

        OLED_Update(); 
    }
}

void TIM1_UP_IRQHandler(void)
{
    static uint16_t Counter_PID = 0;
    static uint16_t Counter_Enc = 0;
    static uint16_t Counter_10ms = 0;
    
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        Counter_PID++;
        Counter_Enc++;
        Counter_10ms++;

        if (Counter_10ms >= 200) 
        {
            Counter_10ms = 0;
            
            if (Beep_Time > 0) {
                Beep_Time--;
             
                if ((Beep_Time / 5) % 2 == 0) { 
                    LED1_ON(); LED2_ON(); GPIO_SetBits(GPIOB, GPIO_Pin_15); 
                } else {
                    LED1_OFF(); LED2_OFF(); GPIO_ResetBits(GPIOB, GPIO_Pin_15); 
                }
            } else {
                LED1_OFF(); LED2_OFF(); GPIO_ResetBits(GPIOB, GPIO_Pin_15);
            }

            if (Run_State == 0 && AutoStart_Timer > 0) {
                if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 1) { 
                    AutoStart_Timer--;
                    if (AutoStart_Timer == 0) {
                        Run_State = 1;            
                        Node_Count = 0;           
                        Target_Yaw = Total_Yaw;   
                        Mode4_State = 0;
                        Corner_Cooldown = 0;
                        Node_Active = 0;
                        Last_Line_State = 0;      
                        Start_Node_Released = 0;
                        Start_Clear_Count = 0;
                        Track_Error = 0;
                        Prev_Track_Error = 0;
                        Last_Track_Error = 0;
                    }
                }
            }
        }

        if (Counter_Enc >= 200) { EspeedL = Encoder_GetL(); EspeedR = Encoder_GetR(); Counter_Enc = 0; }

        if (Counter_PID >= 50) 
        {
            Counter_PID = 0;
            
            if (Run_State == 1) 
            {
                Track_Value(&R3, &R2, &R1, &M, &L3, &L2, &L1);
                
                int Error_Sum = 0;
                int Active_Sensors = 0;
                if(L3 == 1) { Error_Sum -= 30; Active_Sensors++; }
                if(L2 == 1) { Error_Sum -= 20; Active_Sensors++; }
                if(L1 == 1) { Error_Sum -= 10; Active_Sensors++; }
                if(M  == 1) { Error_Sum += 0;  Active_Sensors++; }
                if(R1 == 1) { Error_Sum += 10; Active_Sensors++; }
                if(R2 == 1) { Error_Sum += 20; Active_Sensors++; }
                if(R3 == 1) { Error_Sum += 30; Active_Sensors++; }

                uint8_t Current_Line_State = (Active_Sensors > 0) ? 1 : 0;

                if (Corner_Cooldown > 0) {
                    Corner_Cooldown--; 
                }
                
               
             
                if (Menu_Select == 1) {
                    // Mode 1
                    int is_node = 0;
                    if ((L3 == 1 && L2 == 1 && L1 == 1) || (R3 == 1 && R2 == 1 && R1 == 1)) is_node = 1;
                    
                    if (Start_Node_Released == 0) {
                        if (is_node == 0) {
                            if (Start_Clear_Count < 20) Start_Clear_Count++;
                            if (Start_Clear_Count >= 20) Start_Node_Released = 1;
                        } else Start_Clear_Count = 0;
                        Node_Active = is_node;
                    }
                    else if (is_node && Node_Active == 0 && Corner_Cooldown == 0) {
                        Node_Active = 1;
                        Node_Count++;
                        Corner_Cooldown = 400; 
                        
                        if (Node_Count >= Target_Laps * 4) { Run_State = 2; Beep_Time = 60; }
                    }
                    else if (is_node == 0) Node_Active = 0;
                } 
                else if (Menu_Select == 2) {
                    //  Mode 2
                    if (Start_Node_Released == 0) {
                        if (Current_Line_State == 0) {
                            if (Start_Clear_Count < 20) Start_Clear_Count++;
                            if (Start_Clear_Count >= 20) Start_Node_Released = 1;
                        } else Start_Clear_Count = 0;
                    }
                    else if (Current_Line_State == 1) {
                       
                        Run_State = 2; 
                        Beep_Time = 60; 
                    }
                }
                else if (Menu_Select >= 3 && Menu_Select <= 5) {
                    //  Mode 3/4/5
                    if (Corner_Cooldown == 0) {
                        if (Current_Line_State != Last_Line_State) {
                            int valid_edge = 1; 

                            if (Last_Line_State == 1 && Current_Line_State == 0) {
                                float total_yaw = Total_Yaw - Target_Yaw; 
                                float expected_yaw = (Node_Count + 1) * -19.0f; 
                                
                                if (Menu_Select == 3 || Menu_Select == 5) {
                                   
                                    float tolerance = 8.0f; 
                                    if (Node_Count % 4 == 1) {
                                        tolerance = 6.0f;
                                    } else if (Node_Count % 4 == 3) {
                                        tolerance = 4.0f;
                                    }
                                    
                                    if (total_yaw > expected_yaw + tolerance) {
                                        valid_edge = 0; 
                                    }
                                } else if (Menu_Select == 4) {
                                    // Mode 4 
                                    if (Node_Count == 1) {
                                        // C 点沿着半弧走到 B 点，yaw+36。
                                        if (total_yaw < 34.0f) {
                                            valid_edge = 0; 
                                        }
                                    } 
                                    else if (Node_Count == 3) {
                                        if (total_yaw > 15.0f) {
                                            valid_edge = 0; 
                                        }
                                    }
                                }
                            }

                            // 触发边沿
                            if (valid_edge == 1) {
                                Node_Count++; 
                                Corner_Cooldown = 400; 
                                if (Menu_Select == 3) {
                                    Beep_Time = 20; 
                                    if (Node_Count >= 4) { Run_State = 2; Beep_Time = 60; } 
                                }
                                else if (Menu_Select == 5) {
                                    Beep_Time = 20;
                                    if (Node_Count >= 16) { Run_State = 2; Beep_Time = 60; } 
                                }
                                else if (Menu_Select == 4) {
                                    Beep_Time = 20;
                                    if (Node_Count >= 4) { Run_State = 2; Beep_Time = 60; }
                                }
                                
                                Last_Line_State = Current_Line_State; 
                            }
                        }
                    }
                }
                
                //PID 与 控制输出计算
                if (Menu_Select == 1) {
                    if (Current_Line_State == 1) {
                        Track_Error = Error_Sum / Active_Sensors;
                        Last_Track_Error = Track_Error;
                    } else {
                        if(Last_Track_Error > 0) Track_Error = 70;
                        else if(Last_Track_Error < 0) Track_Error = -70;
                        else Track_Error = 0;
                    }
                }
                else if (Menu_Select == 2) {
                    float Kp_MPU = 10.0f; 
                    float current_yaw_error = Get_Yaw_Error(Total_Yaw, Target_Yaw);
                    Track_Error = (int)(current_yaw_error * Kp_MPU);
                }
                else if (Menu_Select == 3 || Menu_Select == 5) {
                    if (Current_Line_State == 1) {
                        Track_Error = Error_Sum / Active_Sensors;
                        Last_Track_Error = Track_Error;
                    } else {
                        if (Node_Count % 2 == 0) {
                            float Kp_MPU = 10.0f; 
                            float current_yaw_error = 0;
                            float Target_Angle = Target_Yaw; 
                            
                            // Mode35调参
                            if (Node_Count == 0) Target_Angle = Target_Yaw + 0.0f;     
                            if (Node_Count == 2) Target_Angle = Target_Yaw - 39.0f;  
                            if (Node_Count == 4) Target_Angle = Target_Yaw - 77.5f;    
                            if (Node_Count == 6) Target_Angle = Target_Yaw - 117.0f;   
                            if (Node_Count == 8) Target_Angle = Target_Yaw - 156.5f;     
                            if (Node_Count == 10) Target_Angle = Target_Yaw - 196.0f; 
                            if (Node_Count == 12) Target_Angle = Target_Yaw - 235.5f;    
                            if (Node_Count == 14) Target_Angle = Target_Yaw - 275.0f;   
                            
                            current_yaw_error = Get_Yaw_Error(Total_Yaw, Target_Angle);
                            Track_Error = (int)(current_yaw_error * Kp_MPU);
                        } 
                        else {
                            if(Last_Track_Error > 0) Track_Error = 70;
                            else if(Last_Track_Error < 0) Track_Error = -70;
                            else Track_Error = 0;
                        }
                    }
                }
                else if (Menu_Select == 4) {
                    // mode4
                    if (Node_Count == 0) { 
                        // A -> C
                        float Angle_AC = -7.0f;  
                        float yaw_error = Get_Yaw_Error(Total_Yaw, Target_Yaw + Angle_AC);
                        Track_Error = (int)(yaw_error * 10.0f); 
                    }
                    else {
                        if (Current_Line_State == 1) {
                            Track_Error = Error_Sum / Active_Sensors;
                            Last_Track_Error = Track_Error;
                        } else {
                            if (Node_Count == 2) {
                                // B -> D 
                                float Kp_MPU = 10.0f; 
                                float Angle_BD = +47.5f;  
                                float Target_Angle = Target_Yaw + Angle_BD;
                                float current_yaw_error = Get_Yaw_Error(Total_Yaw, Target_Angle);
                                Track_Error = (int)(current_yaw_error * Kp_MPU);
                            } 
                            else {
                                if(Last_Track_Error > 0) Track_Error = 70;
                                else if(Last_Track_Error < 0) Track_Error = -70;
                                else Track_Error = 0;
                            }
                        }
                    }
                }
                
                // 输出区
                int Turn_Output = (int)(Kp_Track * Track_Error + Kd_Track * (Track_Error - Prev_Track_Error));
                Prev_Track_Error = Track_Error;
                
                SpeedL = BaseSpeed + Turn_Output;
                SpeedR = BaseSpeed - Turn_Output;

                if(SpeedL > 95)  SpeedL = 95; if(SpeedL < -60) SpeedL = -60; 
                if(SpeedR > 95)  SpeedR = 95; if(SpeedR < -60) SpeedR = -60; 

                Motor_SetSpeedL(SpeedL); 
                Motor_SetSpeedR(SpeedR); 
            } 
            else 
            { 
                Motor_SetSpeedL(0); 
                Motor_SetSpeedR(0); 
            }
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}