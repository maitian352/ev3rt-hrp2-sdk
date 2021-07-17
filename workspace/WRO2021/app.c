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
    0,
    -220,
};
/**
 * \brief Stores the current item in the bays [RED, GREEN, BLUE, REDB, GREENB, BLUEB, BATTERY, BATTERYx2]
 * \param bay Bay number [LEFT, CENTER, RIGHT]
**/
int bayCars[3] = {
    NONE, BATTERYx2, BATTERYx2
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
 * \brief How do we do this?
 * \param task 0 is the next task to do...
**/
int tasks[99] = {};

// main task
void main_task(intptr_t unused) {
    init();
    test();
    //collectBatteries();
    // runParkingArea1();
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
    if (ev3_battery_voltage_mV() < 7500) {
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
    ev3_motor_set_power(a_motor, 10);
    ev3_motor_set_power(d_motor, 100);
    tslp_tsk(1500);
    ev3_motor_set_power(a_motor, 0);
    ev3_motor_stop(d_motor, false);
    tslp_tsk(500);
    ev3_motor_rotate(d_motor, -810, 20, true);
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
    drive(18.2,10,5);
    tslp_tsk(100);
    moveDoor(RIGHTFULL);
    drive(4,10,5);
    tslp_tsk(100);
    moveDoor(CENTER);
    drive(2.9,10,5);
    tslp_tsk(100);
    moveDoor(RIGHTFULL);
    drive(4,10,5);
    tslp_tsk(100);
    moveDoor(CENTER);
    drive(2.9,10,5);
    tslp_tsk(100);
    moveDoor(RIGHT);
    drive(4,10,5);
    tslp_tsk(100);
    moveDoor(CENTER);
    drive(2.9,10,5);
    tslp_tsk(100);
    moveDoor(RIGHT);
    drive(4.2,10,2);
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
    drive(14, 15, 0);
    motorSteer(10, 0);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) > 25) {tslp_tsk(5);}
    motorSteer(0, 0);
    tslp_tsk(100);
    drive(12, 10, 0);
    tslp_tsk(100);
    motorSteer(10, 100);
    tslp_tsk(200);
    motorSteer(5, 100);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) > 20) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) < 20) {tslp_tsk(5);}
    motorSteer(0,0);
    PID(30, 30, NONE, CENTER, NONE, NONE, true);
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
            if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[0][i] != RED){
                turn(RIGHT);
                doParkingSpot(i);
                turn(LEFT);
            }
            else if(searchforcar(mapPositions[0][i],LEFT) != NONE){
                turn(RIGHT);
                doParkingSpot(i);
                turn(LEFT);
            }
        }
        else if(mapcarPositions[0][i] == mapPositions[0][i]){
            
        }
        else if(searchforcar(NONE,LEFT) != NONE){
            turn(RIGHT);
            doParkingSpot(i);
            turn(LEFT);
        }
        if(mapcarPositions[1][i] == WALL){
            
        }
        else if(mapcarPositions[1][i] == NONE){
            if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[1][i] != RED){
                turn(LEFT);
                doParkingSpot(i + 4);
                turn(RIGHT);
            }
            else if(searchforcar(mapPositions[1][i],LEFT) != NONE){
                turn(LEFT);
                doParkingSpot(i + 4);
                turn(RIGHT);
            }
        }
        else if(mapcarPositions[1][i] == mapPositions[1][i]){
            
        }
        else if(searchforcar(NONE,LEFT) != NONE){
            turn(LEFT);
            doParkingSpot(i + 4);
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
    if(mapcarPositions[1][0] == BLUE && batteryPositions[1][0] == NONE){
        moveDoor(searchforcar(NONE, LEFT));
        drive(10, 10, 0);
        moveDoor(searchforcar(NONE, LEFT) + 3);
        drive(9, 10, 0);
        resetDoor();
        PID(24, 25, LEFT, CENTER, NONE, NONE, true);
    }
    else {
        if (mapcarPositions[1][0] == WALL) {
            turn(RIGHT);
        } else {
            turn(LEFT);
        }
        if(mapcarPositions[1][1] == GREEN && batteryPositions[1][1] == NONE){
            PID(25,30,RIGHT,CENTER,NONE,NONE,true);
            moveDoor(searchforcar(NONE, LEFT));
            drive(10, 10, 0);
            moveDoor(searchforcar(NONE, LEFT) + 3);
            drive(9, 10, 0);
            resetDoor();
            PID(24, 30, RIGHT, CENTER, NONE, NONE, true);
            PID(25, 30, LEFT, CENTER, NONE, NONE, true);
            turn(LEFT);
        }
        else if(mapcarPositions[1][2] == NONE && batteryPositions[1][2] == NONE){
            direction = 1;
            PID(58,30,RIGHT,CENTER,NONE,NONE,true);
            drive(20, 20, 0);
            PID(24, 30, LEFT, CENTER, NONE, NONE, true);
            PID(25, 30, RIGHT, CENTER,NONE, NONE, true);
            turn(RIGHT);
        } else if (mapcarPositions[1][3] == GREEN && batteryPositions[1][3] == NONE) {
            direction = 1;
            PID(88, 35, RIGHT, CENTER, NONE, NONE, true);
            moveDoor(searchforcar(NONE, LEFT));
            drive(10, 10, 0);
            moveDoor(searchforcar(NONE, LEFT) + 3);
            drive(9, 10, 0);
            resetDoor();
            PID(24, 30, RIGHT, CENTER, NONE, NONE, true);
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
            readcar(i+8, NONE);
            if(mapcarPositions[2][i] == WALL){
                
            }
            else if(mapcarPositions[2][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED){
                    turn(LEFT);
                    doParkingSpot(i + 8);
                    turn(RIGHT);
                }
                else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                    turn(LEFT);
                    doParkingSpot(i + 8);
                    turn(RIGHT);
                }
            }
            else if(mapcarPositions[2][i] == mapPositions[2][i]){
                
            }
            else if(searchforcar(NONE,LEFT) != NONE){
                turn(LEFT);
                doParkingSpot(i + 8);
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
            readcar(NONE, i+8);
            if(mapcarPositions[2][i] == WALL){
                
            }
            else if(mapcarPositions[2][i] == NONE){
                if((searchforcar(BATTERY,LEFT) != NONE || searchforcar(BATTERYx2,LEFT) != NONE) && mapPositions[2][i] != RED){
                    turn(RIGHT);
                    doParkingSpot(i + 8);
                    turn(LEFT);
                }
                else if(searchforcar(mapPositions[2][i],LEFT) != NONE){
                    turn(RIGHT);
                    doParkingSpot(i + 8);
                    turn(LEFT);
                }
            }
            else if(mapcarPositions[2][i] == mapPositions[2][i]){
                
            }
            else if(searchforcar(NONE,LEFT) != NONE){
                turn(RIGHT);
                doParkingSpot(i + 8);
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
}
/**
 * \brief Delivers two cars to yellow areas
**/
void deliverCarsToYellow() {
    PID(48, 30, RIGHT, RIGHT, NONE, NONE, false);
    PID(30,20,NONE,NONE,-1,NONE,false);
    waitforButton();
    ev3_motor_rotate(left_motor,230,20,false);
    ev3_motor_rotate(right_motor,230,20,true);
    deliver()
    ev3_motor_rotate(left_motor,200,20,false);
    ev3_motor_rotate(right_motor,200,20,true);
    PID(28,20,NONE,NONE,-1,1,false);
}
/**
 * \brief Does second run through of the parking area
**/
void runParkingArea2() {
    end();
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
    ev3_motor_set_power(a_motor, -30);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Lowers the doors
 * \exception Will raise the sensors
 * \exception Does not reset the doors
**/
void lowerDoor() {
    ev3_motor_set_power(a_motor, 30);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Lowers the sensors
 * \exception Will raise the doors
**/
void raiseSensors() {
    ev3_motor_set_power(a_motor, 30);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}
/**
 * \brief Raises the sensors
 * \exception Will lower the doors
 * \exception Will open all bays and cannot back up or make turns without running "raiseSensors()"
**/
void lowerSensors() {
    ev3_motor_set_power(a_motor, -30);
    tslp_tsk(750);
    ev3_motor_set_power(a_motor, 0);
}

// submodules
/**
 * \brief Reads and records the color of parkingspots
 * \param sensor sensor [1, 4]
 * \param parkingspotleft The id of the parkingspot [0-11] (NONE for nothing)
 * \param parkingspotright The id of the parkingspot [0-11] (NONE for nothing)
**/
void readcar( int parkingspotleft, int parkingspotright) {
    lowerSensors();
    if (parkingspotleft != NONE) {
        char msg[100];
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
        sprintf(msg, "Left: %d %d %d %d", cardetected,rgb1.r,rgb1.g,rgb1.b);
        ev3_lcd_draw_string(msg, 10*4, 15*2.5);
    }
    if (parkingspotright != NONE) {
        char msg[100];
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
        sprintf(msg, "Right: %d %d %d %d", cardetected,rgb4.r,rgb4.g,rgb4.b);
        ev3_lcd_draw_string(msg, 10*0, 15*4);
    }
    raiseSensors();
}
/**
 * \brief Detects and writes down values of all 6 cars on the road
**/
void detectRoadCars(){
    char msg[100];
    rgb_raw_t rgb45;
    int red = 2;
    int green = 2;
    int blue = 2;
    rgb45.r = 100;
    rgb45.g = 100;
    rgb45.b = 100;
    int cardetected = NONE;
    bool_t val4;
    for(int i = 0;i < 500;i++){
        drive(11,20,0);
        val4 = 0;
        
        val4 = val4 + ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb45);
        //assert(val4);
        while (val4 < 2) {
            tslp_tsk(100);
            val4 = val4 + ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb45);
            sprintf(msg, "%d %d %d       %d    ",rgb45.r,rgb45.g,rgb45.b,val4);
            ev3_lcd_draw_string(msg, 10*0, 15*val4);
        }
        if(rgb45.r > 55){
            cardetected = RED;
        }
        else if(rgb45.b > 55){
            cardetected = BLUE;
        }
        else{
            cardetected = GREEN;
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
}
/**
 * \brief Collects 3 cars
 * \param set Set 0 or set 1 where set 0 is the first 3 cars and set 1 is the last 3 cars.
 * \exception Set 0 must start at the intersection facing towards the parking garage.
 * \exception Set 1 must start at the intersection facing away from the parking garage.
**/
void collectRoadCars(int set){
    switch (set)
    {
    case 0:
        ev3_motor_reset_counts(left_motor);
        ev3_motor_reset_counts(right_motor);
        moveDoor(LEFT);
        motorSteer(20, -100);
        while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 <160) {tslp_tsk(5);}
        motorSteer(5, -100);
        while (ev3_color_sensor_get_reflect(color_sensor2) < 40) {tslp_tsk(5);}
        while (ev3_color_sensor_get_reflect(color_sensor2) > 35) {tslp_tsk(5);}
        ev3_motor_stop(left_motor, true);
        ev3_motor_stop(right_motor, true);
        tslp_tsk(20);
        drive(15.5,10,0);
        ev3_motor_rotate(d_motor, -460, 50, true);
        raiseDoor();
        drive(11.5,10,0);
        ev3_motor_rotate(d_motor, -440, 50, true);
        lowerDoor();
        ev3_motor_rotate(d_motor, 600, 50, true);
        raiseDoor();
        drive(7,10,0);
        resetDoor();
        lowerDoor();
        drive(2, -10, 0);
        break;
    case 1:
        turn(LEFT);
        turn(LEFT);
        moveDoor(LEFT);
        ev3_motor_reset_counts(left_motor);
        ev3_motor_reset_counts(right_motor);
        tslp_tsk(20);
        motorSteer(20, -100);
        while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 <160) {tslp_tsk(5);}
        motorSteer(5, -100);
        while (ev3_color_sensor_get_reflect(color_sensor2) < 40) {tslp_tsk(5);}
        while (ev3_color_sensor_get_reflect(color_sensor2) > 35) {tslp_tsk(5);}
        ev3_motor_stop(left_motor, true);
        ev3_motor_stop(right_motor, true);
        moveDoor(RIGHT);
        drive(40, 20, 0);
        tslp_tsk(50);
        drive(6.5,10,0);
        ev3_motor_rotate(d_motor, 500, 50, true);
        raiseDoor();
        drive(10,10,0);
        ev3_motor_rotate(d_motor, 440, 50, true);
        lowerDoor();
        ev3_motor_rotate(d_motor, -640, 50, true);
        raiseDoor();
        ev3_motor_rotate(left_motor, -440, 10, true);
        resetDoor();
        lowerDoor();
        break;
    default:
        exit(127);
        break;
    }
}
/**
 * \brief Returns whether or not we have a car of __ type in our bay 
 * \param cartype Type of thingy to look for [RED, GREEN, BLUE, REDB, GREENB, BLUEB, BATTERY, BATTERYx2]
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
 * \brief Does a parkingspot
 * \param parkingspot The current parking spot where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions [0-11]
 * \exception Function name is bad
**/
void doParkingSpot(int parkingspot) {
    if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == NONE){
        if(mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] != RED){
            deliverBattery(parkingspot);
        }
        deliverCar(parkingspot,mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4]);
    }
    else if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == WALL){

    }
    else{
        collectCar(parkingspot);
        if(mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] == NONE){
            if(mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4] != RED){
                deliverBattery(parkingspot);
            }
            deliverCar(parkingspot,mapPositions[(int)floor(parkingspot / 4)][parkingspot % 4]);
        }
    }
    resetDoor();
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
    drive(18, 15, 0);
    if (battery && bayCars[bay] == BATTERYx2) {
        drive(6.5, -10, 0);
        drive(2, 10, 0);
        if(bay == RIGHT){
            closeDoors(RIGHT);
        }
        else{
            closeDoors(LEFT);
        }
        drive(16.75, -15, 0);
        drive(1.75, 10, 0);
        moveDoor(location);
        drive(2, 10, 0);
    } else {
        drive(19, -15, 0);
        drive(1, 10, 0);
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
    drive(10, 10, 0);
    moveDoor(bay + 3);
    drive(9, 10, 0);
    resetDoor();
    drive(20, -15, 0);
    drive(1, 10, 0);
}

