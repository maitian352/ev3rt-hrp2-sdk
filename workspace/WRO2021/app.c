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
 * \param bay The bay number (LEFT, CENTER, RIGHT) [0-2]
 * \param location End location for bay (LEFT, CENTER, RIGHT) [0-2]
 * \param motor Rack or door motor [0-1]
 * \exception Bay LEFT cannot be delivered to Location RIGHT, and Bay RIGHT cannot be delivered to Location LEFT
 * \exception Opening door to CENTER may open LEFT location as well
**/
int doorLocations[6] = {
    430,
    0,
    -430,
    -80,
    430,
    80,
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

void main_task(intptr_t unused) {
    init();
    //driveOutBase();
    // runAll();
    test();

}

/**
 * \brief Initializes the robot
**/
void init() {
    // Register button handlers
    ev3_button_set_on_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
    
    // Configure motors
    ev3_motor_config(left_motor, MEDIUM_MOTOR);
    ev3_motor_config(right_motor, MEDIUM_MOTOR);
    ev3_motor_config(a_motor, MEDIUM_MOTOR);
    ev3_motor_config(d_motor, MEDIUM_MOTOR);
    
    // Configure sensors
    ev3_sensor_config(color_sensor1, HT_NXT_COLOR_SENSOR);
    ev3_sensor_config(color_sensor2, COLOR_SENSOR);
    ev3_sensor_config(color_sensor3, COLOR_SENSOR);
    ev3_sensor_config(color_sensor4, HT_NXT_COLOR_SENSOR);
    
    // Set up sensors
    //ev3_color_sensor_get_reflect(color_sensor2);
    //ev3_color_sensor_get_reflect(color_sensor3);
    //rgb_raw_t rgb1;
    //bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    //assert(val1);
    //rgb_raw_t rgb4;
    //bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
    //assert(val4);

    // Configure brick
    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    // reset doors and sensors
    ev3_motor_set_power(a_motor, 100);
    ev3_motor_set_power(d_motor, 100);
    tslp_tsk(2000);
    ev3_motor_stop(a_motor, false);
    ev3_motor_stop(d_motor, false);
    tslp_tsk(500);
    ev3_motor_rotate(d_motor, -820, 20, true);
    ev3_motor_reset_counts(d_motor);
    ev3_motor_reset_counts(a_motor);

    // wait for button press
    ev3_lcd_draw_string("Press OK to run", 14, 45);
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
void driveOutBase(){
    ev3_motor_rotate(d_motor, 300, 80, false);
    drive(14,20,5);
    drive(4,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 300, -50, true);
    drive(1.3,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 300, 50, true);
    drive(2.05,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 300, -50, true);
    drive(1.3,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 300, 50, true);
    drive(2.05,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 780, -50, true);
    drive(1.3,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 780, 50, true);
    drive(2.1,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 780, -50, true);
    drive(1.6,10,0);
    tslp_tsk(100);
    ev3_motor_rotate(d_motor, 780, 50, true);
    tslp_tsk(100);
    bayCars[CENTER] = BATTERYx2;
    bayCars[RIGHT] = BATTERYx2;
    resetDoor();
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    ev3_motor_rotate(right_motor, 180, 20, true);
    waitforButton();
    tslp_tsk(100);
    drive(18, 15, 0);
    waitforButton();
    motorSteer(10, 0);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) > 25) {tslp_tsk(5);}
    motorSteer(0, 0);
    waitforButton();
    tslp_tsk(100);
    drive(11, 10, 0);
    tslp_tsk(100);
    waitforButton();
    motorSteer(10, 100);
    tslp_tsk(200);
    motorSteer(5, 100);
    while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) > 20) {tslp_tsk(5);}
    while (ev3_color_sensor_get_reflect(color_sensor3) < 20) {tslp_tsk(5);}
    motorSteer(0,0);
    waitforButton();
    PID(5, 10, CENTER, CENTER, -1,-1,false);
}
/**
 * \brief Runs all things (Maitian put proper description)
**/
void runAll(){
    for(int i = 3;i >= 0;i--){
        //readcar(i + 4, i);
        readcar(i +4,i);
        waitforButton();
        if(mapcarPositions[0][i] == WALL){
            drive(100, -100, 100);
        }
        else{
            turn(RIGHT);
            doParkingSpot(i);
            turn(LEFT);
        }
        //drive(9, 10, 0);
        //turn(LEFT);
        //drive(11, 10, 0);
        //turn(LEFT);
        //drive(1, 10, 0);
        if(mapcarPositions[1][i] == WALL){

        }
        else{
            turn(LEFT);
            doParkingSpot(i + 4);
            turn(RIGHT);
        }
        if(i == 0){
            turn(LEFT);
            drive(11, 10, 0);
        }
        else{
            PID(27,30,LEFT,CENTER,2,1,false);
        }
    }
    if(mapcarPositions[1][0] == NONE){
        PID(0,30,CENTER,CENTER,2,1,false);
        for(int i = 0;i < 4;i++){
            if(mapcarPositions[2][i] == NONE){

            }
            else{
                doParkingSpot(i + 8);
            }
            turn(LEFT);
            PID(30,30,RIGHT,CENTER,-1,1,false);
        }
    }
    else if(mapcarPositions[1][1] == NONE){
        PID(30,30,LEFT,CENTER,2,1,false);
    }
    else if(mapcarPositions[1][2] == NONE){
        PID(62,30,LEFT,CENTER,2,1,false);
    }
    PID(34,30,LEFT,CENTER,2,1,false);
}
/**
 * \brief Drives out of base and collects batteries
**/
void deliverCarsToYellow(){
    PID(7,20,RIGHT,RIGHT,-1,1,false);
    PID(30,20,NONE,NONE,-1,1,false);
    waitforButton();
    if(searchforcar(BLUEB) != -1){
        PID(28 + 6 * searchforcar(BLUEB),20,NONE,NONE,-1,1,false);
    }
    else if(searchforcar(GREENB) != -1){
        PID(28 + 6 * searchforcar(GREENB),20,NONE,NONE,-1,1,false);
    }
    ev3_motor_rotate(left_motor,230,20,false);
    ev3_motor_rotate(right_motor,230,20,true);
    drive(15,20,0);
    drive(-15,-20,0);
    ev3_motor_rotate(left_motor,200,20,false);
    ev3_motor_rotate(right_motor,200,20,true);
    if(searchforcar(BLUEB) != -1){
        PID(28 + 6 * searchforcar(BLUEB),20,NONE,NONE,-1,1,false);
    }
    else if(searchforcar(GREENB) != -1){
        PID(28 + 6 * searchforcar(GREENB),20,NONE,NONE,-1,1,false);
    }
}

