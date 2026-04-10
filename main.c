/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "Motor.h"
#include "clock.h"
#include "interrupt.h"
#include "OLED.h"
#include "hardware_iic.h"
#include "IIC.h"
#include "wit.h"
#include "pid.h"
#include "stepmotor.h"
#include "xun.h"
#include "others.h"
#include "lsm6dsv16x.h"
#include "string.h"
#include <stdint.h>  // 提供 int32_t 类型
#include <stdlib.h>  // 提供 atoi 函数

uint8_t Key=0;
float Mode3_angle_X_last = 0.0;
float Mode3_angle_Y_last = 0.0;
float Mode3_angle_X = 0.0;
float Mode3_angle_Y = 0.0;

char txbuf[16];


int main(void)

{
    SYSCFG_DL_init();
    SysTick_Init();
    OLED_Init();
    OLED_Printf(0, 0, 16, "yes");

//    WIT_Init();//该死的陀螺仪
    //小车的PWM定时器预设已经打开
    LSM6DSV16X_Init();
    Interrupt_Init();
    NVIC_EnableIRQ(GPIO_Encoder_INT_IRQN);//打开编码器的中断

    //这是角度环PID的参数赋值
    PID_Parm_Init(&Motor_left);
    PID_Parm_Init(&Motor_right);
    // 这是步进电机有关的定时中断开启
    NVIC_ClearPendingIRQ(PWM_X_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(PWM_Y_INST_INT_IRQN);
    NVIC_EnableIRQ(PWM_X_INST_INT_IRQN);
    NVIC_EnableIRQ(PWM_Y_INST_INT_IRQN);

//    DL_TimerG_startCounter(TIMER_0_INST);//定时器打开
//    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//打开定时器中断(定时器配置时已开始)
//    Serial_JY61P_Zero_Yaw();//清空陀螺仪角度

//    Set_Speed(0, -30);
//    Set_Speed(1, 30);
//    delay_cycles(160000000);

    //步进电机转过一定角度测试
    //  Set_X(500);
    //  Set_Y(500);
    // while(!Y_End_Fin);
    // relay_OFF();
    // delay_cycles(160000000);  
    relay_OFF(); 
    NVIC_ClearPendingIRQ(UART_Camera_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Camera_INST_INT_IRQN);//打开摄像头的串口中断

//    delay_cycles(80000000);
    OLED_Printf(0, 0, 16, "ok");

//    Turn_Angle(10, 10);
//   Set_Y(1000);
//    SendString_Camera("start\r\n");
//    SendString_Camera("start\r\n");
    while (1) {
//这个是云台角度的陀螺仪测试
//        OLED_Printf(0, 4, 16,"%3.2f",yaw);

        //OLED_Printf(0, 2, 16, "state:%d",Now_state);
        // OLED_Printf(0, 0, 16, "Data: %d",cx); // 显示接收的8位数字
        // parseInput(cx,REZULT);      
        // OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
        // OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
        // MODE3_Move_Control(REZULT[0],REZULT[1]);
        // while(!(X_End_Fin&&Y_End_Fin));
//            OLED_Printf(60,4, 16, "2:%d",y1);            
//            while(!(X_End_Fin&&Y_End_Fin));
            //rx_state = 0; // 重置状态机[5](@ref)
                
        //按键的测试代码
        // Key = Key_GetNum();
        // if(Key == 1){
        //     OLED_Printf(0, 0, 16, "key1");                   
        // } 
        // if(Key == 2){
        //     OLED_Printf(0, 0, 16, "key2");            
        // }
        // if(Key == 3){
        //     OLED_Printf(0, 0, 16, "key3");            
        // }  


        /* 按键逻辑核心 */
       Key = Key_GetNum();
        /* 按键逻辑核心 */
        // 模式选择状态处理
        if (g_mode_select_flag) {
            if (Key == 1) {
                // 模式一需要设置圈数
                if (g_current_mode == 1) {
                    g_lap_setting_flag = 1;  // 进入圈数设置状态
                    g_mode_select_flag = 0;  // ★ 关键修复：退出模式选择状态
                    OLED_Clear();
                    OLED_Printf(0, 0, 16, "Set Laps:%d", g_lap_count);
                } 
                // 其他模式直接确认
                else {
                    g_mode_select_flag = 0;
                    OLED_Clear();
                    OLED_Printf(0, 0, 16, "Mode %d Active", g_current_mode);
                    
                    if(g_current_mode==2){
                        // DL_TimerG_setLoadValue(PWM_X_INST,2500);
                        // DL_TimerG_setLoadValue(PWM_Y_INST,2500);
                        // Turn_Angle(-39,2.2 );
                        // while(!(X_End_Fin&&Y_End_Fin));
                        // delay_cycles(40000000);
                        // relay_ON();

                        //视觉调节
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n");
                        MODE_Flag = 4;
                        DL_TimerG_startCounter(TIMER_0_INST);//定时器打开
                        NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//打开定时器中断(定时器配置时已开始)
                        while(1){
                            OLED_Printf(0, 0, 16, "Data: %d",cx); // 显示接收的8位数字
                            parseInput(cx,REZULT);      
                            OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
                            OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
                            MODE3_Move_Control(REZULT[0],REZULT[1]);
                            //while(!(X_End_Fin&&Y_End_Fin));
                            if(Time_CCC>=80){
                                relay_ON();
                                break;
                            }
                        }
                    
                    }
                    if(g_current_mode==3)
                    {
                        //这个是开启云台的陀螺仪
                        LSM6DSV16X_Init();
                        Interrupt_Init();
                        OLED_Clear();
                        OLED_Printf(0, 0, 16, "ready");
                        Mode3_angle_X_last = yaw;
                        Mode3_angle_Y_last = pitch;
                        OLED_Printf(0, 2, 16, "xl:%3.2f",Mode3_angle_X_last);
                        OLED_Printf(60, 2, 16,"yl:%3.2f",Mode3_angle_Y_last);
                        while(Key_GetNum() != 2){
                            Mode3_angle_X = yaw;
                            Mode3_angle_Y = pitch;
                            OLED_Printf(0, 4, 16, "x:%3.2f",Mode3_angle_X);
                            OLED_Printf(60, 4, 16,"y:%3.2f",Mode3_angle_Y);
                        }
                        float delta_x=Mode3_angle_X-Mode3_angle_X_last;
                        float delta_y=Mode3_angle_Y-Mode3_angle_Y_last;
                        if(delta_x>180){delta_x -= 360;}
                        else if(delta_x<-180){delta_x +=360;}
                        if(delta_y>180){delta_y -= 360;}
                        else if(delta_y<-180){delta_y +=360;} 
                        OLED_Clear();
                        OLED_Printf(0, 4, 16, "x:%3.2f",delta_x);
                        OLED_Printf(60, 4, 16,"y:%3.2f",delta_y);         
                        X_End_Fin=0;
                        Y_End_Fin=0;
                        Turn_Angle(delta_x, delta_y);
                        while(!(X_End_Fin&&Y_End_Fin));
                        //delay_cycles(40000000);
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n"); 
                        MODE_Flag = 3;
                        DL_TimerG_startCounter(TIMER_0_INST);//定时器打开
                        NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//打开定时器中断(定时器配置时已开始)
                        while(1){
                            OLED_Printf(0, 0, 16, "Data: %d",cx); // 显示接收的8位数字
                            parseInput(cx,REZULT);      
                            OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
                            OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
                            MODE3_Move_Control(REZULT[0],REZULT[1]);
                            while(!(X_End_Fin&&Y_End_Fin));
                            if(Time_CCC>=150){
                                relay_ON();
                                OLED_Clear();
                                OLED_Printf(0, 0, 16, "end");
                                while (1) ;
                            }
                        }
                    }
                    if(g_current_mode==4){
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n");
                        SendString_Camera("MODE1\r\n");
                        MODE_Flag = 4;
                        DL_TimerG_startCounter(TIMER_0_INST);//定时器打开
                        NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//打开定时器中断(定时器配置时已开始)
                        while(1){
                            OLED_Printf(0, 0, 16, "Data: %d",cx); // 显示接收的8位数字
                            parseInput(cx,REZULT);      
                            OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
                            OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
                            MODE3_Move_Control(REZULT[0],REZULT[1]);
                            //while(!(X_End_Fin&&Y_End_Fin));
                            if(Time_CCC>=175){
                                relay_ON();
                                break;
                            }
                        }
                        x_err_sum=0;
                        y_err_sum=0;
                        x_err_last=0;
                        y_err_last=0;
                        DL_Timer_stopCounter(TIMER_0_INST);//关掉定时器
                        delay_cycles(160000000);
                        MODE_Flag = 0;
                        // DL_TimerG_setLoadValue(PWM_X_INST,5000);
                        // DL_TimerG_setLoadValue(PWM_Y_INST,5000);//这是水平直线的速度

                        // Turn_Angle(90, 0);
                        DL_Timer_startCounter(TIMER_0_INST);
                        MODE_Flag = 1;
                        while(1){
                            OLED_Printf(0, 0, 16, "Data: %d",cx);
                            parseInput(cx,REZULT);      
                            OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
                            OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
                            //MODE3_Move_Control(REZULT[0],REZULT[1]);
                            if((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=300){
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,5000);
                                DL_TimerG_setLoadValue(PWM_Y_INST,5000);//这是转弯
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(7, 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=700) {
                                //设置速度，开始动
                                //DL_TimerG_setLoadValue(PWM_X_INST,200000);
                                //DL_TimerG_setLoadValue(PWM_Y_INST,200000);//这是竖直
                                DL_TimerG_setLoadValue(PWM_X_INST,1225);
                                DL_TimerG_setLoadValue(PWM_Y_INST,1225);//这是竖直
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                
                                Turn_Angle(0.02*MODE4_Move_ControlX(REZULT[0]), 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=1000) {
                                //设置速度，开始动
                                //DL_TimerG_setLoadValue(PWM_X_INST,200000);
                                //DL_TimerG_setLoadValue(PWM_Y_INST,200000);//这是竖直
                                DL_TimerG_setLoadValue(PWM_X_INST,1225);
                                DL_TimerG_setLoadValue(PWM_Y_INST,1225);//这是竖直
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                
                                Turn_Angle(0.03*MODE4_Move_ControlX(REZULT[0]), 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=1700) {
                                //设置速度，开始动
                                //DL_TimerG_setLoadValue(PWM_X_INST,200000);
                                //DL_TimerG_setLoadValue(PWM_Y_INST,200000);//这是竖直
                                DL_TimerG_setLoadValue(PWM_X_INST,1225);
                                DL_TimerG_setLoadValue(PWM_Y_INST,1225);//这是竖直
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                
                                Turn_Angle(0.03*MODE4_Move_ControlX(REZULT[0]), 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=2000) {
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,6400);
                                DL_TimerG_setLoadValue(PWM_Y_INST,6400);//这是转弯
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(10, 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=3200) {
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,2000);
                                DL_TimerG_setLoadValue(PWM_Y_INST,2000);//这是水平直线的速度
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(1+0.03*MODE4_Move_ControlX(REZULT[0]), 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=3600) {
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,6400);
                                DL_TimerG_setLoadValue(PWM_Y_INST,6400);//这是转弯
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(10, 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=4800) {
                                DL_TimerG_setLoadValue(PWM_X_INST,1100);
                                DL_TimerG_setLoadValue(PWM_Y_INST,1100);//这是竖直
                                X_End_Fin = 0;
                                Y_End_Fin = 0;   
                                Turn_Angle(0.03*MODE4_Move_ControlX(REZULT[0]), 0);
                            }
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=5200) {
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,6400);
                                DL_TimerG_setLoadValue(PWM_Y_INST,6400);//转弯
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(10, 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }     
                            else if ((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2<=6400) {
                                //设置速度，开始动
                                DL_TimerG_setLoadValue(PWM_X_INST,1000);
                                DL_TimerG_setLoadValue(PWM_Y_INST,1000);//这是水平直线的速度
                                X_End_Fin = 0;
                                Y_End_Fin = 0;
                                Turn_Angle(5, 0);
                                //while (!(X_End_Fin&&Y_End_Fin)) ;
                            }                             
                            OLED_Printf(0, 0, 16, "Data: %d",cx); // 显示接收的8位数字
                            // sprintf(txbuf, "%d\r\n",cx);
                            // SendString_test(txbuf);

                            // parseInput(cx,REZULT);     
                            // OLED_ShowSignedNum(0, 2, REZULT[0], 4, 16);
                            // OLED_ShowSignedNum(60, 2, REZULT[1], 4, 16);
                            // MODE4_Move_Control(REZULT[0],REZULT[1]);
                            // OLED_ShowSignedNum(0, 6, x1, 3, 16);
                            // OLED_ShowSignedNum(50, 6, y1, 3, 16);

                            //delay_cycles(1600000);//这里存疑，不知道为什么产生了y抖动(是因为陀螺仪)
                            //while(!(X_End_Fin&&Y_End_Fin));
                            // OLED_ShowNum(0, 6, Time_CCC, 5, 16);
                            // if((FL_Wheel_Cnt_Sum + FR_Wheel_Cnt_Sum)/2>=1000){
                            //     Set_Speed(0, 0);
                            //     Set_Speed(1, 0);
                            //     DL_TimerG_stopCounter(TIMER_0_INST);//关闭定时器
                            //     while (1);
                            //}
                            // while(!(X_End_Fin&&Y_End_Fin));
                        }                                                                        

                    }
                }
            }
            else if (Key == 2) {
                // 在模式选择状态下切换模式
                g_current_mode = (g_current_mode % 5) + 1;
                OLED_Clear();
                OLED_Printf(0, 0, 16, "Select Mode:%d", g_current_mode);
            }
        }
        // 圈数设置状态处理
        else if (g_lap_setting_flag) {
            if (Key == 2) {
                // 按键2：圈数递增(1-5循环)
                g_lap_count = (g_lap_count % 5) + 1;
                OLED_Printf(0, 0, 16, "Set Laps:%d", g_lap_count);
            }
            else if (Key == 1) {
                // 按键1：确认圈数并退出设置
                g_lap_setting_flag = 0;
                g_mode_select_flag = 0;
                OLED_Clear();
                OLED_Printf(0, 0, 16, "Mode1 Active");
                OLED_Printf(0, 2, 16, "Laps:%d", g_lap_count);
                MODE_Flag = 1;
                delay_cycles(40000000);
                DL_TimerG_startCounter(TIMER_0_INST);//定时器打开
                NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);//打开定时器中断(定时器配置时已开始)
                OLED_Clear();
                while(1){
                    Get_INF(Xun_data);
                    OLED_ShowNum(0, 0,Xun_data[0], 1, 16);
                    OLED_ShowNum(20, 0,Xun_data[1], 1, 16);
                    OLED_ShowNum(40, 0,Xun_data[2], 1, 16);
                    OLED_ShowNum(60, 0,Xun_data[3], 1, 16);
                    OLED_ShowNum(80, 0,Xun_data[4], 1, 16);
                    OLED_ShowNum(0, 4,XUN_flag, 1, 16);
                };
            }
        }
        // 正常模式处理
        else {
            if (Key == 1) {
                // 首次按KEY1：进入选择模式
                g_mode_select_flag = 1;
                OLED_Clear();
                OLED_Printf(0, 0, 16, "Select Mode:%d", g_current_mode);
            }
            // 其他按键处理...
        }
        //按键到此结束

        //这是步进电机相关试验代码
        // Turn_Angle(45, 45); 
        // // while(!(X_End_Fin&&Y_End_Fin));
        // OLED_Printf(0, 0, 16,"X:%4d",PulseCnt_X);
        // OLED_Printf(0, 2, 16,"Y:%4d",PulseCnt_Y);

        //这是小车轮子和编码器有关代码
        // OLED_Printf(0, 0, 16, "ok");
        // Set_Speed(0,20);
        // Set_Speed(1, 20);
        // delay_ms(3000);
        // Set_Speed(0, -40);
        // Set_Speed(1, -20);
        // delay_ms(3000);
        // OLED_ShowSignedNum(0, 2, FL_Wheel_Cnt, 4, 16);
        // OLED_ShowSignedNum(0, 4, FR_Wheel_Cnt, 4, 16);
        // OLED_ShowSignedNum(0, 0, L_speed, 4, 16);
        // OLED_ShowSignedNum(0, 2, R_speed, 4, 16);
        // OLED_ShowSignedNum(0, 0, (int)FL_Wheel_Cnt_Sum, 6, 16);
        // OLED_ShowSignedNum(0, 2, (int)FR_Wheel_Cnt_Sum, 6, 16);        
        
        //这是分离器件有关的获取数据和计算的代码
        // Get_INF(XUN_data);
        // OLED_ShowNum(0, 0, Xun_data[0], 3, 16);
        // OLED_ShowNum(50, 0, Xun_data[1], 3, 16);
        // OLED_ShowNum(0, 2, Xun_data[2], 3, 16);
        // OLED_ShowNum(50, 2, Xun_data[3], 3, 16);
        // OLED_ShowNum(0, 4, Xun_data[4], 3, 16);
        // OLED_Printf(0, 6, 16,"%3.2f",Xun_cal(Xun_data));      

        //感为相关
        /*OLED_ShowNum(0, 0, Normal[0], 3, 16);
        OLED_ShowNum(50, 0, Normal[1], 3, 16);
        OLED_ShowNum(0, 2, Normal[2], 3, 16);
        OLED_ShowNum(50, 2, Normal[3], 3, 16);
        OLED_ShowNum(0, 4, Normal[4], 3, 16);
        OLED_ShowNum(50, 4, Normal[5], 3, 16);
        OLED_ShowNum(0, 6, Normal[6], 3, 16);
        OLED_ShowNum(50, 6, Normal[7], 3, 16);
        OLED_ShowNum(90, 0, Time_CCC,4, 16);
        OLED_Printf(90, 2, 16, "%3.1f",Trace_OUT);*/

        //陀螺仪的代码
        // OLED_Printf(0, 6, 16, "ok");
        // OLED_Printf(0, 0, 16, "pitch:%5.2f",wit_data.pitch);
        // OLED_Printf(0, 2, 16, "roll:%5.2f",wit_data.roll);
        // OLED_Printf(0, 4, 16, "yaw:%5.2f",wit_data.yaw);//陀螺仪*/

        //一路ADC采样的代码
        // gCheckADC = false;
        // DL_ADC12_startConversion(ADC12_0_INST);
        // while(gCheckADC == false);
        // gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
        // OLED_Printf(0, 0, 16, "%d",gAdcResult);
        // DL_ADC12_enableConversions(ADC12_0_INST);
    }
}

