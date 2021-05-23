#include <stdlib.h>
#include <stdio.h>
#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

// define motors and sensors
const int color_sensor1 = EV3_PORT_1, color_sensor2 = EV3_PORT_2, color_sensor3 = EV3_PORT_3, color_sensor4 = EV3_PORT_4, left_motor = EV3_PORT_B, right_motor = EV3_PORT_C, a_motor = EV3_PORT_A, d_motor = EV3_PORT_D;

// define variables
rgb_raw_t rgb1;
rgb_raw_t rgb4;
int doorLocations[3][3][2] = {
    {
        {0, 340},
        {450, 340},
        {999, 999}
    },
    {
        {-420, -90},
        {0, 340},
        {450, 90}
    },
    {
        {999, 999},
        {-420, 350},
        {0, 120}
    }
};
int rackPositions[2] = {
    CENTER, CENTER
};
int bayPositions[3] = {
    NONE, NONE, NONE, NONE
};
int roadcarPositions[6] = {
    NONE, NONE, NONE, NONE, NONE, NONE
};
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
int mapPositions[3][4] = {
    {
        BLUE,BLUE,RED,GREEN
    },
    {
        GREEN,RED,GREEN,BLUE
    },
    {
        RED,RED,GREEN,BLUE
    },
};

void main_task(intptr_t unused) {
    init();
    ///*
    test();
    //*/
}

void init() {
    // Register button handlers
    ev3_button_set_on_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
    
    // Configure motors
    ev3_motor_config(left_motor, MEDIUM_MOTOR);
    ev3_motor_config(right_motor, MEDIUM_MOTOR);
    ev3_motor_config(a_motor, MEDIUM_MOTOR);
    ev3_motor_config(d_motor, MEDIUM_MOTOR);
    
    // Configure sensors
    //ev3_sensor_config(color_sensor1, HT_NXT_COLOR_SENSOR);
    ev3_sensor_config(color_sensor2, COLOR_SENSOR);
    ev3_sensor_config(color_sensor3, COLOR_SENSOR);
    //ev3_sensor_config(color_sensor4, HT_NXT_COLOR_SENSOR);
    
    // Set up sensors
    ev3_color_sensor_get_reflect(color_sensor2);
    ev3_color_sensor_get_reflect(color_sensor3);
    bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    assert(val1);
    bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
    assert(val4);

    // Configure brick
    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    // reset bays
    ev3_motor_set_power(a_motor, 50);
    ev3_motor_set_power(d_motor, -50);
    tslp_tsk(1000);
    ev3_motor_set_power(a_motor, 0);
    ev3_motor_set_power(d_motor, 0);
    tslp_tsk(500);
    ev3_motor_rotate(a_motor, -480, 20, false);
    ev3_motor_rotate(d_motor, 190, 20, true);
    ev3_motor_reset_counts(a_motor);
    ev3_motor_reset_counts(d_motor);

    // wait for button press
    ev3_lcd_draw_string("Press OK to run", 14, 45);
    ev3_lcd_fill_rect(77, 87, 24, 20, EV3_LCD_BLACK);
    ev3_lcd_fill_rect(79, 89, 20, 1, EV3_LCD_WHITE);
    ev3_lcd_draw_string("OK", 79, 90);
    while (!ev3_button_is_pressed(ENTER_BUTTON));
    while (ev3_button_is_pressed(ENTER_BUTTON));
    ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
}

