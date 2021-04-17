#include "target_test.h"

#ifndef _TEST_FILE_H_
#define _TEST_FILE_H_

void display_values();
void linePID(int distance);
static void button_clicked_handler(intptr_t button);

#define Red = 1
#define Green = 2
#define Blue = 3
#define collision = false

extern void	main_task(intptr_t exinf);
#endif /* TOPPERS_MACRO_ONLY */
