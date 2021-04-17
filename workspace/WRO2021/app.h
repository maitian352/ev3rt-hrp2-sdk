#include "target_test.h"

#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

void display_values();
void linePID(int distance);
void button_clicked_handler(intptr_t button);

#define RED = 1
#define GREEN = 2
#define BLUE = 3

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
