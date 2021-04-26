#include <stdlib.h>
#include <stdio.h>
#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>

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
int bayPositions[3] = {
    NONE, NONE, NONE, NONE
}
int roadcarPositions[6] = {
    NONE, NONE, NONE, NONE, NONE, NONE
}
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

// do has discord?
// how to collision?

void main_task(intptr_t unused) {
    init();
    /*
    while (true) {
        char msg[100];
        sprintf(msg, "a: %d", read_car(LEFT));
        ev3_lcd_draw_string(msg, 0, 0);
    }
    */
}

void init() {
    // Register button handlers
    ev3_button_set_on_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
    
    // Configure motors
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(a_motor, MEDIUM_MOTOR);
    ev3_motor_config(d_motor, MEDIUM_MOTOR);
    
    // Configure sensors
    ev3_sensor_config(color_sensor1, HT_NXT_COLOR_SENSOR);
    ev3_sensor_config(color_sensor2, COLOR_SENSOR);
    ev3_sensor_config(color_sensor3, COLOR_SENSOR);
    ev3_sensor_config(color_sensor4, HT_NXT_COLOR_SENSOR);
    
    // Set up sensors
    ev3_color_sensor_get_reflect(color_sensor2);
    ev3_color_sensor_get_reflect(color_sensor3);
    bool_t val1 = ht_nxt_color_sensor_measure_rgb(color_sensor1, &rgb1);
    assert(val1);
    bool_t val4 = ht_nxt_color_sensor_measure_rgb(color_sensor4, &rgb4);
    assert(val4);

    // Configure brick
    ev3_lcd_set_font(EV3_FONT_MEDIUM);

    // wait for button press
    ev3_lcd_draw_string("Press OK to run", 14, 45);
    ev3_lcd_fill_rect(77, 87, 24, 20, EV3_LCD_BLACK);
    ev3_lcd_fill_rect(79, 89, 20, 1, EV3_LCD_WHITE);
    ev3_lcd_draw_string("OK", 79, 90);
    while (1) {
        if (ev3_button_is_pressed(ENTER_BUTTON)) {
            while (ev3_button_is_pressed(ENTER_BUTTON));
            break;
        }
    }
    ev3_lcd_fill_rect(0, 0, 178, 128, EV3_LCD_WHITE);
}

void read_car(int sansar) {
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

void display_values() {
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

void motorSteer(int power, int curve) {
    if(curve == 0){
        ev3_motor_set_power(left_motor,-power);
        ev3_motor_set_power(right_motor,power);
    }
    else if(curve < 0){
        ev3_motor_set_power(left_motor,-power + curve * power / 50);
        ev3_motor_set_power(right_motor,power);
    }
    else{
        ev3_motor_set_power(left_motor,-power);
        ev3_motor_set_power(right_motor,power - curve * power / 50);
    }
    return;
}
void drive(int distance, int power, int curve) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    float wheelDistance = -ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2;
    while (wheelDistance < distance) {
        wheelDistance = -ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2;
        motorSteer(power,curve);
        tslp_tsk(1);
    }
    ev3_motor_steer(left_motor, right_motor, 0, 0);
    return;
}
void drivePID(int distance, int power) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);
    float wheelDistance = -ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2;
    float lasterror = 0, integral = 0;
    while (wheelDistance < distance) {
        wheelDistance = -ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2;
        wheelDistance = -ev3_motor_get_counts(left_motor) / 2 + ev3_motor_get_counts(right_motor) / 2;
        float error = ev3_color_sensor_get_reflect(color_sensor2) - ev3_color_sensor_get_reflect(color_sensor3);
        integral = error + integral * 0.5;
        float curve = 0.04 * error + 0.5 * integral + 0.25 * (error - lasterror);
        motorSteer(power,curve);
        tslp_tsk(1);
    }
    ev3_motor_steer(left_motor, right_motor, 0, 0);
    return;
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
