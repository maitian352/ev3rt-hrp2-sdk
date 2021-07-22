#include <stdlib.h>
#include <stdio.h>
#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <math.h>


// define motors and sensors
const int color_sensor1 = EV3_PORT_1, color_sensor2 = EV3_PORT_2, color_sensor3 = EV3_PORT_3, color_sensor4 = EV3_PORT_4, left_motor = EV3_PORT_B, right_motor = EV3_PORT_C, a_motor = EV3_PORT_A, d_motor = EV3_PORT_D;
int finished = false;

// define variables
/**
 * \brief Stores locations for doors
 * \param location End location for door (LEFT, CENTER, RIGHT, LEFTFULL, CENTERFULLLEFT, CENTERFULLRIGHT, RIGHTFULL) [0-5]
 * \exception FULL variants of locations are only for collecting and do not gaurentee that all other doors are closed
**/
int doorLocations[6] = {
    760,
    0,
    -760,
    220,
    760,
    -220,
};
/**
 * \brief Stores the current item in the bays [RED, GREEN, BLUE, REDB, GREENB, BLUEB, BATTERY, BATTERYx2]
 * \param bay Bay number [LEFT, CENTER, RIGHT]
**/
int bayCars[3] = {
    NONE, NONE, NONE
};
/**
 * \brief stores the values of the cars on the road [RED, GREEN, BLUE]
 * \param index Car index [0-5]
**/
int roadcarPositions[6] = {
    NONE, NONE, NONE, NONE, NONE, NONE
};
/**
 * \brief Stores the current locations of the cars in the parking garage
 * \param row The row of parking spots [0-2]
 * \param number The number of the parking spot from entrance [0-3]
**/
int mapcarPositions[3][4] = {
    {
        NONE,NONE,NONE,NONE
    },
    {
        NONE,NONE,NONE,NONE
    },
    {
        NONE,NONE,NONE,NONE
    }
};
/**
 * \brief Stores the current locations of the batteries in the parking garage
 * \param row The row of parking spots [0-2]
 * \param number The number of the parking spot from entrance [0-3]
**/
int batteryPositions[3][4] = {
    {
        NONE,NONE,NONE,NONE
    },
    {
        NONE,NONE,NONE,NONE
    },
    {
        NONE,NONE,NONE,NONE
    }
};
/**
 * \brief Stores the color of the parking spots in the parking garage
 * \param row The row of the parking spots [0-2]
 * \param number The number of the parking spot from entrance [0-3]
**/
int mapPositions[3][4] = {
    {
        GREEN,RED,BLUE,BLUE
    },
    {
        BLUE,GREEN,RED,GREEN
    },
    {
        BLUE,GREEN,RED,RED
    },
};
/**
 * \brief Tasks remaining
 * \param 0 Green batteries to deliver
 * \param 1 Blue batteries to deliver
 * \param 2 Red cars to collect
 * \param 3 Red cars to deliver
**/
int tasks[4] = {2,2,1,1};

// main task
void main_task(intptr_t unused) {
    init();
    // test();
    collectBatteries();
    runParkingArea1();
    deliverCarsToYellow();
    detectWaitingCars();
    collectWaitingCars(0);
    end();
}

