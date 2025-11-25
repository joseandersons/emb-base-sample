/* src/control_thread.c */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/printk.h>
#include "camera_service.h"
#include "display.h"
#include "radar_core.h"
#include "plate_validator.h"
#include "radar_types.h"
#include "radar_threads.h"


extern struct zbus_subscriber msub_camera_evt;
extern const struct zbus_channel chan_camera_evt;

void control_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct data vehicle;
    int err;
    const struct zbus_channel *chan;

    while (1) {
        err = k_msgq_get(&vehicle_queue, &vehicle, K_FOREVER);
        if (err) {
            continue;
        }

        enum tipo_veiculo_t tipo =
            (vehicle.item.type == LEVE) ? VEICULO_LEVE : VEICULO_PESADO;

        struct radar_result_t r = radar_analisar((int)vehicle.value.speed, tipo);

        struct display_msg d = {
            .velocidade = vehicle.value.speed,
            .tipo       = tipo
        };

        k_msgq_put(&display_queue, &d, K_NO_WAIT);

        if (r.status == STATUS_INFRACAO) {
            err = camera_api_capture(K_FOREVER);
            if (err) {
                printk("Could not init capture. Error: %d\n", err);
                continue;
            }

            struct msg_camera_evt rsp;

            err = zbus_sub_wait_msg(
                (const struct zbus_observer *)&msub_camera_evt,
                &chan,
                &rsp,
                K_FOREVER
            );

            if (err) {
                printk("ERROR: %d\n", err);
                continue;
            }

            if (rsp.type == MSG_CAMERA_EVT_TYPE_ERROR) {
                printk("Camera service unavailable. Error code %d\n", rsp.error_code);
                continue;
            }

            if (rsp.type == MSG_CAMERA_EVT_TYPE_DATA) {
                const char *plate = rsp.captured_data->plate;

                printk("Camera data: plate=%s, hash=%s\n",
                       plate, rsp.captured_data->hash);

                if (plate_is_valid(plate)) {
                    printk("Placa válida\n");
                } else {
                    printk("Placa inválida\n");
                }
            }
        }
    }
}