/**
 * \brief Moves the selected door number to the center
 * \param door Which door that is moved to the center [LEFT, RIGHT]
 * \exception CENTER will do the same thing as resetDoor() and will block the CENTER bay
 * \exception Cannot back up or make turns without running "closeDoors()"
**/
void moveDoor(int door) {
    ev3_motor_rotate(d_motor, doorLocations[door]-ev3_motor_get_counts(d_motor), 100, true);
}
/**
 * \brief Resets the to neutral position
 * \exception Cannot back up or make turns without running "closeDoors()"
**/
void resetDoor() {
    ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 100, true);
}
/**
 * \brief Raises the doors
 * \exception Will lower the sensors
 * \exception Will open all bays and cannot back up or make turns without running "closeDoors()"
 * \exception Does not reset the doors
**/
void raiseDoor() {
    ev3_motor_set_power(a_motor, -100);
    tslp_tsk(1000);
    ev3_motor_stop(a_motor, false);
}
/**
 * \brief Lowers the doors
 * \exception Will raise the sensors
 * \exception Does not reset the doors
**/
void lowerDoor() {
    ev3_motor_set_power(a_motor, 20);
    tslp_tsk(1000);
    ev3_motor_stop(a_motor, false);
}
/**
 * \brief Lowers the sensors
 * \exception Will raise the doors
**/
void raiseSensors() {
    ev3_motor_set_power(a_motor, 20);
    tslp_tsk(1000);
    ev3_motor_stop(a_motor, false);
}
/**
 * \brief Raises the sensors
 * \exception Will lower the doors
 * \exception Will open all bays and cannot back up or make turns without running "raiseSensors()"
**/
void lowerSensors() {
    ev3_motor_set_power(a_motor, -100);
    tslp_tsk(1000);
    ev3_motor_stop(a_motor, false);
}
/**
 * \brief Closes all bays
 * \exception Cannot line follow or turn without running "resetDoor()"
 * \exception \b DOES \b NOT \b GUARANTEE \b THAT \b CARS \b WILL \b NOT \b LEAVE \b BAYS
**/
void closeDoors() {
    ev3_motor_rotate(d_motor, 660-ev3_motor_get_counts(d_motor), 100, true);
}
/**
 * \brief Returns the color (NONE, RED, GREEN, BLUE, WALL) of the selected sensor (1 or 4)
 * \param sensor sensor [1, 4]
 * \param parkingspotleft The id of the parkingspot [0-11] (NONE for nothing)
 * \param parkingspotright The id of the parkingspot [0-11] (NONE for nothing)
**/
void readcar( int parkingspotleft, int parkingspotright) {
    lowerSensors();
    tslp_tsk(1000);
    if (parkingspotleft != NONE) {
        char msg[100];
        rgb_raw_t rgb1;
        bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        assert(val1);
        val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        assert(val1);
        tslp_tsk(5);
        int cardetected = NONE;
        if(rgb1.r > 100 && rgb1.g > 60){
            cardetected = WALL;
        }
        else if(rgb1.r < 5 && rgb1.g < 5 && rgb1.b < 5){
            cardetected = NONE;
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == RED){
            if(rgb1.b > 40){
                cardetected = BLUE;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == GREEN){
            if(rgb1.r > 30){
                cardetected = RED;
            }
            else{
                cardetected = BLUE;
            }
        }
        else if(mapPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] == BLUE){
            if(rgb1.r > 30){
                cardetected = RED;
            }
            else{
                cardetected = GREEN;
            }
        }
        mapcarPositions[(int)floor(parkingspotleft / 4)][parkingspotleft % 4] = cardetected;
        sprintf(msg, "Left: %d", cardetected);
        ev3_lcd_draw_string(msg, 10*4, 15*2.5);
    }
    if (parkingspotright != NONE) {
        char msg[100];
        rgb_raw_t rgb4;
        bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
        assert(val4);
        val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
        assert(val4);
        tslp_tsk(5);
        int cardetected = NONE;
        if(rgb4.r > 100 && rgb4.g > 60){
            cardetected = WALL;
        }
        else if(rgb4.r < 5 && rgb4.g < 5 && rgb4.b < 5){
            cardetected = NONE;
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == RED){
            if(rgb4.b > 40){
                cardetected = BLUE;
            }
            else{
                cardetected = GREEN;
            }
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == GREEN){
            if(rgb4.r > 30){
                cardetected = RED;
            }
            else{
                cardetected = BLUE;
            }
        }
        else if(mapPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] == BLUE){
            if(rgb4.r > 30){
                cardetected = RED;
            }
            else{
                cardetected = GREEN;
            }
        }
        mapcarPositions[(int)floor(parkingspotright / 4)][parkingspotright % 4] = cardetected;
        sprintf(msg, "Right: %d %d %d %d", cardetected,rgb4.r,rgb4.g,rgb4.b);
        ev3_lcd_draw_string(msg, 10*0, 15*4);
        tslp_tsk(500);
    }
    displayvalues();
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
        waitforButton();
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
        ev3_motor_steer(left_motor, right_motor, 0, 0);
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
        closeDoors();
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
        ev3_motor_steer(left_motor, right_motor, 0, 0);
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
        closeDoors();
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
int searchforcar(int cartype) {
    if(bayCars[0] == cartype){
        return 0;
    }
    if(bayCars[1] == cartype){
        return 1;
    }
    if(bayCars[2] == cartype){
        return 2;
    }
    return -1;
}
/**
 * \brief Does everything related to moving bays
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
    if(searchforcar(BATTERY) != -1){
        deliver(searchforcar(BATTERY),CENTERFULL,true);
        batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
        bayCars[searchforcar(BATTERY)] = NONE;
    }
    else if(searchforcar(BATTERYx2) != -1){
        deliver(searchforcar(BATTERYx2),CENTERFULL,true);
        batteryPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = BATTERY;
        bayCars[searchforcar(BATTERYx2)] = NONE;
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
        collect(CENTER);
        bayCars[1] = mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4];
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = NONE;
    }
    else if(bayCars[2] == NONE){
        collect(RIGHT);
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
    if(searchforcar(car) != -1){
        deliver(searchforcar(car),CENTER,false);
        mapcarPositions[(int)floor(parkingspot / 4)][parkingspot % 4] = car;
        bayCars[searchforcar(car)] = NONE;
    }
}
/**
 * \brief Delivers specified bay to a location
 * \param bay The bay number that is to be delivered [LEFT, CENTER, RIGHT] ILLIGAL 666
 * \param location The location where the selected bay is to be delivered [LEFT, CENTER, RIGHT]
 * \param battery Deliver batteries or not [true, false]
 * \exception Bay LEFT cannot be delivered to Location RIGHT, and Bay RIGHT cannot be delivered to Location LEFT
 * \exception Opening door to CENTER will open LEFT location as well
**/
void deliver(int bay, int location, int battery) {
    moveDoor(location);
    drive(18, 10, 0);
    tslp_tsk(100);
    if (battery && bayCars[bay] == BATTERYx2) {
        drive(6.5, -10, 0);
        tslp_tsk(100);
        drive(1, 10, 0);
        closeDoors();
        drive(14.5, -10, 0);
        tslp_tsk(100);
        drive(2, 10, 0);
    } else {
        drive(18, -10, 0);
        tslp_tsk(10);
        resetDoor();
        tslp_tsk(10);
    }
}
/**
 * \brief Collects a car
 * \param bay The bay number that is to be delivered [LEFT, CENTER, RIGHT]
 * \exception PURPLE is not 1000 in this function
**/
void collect(int bay) {
    moveDoor(LEFT);
    drive(8.2, 10, 0);
    moveDoor(LEFTFULL);
    drive(9, 10, 0);
    closeDoors();
    drive(17, -10, 0);
}
/**
 * \brief Starts motors at selected power and curve
 * \param power Power of the motors as a percent, where negative means backwards, and 0 means nothing [-100-100]
 * \param curve Curve ratio of motors as a percent, where negative means left, and 0 means straight [-100-100]
**/
void motorSteer(int power, int curve) {
    if(curve == 0){
        ev3_motor_set_power(left_motor,-power);
        ev3_motor_set_power(right_motor,power);
    }
    else if(curve < 0){
        ev3_motor_set_power(left_motor,-power - curve * power / 50);
        ev3_motor_set_power(right_motor,power);
    }
    else{
        ev3_motor_set_power(right_motor,power - curve * power / 50);
        ev3_motor_set_power(left_motor,-power);
    }
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
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    tslp_tsk(50);
    float wheelDistance = (-ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2) * ((3.141592653 * 8.1) / 360);
    float error = 0, lasterror = 0, integral = 0;
    while (abs(wheelDistance) < distance) {
        wheelDistance = (-ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2) * ((3.141592653 * 8.1) / 360);
        error = (ev3_motor_get_counts(left_motor)*ratioleft+ev3_motor_get_counts(right_motor)*ratioright);
        integral = error + integral * 0.5;
        float curve = 0.05*error + 0.001*integral + 0.2*lasterror;
        motorSteer(power,curve);
        lasterror = error;
        tslp_tsk(2);
    }
    ev3_motor_steer(left_motor, right_motor, 0, 0);
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
    while (wheelDistance < distance) {
        wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
        error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
        // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
        integral = error + integral * 0.5;
        float curve = 0.1 * error + 0.01 * integral + 1 * (error - lasterror);
        motorSteer(power,curve);
        lasterror = error;
        tslp_tsk(2);
    }
    if (line_detect == NONE) {
        ev3_motor_steer(left_motor, right_motor, 0, 0);
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
            ev3_motor_steer(left_motor, right_motor, 0, 0);
        if (pidA) {
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(100);
            error = 0, lasterror = 0, wheelDistance = 0;
            while (wheelDistance < 12.5) {
                wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
                error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
                // float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
                float curve = 0.2 * error + 5 * (error - lasterror);
                motorSteer(15,curve);
                lasterror = error;
                tslp_tsk(2);
            }
            ev3_motor_steer(left_motor, right_motor, 0, 0);
            // drive(12, 15, 0);
        } else {
            drive(12, 15, 0);
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
    //
/**
 * \brief Turns robot 90 degrees following lines
 * \param direction Turn direction [LEFT, RIGHT]
 * \param offsetSensors Whether to offset the sensors (to be used when there are contents in the bay) [true, false]
 */
void turn(int direction) {
    switch (direction) {
        case LEFT:
            ev3_motor_rotate(d_motor, 200-ev3_motor_get_counts(d_motor), 100, false);
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(200);
            motorSteer(20, -100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, -100);
            while (ev3_color_sensor_get_reflect(color_sensor2) < 50) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor2) > 30) {tslp_tsk(5);}
            ev3_motor_steer(left_motor, right_motor, 0, 0);
            resetDoor();
            break;
        case RIGHT:
            ev3_motor_rotate(d_motor, -160-ev3_motor_get_counts(d_motor), 100, false);
            ev3_motor_reset_counts(left_motor);
            ev3_motor_reset_counts(right_motor);
            tslp_tsk(200);
            motorSteer(20, 100);
            while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
            motorSteer(10, 100);
            while (ev3_color_sensor_get_reflect(color_sensor3) < 50) {tslp_tsk(5);}
            while (ev3_color_sensor_get_reflect(color_sensor3) > 30) {tslp_tsk(5);}
            ev3_motor_steer(left_motor, right_motor, 0, 0);
            resetDoor();
            break;
        default:
            exit(127);
            break;
    }
}


























    // void turn(int direction, int slow) {
    //     switch (direction) {
    //         case LEFT:
    //             if(slow == true){
    //                 ev3_motor_reset_counts(left_motor);
    //                 ev3_motor_reset_counts(right_motor);
    //                 tslp_tsk(20);
    //                 motorSteer(5, -100);
    //                 while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
    //                 motorSteer(5, -100);
    //                 //ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 50, false);
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) < 40) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) > 20) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) < 35) {}
    //                 ev3_motor_steer(left_motor, right_motor, 0, 0);
    //                 tslp_tsk(20);
    //             }
    //             else{
    //                 ev3_motor_reset_counts(left_motor);
    //                 ev3_motor_reset_counts(right_motor);
    //                 tslp_tsk(20);
    //                 motorSteer(20, -100);
    //                 while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
    //                 motorSteer(5, -100);
    //                 //ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 50, false);
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) < 40) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) > 20) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor2) < 35) {}
    //                 ev3_motor_steer(left_motor, right_motor, 0, 0);
    //                 tslp_tsk(20);
    //             }
    //             break;
    //         case RIGHT:
    //             if(slow == true){
    //                 ev3_motor_reset_counts(left_motor);
    //                 ev3_motor_reset_counts(right_motor);
    //                 tslp_tsk(20);
    //                 motorSteer(5, 100);
    //                 while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
    //                 motorSteer(5, 100);
    //                 //ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 50, false);
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) > 25) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) < 35) {}
    //                 ev3_motor_steer(left_motor, right_motor, 0, 0);
    //             }
    //             else{
    //                 ev3_motor_reset_counts(left_motor);
    //                 ev3_motor_reset_counts(right_motor);
    //                 tslp_tsk(20);
    //                 motorSteer(20, 100);
    //                 while((abs(ev3_motor_get_counts(left_motor)) + abs(ev3_motor_get_counts(right_motor)))/2 < 180) {tslp_tsk(5);}
    //                 motorSteer(5, 100);
    //                 //ev3_motor_rotate(d_motor, -ev3_motor_get_counts(d_motor), 50, false);
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) < 40) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) > 25) {}
    //                 while (ev3_color_sensor_get_reflect(color_sensor3) < 35) {}
    //                 ev3_motor_steer(left_motor, right_motor, 0, 0);
    //             }
    //             break;
    //         default:
    //             exit(127);
    //             break;
    //     }
    // }


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

/**
 * \brief Test program
**/
void test() {
 //   // detectRoadCars();
//         // waitforButton();
    //   //  // PID(70, 30, RIGHT, CENTER, 7, 1);
//     // PID(0, 30, CENTER, CENTER, 3, RIGHT);
        //     // // PID(70, 30, RIGHT, CENTER, 3, RIGHT);

PID(35, 40, NONE, CENTER, NONE, NONE, true);
                waitforButton();
   runAll();

//      // // //    // drive(1000, 30, 10);
}

void alignWithWall() {
    ev3_motor_set_power(a_motor,1000);
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