// main functions
/**
 * \brief Initializes the robot
**/
void init() {
    ev3_led_set_color(LED_ORANGE);

    // Register button handlers
    ev3_lcd_draw_string("Registering buttons...", 0, 9);
    ev3_button_set_on_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
    tslp_tsk(62);
    
    // Configure motors
    ev3_lcd_draw_string("Configuring motors...", 0, 18);
    ev3_motor_config(left_motor, MEDIUM_MOTOR);
    ev3_motor_config(right_motor, MEDIUM_MOTOR);
    ev3_motor_config(a_motor, MEDIUM_MOTOR);
    ev3_motor_config(d_motor, MEDIUM_MOTOR);
    tslp_tsk(184);
    
    // Configure sensors
    ev3_lcd_draw_string("Configuring Sensors...", 0, 27);
    ev3_sensor_config(color_sensor1, HT_NXT_COLOR_SENSOR);
    ev3_sensor_config(color_sensor2, COLOR_SENSOR);
    ev3_sensor_config(color_sensor3, COLOR_SENSOR);
    ev3_sensor_config(color_sensor4, HT_NXT_COLOR_SENSOR);
    tslp_tsk(149);
    
    // Set up sensors
    ev3_lcd_draw_string("Initializing sensors...", 0, 36);
    ev3_color_sensor_get_reflect(color_sensor2);
    ev3_color_sensor_get_reflect(color_sensor3);
    rgb_raw_t rgb1;
    bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    assert(val1);
    rgb_raw_t rgb4;
    bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
    assert(val4);

    // Configure brick
    ev3_lcd_draw_string("Configuring brick...", 0, 45);
    if (ev3_battery_voltage_mV() < 8000) {
        ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
        ev3_motor_stop(left_motor, false);
        ev3_motor_stop(right_motor, false);
        ev3_motor_stop(a_motor, false);
        ev3_motor_stop(d_motor, false);
        ev3_led_set_color(LED_RED);
        ev3_lcd_set_font(EV3_FONT_MEDIUM);
        ev3_lcd_draw_string("! LOW  BATTERY !", 10, 60);
        ev3_speaker_play_tone(2093, 500);
        tslp_tsk(500);
        ev3_speaker_play_tone(1046, 500);
        tslp_tsk(500);
        ev3_speaker_play_tone(2093, 500);
        tslp_tsk(500);
        ev3_speaker_play_tone(1046, 500);
        tslp_tsk(500);
        ev3_speaker_play_tone(2093, 500);
        tslp_tsk(500);
        ev3_speaker_play_tone(1046, 500);
        exit(1);
    }
    tslp_tsk(13);

    ev3_led_set_color(LED_OFF);

    // reset doors and sensors
    ev3_lcd_draw_string("Resetting motors...", 0, 54);
    ev3_motor_set_power(a_motor, 20);
    ev3_motor_set_power(d_motor, 100);
    tslp_tsk(1500);
    ev3_motor_set_power(a_motor, 0);
    ev3_motor_stop(d_motor, false);
    tslp_tsk(500);
    ev3_motor_rotate(d_motor, -810, 100, true);
    ev3_motor_reset_counts(d_motor);
    ev3_motor_reset_counts(a_motor);

    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    // wait for button press
    ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
    ev3_lcd_draw_string("Press OK to run", 14, 45);
    ev3_led_set_color(LED_GREEN);
    ev3_lcd_fill_rect(77, 87, 24, 20, EV3_LCD_BLACK);
    ev3_lcd_fill_rect(79, 89, 20, 1, EV3_LCD_WHITE);
    ev3_lcd_draw_string("OK", 79, 90);
    while (!ev3_button_is_pressed(ENTER_BUTTON));
    while (ev3_button_is_pressed(ENTER_BUTTON));
    ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
}
/**
 * \brief Drives out of base and collects batteries
**/
void collectBatteries() {
    wallFollow(18.1,15);
    tslp_tsk(100);
    moveDoor(RIGHTFULL);
    wallFollow(4,10);
    tslp_tsk(100);
    moveDoor(CENTER);
    wallFollow(2.9,10);
    tslp_tsk(100);
    moveDoor(RIGHTFULL);
    wallFollow(4,10);
    tslp_tsk(100);
    moveDoor(CENTER);
    wallFollow(2.9,10);
    tslp_tsk(100);
    moveDoor(RIGHT);
    wallFollow(4,10);
    tslp_tsk(100);
    moveDoor(CENTER);
    wallFollow(2.9,10);
    tslp_tsk(100);
    moveDoor(RIGHT);
    wallFollow(4.2,15);
    tslp_tsk(100);
    moveDoor(CENTER);
    tslp_tsk(100);
    bayCars[CENTER] = BATTERYx2;
    bayCars[RIGHT] = BATTERYx2;
    resetDoor();
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    ev3_motor_rotate(right_motor, 180, 20, true);
    tslp_tsk(100);
    drive(14, 20);
    motorSteer(10, 0);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) > 25) {tslp_tsk(5);}
    motorSteer(0, 0);
    tslp_tsk(100);
    drive(13, 15);
    tslp_tsk(100);
    motorSteer(20, 100);
    tslp_tsk(200);
    motorSteer(10, 100);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 30) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) >= 15) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) < 15) {tslp_tsk(5);}
    motorSteer(0,0);
    PID(35, 30, NONE, CENTER, NONE, NONE, true);
}
/**
 * \brief Does first run through of the parking area
**/
void runParkingArea1() {
    // rows 0 and 1
    for(int i = 3;i >= 0;i--){
        readcar(i+4, i);
        if(mapcarPositions[0][i] == WALL){
            
        }
        else if(mapcarPositions[0][i] == NONE){
            if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[0][i] != RED && tasks[mapPositions[0][i]] != 0){
                turn(RIGHT);
                doParkingSpot(i, false, true);
                turn(LEFT);
            }
            else if(searchforcar(mapPositions[0][i],LEFT) != NONE){
                turn(RIGHT);
                doParkingSpot(i, false, true);
                turn(LEFT);
            }
        }
        else if(searchforcar(NONE,LEFT) != NONE){
            turn(RIGHT);
            doParkingSpot(i, false, true);
            turn(LEFT);
        }
        
        if(mapcarPositions[1][i] == WALL){
            
        }
        else if(mapcarPositions[1][i] == NONE){
            if(searchforcar(mapPositions[1][i],LEFT) != NONE){
                turn(LEFT);
                doParkingSpot(i + 4, false, false);
                turn(RIGHT);
            }
        }
        else if(searchforcar(NONE,LEFT) != NONE && mapPositions[1][i] != RED){
            turn(LEFT);
            doParkingSpot(i + 4, false, false);
            turn(RIGHT);
        }
        
        if(i == 0){
            if (mapcarPositions[1][0] == WALL) {
                turn(RIGHT);
            } else {
                turn(LEFT);
            }
        }
        else{
            PID(25,25,NONE,CENTER,NONE, NONE,true);
        }
    }
    // decision for row 2
    // NOTE: also check if the robot has a green and blue car
    if (searchforcar(BATTERY, LEFT) != NONE && searchforcar(BATTERYx2, LEFT) != NONE) {

    }
    // get to row 2
    int direction = 0;
    int driveThrough = false;
    for(int i = 0;i <= 3;i++){
        if(mapcarPositions[1][i] != WALL && mapcarPositions[1][i] != NONE && batteryPositions[1][i] == NONE && driveThrough == false){
            if(i == 0){

            }
            else{
                PID(i*27-2,30,RIGHT,CENTER,NONE,NONE,true);
            }
            moveDoor(searchforcar(NONE, LEFT));
            drive(10, 10);
            moveDoor(searchforcar(NONE, LEFT) + 3);
            drive(9, 10);
            resetDoor();
            PID(24, 25, CENTER, CENTER, NONE, NONE, true);
            if(i == 0){
                turn(LEFT);
            }
            else if(i == 1){
                turn(LEFT);
                PID(56,30,RIGHT,CENTER,NONE,NONE,true);
            }
            else if(i == 2){
                direction = 1;
                turn(LEFT);
                PID(25, 30, RIGHT, CENTER,NONE, NONE, true);
            }
            else if(i == 3){
                direction = 1;
                turn(RIGHT);
            }
            driveThrough = true;
        }
        if(mapcarPositions[1][i] == NONE && batteryPositions[1][i] == NONE && driveThrough == false){
            if(i == 0){

            }
            else{
                PID(i*27-2,30,RIGHT,CENTER,NONE,NONE,true);
            }
            drive(20, 20);
            PID(24, 25, CENTER, CENTER, NONE, NONE, true);
            if(i == 0){
                turn(LEFT);
            }
            else if(i == 1){
                turn(LEFT);
                PID(56,30,RIGHT,CENTER,NONE,NONE,true);
            }
            else if(i == 2){
                direction = 1;
                turn(LEFT);
                PID(25, 30, RIGHT, CENTER,NONE, NONE, true);
            }
            else if(i == 3){
                direction = 1;
                turn(RIGHT);
            }
            driveThrough = true;
        }
    }
    if(driveThrough == false){
        direction = 1;
        PID(134, 35, RIGHT, RIGHT, NONE, NONE, true);
        PID(46, 30, RIGHT,RIGHT, NONE, NONE, false);
        PID(44, 30, NONE, CENTER, NONE, NONE, true);
    }
    if (direction == 1) {
        // row 2 from 11
        for (int i = 3; i >= 0; i--) {
            readcar(i+8, NONE);
            if(mapcarPositions[2][i] == WALL){
                
            }
            else if(mapcarPositions[2][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED && tasks[mapPositions[2][i]] != 0){
                    turn(LEFT);
                    doParkingSpot(i + 8, false, true);
                    turn(RIGHT);
                }
                else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                    turn(LEFT);
                    doParkingSpot(i + 8, false, true);
                    turn(RIGHT);
                }
            }
            else if(mapcarPositions[2][i] == mapPositions[2][i]){
                
            }
            else if(searchforcar(NONE,LEFT) != NONE){
                turn(LEFT);
                doParkingSpot(i + 8, false, true);
                turn(RIGHT);
            }
            if(mapcarPositions[1][i] == WALL){
                
            }
            else if(mapcarPositions[1][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[1][i] != RED && tasks[mapPositions[1][i]] != 0){
                    turn(RIGHT);
                    doParkingSpot(i+4, false, true);
                    turn(LEFT);
                }
                else if(searchforcar(mapPositions[1][i],LEFT) != NONE){
                    turn(RIGHT);
                    doParkingSpot(i+4, false, true);
                    turn(LEFT);
                }
            }
            else if(searchforcar(NONE,LEFT) != NONE && tasks[mapcarPositions[1][i] + 1]){
                turn(RIGHT);
                doParkingSpot(i+4, false, true);
                turn(LEFT);
            }
            if(i == 0){
                turn(LEFT);
                turn(LEFT);
                PID(134, 35,LEFT, LEFT, NONE, NONE, false);
            }
            else{
                PID(25,25,NONE,CENTER,NONE,NONE,true);
            }
        }
    } else {
        // row 2 from 8
        for (int i = 0; i <= 3; i++) {
            readcar(NONE, i+8);
            if(mapcarPositions[2][i] == WALL){
                
            }
            else if(mapcarPositions[2][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED && tasks[mapPositions[2][i]] != 0){
                    turn(RIGHT);
                    doParkingSpot(i + 8, false, true);
                    turn(LEFT);
                }
                else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                    turn(RIGHT);
                    doParkingSpot(i + 8, false, true);
                    turn(LEFT);
                }
            }
            else if(mapcarPositions[2][i] == mapPositions[2][i]){
                
            }
            else if(searchforcar(NONE,LEFT) != NONE){
                turn(RIGHT);
                doParkingSpot(i + 8, false, true);
                turn(LEFT);
            }
            if(mapcarPositions[1][i] == WALL){
                
            }
            else if(mapcarPositions[1][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[1][i] != RED && tasks[mapPositions[1][i]] != 0){
                    turn(LEFT);
                    doParkingSpot(i+4, false, true);
                    turn(RIGHT);
                }
                else if(searchforcar(mapPositions[1][i],LEFT) != NONE){
                    turn(LEFT);
                    doParkingSpot(i+4, false, true);
                    turn(RIGHT);
                }
            }
            else if(searchforcar(NONE,LEFT) != NONE && tasks[mapcarPositions[1][i] + 1]){
                turn(LEFT);
                doParkingSpot(i+4, false, true);
                turn(RIGHT);
            }
            if(i == 3){
                PID(42, 30, LEFT, LEFT, NONE, NONE, false);
            }
            else{
                PID(25,25,NONE,CENTER,NONE,NONE,true);
            }
        }
    }
    PID(44, 30, RIGHT, CENTER, NONE, NONE, false);
}
/**
 * \brief Does second run through of the parking area
**/
void runParkingArea2(){
    int red[3] = {-1,-1,-1};
    int green[2] = {-1,-1};
    int blue[2] = {-1,-1};
    int redindex = 0;
    int greenindex = 0;
    int blueindex = 0;
    for(int i = 0;i < 6;i++){
        for(int j = 0;j < 3;j++){
            for(int k = 0;k < 4;k++){
                if(roadcarPositions[i] == RED){
                    if(mapPositions[j][k] == RED){
                        red[redindex] = j * 4 + k;
                        redindex += 1;
                    }
                }
                if(roadcarPositions[i] == GREEN){
                    if(mapPositions[j][k] == GREEN && batteryPositions[j][k] == BATTERY){
                        green[greenindex] = j * 4 + k;
                        greenindex += 1;
                    }
                }
                if(roadcarPositions[i] == BLUE){
                    if(mapPositions[j][k] == BLUE && batteryPositions[j][k] == BATTERY){
                        blue[blueindex] = j * 4 + k;
                        blueindex += 1;
                    }
                }
            }
        }
    }
    int currentCars[3] = {0,0,0};
    for(int i = 0;i < 3;i++){
        currentCars[bayCars[i]] += 1;
    }
    int side1[3] = {0,0,0};
    int side2[3] = {0,0,0};
    for(int i = 0;i < 3;i++){
        if(i != -1){
            if(red[i] > 7){
                side2[RED] += 1;
            }
            else if(red[i] > 3){
                side2[RED] += 1;
                side1[RED] += 1;
            }
            else{
                side1[RED] += 1;
            }
        }
    }
    for(int i = 0;i < 2;i++){
        if(i != -1){
            if(green[i] > 7){
                side2[GREEN] += 1;
            }
            else if(green[i] > 3){
                side2[GREEN] += 1;
                side1[GREEN] += 1;
            }
            else{
                side1[GREEN] += 1;
            }
        }
    }
    for(int i = 0;i < 2;i++){
        if(i != -1){
            if(blue[i] > 7){
                side2[BLUE] += 1;
            }
            else if(blue[i] > 3){
                side2[BLUE] += 1;
                side1[BLUE] += 1;
            }
            else{
                side1[BLUE] += 1;
            }
        }
    }

    int canDoSide1 = true;
    for(int i = 0;i < 3;i++){
        if(side1[i] < currentCars[i]){
            canDoSide1 = false;
        }
    }
    int canDoSide2 = true;
    for(int i = 0;i < 3;i++){
        if(side2[i] < currentCars[i]){
            canDoSide2 = false;
        }
    }
    if(canDoSide1){
        int currentDelivers[3] = {-1,-1,-1};
        int currentIndex = 0;
        for(int i = 0;i < 3;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(red[i] < 8){
                        currentDelivers[currentIndex] = red[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        for(int i = 0;i < 2;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(green[i] < 8){
                        currentDelivers[currentIndex] = green[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        for(int i = 0;i < 2;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(blue[i] < 8){
                        currentDelivers[currentIndex] = blue[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        for(int i = 3;i >= 0;i--){
            if(bayCars[0] == NONE && bayCars[1] == NONE && bayCars[2] == NONE){
                turn(LEFT);
                turn(LEFT);
                PID(27 * (4 - i) - 2,25,NONE,CENTER,NONE, NONE,true);
                continue;
            }
            else{
                for(int j = 0;j < 3;j++){
                    if(currentDelivers[j] == i){
                        turn(RIGHT);
                        doParkingSpot(i, false, false, false);
                        turn(LEFT);
                    }
                }
                for(int j = 0;j < 3;j++){
                    if(currentDelivers[j] == i + 4){
                        turn(LEFT);
                        doParkingSpot(i + 4, false, false, false);
                        turn(RIGHT);
                    }
                }
                if(i == 0){
                    turn(LEFT);
                    turn(LEFT);
                    PID(27 * 4 - 2,25,NONE,CENTER,NONE, NONE,true);
                }
                else{
                    PID(25,25,NONE,CENTER,NONE, NONE,true);
                }
            }
        }
    }
    else if(canDoSide2){
        int currentDelivers[3] = {-1,-1,-1};
        int currentIndex = 0;
        for(int i = 0;i < 3;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(red[i] >= 4){
                        currentDelivers[currentIndex] = red[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        for(int i = 0;i < 2;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(green[i] >= 4){
                        currentDelivers[currentIndex] = green[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        for(int i = 0;i < 2;i++){
            if(i != -1){
                if(currentIndex < 3){
                    if(blue[i] >= 4){
                        currentDelivers[currentIndex] = blue[i];
                        currentIndex += 1;
                    }
                }
            }
        }
        
        turn(LEFT);
        PID(46, 30, RIGHT,RIGHT, NONE, NONE, false);
        PID(44, 30, NONE, CENTER, NONE, NONE, true);
        for(int i = 3;i >= 0;i--){
            if(bayCars[0] == NONE && bayCars[1] == NONE && bayCars[2] == NONE){
                turn(LEFT);
                turn(LEFT);
                PID(27 * (4 - i) - 2 + 44,25,LEFT,LEFT,NONE, NONE,true);
                PID(46, 30, RIGHT,CENTER, NONE, NONE, false);
                continue;
            }
            else{
                for(int j = 0;j < 3;j++){
                    if(currentDelivers[j] == i){
                        turn(RIGHT);
                        doParkingSpot(i + 4, false, false, false);
                        turn(LEFT);
                    }
                }
                for(int j = 0;j < 3;j++){
                    if(currentDelivers[j] == i + 4){
                        turn(LEFT);
                        doParkingSpot(i + 8, false, false, false);
                        turn(RIGHT);
                    }
                }
                if(i == 0){
                    turn(LEFT);
                    turn(LEFT);
                    PID(27 * 4 - 2 + 44,25,LEFT,LEFT,NONE, NONE,true);
                    PID(46, 30, RIGHT,CENTER, NONE, NONE, false);
                }
                else{
                    PID(25,25,NONE,CENTER,NONE, NONE,true);
                }
            }
        }
    }
}
/*
void runParkingArea2() {
    // rows 0 and 1
    for(int i = 3;i >= 0;i--){
        if(mapcarPositions[0][i] == WALL){
            
        }
        else if(mapcarPositions[0][i] == NONE){
            if(searchforcar(mapPositions[0][i],LEFT) != NONE && batteryPositions[0][i] == BATTERY){
                turn(RIGHT);
                doParkingSpot(i, true);
                turn(LEFT);
            }
        }
        else if(mapcarPositions[0][i] == mapPositions[0][i]){
            
        }
        else if(searchforcar(NONE,LEFT) != NONE){
            turn(RIGHT);
            doParkingSpot(i, true);
            turn(LEFT);
        }
        if(mapcarPositions[1][i] == WALL){
            
        }
        else if(mapcarPositions[1][i] == NONE){
            if(searchforcar(mapPositions[1][i],LEFT) != NONE){
                turn(LEFT);
                doParkingSpot(i + 4, true);
                turn(RIGHT);
            }
        }
        else if(mapcarPositions[1][i] == mapPositions[1][i]){
            
        }
        else if(searchforcar(NONE,LEFT) != NONE){
            turn(LEFT);
            doParkingSpot(i + 4, true);
            turn(RIGHT);
        }
        if(i == 0){
            if (mapcarPositions[1][0] == WALL) {
                turn(RIGHT);
            } else {
                turn(LEFT);
            }
        }
        else{
            PID(25,25,NONE,CENTER,NONE, NONE,true);
        }
    }
    // decision for row 2
    // NOTE: also check if the robot has a green and blue car
    if (searchforcar(RED, LEFT) == NONE && searchforcar(GREEN, LEFT) == NONE && searchforcar(BLUE, LEFT) == NONE) {
        // get to row 2
        int direction = 0;
        if(mapcarPositions[1][0] == NONE && batteryPositions[1][0] == NONE){
            PID(46, 30, LEFT, CENTER, NONE, NONE, true);
        }
        else {
            if (mapcarPositions[1][0] == WALL) {
                turn(RIGHT);
            } else {
                turn(LEFT);
            }
            if(mapcarPositions[1][1] == NONE && batteryPositions[1][1] == NONE){
                PID(25,30,RIGHT,CENTER,NONE,NONE,true);
                PID(46, 30, RIGHT, CENTER, NONE, NONE, true);
                PID(25, 30, LEFT, CENTER, NONE, NONE, true);
                turn(LEFT);
            }
            else if(mapcarPositions[1][2] == NONE && batteryPositions[1][2] == NONE){
                direction = 1;
                PID(58,30,RIGHT,CENTER,NONE,NONE,true);
                drive(20, 20);
                PID(24, 30, LEFT, CENTER, NONE, NONE, true);
                PID(25, 30, RIGHT, CENTER,NONE, NONE, true);
                turn(RIGHT);
            } else if (mapcarPositions[1][3] == NONE && batteryPositions[1][3] == NONE) {
                direction = 1;
                PID(88, 35, RIGHT, CENTER, NONE, NONE, true);
                PID(46, 30, RIGHT, CENTER, NONE, NONE, true);
            } else {
                direction = 1;
                PID(134, 35, RIGHT, RIGHT, NONE, NONE, true);
                PID(46, 30, RIGHT,RIGHT, NONE, NONE, false);
                PID(44, 30, NONE, CENTER, NONE, NONE, true);
            }
        }
        if (direction == 1) {
            // row 2 from 11
            for (int i = 3; i >= 0; i--) {
                if(mapcarPositions[2][i] == WALL){
                    
                }
                else if(mapcarPositions[2][i] == NONE){
                    if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED){
                        turn(LEFT);
                        doParkingSpot(i + 8, true);
                        turn(RIGHT);
                    }
                    else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                        turn(LEFT);
                        doParkingSpot(i + 8, true);
                        turn(RIGHT);
                    }
                }
                else if(mapcarPositions[2][i] == mapPositions[2][i]){
                    
                }
                else if(searchforcar(NONE,LEFT) != NONE){
                    turn(LEFT);
                    doParkingSpot(i + 8, true);
                    turn(RIGHT);
                }
                if(i == 0){
                    turn(LEFT);
                    turn(LEFT);
                    PID(134, 35,LEFT, LEFT, NONE, NONE, false);
                }
                else{
                    PID(25,25,NONE,CENTER,NONE,NONE,true);
                }
            }
        } else {
            // row 2 from 8
            for (int i = 0; i <= 3; i++) {
                if(mapcarPositions[2][i] == WALL){
                    
                }
                else if(mapcarPositions[2][i] == NONE){
                    if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED){
                        turn(RIGHT);
                        doParkingSpot(i + 8, true);
                        turn(LEFT);
                    }
                    else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                        turn(RIGHT);
                        doParkingSpot(i + 8, true);
                        turn(LEFT);
                    }
                }
                else if(mapcarPositions[2][i] == mapPositions[2][i]){
                    
                }
                else if(searchforcar(NONE,LEFT) != NONE){
                    turn(RIGHT);
                    doParkingSpot(i + 8, true);
                    turn(LEFT);
                }
                if(i == 3){
                    PID(42, 30, LEFT, LEFT, NONE, NONE, false);
                }
                else{
                    PID(25,25,NONE,CENTER,NONE,NONE,true);
                }
            }
        }
        PID(44, 30, RIGHT, CENTER, NONE, NONE, false);
    } else {
        if (mapcarPositions[1][0] == WALL) {
            turn(RIGHT);
        } else {
            turn(LEFT);
        }
        PID(134, 35, NONE, CENTER, NONE, NONE, true);
    }
}
*/
/**
 * \brief Does third run through of the parking area
**/
void runParkingArea3(){
    int red[3] = {-1,-1,-1};
    int green[2] = {-1,-1};
    int blue[2] = {-1,-1};
    int redindex = 0;
    int greenindex = 0;
    int blueindex = 0;
    for(int i = 0;i < 6;i++){
        for(int j = 0;j < 3;j++){
            for(int k = 0;k < 4;k++){
                if(roadcarPositions[i] == RED){
                    if(mapPositions[j][k] == RED){
                        red[redindex] = j * 4 + k;
                        redindex += 1;
                    }
                }
                if(roadcarPositions[i] == GREEN){
                    if(mapPositions[j][k] == GREEN && batteryPositions[j][k] == BATTERY){
                        green[greenindex] = j * 4 + k;
                        greenindex += 1;
                    }
                }
                if(roadcarPositions[i] == BLUE){
                    if(mapPositions[j][k] == BLUE && batteryPositions[j][k] == BATTERY){
                        blue[blueindex] = j * 4 + k;
                        blueindex += 1;
                    }
                }
            }
        }
    }
}

/**
 * \brief Delivers two cars to yellow areas
**/
void deliverCarsToYellow() {
    PID(48, 30, RIGHT, RIGHT, NONE, NONE, false);
    PID(33,20,NONE,NONE,-1,NONE,false);
    ev3_motor_rotate(left_motor,230,20,false);
    ev3_motor_rotate(right_motor,230,20,true);
    if(searchforcar(BLUE,LEFT) > searchforcar(GREEN,LEFT)){
        moveDoor(searchforcar(GREEN,LEFT) + 3);
        drive(12, 15);
        drive(14, -15);
        resetDoor();
        drive(7, 15);
        drive(5, -15);
        bayCars[searchforcar(GREEN,LEFT)] = NONE;
    }
    else{
        moveDoor(searchforcar(BLUE,LEFT) + 3);
        drive(12, 15);
        drive(14, -15);
        resetDoor();
        drive(7, 15);
        drive(5, -15);
        bayCars[searchforcar(BLUE,LEFT)] = NONE;
    }
    ev3_motor_rotate(left_motor,230,-20,false);
    ev3_motor_rotate(right_motor,230,-20,true);
    drive(6, 15);
    motorSteer(10, -100);
    tslp_tsk(250);
    motorSteer(10, 100);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 30) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) >= 15) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) < 15) {tslp_tsk(5);}
    motorSteer(0, 0);
    PID(21,20,NONE,NONE,-1,1,false);
    ev3_motor_rotate(left_motor,230,20,false);
    ev3_motor_rotate(right_motor,230,20,true);
    if(searchforcar(BLUE,LEFT) < searchforcar(GREEN,LEFT)){
        moveDoor(searchforcar(GREEN,LEFT) + 3);
        drive(12, 15);
        drive(14, -15);
        resetDoor();
        drive(7, 15);
        drive(5, -15);
        bayCars[searchforcar(GREEN,LEFT)] = NONE;
    }
    else{
        moveDoor(searchforcar(BLUE,LEFT) + 3);
        drive(12, 15);
        drive(14, -15);
        resetDoor();
        drive(7, 15);
        drive(5, -15);
        bayCars[searchforcar(BLUE,LEFT)] = NONE;
    }
    ev3_motor_rotate(left_motor,230,20,false);
    ev3_motor_rotate(right_motor,230,20,true);
    drive(6, -15);
    motorSteer(10, -100);
    tslp_tsk(250);
    motorSteer(10, 100);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 30) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) >= 15) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) < 15) {tslp_tsk(5);}
    motorSteer(0, 0);
}
/**
 * \brief Detects and writes down values of all 6 cars on the road
**/
void detectWaitingCars(){
    int red = 2;
    int green = 2;
    int blue = 2;
    rgb_raw_t rgb1;
    bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    int timesdetected = val1;
    while(timesdetected < 2){
        val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        timesdetected += val1;
        tslp_tsk(10);
    }
    tslp_tsk(5);
    int cardetected = NONE;
    for(int i = 4;i >= 0;i--) {
        lowerSensors();
        val1 = 0;
        val1 = val1 + ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        while (val1 < 2) {
            tslp_tsk(100);
            val1 = val1 + ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        }
        if(rgb1.r > 25 && red != 0){
            cardetected = RED;
            red--;
        }
        else if(rgb1.b > 25 && blue != 0){
            cardetected = BLUE;
            blue--;
        }
        else if (green != 0) {
            cardetected = GREEN;
            green--;
        } else {
            if(rgb1.r > rgb1.b && red != 0){
                cardetected = RED;
                red--;
            }
            else if (blue != 0){
                cardetected = BLUE;
                blue--;
            } else if (red != 0){
                cardetected = RED;
                red--;
            }
        }
        ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
        // sprintf(msg, "%d %d %d",rgb1.r,rgb1.g,rgb1.b);
        // ev3_lcd_draw_string(msg, 10*4, 15*2.5);
        roadcarPositions[i] = cardetected;
        raiseSensors();
        if(i != 0){
            PID(11,20,NONE,NONE,NONE,NONE,false);
        }
    }
    if(red == 1){
        roadcarPositions[5] = RED;
    }
    if(green == 1){
        roadcarPositions[5] = GREEN;
    }
    if(blue == 1){
        roadcarPositions[5] = BLUE;
    }
    // sprintf(msg, "0: %d", roadcarPositions[0]);
    // ev3_lcd_draw_string(msg, 0, 15*0);
    // sprintf(msg, "1: %d", roadcarPositions[1]);
    // ev3_lcd_draw_string(msg, 0, 15*1);
    // sprintf(msg, "2: %d", roadcarPositions[2]);
    // ev3_lcd_draw_string(msg, 0, 15*2);
    // sprintf(msg, "3: %d", roadcarPositions[3]);
    // ev3_lcd_draw_string(msg, 0, 15*3);
    // sprintf(msg, "4: %d", roadcarPositions[4]);
    // ev3_lcd_draw_string(msg, 0, 15*4);
    // sprintf(msg, "5: %d", roadcarPositions[5]);
    // ev3_lcd_draw_string(msg, 0, 15*5);
    PID(16, 30, NONE, LEFT, NONE, NONE, false);
}
/**
 * \brief Collects 3 cars
 * \param set Set 0 or set 1 where set 0 is the first 3 cars and set 1 is the last 3 cars.
 * \exception Set 0 must start at the end of deliverCarsToYellow
 * \exception Set 1 must start somewhere
**/
void collectWaitingCars(int set){
    switch (set)
    {
    case 0:
        drive(18, 30);
        ev3_motor_rotate(left_motor,220,20,flase);
        ev3_motor_rotate(right_motor,220,20,ture);
        drive(3, 20);
        motorSteer(10, 0);
        while (ev3_color_sensor_get_reflect(color_sensor2) > 10 || ev3_color_sensor_get_reflect(color_sensor3) > 10) {tslp_tsk(5);}
        motorSteer(0, 0);
        drive(13.5, 15);
        turn(LEFT);
        PID(3, 20, NONE, NONE, NONE, NONE, false);
        motorSteer(10, 0);
        while (ev3_color_sensor_get_reflect(color_sensor2) > 10 || ev3_color_sensor_get_reflect(color_sensor3) > 10) {tslp_tsk(5);}
        motorSteer(0, 0);
        moveDoor(LEFT);
        drive(31,10);
        moveDoor(LEFTFULL);
        raiseDoor();
        drive(10,10);
        moveDoor(RIGHT);
        lowerDoor();
        moveDoor(RIGHTFULL);
        raiseDoor();
        drive(5,10);
        resetDoor();
        ev3_motor_set_power(a_motor, 20);
        tslp_tsk(1000);
        ev3_motor_set_power(a_motor, 0);
        drive(10, -10);
        raiseDoor();
        drive(3, 10);
        lowerDoor();
        bayCars[LEFT] = roadcarPositions[1];
        bayCars[CENTER] = roadcarPositions[0];
        bayCars[RIGHT] = roadcarPositions[2];
        ev3_motor_rotate(left_motor,440,20,flase);
        ev3_motor_rotate(right_motor,440,20,ture);
        drive(5,20);
        motorSteer(10, 0);
        while (ev3_color_sensor_get_reflect(color_sensor2) > 10 || ev3_color_sensor_get_reflect(color_sensor3) > 10) {tslp_tsk(5);}
        motorSteer(0, 0);
        drive(13.5, 10);
        turn(LEFT);
        PID(25, 30, NONE, LEFT, NONE, NONE, false);
        break;
    case 1:
        PID(25, 30, NONE, LEFT, NONE, NONE, false);
        // ev3_motor_reset_counts(left_motor);
        // ev3_motor_reset_counts(right_motor);
        // ev3_motor_rotate(d_motor, 740-ev3_motor_get_counts(d_motor), 100, true);
        // motorSteer(20, 100);
        // while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 <160) {tslp_tsk(5);}
        // motorSteer(10, 100);
        // while (ev3_color_sensor_get_reflect(color_sensor2) < 40) {tslp_tsk(5);}
        // // while (ev3_color_sensor_get_reflect(color_sensor2) > 35) {tslp_tsk(5);}
        // ev3_motor_stop(left_motor, true);
        // ev3_motor_stop(right_motor, true);
        // ev3_motor_rotate(left_motor,220,-20,flase);
        // ev3_motor_rotate(right_motor,220,-20,ture);
        ev3_motor_rotate(d_motor, 440-ev3_motor_get_counts(d_motor), 100, true);
        ev3_motor_reset_counts(left_motor);
        ev3_motor_reset_counts(right_motor);
        tslp_tsk(100);
        motorSteer(20, 100);
        while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 200) {tslp_tsk(5);}
        motorSteer(10, 100);
        while (ev3_color_sensor_get_reflect(color_sensor3) < 45) {tslp_tsk(5);}
        while (ev3_color_sensor_get_reflect(color_sensor3) >= 40) {tslp_tsk(5);}
        motorSteer(0, 0);
        // waitforButton();
        // ev3_motor_set_power(left_motor,30);
        // ev3_motor_set_power(right_motor,-30);
        // tslp_tsk(1500);
        // ev3_motor_set_power(left_motor,0);
        // ev3_motor_set_power(right_motor,0);
        // PID(24,10,NONE,CENTER,NONE,NONE,true);
        moveDoor(LEFT);
        drive(50.5,10);
        waitforButton();
        moveDoor(LEFTFULL);
        raiseDoor();
        drive(10,10);
        moveDoor(RIGHT);
        lowerDoor();
        moveDoor(RIGHTFULL);
        raiseDoor();
        drive(5,10);
        resetDoor();
        ev3_motor_set_power(a_motor, 20);
        tslp_tsk(1000);
        ev3_motor_set_power(a_motor, 0);
        drive(10, -10);
        raiseDoor();
        drive(3, 10);
        lowerDoor();
        bayCars[LEFT] = roadcarPositions[4];
        bayCars[CENTER] = roadcarPositions[3];
        bayCars[RIGHT] = roadcarPositions[5];
        ev3_motor_rotate(left_motor,440,20,flase);
        ev3_motor_rotate(right_motor,440,20,ture);
        PID(50,30,NONE,CENTER,NONE,NONE,ture);
        turn(LEFT);
        PID(25, 30, NONE, LEFT, NONE, NONE, false);
        // moveDoor(RIGHT);
        // drive(40, 30, 0);
        // drive(6.5,10,0);
        // ev3_motor_rotate(d_motor, 500, 50, true);
        // raiseDoor();
        // drive(10.5,10,0);
        // ev3_motor_rotate(d_motor, 440, 50, true);
        // lowerDoor();
        // ev3_motor_rotate(d_motor, -640, 50, true);
        // raiseDoor();
        // ev3_motor_rotate(left_motor, -440, 10, true);
        // resetDoor();
        // lowerDoor();
        break;
    default:
        exit(127);
        break;
    }
}
/**
 * \brief End function
**/
void end() {
    ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
    ev3_motor_stop(left_motor, false);
    ev3_motor_stop(right_motor, false);
    ev3_motor_stop(a_motor, false);
    ev3_motor_stop(d_motor, false);
    finished = true;
    ev3_led_set_color(LED_GREEN);
    ev3_lcd_draw_string("Program  Ended", 20, 60);
    exit(0);
}

// door functions
/**
 * \brief Moves the selected door number to the center
 * \param door Which door that is moved to the center [LEFT, RIGHT]
 * \exception CENTER will do the same thing as resetDoor() and blocks all bays
**/
void moveDoor(int door) {
    ev3_motor_rotate(d_motor, doorLocations[door]-ev3_motor_get_counts(d_motor), 100, true);
}
/**
 * \brief Resets the to neutral position and blocks all bays
**/
void resetDoor() {
    ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 100, true);
}
/**
 * \brief Closes all bays
 * \param side The side the door sticks out [LEFT, RIGHT]
**/
void closeDoors(int side) {
    switch (side)
    {
    case LEFT:
        ev3_motor_rotate(d_motor, 520-ev3_motor_get_counts(d_motor), 100, true);
        break;
    case RIGHT:
        ev3_motor_rotate(d_motor, -520-ev3_motor_get_counts(d_motor), 100, true);
        break;
    default:
        exit(127);
        break;
    }
}
/**
 * \brief Raises the doors
 * \exception Will lower the sensors
 * \exception Will open all bays and cannot back up or make turns without running "resetDoor()"
 * \exception Does not reset the doors
**/
void raiseDoor() {
    ev3_motor_set_power(a_motor, -40);
    tslp_tsk(1000);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Lowers the doors
 * \exception Will raise the sensors
 * \exception Does not reset the doors
**/
void lowerDoor() {
    ev3_motor_set_power(a_motor, 40);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Lowers the sensors
 * \exception Will raise the doors
**/
void raiseSensors() {
    ev3_motor_set_power(a_motor, 40);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Raises the sensors
 * \exception Will lower the doors
 * \exception Will open all bays and cannot back up or make turns without running "raiseSensors()"
**/
void lowerSensors() {
    ev3_motor_set_power(a_motor, -40);
    tslp_tsk(1000);
    ev3_motor_set_power(a_motor, 0);
}

// submodules
/**
 * \brief Reads and records the color of parkingspots
 * \param sensor sensor [1, 4]
 * \param parkingspotleft The id of the parkingspot [0-11] (NONE for nothing)
 * \param parkingspotright The id of the parkingspot [0-11] (NONE for nothing)
**/
void readcar(int parkingspotleft, int parkingspotright) {
    lowerSensors();
    if (parkingspotleft != NONE) {
        // char msg[100];
        rgb_raw_t rgb1;
        bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        int timesdetected = val1;
        while(timesdetected < 2){
            val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
            timesdetected += val1;
            tslp_tsk(10);
        }
        tslp_tsk(5);
        int cardetected = NONE;
        if(rgb1.r > 50 && rgb1.g > 50){
            cardetected = WALL;
        }
        else if(rgb1.r < 7 && rgb1.g < 7 && rgb1.b < 7){
            cardetected = NONE;
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == RED){
            if(rgb1.b > 30){
                cardetected = BLUE;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == GREEN){
            if(rgb1.r > 40){
                cardetected = RED;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == BLUE){
            if(rgb1.r > 40){
                cardetected = RED;
            }
            else{
                cardetected = BLUE;
            }
        }
        mapcarPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] = cardetected;
        // sprintf(msg, "Left: %d %d %d %d", cardetected,rgb1.r,rgb1.g,rgb1.b);
        // ev3_lcd_draw_string(msg, 10*4, 15*2.5);
    }
    if (parkingspotright != NONE) {
        // char msg[100];
        rgb_raw_t rgb4;
        int timesdetected = 0;
        bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
        timesdetected += val4;
        while(timesdetected < 2){
            val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
            timesdetected += val4;
            tslp_tsk(10);
        }
        assert(val4);
        tslp_tsk(5);
        int cardetected = NONE;
        if(rgb4.r > 50 && rgb4.g > 50){
            cardetected = WALL;
        }
        else if(rgb4.r < 7 && rgb4.g < 7 && rgb4.b < 7){
            cardetected = NONE;
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == RED){
            if(rgb4.b > 30){
                cardetected = BLUE;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == GREEN){
            if(rgb4.r > 40){
                cardetected = RED;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == BLUE){
            if(rgb4.r > 40){
                cardetected = RED;
            }
            else{
                cardetected = BLUE;
            }
        }
        mapcarPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] = cardetected;
        // sprintf(msg, "Right: %d %d %d %d", cardetected,rgb4.r,rgb4.g,rgb4.b);
        // ev3_lcd_draw_string(msg, 10*0, 15*4);
    }
    raiseSensors();
}
/**
 * \brief Does a parkingspot
 * \param parkingspot The current parking spot where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions [0-11]
 * \param deliverGreenBlue Whether to deliver green or blue cars
 * \param doBattery Whether to deliver batteries
 * \exception Function name is bad
**/
void doParkingSpot(int parkingspot, int deliverGreenBlue, int doBattery, int collectCars) {
    if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == NONE){
        if(mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] != RED && tasks[mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] - 1] != 0 && doBattery == true){
            deliverBattery(parkingspot);
        }
        if (mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == RED || deliverGreenBlue == true) {
            deliverCar(parkingspot,mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4]);
        }
    }
    else if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == WALL){

    }
    else{
        if(collectCars){
            collectCar(parkingspot);
        }
        if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == NONE){
            if(mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] != RED && tasks[mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] - 1] != 0 && doBattery == true){
                deliverBattery(parkingspot);
            }
            if (mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == RED || deliverGreenBlue == true) {
                deliverCar(parkingspot,mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4]);
            }
        }
    }
    resetDoor();
}
/**
 * \brief Returns whether or not we have a car of __ type in our bay 
 * \param cartype Type of object to look for [RED, GREEN, BLUE, BATTERY, BATTERYx2]
**/
int searchforcar(int cartype, int direction) {
    if(direction == LEFT){
        if(bayCars[0] == cartype){
            return 0;
        }
        if(bayCars[1] == cartype){
            return 1;
        }
        if(bayCars[2] == cartype){
            return 2;
        }
    }
    if(bayCars[2] == cartype){
        return 2;
    }
    if(bayCars[1] == cartype){
        return 1;
    }
    if(bayCars[0] == cartype){
        return 0;
    }
    return -1;
}
/**
 * \brief Battery module for doParkingSpot
 * \param parkingspot The current parking spot where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions [0-11]
**/
void deliverBattery(int parkingspot) {
    if (parkingspot == 2 || parkingspot == 6 || parkingspot == 9 || parkingspot == 11) {
        if(bayCars[RIGHT] == BATTERY){
            deliver(RIGHT,RIGHTFULL,true);
            batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
            bayCars[RIGHT] = NONE;
        }
        else if(bayCars[RIGHT] == BATTERYx2){
            deliver(RIGHT,RIGHTFULL,true);
            batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
            bayCars[RIGHT] = BATTERY;
        }
    }
    else{
        if(searchforcar(BATTERY,LEFT) != NONE){
            deliver(searchforcar(BATTERY,LEFT),CENTERFULLLEFT,true);
            batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
            bayCars[searchforcar(BATTERY,LEFT)] = NONE;
        }
        else if(searchforcar(BATTERYx2,LEFT) != NONE){
            if(searchforcar(BATTERYx2,LEFT) == CENTER){
                deliver(searchforcar(BATTERYx2,LEFT),CENTERFULLLEFT,true);
            }
            else{
                deliver(searchforcar(BATTERYx2,LEFT),RIGHTFULL,true);
            }
            batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
            bayCars[searchforcar(BATTERYx2,LEFT)] = BATTERY;
        }
    }
}
/**
 * \brief Collect car module for doParkingSpot
 * \param parkingspot The current parking spot where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions [0-11]
**/
void collectCar(int parkingspot) {
    if(bayCars[0] == NONE){
        collect(LEFT);
        bayCars[0] = mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4];
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = NONE;
    }
    else if(bayCars[1] == NONE){
        collect(LEFT);
        bayCars[1] = mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4];
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = NONE;
    }
    else if(bayCars[2] == NONE){
        collect(LEFT);
        bayCars[2] = mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4];
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = NONE;
    }
}
/**
 * \brief Car module for doParkingSpot
 * \param parkingspot The current parking spot where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions [0-11]
 * \param car Car to deliver
**/
void deliverCar(int parkingspot, int car) {
    if(searchforcar(car,LEFT) != -1){
        deliver(searchforcar(car,LEFT),searchforcar(car,LEFT) + 3,false);
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = car;
        bayCars[searchforcar(car,LEFT)] = NONE;
    }
}
/**
 * \brief Delivers specified bay to a location
 * \param bay The bay number that is to be delivered [LEFT, CENTER, RIGHT]
 * \param location The location where the selected bay is to be delivered [LEFT, CENTER, RIGHT]
 * \param battery Deliver batteries or not [true, false]
**/
void deliver(int bay, int location, int battery) {
    moveDoor(location);
    drive(18, 15);
    if (battery && bayCars[bay] == BATTERYx2) {
        drive(6.5, -10);
        drive(2, 10);
        if(bay == RIGHT){
            closeDoors(RIGHT);
        }
        else{
            closeDoors(LEFT);
        }
        drive(16.75, -15);
        drive(1.75, 10);
        moveDoor(location);
        drive(2, 10);
    } else {
        drive(19, -15);
        drive(1, 10);
    }
    closeDoors(LEFT);
}
/**
 * \brief Collects a car
 * \param bay The bay number that is to be delivered [LEFT, CENTER, RIGHT]
 * \exception PURPLE is not 1000 in this function
**/
void collect(int bay) {
    moveDoor(bay);
    drive(10.5, 10);
    moveDoor(bay + 3);
    drive(9, 10);
    resetDoor();
    drive(20, -15);
    drive(1, 10);
}

