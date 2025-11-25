#ifndef RADAR_THREADS_H
#define RADAR_THREADS_H

#include <zephyr/kernel.h>
#include "radar_types.h"

extern struct k_msgq display_queue;
extern struct k_msgq sensor_queue;
extern struct k_msgq vehicle_queue;

void display_thread(void *arg1, void *arg2, void *arg3);
void sensor_thread(void *arg1, void *arg2, void *arg3);
void control_thread(void *arg1, void *arg2, void *arg3);

#endif
