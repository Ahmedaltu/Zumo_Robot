/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/

#include <project.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/

void progEnd(uint32_t delay);
void tankT(uint8_t lspeed, uint8_t rspeed, uint32_t delay);
void motor_longturn(uint8 lspeed,uint8 rspeed,uint32 delay);

//week 3 Ex.1

#if 1
//motor
void zmain(void)
{
    motor_start();              // enable motor controller
    motor_forward(0,0);         // set speed to zero to stop motors
    while(SW1_Read());
    BatteryLed_Write(true);
    vTaskDelay(500);
    BatteryLed_Write(false);
    
    motor_forward(255,1400);          // moving forward
    tankT(0, 255, 205);               // first turn
    motor_forward(255,1278);          // moving forward 
    tankT(0, 255, 205);               //second turn
    motor_forward(255,1360);
    tankT(0, 255, 220);               //third turn before the curve
    
    motor_longturn(115, 99, 3200);    //moving on the curve line

    motor_forward(0,0);         // stop motors
   
    
    motor_stop();               // disable motor controller
    
    progEnd(100);
}
#endif


//*********** Custom Function Defs **************
void progEnd(uint32_t delay){
    bool led = false;
    while(true) {
        BatteryLed_Write(led^=1);
        vTaskDelay(delay);
    }    
}
void tankT(uint8_t lspeed, uint8_t rspeed, uint32_t delay){
   SetMotors(1,1,lspeed,rspeed,delay);
}
void motor_longturn(uint8 lspeed,uint8 rspeed,uint32 delay)
{
    // set LeftMotor forward mode
    // set RightMotor forward mode
    SetMotors(0,0, lspeed, rspeed, delay);
}
//void SetMotors(uint8 left_dir, uint8 right_dir, uint8 left_speed, uint8 right_speed, uint32 delay)
/* [] END OF FILE */
