/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/printk.h>
#include "camera_service.h"
#include "plate_validator.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_camera_evt);

ZBUS_CHAN_ADD_OBS(chan_camera_evt, msub_camera_evt, 3);

int main(void)
{
    int err;
    const struct zbus_channel *chan;

    printk("=== Teste com validação de placa ===\n");

    while (1) {

        k_msleep(1000);

        /* Solicita captura da câmera */
        err = camera_api_capture(K_FOREVER);
        if (err) {
            printk("Could not init capture. Error: %d\n", err);
            continue;
        }

        struct msg_camera_evt rsp;

        /* Espera evento da câmera via ZBus */
        err = zbus_sub_wait_msg(&msub_camera_evt, &chan, &rsp, K_FOREVER);
        if (err) {
            printk("ERROR: %d\n", err);
            continue;
        }

        /* Evento de erro da câmera */
        if (rsp.type == MSG_CAMERA_EVT_TYPE_ERROR) {
            printk("Camera service unavailable. Error code %d\n", rsp.error_code);
            continue;
        }

        /* Se chegou aqui, houve captura real */
        if (rsp.type == MSG_CAMERA_EVT_TYPE_DATA) {

            const char *plate = rsp.captured_data->plate;

            printk("Camera data: plate=%s, hash=%s\n",
                   plate,
                   rsp.captured_data->hash);

            /* 🔥 Validação da placa Mercosul */
            if (plate_is_valid(plate)) {
                printk("✔ Placa válida\n");
            } else {
                printk("❌ Placa inválida\n");
            }
        }
    }

    return 0;
}