int readcar(int sansar) {
    int yeet;
    switch (sansar)
    {
    case 1:
        ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
        if(rgb1.r > 20 && rgb1.g > 20){
        yeet = WALL;
    }
    else if(rgb1.r > 9 && rgb1.g > 5 && rgb1.b > 5){
        yeet = RED;
    }
    else if(rgb1.r > 5 && rgb1.g > 5 && rgb1.b > 10){
        yeet = BLUE;
    }
    else if(rgb1.r < 5 && rgb1.g < 5 && rgb1.b < 5){
        yeet = NONE;
    }
    else{
        yeet = GREEN;
    }
        break;
    case 2:
        ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
        if(rgb4.r > 25 && rgb4.g > 20 && rgb4.b > 20){
        yeet = WALL;
    }
    else if(rgb4.r > 5 && rgb4.g > 5){
        yeet = RED;
    }
    else if(rgb4.g > 5 && rgb4.b > 10){
        yeet = BLUE;
    }
    else if(rgb4.r < 5 && rgb4.g < 5 && rgb4.b < 5){
        yeet = NONE;
    }
    else{
        yeet = GREEN;
    }
    default:
        break;
    }
    return yeet;
}
void detectRoadCars(){
    int red = 2;
    int green = 2;
    int blue = 2;
    for(int i = 0;i < 5;i++){
        drive(11,20,0);
        roadcarPositions[i] = readcar(2);
        if(roadcarPositions[i] == RED){
            red--;
        }
        if(roadcarPositions[i] == GREEN){
            green--;
        }
        if(roadcarPositions[i] == BLUE){
            blue--;
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

void deliver(int car, int location) {

}
void openDoor(int car, int location) {
    ev3_motor_rotate(a_motor, (doorLocations[car][location][0]-ev3_motor_get_counts(a_motor)), 20, false);
    ev3_motor_rotate(d_motor, (doorLocations[car][location][1]-ev3_motor_get_counts(d_motor)), 20, true);
}
void closeDoor() {
    ev3_motor_rotate(a_motor, (-ev3_motor_get_counts(a_motor)), 20, false);
    ev3_motor_rotate(d_motor, (-ev3_motor_get_counts(d_motor)), 20, true);
}

void motorSteer(int power, int curve) {
    if(curve == 0){
        ev3_motor_set_power(right_motor,power);
        ev3_motor_set_power(left_motor,-power);
    }
    else if(curve < 0){
        ev3_motor_set_power(right_motor,power);
        ev3_motor_set_power(left_motor,-power - curve * power / 50);
    }
    else{
        ev3_motor_set_power(right_motor,power - curve * power / 50);
        ev3_motor_set_power(left_motor,-power);
    }
}
void drive(int distance, int power, int curve) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    float wheelDistance = (-ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2) * ((3.1415926535 * 8.1) / 360);
    while (abs(wheelDistance) < distance) {
        wheelDistance = (-ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2) * ((3.1415926535 * 8.1) / 360);
        motorSteer(power,curve);
        tslp_tsk(1);
    }
    ev3_motor_steer(left_motor, right_motor, 0, 0);
}
void PID(int distance, int power, int turn, int turn_sensor, int readCar) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    float wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
    float lasterror = 0, integral = 0;
    while (wheelDistance < distance) {
        wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
        //float error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
        float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
        integral = error + integral * 0.5;
        //float curve = 0.06 * error + 0.001 * integral + 0.11 * (error - lasterror);
        float curve = 0.1 * error + 0 * integral + 0 * (error - lasterror);
        motorSteer(power,curve);
        lasterror = error;
        tslp_tsk(1);
    }
    ev3_motor_steer(left_motor, right_motor, 0, 0);
    if (turn != CENTER) {
        int sansar1;
        int sansar2;
        if (turn_sensor == LEFT) {
            sansar1 = color_sensor2;
            sansar2 = color_sensor2;
        } else if (turn_sensor == RIGHT) {
            sansar1 = color_sensor3;
            sansar2 = color_sensor3;
        } else if (turn_sensor == CENTER) {
            sansar1 = color_sensor2;
            sansar2 = color_sensor3;
        } else {
            exit(127);
        }
        while (ev3_color_sensor_get_reflect(sansar1) > 15 && ev3_color_sensor_get_reflect(sansar2) > 15) {
            wheelDistance = (abs(ev3_motor_get_counts(left_motor) / 2) + abs(ev3_motor_get_counts(right_motor) / 2)) * ((3.1415926535 * 8.1) / 360);
            //float error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
            float error = 50 - ev3_color_sensor_get_reflect(color_sensor3);
            integral = error + integral * 0.5;
            float curve = 0.13 * error + 0 * integral + 0 * (error - lasterror);
            motorSteer(10,curve);
        }
        ev3_motor_steer(left_motor, right_motor, 0, 0);
        if(readCar != 0){
            mapcarPositions[(int)floor(readCar / 4)][readCar % 4] = readcar(4);
        }
        drive(15, 10, 0);
        tslp_tsk(200);
        switch (turn)
        {
            case LEFT:
                ev3_motor_set_power(left_motor,20);
                ev3_motor_set_power(right_motor,3);
                //motorSteer(10, -50);
                tslp_tsk(800);
                //motorSteer(5, -50);
                ev3_motor_set_power(left_motor,5);
                while (ev3_color_sensor_get_reflect(color_sensor2) > 15) {}
                while (ev3_color_sensor_get_reflect(color_sensor2) < 25) {}
                ev3_motor_steer(left_motor, right_motor, 0, 0);
                break;
            case RIGHT:
                ev3_motor_set_power(left_motor,-3);
                ev3_motor_set_power(right_motor,-20);
                //motorSteer(10, -50);
                tslp_tsk(800);
                //motorSteer(5, -50);
                ev3_motor_set_power(right_motor,-5);
                while (ev3_color_sensor_get_reflect(color_sensor3) > 15) {}
                while (ev3_color_sensor_get_reflect(color_sensor3) < 25) {}
                ev3_motor_steer(left_motor, right_motor, 0, 0);
                break;
            default:
                exit(127);
                break;
        }
    }
}

void driveOutBase(){
    ev3_motor_rotate(a_motor, 220, 20, true);
    ev3_motor_rotate(d_motor, 340, 20, true);
    drive(26,10,5);
    drive(5.75,10,5);
    ev3_motor_rotate(d_motor, 460, -20, true);
    drive(4,10,5);
    ev3_motor_rotate(d_motor, 460, 20, true);
    drive(1.75,10,5);
    ev3_motor_rotate(d_motor, 460, -20, true);
    drive(6.5,10,5);
    ev3_motor_rotate(a_motor, 220, -20, true);
    ev3_motor_rotate(d_motor, 120, 20, true);
    motorSteer(10,-100);
    tslp_tsk(1000);
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    while (ev3_color_sensor_get_reflect(color_sensor2) > 15) {}
    while (ev3_color_sensor_get_reflect(color_sensor2) < 25) {}
    motorSteer(0,0);
    PID(15,10,RIGHT,CENTER,0);
}

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

void button_clicked_handler(intptr_t button) {
    switch(button) {
    case BACK_BUTTON:
            ev3_motor_stop(left_motor, false);
            ev3_motor_stop(right_motor, false);
            ev3_motor_stop(a_motor, false);
            ev3_motor_stop(d_motor, false);
        exit(0);
        break;
    }
}
void waitforButton() {
    ev3_led_set_color(LED_OFF);
    while (!ev3_button_is_pressed(ENTER_BUTTON)) {}
    ev3_led_set_color(GREEN);
    while (ev3_button_is_pressed(ENTER_BUTTON)) {}
}

void test() {
    //driveOutBase();
    PID(72,40,RIGHT,CENTER,3);
    PID(14,20,CENTER,CENTER,0);
    /*
    if(mapcarPositions[0][3] == NONE){
        openDoor(RIGHT, RIGHT);
        drive(4,10,0);
        closeDoor();
        drive(12,-10,0);
        openDoor(RIGHT, RIGHT);
        drive(4,10,0);
        closeDoor();
    }
    else{*/
        ev3_motor_rotate(a_motor, 460, 20, true);
        openDoor(LEFT, LEFT);
        tslp_tsk(5000);
        drive(4,10,0);
        tslp_tsk(5000);
        closeDoor();
        ev3_motor_rotate(a_motor, 460, -20, true);
        tslp_tsk(500);
        drive(12,-10,0);
        tslp_tsk(500);
        ev3_motor_rotate(a_motor, 460, 20, true);
        openDoor(LEFT, LEFT);
        tslp_tsk(500);
        drive(4,10,0);
        tslp_tsk(500);
        closeDoor();
        ev3_motor_rotate(a_motor, 460, -20, true);
        tslp_tsk(500);
        openDoor(RIGHT, RIGHT);
        tslp_tsk(500);
        drive(4,-10,0);
        tslp_tsk(500);
        closeDoor();
        tslp_tsk(500);
        drive(12,-10,0);
        tslp_tsk(500);
        openDoor(RIGHT, RIGHT);
        tslp_tsk(500);
        drive(4,10,0);
        tslp_tsk(500);
        closeDoor();
    //}
}