// drive functions
/**
 * \brief Starts motors at selected power and curve
 * \param power Power of the motors as a percent, where negative means backwards, and 0 means nothing [-100-100]
 * \param curve Curve ratio of motors as a percent, where negative means left, and 0 means straight [-100-100]
**/
void motorSteer(int power, int curve) {
    float left_power = -power;
    float right_power = power;
    if(curve < 0) {
        left_power = -power-(curve*power/50);
    } else if (curve > 0) {
        right_power= power-(curve*power/50);
    }
    ev3_motor_set_power(right_motor, right_power);
    ev3_motor_set_power(left_motor, left_power);
}
/**
 * \brief Drives robot at selected power and curve for a distance
 * \param distance The absolute distance in centimeters calculated as average between two motors
 * \param power Power of the motors as a percent, where negative means backwards, and 0 means nothing [-100-100]
 * \param curve Curve ratio of motors as a percent, where negative means left, and 0 means straight [-100-100]
**/
void drive(float distance, int power, int curve) {
    float ratioleft, ratioright;
    if(curve == 0){
        ratioleft = 1;
        ratioright = 1;
    }
    else if(curve < 0){
        ratioleft = -50/curve;
        ratioright = 1;
    }
    else{
        ratioleft = 1;
        ratioright = 1+curve/20;
    }
    int backwards = 1;
    if (power < 0) {
        backwards = -1;
    }
    // ev3_motor_reset_counts(left_motor);
    // ev3_motor_reset_counts(right_motor);
    int leftstartcounts = ev3_motor_get_counts(left_motor);
    int rightstartcounts = ev3_motor_get_counts(right_motor);
    float wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts)*ratioleft / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts)*ratioright / 2)) * ((3.1415926535 * 8.1) / 360);
    float error = 0, lasterror = 0, integral = 0;
    while (abs(wheelDistance) < distance) {
        wheelDistance = (abs((ev3_motor_get_counts(left_motor)-leftstartcounts)*ratioleft / 2) + abs((ev3_motor_get_counts(right_motor)-rightstartcounts)*ratioright / 2)) * ((3.1415926535 * 8.1) / 360);
        error = ((ev3_motor_get_counts(left_motor)-leftstartcounts)*ratioleft+(ev3_motor_get_counts(right_motor)-rightstartcounts)*ratioright);
        integral = error + integral * 0.5;
        float curve = (1*error + 0.0*integral + 10*(error-lasterror)) * backwards;
        motorSteer(power,curve);
        // motorSteer(power,0);
        lasterror = error;
        tslp_tsk(2);
    }
    ev3_motor_stop(left_motor, true);
    ev3_motor_stop(right_motor, true);
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    tslp_tsk(200);
}
/**
 * \brief Drives robot following a line at a selected power for a distance, turning at the end if needed, and reading cars
 * \param distance The absolute distance in centimeters calculated as average between two motors
 * \param power Power of the motors as a percent, where negative means backwards, and 0 means nothing [-100-100]
 * \param turn_dir Turn selection where NONE means no turn [LEFT, NONE, RIGHT]
 * \param line_detect Sensors used to detect line where CENTER means both sensors and NONE is no line detection [NONE, LEFT, CENTER, RIGHT]
 * \param readCarLeft The parking spot that the robot will detect where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions. NONE means no readcar [0-11, NONE]
 * \param readCarRight The parking spot that the robot will detect where 0 is [0][0], 3 is [0][3], and 4 is [1][0] in mapPositions. NONE means no readcar [0-11, NONE]
 * \param pidA To run another PID after detecting line [true, false]
 * \exception \b turn and \b readCar do not apply when \b line_detect is NONE
**/
void PID(float distance, int power, int turn_dir, int line_detect, int readCarLeft, int readCarRight, int pidA) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
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
        ev3_motor_stop(left_motor, true);
        ev3_motor_stop(right_motor, true);
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
        tslp_tsk(100);
        error = 0, lasterror = 0;
        while (ev3_color_sensor_get_reflect(sansar1) > 10 || ev3_color_sensor_get_reflect(sansar2) > 10) {
            error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
            // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
            float curve = 0.2 * error + 2 * (error - lasterror);
            motorSteer(10,curve);
            lasterror = error;
            tslp_tsk(2);
        }
        ev3_motor_stop(left_motor, true);
        ev3_motor_stop(right_motor, true);
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
            ev3_motor_stop(left_motor, true);
            ev3_motor_stop(right_motor, true);
        } else {
            drive(13.5, 15, 0);
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
            tslp_tsk(100);
        }
    }
}
/**
 * \brief Turns robot 90 degrees following lines
 * \param direction Turn direction [LEFT, RIGHT]
 * \param offsetSensors Whether to offset the sensors (to be used when there are contents in the bay) [true, false]
 */
