#include "zephyr/toolchain.h"
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zbus/zbus.h>
#include "camera_service.h"
#include "display.h"
#include "radar_types.h"


LOG_MODULE_REGISTER(semaforo);

K_MSGQ_DEFINE(display_queue, sizeof(struct display_msg), 10, 4);
K_MSGQ_DEFINE(sensor_queue, sizeof(struct data), 10, 4);
K_MSGQ_DEFINE(vehicle_queue, sizeof(struct data), 10, 4);

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_camera_evt);
ZBUS_CHAN_ADD_OBS(chan_camera_evt, msub_camera_evt, 3);

static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static const struct gpio_dt_spec sensor0 = GPIO_DT_SPEC_GET(DT_ALIAS(button0), gpios);
static const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(DT_ALIAS(button1), gpios);

extern void sensor_thread(void *, void *, void *);
extern void control_thread(void *, void *, void *);
extern void display_thread(void *, void *, void *);

K_THREAD_DEFINE(control_thread_id, 1024, control_thread, NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(sensor_thread_id, 1024, sensor_thread, NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(display_thread_id, 1024, display_thread, NULL, NULL, NULL, 4, 0, 0);

static struct gpio_callback sensor0_data;
static struct gpio_callback sensor1_data;



static void sensor0_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct data data;

	data.item.from = SENSOR_0;
	data.value.timestamp = k_uptime_get_32();

	k_msgq_put(&sensor_queue, &data, K_NO_WAIT);
}

static void sensor1_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct data data;

	data.item.from = SENSOR_1;
	data.value.timestamp = k_uptime_get_32();

	k_msgq_put(&sensor_queue, &data, K_NO_WAIT);
}


int main(void)
{
	int ret;
	uint8_t c;

	LOG_INF("Starting system...");

	if (!gpio_is_ready_dt(&sensor0) || !gpio_is_ready_dt(&sensor1)) {
		LOG_ERR("Error: Sensor GPIO devices are not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&sensor0, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Error: Failed to configure Sensor 0 pin (err: %d)\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&sensor1, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Error: Failed to configure Sensor 1 pin (err: %d)\n", ret);
		return 0;
	}

	gpio_init_callback(&sensor0_data, sensor0_cb, BIT(sensor0.pin));
	gpio_add_callback(sensor0.port, &sensor0_data);

	gpio_init_callback(&sensor1_data, sensor1_cb, BIT(sensor1.pin));
	gpio_add_callback(sensor1.port, &sensor1_data);

	ret = gpio_pin_interrupt_configure_dt(&sensor0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error configuring interrupt: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error configuring interrupt: %d\n", ret);
		return 0;
	}

	while (1) {
		if (!uart_poll_in(console_dev, &c)) {
			switch (c) {
			case 49:
				gpio_emul_input_set(sensor0.port, 5, 1);
				gpio_emul_input_set(sensor0.port, 5, 0);
				break;
			case 50:
				gpio_emul_input_set(sensor1.port, 6, 1);
				gpio_emul_input_set(sensor1.port, 6, 0);
				break;
			default:
				break;
			}
		}

		k_msleep(20);
	}

	return 0;
}


