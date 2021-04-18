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
void open_carbay(int door);
void close_carbay(int door);
void readcar(int sansar);
void display_values();
void linePID(int distance);
void drive(int distance, int power, int curve);
void button_clicked_handler(intptr_t button);

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