void turn(int direction) {
    switch (direction) {
        case LEFT:
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(200);
            motorSteer(20, -100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, -100);
            while (ev3_color_sensor_get_reflect(color_sensor2) < 30) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor2) >= 15) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor2) < 15) {tslp_tsk(5);}
            ev3_motor_stop(left_motor, true);
            ev3_motor_stop(right_motor, true);
            break;
        case RIGHT:
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(200);
            motorSteer(20, 100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, 100);
            while (ev3_color_sensor_get_reflect(color_sensor3) < 30) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor3) >= 15) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor3) < 15) {tslp_tsk(5);}
            ev3_motor_stop(left_motor, true);
            ev3_motor_stop(right_motor, true);
            break;
        default:
            exit(127);
            break;
    }
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
            ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
            ev3_motor_stop(left_motor, false);
            ev3_motor_stop(right_motor, false);
            ev3_motor_stop(a_motor, false);
            ev3_motor_stop(d_motor, false);
            ev3_led_set_color(LED_RED);
            ev3_lcd_draw_string("Program  Stopped", 10, 60);
            exit(0);
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
    // PID(49, 40, NONE, CENTER, NONE, NONE, true);
    // waitforButton();
    // runParkingArea1();
    //deliverCarsToYellow();

    //collectBatteries();
    // PID(45, 30, NONE, CENTER, NONE, NONE, true);
    // runParkingArea1();
    deliverCarsToYellow();

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

    // drive(10, 10, 0);
    // waitforButton();
    // drive(10, 50, 0);

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

 //   // detectRoadCars();
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
