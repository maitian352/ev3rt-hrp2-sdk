#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

#define NONE 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define WALL 9
#define BATTERY 7
#define BATTERYx2 8
#define REDB 4
#define GREENB 5
#define BLUEB 6
#define LEFT 0
#define CENTER 1
#define RIGHT 2

#define PURPLE 1000
#define F 100

void init();
int readcar(int sansar);
void detectRoadCars();
void deliver(int parkingspot, int car, int location, bool battery);
void openDoor(int car, int location);
void closeDoor();
int searchforcar(int cartype);
void doParkingSpot(int parkingspot);
void motorSteer(int power, int curve);
void drive(int distance, int power, int curve);
void PID(int distance, int power, int turn, int turn_sensor, int readCar);
void displayvalues();
void waitforButton();
void button_clicked_handler(intptr_t button);
void test();

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
