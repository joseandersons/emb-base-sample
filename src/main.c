#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(semaforo);

static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static const struct gpio_dt_spec sensor0 = GPIO_DT_SPEC_GET(DT_ALIAS(button0), gpios);
static const struct gpio_dt_spec sensor1 = GPIO_DT_SPEC_GET(DT_ALIAS(button1), gpios);

static struct gpio_callback sensor0_data;
static struct gpio_callback sensor1_data;

static void sensor0_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	static int count = 0;

	LOG_INF("Test cb 1: %d\n", ++count);
}

static void sensor1_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	static int count = 0;

	LOG_INF("Test cb 2: %d\n", ++count);
}

void control_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	printk("Test\n");
}

int main(void)
{
	int ret;
	uint8_t c;

	if (!gpio_is_ready_dt(&sensor0) || !gpio_is_ready_dt(&sensor1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&sensor0, GPIO_INPUT);
	if (ret) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&sensor1, GPIO_INPUT);
	if (ret) {
		return 0;
	}

	gpio_init_callback(&sensor0_data, sensor0_cb, BIT(sensor0.pin));
	gpio_add_callback(sensor0.port, &sensor0_data);

	gpio_init_callback(&sensor1_data, sensor1_cb, BIT(sensor1.pin));
	gpio_add_callback(sensor1.port, &sensor1_data);

	ret = gpio_pin_interrupt_configure_dt(&sensor0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Erro ao configurar interrupção: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&sensor1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Erro ao configurar interrupção: %d\n", ret);
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

K_THREAD_DEFINE(my_tid, 1024, control_thread, NULL, NULL, NULL, 4, 0, 0);
