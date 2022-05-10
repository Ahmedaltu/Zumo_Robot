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
//IAROSLAV EREMEEV 2021
//Functions declaration
void progEnd(uint32_t delay);
void line_follower(struct sensors_ *sensors, TickType_t *startt);
void start_up(bool motor, bool ultrasonic, bool reflectance, bool IR);
void stop_down(void);
int getRefValues (struct sensors_ *sensors, int SL3, int SL2, int SL1, int SR1, int SR2, int SR3);
void turn_right(uint8 speed,uint32 delay);
void turn_left(uint8 speed,uint32 delay);
//Project Line Follower
#if 1
void zmain(){
    //declare variables
    int lines = 0;
    struct sensors_ sensors;
   
    TickType_t startt = 0;
    TickType_t stopp = 0;
    //startup 
    start_up(true, true, true, true);
    //LED on
    BatteryLed_Write(1);
    //Start after button press
    while(SW1_Read() == 1);
    BatteryLed_Write(0); //LED off
    //delay
    vTaskDelay(600);

    while(lines<3){
        
        line_follower(&sensors, &startt);   //line follower function
        
        lines++;
        
        // do not need this:( //printf("Robot is on line %d\n", lines);     //print, on which line robot is
        
        if(lines == 1){
            print_mqtt("Zumo10/ready", "line");    //print mqtt 
            
            IR_flush();
            IR_wait();
            
            startt = xTaskGetTickCount();
            print_mqtt("Zumo10/start","%d", startt);    //print mqtt
        }
        else if(lines == 3){
            stopp = xTaskGetTickCount();
            print_mqtt("Zumo10/stop","%d", stopp);    //print mqtt
            
            int tmd = (int)(stopp)-(int)(startt);
            print_mqtt("Zumo10/time", "%d", tmd);    //print mqtt
        }
    }
    //stop the engines
    stop_down();
    progEnd(100);
}
#endif
  
//Custom Function Defs//
void line_follower(struct sensors_ *sensors, TickType_t *startt){
    bool on_line = true;
    reflectance_digital(sensors);
    
    //robot goes on line intersect.
    while(getRefValues(sensors, 1, 1, 1, 1, 1, 1)){
        motor_forward(255,10);
        reflectance_digital(sensors);
    }
    
    while(!getRefValues(sensors, 1 ,1 ,1 ,1 ,1 ,1)){
        //turn left
        while(sensors->R2 == 0 && sensors->L2 == 1)
            {
                turn_left(255,1);
                reflectance_digital(sensors);
            }
        //turn right
        while(sensors->R2 == 1 && sensors->L2 == 0)
            {
                turn_right(255, 1);
                reflectance_digital(sensors);
            }
        //BONUS MQTT
        if(on_line == true && getRefValues(sensors, 0,0,0,0,0,0)){ 
            on_line = false;
            print_mqtt("Zumo10/miss", "%d", xTaskGetTickCount()-*startt);   //print mqtt
        }
        else if(on_line == false && getRefValues(sensors, 0,0,1,1,0,0)){
            on_line = true;
            print_mqtt("Zumo10/line", "%d", xTaskGetTickCount()-*startt);   //print mqtt
        }
            
    motor_forward(255, 10);
    reflectance_digital(sensors);
    }
    //motor stop
    motor_forward(0, 0);
}
void start_up(bool motor, bool ultrasonic, bool reflectance, bool IR){
    if(motor){
        motor_start();
        motor_forward(0, 0);
    }
    if(ultrasonic){
        Ultra_Start();
    }
    if(reflectance){
        reflectance_start();
        reflectance_set_threshold(15000, 15000, 15000, 15000, 15000, 15000);
    }
    if(IR){
        IR_Start();
        IR_flush();
    }
    printf("Robot startup finished!\n");
}    
void stop_down(void){
    motor_forward(0, 0);
    motor_stop();
}
void progEnd(uint32_t delay){
    bool led = false;
    while(true) {
        BatteryLed_Write(led^=1);
        vTaskDelay(delay);
    }  
} 
int getRefValues (struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3){
    if (sensors->L1 == L1 && sensors->L2 == L2 && sensors->L3 == L3 && sensors->R1 == R1 && sensors->R2 == R2 &&sensors->R3 == R3 ){
        return 1;
    }else{
        return 0;
    }
}
void turn_right(uint8 speed,uint32 delay){
    SetMotors(0,1, speed, speed, delay);
}
void turn_left(uint8 speed,uint32 delay){
    SetMotors(1,0, speed, speed, delay);
}
/* [] END OF FILE */