// drive functions
/**
 * \brief Starts motors at selected power and curve
 * \param power Power of the motors as a percent, where negative means backwards, and 0 will stop the motors [-100-100]
 * \param curve Curve ratio of motors as a percent, where negative means left, and 0 means straight [-100-100]
**/
void motorSteer(int power, int curve) {
    if (power == 0) {
        ev3_motor_stop(left_motor, true);
        ev3_motor_stop(right_motor, true);
    } else {
        float left_power = -power;
        float right_power = power;
        if(curve < 0) {
            left_power = -power-(curve*power/50);
        } else if (curve > 0) {
            right_power= power-(curve*power/50);
        }
        ev3_motor_set_power(left_motor, left_power);
        ev3_motor_set_power(right_motor, right_power);
    }
}
/**
 * \brief Drives robot at selected power and curve for a distance
 * \param distance The absolute distance in centimeters calculated as average between two motors
 * \param power Power of the motors as a percent, where negative means backwards, and 0 is not accepted [-100-100 !0]
**/
void drive(float distance, int power) {
    // ev3_motor_reset_counts(left_motor);
    // ev3_motor_reset_counts(right_motor);
    int backwards = 1;
    if (power < 0) {
        backwards = -1;
    }
    int leftstartcounts = ev3_motor_get_counts(left_motor);
    int rightstartcounts = ev3_motor_get_counts(right_motor);
    float wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts) / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts) / 2)) * ((3.1415926535 * 8.1) / 360);
    float error = 0, lasterror = 0, integral = 0;
    while (abs(wheelDistance) < distance) {
        wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts) / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts) / 2)) * ((3.1415926535 * 8.1) / 360);
        error = ((ev3_motor_get_counts(left_motor)-leftstartcounts)+(ev3_motor_get_counts(right_motor)-rightstartcounts));
        integral = error + integral * 0.5;
        float curve2 = (2*error + 0.0*integral + 10*(error-lasterror)) * backwards;
        motorSteer(power,curve2);
        lasterror = error;
        tslp_tsk(2);
    }
    motorSteer(0, 0);
    tslp_tsk(100);
}
/**
 * \brief follows right wall
 * \param distance The absolute distance in centimeters calculated as average between two motors
 * \param power Power of the motors as a percent where 0 is not accepted [1-100]
**/
void wallFollow(float distance, int power) {
    // ev3_motor_reset_counts(left_motor);
    // ev3_motor_reset_counts(right_motor);
    int leftstartcounts = ev3_motor_get_counts(left_motor);
    int rightstartcounts = ev3_motor_get_counts(right_motor);
    float wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts) / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts) / 2)) * ((3.1415926535 * 8.1) / 360);
    motorSteer(power, 5);
    while (abs(wheelDistance) < distance) {
        wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts) / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts) / 2)) * ((3.1415926535 * 8.1) / 360);
        tslp_tsk(5);
    }
    motorSteer(0, 0);
    tslp_tsk(100);
}
/**
 * \brief Drives robot following a line at a selected power for a distance, turning at the end if needed, and reading cars
 * \param distance The absolute distance in centimeters calculated as average between two motors
 * \param power Power of the motors as a percent, where negative means backwards, and 0 is not accepted [-100-100 !0]
 * \param turn_dir Turn selection where NONE means no turn [LEFT, NONE, RIGHT]
 * \param line_detect Sensors used to detect line where CENTER means both sensors and NONE is no line detection [NONE, LEFT, CENTER, RIGHT]
 * \param readCarLeft The parking spot that the robot will detect where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions. NONE means no readcar [0-11, NONE]
 * \param readCarRight The parking spot that the robot will detect where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions. NONE means no readcar [0-11, NONE]
 * \param pidA To run another PID after detecting line [true, false]
 * \exception turn_dir, line_detect, readCarLeft, readCarRight, and pidA do not apply when line_detect is NONE
**/
void PID(float distance, int power, int turn_dir, int line_detect, int readCarLeft, int readCarRight, int pidA) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    tslp_tsk(100);
    // line follow
    float wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.141592653 * 8.1) / 360);
    float error = 0, lasterror = 0, integral = 0;
    float targetdistance = distance;
    if (line_detect != NONE) {
        targetdistance = distance-15;
    }
    while (wheelDistance < targetdistance) {
        wheelDistance = (-ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2) * ((3.141592653 * 8.1) / 360);
        error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
        // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
        integral = error + integral * 0.5;
        float curve = 0.1 * error + 0.01 * integral + 2.5 * (error - lasterror);
        motorSteer(power,curve);
        lasterror = error;
        tslp_tsk(2);
    }
    if (line_detect == NONE) {
        motorSteer(0, 0);
    } else {
        // detect line
        int sansar1;
        int sansar2;
        if (line_detect == LEFT) {
            sansar1 = color_sensor2;
            sansar2 = color_sensor2;
        } else if (line_detect == RIGHT) {
            sansar1 = color_sensor3;
            sansar2 = color_sensor3;
        } else if (line_detect == CENTER) {
            sansar1 = color_sensor2;
            sansar2 = color_sensor3;
        } else {
            exit(127);
        }
        error = 0, lasterror = 0;
        while (ev3_color_sensor_get_reflect(sansar1) > 10 || ev3_color_sensor_get_reflect(sansar2) > 10) {
            error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
            // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
            float curve = 0.2 * error + 2 * (error - lasterror);
            motorSteer(10,curve);
            lasterror = error;
            tslp_tsk(2);
        }
        motorSteer(0, 0);
        if (pidA) {
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(100);
            error = 0, lasterror = 0, wheelDistance = 0;
            while (wheelDistance < 13.5) {
                wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
                error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
                // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
                float curve = 0.2 * error + 2 * (error - lasterror);
                motorSteer(20,curve);
                lasterror = error;
                tslp_tsk(2);
            }
            motorSteer(0, 0);
        } else {
            drive(13.5, 20);
        }
        // detect cars
        int doturn = true;
        if(readCarLeft != NONE || readCarRight != NONE){
            readcar(readCarLeft, readCarRight);
            if (turn_dir == LEFT) {
                if(mapcarPositions[(int)floor(readCarLeft / 4)][readCarLeft % 4] == WALL){
                    doturn = false;
                }
                if(bayCars[0] == NONE && bayCars[1] == NONE && bayCars[2] == NONE){
                    doturn = false;
                }
            }
            if (turn_dir == RIGHT) {
                if(mapcarPositions[(int)floor(readCarRight / 4)][readCarRight % 4] == WALL){
                    doturn = false;
                }
                if(bayCars[0] == NONE && bayCars[1] == NONE && bayCars[2] == NONE){
                    doturn = false;
                }
            }
        }
        // turn
        if(doturn == true && turn_dir != NONE){
            tslp_tsk(100);
            turn(turn_dir);
        }
    }
    tslp_tsk(100);
}
/**
 * \brief Turns robot 90 degrees following lines
 * \param direction Turn direction [LEFT, RIGHT]
 * \param offsetSensors Whether to offset the sensors (to be used when there are contents in the bay) [true, false]
 */
