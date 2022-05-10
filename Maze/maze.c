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
void shut(void);
void obstacle();
struct position{int x; int y; int direction;};
void startup(bool launch_button, bool motor, bool IR, bool reflectance, bool ultrasound);
void turn_maze(struct sensors_ *sensors, struct position *pos,int final_direction, int dist_obstacle);
int getRefValues(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3);
void tank_turn(int16 angle);
void wait_for_IR(void);
void maze_line_follow(struct sensors_ *sensors);

void zmain () 
{
    TickType_t start_time = 0;
    
    const int north = 0;
    const int west = -1;
    const int east = 1;
    const int x_limit = 3;
    const int y_limit = 11;
    const int dist_obstacle = 15;
    
    struct sensors_ sensors; 
    struct position pos = {0, 0, north};
    
    
    startup(true, true, true, true, true);
    start_time = xTaskGetTickCount();    
    maze_line_follow(&sensors);
    print_mqtt("Zumo10/ready", "maze");
    wait_for_IR();
    print_mqtt("Zumo10/start", "%d", start_time);
    maze_line_follow(&sensors);
   

    
    while(pos.y < 11)
    {
        
       
        if (Ultra_GetDistance() > dist_obstacle)
        {

            maze_line_follow(&sensors);
            pos.y++;
            print_mqtt("Zumo10/position", "%d %d", pos.x, pos.y);

        }else
        {
            if(pos.direction == north)
            {
                if(pos.x == x_limit)
                {
                    turn_maze(&sensors, &pos, west, dist_obstacle);
                }else if(pos.x == -x_limit)
                {
                    turn_maze(&sensors, &pos, east, dist_obstacle);
                }
            }
           
            if(pos.direction == north)
            {
                turn_maze(&sensors, &pos, west, dist_obstacle);
                if(pos.direction != west)
                {
                    turn_maze(&sensors, &pos, east, dist_obstacle);
                }

            }

           
           
            while(pos.direction != north)
            {
               
                maze_line_follow(&sensors);
                pos.x+=pos.direction;
                print_mqtt("Zumo10/position", "%d %d", pos.x, pos.y);
                turn_maze(&sensors, &pos, north, dist_obstacle);
                if(pos.direction != north && pos.x == x_limit*pos.direction)
                {
                
                    tank_turn(180);
                    pos.direction *= -1;
                }
                
            }

        
        }
        
         
    }

            if (pos.y == y_limit)
            {
                if (pos.x != 0) 
                {
                    turn_maze(&sensors, &pos, pos.x >= 0 ? -1 : 1, dist_obstacle);
                    while (pos.x != 0) 
                    {
                        maze_line_follow(&sensors);
                        pos.x += pos.direction;
                        print_mqtt("Zumo10/position", "%d %d", pos.x, pos.y);
                    }
                    turn_maze(&sensors, &pos, north, dist_obstacle);
                }
                maze_line_follow(&sensors);
                pos.y++;
                print_mqtt("Zumo10/position", "%d %d", pos.x, pos.y);
                maze_line_follow(&sensors);
                
                motor_forward(255, 300);

            }
            TickType_t complete = xTaskGetTickCount();
            print_mqtt("Zumo10/stop", "%d", complete);
            shut();
            print_mqtt("Zumo10/time", "%d", complete - start_time);
            
            
            progEnd(100);
}





void maze_line_follow(struct sensors_ *sensors)
{
    reflectance_digital(sensors);

    while(!getRefValues(sensors, 0,0,1,1,0,0))
    {
        while(sensors->L1 == 0 && sensors->R1 == 1)
        {
            tank_turn(-1);
            reflectance_digital(sensors);
        }
        while(sensors->L1 == 1 && sensors->R1 == 0)
        {
            tank_turn(1);
            reflectance_digital(sensors);
        }
        motor_forward(100,1);
        reflectance_digital(sensors);
    }
    while(sensors->L3 == 0 && sensors->R3 == 0)
    {
        if(sensors->R2 == 0 && sensors->L2 == 1)
        { 
          
            while(sensors->R2 == 0 && sensors->L2 == 1)
            {
                tank_turn(1);
                reflectance_digital(sensors);
            }
         
            tank_turn(3);
        }
        if(sensors->R2 == 1 && sensors->L2 == 0)
        { 
          
            while(sensors->R2 == 1 && sensors->L2 == 0)
            {
                tank_turn(-1);
                reflectance_digital(sensors);
            }
         
            tank_turn(-3);
        }
        motor_forward(100,1);
        reflectance_digital(sensors);
    }
    motor_forward(0,0);
    
    
    
}
void turn_maze(struct sensors_ *sensors, struct position *pos,int final_direction, int dist_obstacle)
{
 
    int turn_direction = pos->direction - final_direction;
    int left_speed = turn_direction >= 0 ? 20 : 200 ;
    int right_speed = turn_direction >= 0 ? 200 : 20;
    uint16 delay = 0;
   
    while(!getRefValues(sensors, 0, 0, 1, 1, 0, 0))
    {
        SetMotors(0,0,left_speed, right_speed, 20);
        reflectance_digital(sensors);
        delay+=20;
    }

  
    if (Ultra_GetDistance() > dist_obstacle)
    {
        pos->direction = final_direction;
    }else 
    {
        motor_forward(0,0);
        vTaskDelay(300);
        SetMotors(1, 1,pos->direction >= 0 ? 20 : 200, pos->direction >= 0 ? 200 : 20, delay);
    }
   
    motor_forward(0,0);
}
void startup(bool launch_button, bool motor, bool IR, bool reflectance, bool ultrasound) {
    
    // Print startup message
    printf("Starting up.\n");

    // Start up motors
    if (motor) {
        motor_start();
        motor_forward(0, 0);
    }
    
    // Start up IR sensors
    if (IR) {
        IR_Start();
        IR_flush();
    }
    
    // Start reflectance
    if (reflectance) {
        reflectance_start();
        reflectance_set_threshold(15000, 15000, 17000, 17000, 15000, 15000);
    }
    
    // Start ultrasound
    if (ultrasound) {
        Ultra_Start();
    }
    
    // Wait for button press to start
    if (launch_button) {
        printf("Press start.\n");
        BatteryLed_Write(1);
        while(SW1_Read() == 1);
        BatteryLed_Write(0);
        vTaskDelay(1000);
    }

}          

void shut(void){
    motor_forward(0,0);
    motor_stop();
}

void obstacle (){
    motor_forward(0,10);
    motor_backward(100, 150);
    motor_turn(0, 150, 462);
}
int getRefValues(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3)
{
 if (sensors->L1 == L1 && sensors->L2 == L2 && sensors->L3 == L3 && sensors->R1 == R1 && sensors->R2 == R2 &&sensors->R3 == R3 )
{
   
    return 1;
}else
{
    
    return 0;
}
}
void tank_turn(int16 angle) {

    uint8 left_dir = (angle < 0) ? 0 : 1;
    uint8 right_dir = (angle >= 0) ? 0 : 1;
    
    uint8 angle_corrected = ((angle < 0) ? angle * -1 : angle) % 360;
    
    uint32 delay = (angle_corrected * 1048) / 360; // 1048 delay is ~a whole turn at 200 speed
    
    SetMotors(left_dir, right_dir, 100, 100, delay);
}
void progEnd(uint32_t delay){
    bool led = false;
    while(true) {
        BatteryLed_Write(led^=1);
        vTaskDelay(delay);
    } 
}
void wait_for_IR(void) {
    IR_flush();
    IR_wait();
}