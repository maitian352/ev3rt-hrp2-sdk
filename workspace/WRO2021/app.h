#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

#define NONE 0
#define RED 1
#define GREEN 2
#define BLUE 3
#define WALL 4
#define LEFT 0
#define CENTER 1
#define RIGHT 2

#define PURPLE 1000
#define F 7

void init();
void readcar(int sansar);
void display_values();
void motorSteer(int power, int curve);
void drive(int distance, int power, int curve);
void PID(int distance, int power, int turn, int turn_sensor, int readCar);
void openDoor(int car, int location);
void closeDoor();
void test();
void button_clicked_handler(intptr_t button);
void waitforButton();

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
