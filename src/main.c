#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

#define LED_BLINK_INTERVAL_MS CONFIG_LED_DELAY_MS
LOG_MODULE_REGISTER(app);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;
static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
static struct k_timer led_timer;
static bool is_blinking = true;


void led_timer_handler(struct k_timer *dummy)
{

    static int led_state = 0;

    gpio_pin_set_dt(&led, led_state);

    if (led_state) {
        LOG_INF("LED Ligado (Timer)");
    } else {
        LOG_INF("LED Apagado (Timer)");
    }

    led_state = !led_state;
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    if (is_blinking) {
        k_timer_stop(&led_timer);
        is_blinking = false;
        LOG_INF("Botão pressionado! Pisca-pisca PARADO.");
    } else {
        k_timer_start(&led_timer, K_MSEC(LED_BLINK_INTERVAL_MS), K_MSEC(LED_BLINK_INTERVAL_MS));
        is_blinking = true;
        LOG_INF("Botão pressionado! Pisca-pisca REINICIADO.");
    }
}

int main(void)
{
    int ret;
    unsigned char c;

    if (!device_is_ready(led.port) || !device_is_ready(button.port)) {
        LOG_ERR("Erro: Dispositivo GPIO não está pronto!");
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Falha ao configurar o LED!");
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    k_timer_init(&led_timer, led_timer_handler, NULL);
    k_timer_start(&led_timer, K_MSEC(LED_BLINK_INTERVAL_MS), K_MSEC(LED_BLINK_INTERVAL_MS));

    LOG_INF(">>> Pressione a tecla ENTER nesta janela para simular o botão. <<<");

    while (1) {
        if (!uart_poll_in(console_dev, &c)) {
            if (c == '\n' || c == '\r') {
                button_pressed(button.port, &button_cb_data, BIT(button.pin));
            }
        }
        k_msleep(50);
    }
    return 0;
}