void turn(int direction) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    tslp_tsk(100);
    switch (direction) {
        case LEFT:
            motorSteer(20, -100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, -100);
            while (ev3_color_sensor_get_reflect(color_sensor2) < 30) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor2) >= 15) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor2) < 15) {tslp_tsk(5);}
            motorSteer(0, 0);
            break;
        case RIGHT:
            motorSteer(20, 100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, 100);
            while (ev3_color_sensor_get_reflect(color_sensor3) < 30) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor3) >= 15) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor3) < 15) {tslp_tsk(5);}
            motorSteer(0, 0);
            break;
        default:
            exit(127);
            break;
    }
    tslp_tsk(100);
}

// debug
/**
 * \brief Displays sensor values of drive motors and color sensors on the EV3 LCD screen
**/
void displayvalues() {
    // declare variables
    char msg[100];
    int value;

    // wait for values to be refreshed
    tslp_tsk(3);

    // read motor counts
    value = ev3_motor_get_counts(left_motor);
    sprintf(msg, "L: %d   ", value);
    ev3_lcd_draw_string(msg, 10*0, 15*0);
    value = ev3_motor_get_counts(right_motor);
    sprintf(msg, "R: %d   ", value);
    ev3_lcd_draw_string(msg, 10*8, 15*0);

    // read sensor rgb1
    rgb_raw_t rgb1;
    bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    assert(val1);
    sprintf(msg, "RGB1:");
    sprintf(msg, "R: %d", rgb1.r);
    ev3_lcd_draw_string(msg, 10*0, 15*2.5);
    sprintf(msg, "G: %d", rgb1.g);
    ev3_lcd_draw_string(msg, 10*6, 15*2.5);
    sprintf(msg, "B: %d", rgb1.b);
    ev3_lcd_draw_string(msg, 10*12, 15*2.5);

    // read sensor rgb4
    rgb_raw_t rgb4;
    bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
    assert(val4);
    sprintf(msg, "RGB4:");
    ev3_lcd_draw_string(msg, 10*0, 15*4);
    sprintf(msg, "R: %d  ", rgb4.r);
    ev3_lcd_draw_string(msg, 10*0, 15*5);
    sprintf(msg, "G: %d  ", rgb4.g);
    ev3_lcd_draw_string(msg, 10*6, 15*5);
    sprintf(msg, "B: %d  ", rgb4.b);
    ev3_lcd_draw_string(msg, 10*12, 15*5);

    // read linefollow sensors
    sprintf(msg, "Light2 & Light3:");
    ev3_lcd_draw_string(msg, 10*0, 15*6.5);
    value = ev3_color_sensor_get_reflect(color_sensor2);
    sprintf(msg, "L: %d  ", value);
    ev3_lcd_draw_string(msg, 10*0, 15*7.5);
    value = ev3_color_sensor_get_reflect(color_sensor3);
    sprintf(msg, "L: %d  ", value);
    ev3_lcd_draw_string(msg, 10*7, 15*7.5);
}

