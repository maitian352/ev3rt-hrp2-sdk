#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

#define NONE 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define WALL 6
#define BATTERY 4
#define BATTERYx2 5
#define LEFT 1
#define CENTER 2
#define RIGHT 3

#define PURPLE 1000
#define F 7

void init();
int readcar(int sansar);
void detectRoadCars();
void deliver(int parkingspot, int car, int location);
void openDoor(int car, int location);
void closeDoor();
void motorSteer(int power, int curve);
void drive(int distance, int power, int curve);
void PID(int distance, int power, int turn, int turn_sensor, int readCar);
void displayvalues();
void waitforButton();
void button_clicked_handler(intptr_t button);
void test();

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
