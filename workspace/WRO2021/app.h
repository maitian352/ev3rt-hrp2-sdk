#include "target_test.h"

#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

#define NONE 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define WALL 4
#define LEFT 1
#define RIGHT 4

void init();
void open_carbay(int door, int blocking);
void close_carbay(int door, int blocking);
void readcar(int sansar);
void display_values();
void motor_steer(int power, int curve);
void drive(int distance, int power, int curve);
void drivePID(int distance, int power);
void driveOutBase();
int readcar(int sensor, int parkingspot);
void detectRoadCars();
void openDoor(int bay, int location);
void closeDoor();
int searchforcar(int cartype);
void doParkingSpot(int parkingspot);
void deliverBattery(int parkingspot);
void collectCar(int parkingspot);
void deliverCar(int parkingspot, int car);
void deliver(int bay, int location, int battery);
void collect(int bay);
void motorSteer(int power, int curve);
void drive(float distance, int power, int curve);
void PID(float distance, int power, int turn, int turn_sensor, int readCar, int side);
void displayvalues();
void waitforButton();
void button_clicked_handler(intptr_t button);

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