// buttons
/**
 * \brief Handles buttons
**/
void button_clicked_handler(intptr_t button) {
    switch(button) {
    case BACK_BUTTON:
        if (!finished) {
            ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
            ev3_motor_stop(left_motor, false);
            ev3_motor_stop(right_motor, false);
            ev3_motor_stop(a_motor, false);
            ev3_motor_stop(d_motor, false);
            ev3_led_set_color(LED_RED);
            ev3_lcd_draw_string("Program  Stopped", 10, 60);
            exit(0);
        }
        break;
    }
}
/**
 * \brief Pauses a thread until the CENTER button is pressed on the EV3
**/
void waitforButton() {
    ev3_led_set_color(LED_ORANGE);
    while (!ev3_button_is_pressed(ENTER_BUTTON)) {}
    ev3_led_set_color(LED_GREEN);
    while (ev3_button_is_pressed(ENTER_BUTTON)) {}
}

// test
/**
 * \brief Test program
**/
void test() {
    // collectBatteries();
    // runParkingArea1();
    // deliverCarsToYellow();
    // detectWaitingCars();
    // collectWaitingCars(0);
    // runParkingArea2();
    // collectWaitingCars(1);
    // runParkingArea3();

    // moveDoor(LEFT);
    // waitforButton();
    // moveDoor(CENTER);
    // waitforButton();
    // moveDoor(RIGHT);
    // waitforButton();
    // moveDoor(LEFTFULL);
    // waitforButton();
    // moveDoor(CENTERFULLLEFT);
    // waitforButton();
    // moveDoor(CENTERFULLRIGHT);
    // waitforButton();
    // moveDoor(RIGHTFULL);
    // waitforButton();
    // resetDoor();

    // while (true) {
    //     turn(LEFT);
    //     waitforButton();
    // }
    // while (true) {
    //     turn(RIGHT);
    //     waitforButton();
    // }

    // drive(50, 20, 0);
    // waitforButton();
    // drive(50, 20, 5);
    // waitforButton();
    // drive(50, 20, 25);
    // waitforButton();
    // drive(50, 20, 50);
    // waitforButton();
    // drive(50, 20, 75);
    // waitforButton();
    // drive(50, 20, 100);

    // while (true) {
    //     turn(RIGHT);
    //     waitforButton();
    //     drive(10,10,0);
    //     waitforButton();
    //     turn(LEFT);
    //     waitforButton();
    //     drive(10,-10,0);
    //     waitforButton();
    // }

    // for(int i = 10;i < 30;i+= 10){
    //     for(int j = 0;j < 3;j++){
    //         drive(5,i,0);
    //         waitforButton();
    //         drive(7,i,0);
    //         waitforButton();
    //         drive(9,i,0);
    //         waitforButton();
    //         drive(11,i,0);
    //         waitforButton();
    //         drive(13,i,0);
    //         waitforButton();
    //         drive(15,i,0);
    //         waitforButton();
    //     }
    // }

    // motorSteer(40,0);
    // tslp_tsk(1000);
    // ev3_motor_steer(left_motor,right_motor,0,0);
    // waitforButton();
    // motorSteer(40,0);
    // tslp_tsk(1000);
    // ev3_motor_stop(left_motor,false);
    // ev3_motor_stop(right_motor,false);
    // waitforButton();
    // motorSteer(40,0);
    // tslp_tsk(1000);
    // ev3_motor_stop(left_motor,true);
    // ev3_motor_stop(right_motor,true);

 //   // detectWaitingCars();
//         // waitforButton();
    //   //  // PID(70, 30, RIGHT, CENTER, 7, 1);
//     // PID(0, 30, CENTER, CENTER, 3, RIGHT);
        //     // // PID(70, 30, RIGHT, CENTER, 3, RIGHT);

//      // // //    // drive(1000, 30, 10);
}

// what
/**
 * \brief Does things
**/
void alignWithWall() {
    /*
    AAAAAAAAAAAAAAAAAA I am a Steve I must find the diamonds and build an house.
    Wowowowowowowow I am building an house.
    Wowowowowowowow I have found the diamond YES.
    Yay I have found the diamond and build an house.
    ...
    Haha I am a crapper I will explode the diamond and an house.
    Ohno.
    Explosion sounds and pixelated explosions Boom Ha Muahaha.
    ...
    Oh no me house and diamond I am dead.

    Haha plot twist I am sp.

    DISCONNECTED
    */
}
