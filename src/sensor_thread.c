#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include "radar_types.h"
#include "radar_threads.h"


enum STATE radar_state = IDLE;

void timeout_vehicle_handler(struct k_timer *dummy);

K_TIMER_DEFINE(timeout_vehicle, timeout_vehicle_handler, NULL);

void timeout_vehicle_handler(struct k_timer *dummy)
{
    ARG_UNUSED(dummy);
    struct data vehicle_stop;
    vehicle_stop.item.from = TIMER;
    k_msgq_put(&sensor_queue, &vehicle_stop, K_NO_WAIT);
}

void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct data vehicle;
    struct data recv_data;

    bool speed_done = false;
    int count_sensor_0 = 0;
    int init_timestamp = 0;
    int err;

    while (1) {
        if (radar_state != DONE) {
            err = k_msgq_get(&sensor_queue, &recv_data, K_MSEC(20));
            if (err) {
                continue;
            }
        }

        switch (radar_state) {
        case IDLE:
            if (recv_data.item.from == SENSOR_0) {
                count_sensor_0 = 1;
                init_timestamp = recv_data.value.timestamp;
                radar_state = READING;
                k_timer_start(&timeout_vehicle, K_MSEC(700), K_NO_WAIT);
            }
            break;
        case READING:
            if (recv_data.item.from == SENSOR_0) {
                count_sensor_0++;
                k_timer_stop(&timeout_vehicle);
                k_timer_start(&timeout_vehicle, K_MSEC(700), K_NO_WAIT);
            } else if (recv_data.item.from == SENSOR_1) {
                if (!speed_done) {
                    uint32_t dt = recv_data.value.timestamp - init_timestamp;
                    vehicle.value.speed =
                        (CONFIG_RADAR_SENSOR_DISTANCE_MM / dt) * 3.6;
                    speed_done = true;
                }
            } else if (recv_data.item.from == TIMER) {
                radar_state = DONE;
            }
            break;
        case DONE:
            if (!speed_done) {
                goto end;
            }

            if (count_sensor_0 == 2) {
                vehicle.item.type = LEVE;
            } else if (count_sensor_0 > 2) {
                vehicle.item.type = PESADO;
            } else {
                goto end;
            }

            vehicle.eixos = count_sensor_0;
            k_msgq_put(&vehicle_queue, &vehicle, K_NO_WAIT);

        end:
            radar_state = IDLE;
            count_sensor_0 = 0;
            speed_done = false;
            break;
        default:
            break;
        }
    }
}
