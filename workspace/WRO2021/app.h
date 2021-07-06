#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

#define NONE -1
#define RED 0
#define GREEN 1
#define BLUE 2
#define WALL 8
#define BATTERY 6
#define BATTERYx2 7
#define REDB 3
#define GREENB 4
#define BLUEB 5
#define LEFT 0
#define CENTER 1
#define RIGHT 2
#define LEFTFULL 3
#define RIGHTFULL 4

#define PURPLE 1000
#define F 100

void init();
void driveOutBase();
void runAll();
void readcar(int parkingspotleft, int parkingspotright);
void deliverCarsToYellow();
void detectRoadCars();
void collectRoadCars(int set);
void moveDoor(int door);
void resetDoor();
int searchforcar(int cartype);
void doParkingSpot(int parkingspot);
void deliverBattery(int parkingspot);
void collectCar(int parkingspot);
void deliverCar(int parkingspot, int car);
void deliver(int bay, int location, int battery);
void collect(int bay);
void motorSteer(int power, int curve);
void drive(float distance, int power, int curve);
void PID(float distance, int power, int turn_dir, int turn_sensor, int readCarLeft, int readCarright, int pidA);
void turn(int direction);
void displayvalues();
void waitforButton();
void button_clicked_handler(intptr_t button);
void test();

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
