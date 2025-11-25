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

LOG_MODULE_REGISTER(semaforo);

enum STATE {
	IDLE = 0,
	READING,
	DONE
};

enum TYPE {
	LEVE = 0,
	PESADO
};

enum FROM {
	SENSOR_0 = 0,
	SENSOR_1,
	TIMER
};

struct data {
	union item {
		enum FROM from;
		enum TYPE type;
	} item;

	union value {
		double speed;
		uint32_t timestamp;
	} value;

	int eixos;
};

K_MSGQ_DEFINE(sensor_queue, sizeof(struct data), 10, 4);
K_MSGQ_DEFINE(vehicle_queue, sizeof(struct data), 10, 4);

static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static const struct gpio_dt_spec sensor0 = GPIO_DT_SPEC_GET(DT_ALIAS(button0), gpios);
static const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(DT_ALIAS(button1), gpios);

static struct gpio_callback sensor0_data;
static struct gpio_callback sensor1_data;

static enum STATE radar_state = IDLE;

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

void timeout_vehicle_handler(struct k_timer *dummy)
{
	ARG_UNUSED(dummy);
	struct data vehicle_stop;

	vehicle_stop.item.from = TIMER;

	k_msgq_put(&sensor_queue, &vehicle_stop, K_NO_WAIT);
}

K_TIMER_DEFINE(timeout_vehicle, timeout_vehicle_handler, NULL);

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

	while (true) {

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
				if (speed_done) {
					break;
				}
				uint32_t dt = recv_data.value.timestamp - init_timestamp;
				vehicle.value.speed = (CONFIG_RADAR_SENSOR_DISTANCE_MM / dt) * 3.6;
				speed_done = true;
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

void control_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	struct data vehicle;
	int err;

	while (true) {
		err = k_msgq_get(&vehicle_queue, &vehicle, K_FOREVER);
		if (err) {
			continue;
		}

		if (vehicle.value.speed > CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH &&
		    vehicle.item.type == LEVE) {
			LOG_INF("Infracao, veiculo LEVE: %d km/h", (int)(vehicle.value.speed));
		} else if (vehicle.value.speed > CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH &&
			   vehicle.item.type == PESADO) {
			LOG_INF("Infracao, veiculo PESADO: %d km/h", (int)(vehicle.value.speed));
		}
	}
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

K_THREAD_DEFINE(control_thread_id, 1024, control_thread, NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(sensor_thread_id, 1024, sensor_thread, NULL, NULL, NULL, 4, 0, 0);
