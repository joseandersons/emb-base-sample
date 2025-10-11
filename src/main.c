#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app);

#define PWM_PERIOD_NS CONFIG_APP_PWM_PERIOD_NS
#define LED_BLINK_INTERVAL_MS CONFIG_APP_BLINK_INTERVAL_MS

static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(led0));
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static struct gpio_callback button_cb_data;
static struct k_timer led_timer;
static bool is_pwm_mode = false;


static inline void pwm_set_or_log(uint32_t period, uint32_t pulse) {
    int err = pwm_set_dt(&pwm_led, period, pulse);
    if (err) {
        LOG_ERR("pwm_set_dt(period=%u, pulse=%u) falhou (%d)", period, pulse, err);
    }
}

void led_timer_handler(struct k_timer *dummy) {
    static bool led_state = false;
    pwm_set_or_log(PWM_PERIOD_NS, led_state ? PWM_PERIOD_NS : 0);
    led_state = !led_state;
}

void fade_pwm(void) {
    static int bright = 0;
    static int step = CONFIG_APP_FADE_STEP;
    bright += step;
    if (bright >= 100 || bright <= 0) step = -step;
    uint32_t pulse = (PWM_PERIOD_NS * bright) / 100U;
    pwm_set_or_log(PWM_PERIOD_NS, pulse);
    k_msleep(CONFIG_APP_FADE_DELAY_MS);
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    static int64_t last = 0;
    int64_t now = k_uptime_get();
    if (now - last < 150) return; // 150 ms de debounce
    last = now;
    
    is_pwm_mode = !is_pwm_mode;
    if (is_pwm_mode) {
        k_timer_stop(&led_timer);
        LOG_INF("Modo PWM (fade) ativado");
    } else {
        k_timer_start(&led_timer, K_MSEC(LED_BLINK_INTERVAL_MS), K_MSEC(LED_BLINK_INTERVAL_MS));
        LOG_INF("Modo digital (pisca) ativado");
    }
}

int main(void) {

    if (!device_is_ready(pwm_led.dev) || !device_is_ready(button.port)) {
        LOG_ERR("Erro: dispositivos não estão prontos!");
        return 0;
    }

    gpio_pin_configure_dt(&button, GPIO_INPUT);
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

    k_timer_init(&led_timer, led_timer_handler, NULL);
    k_timer_start(&led_timer, K_MSEC(LED_BLINK_INTERVAL_MS), K_MSEC(LED_BLINK_INTERVAL_MS));

    LOG_INF("Sistema iniciado! Pressione o botão (GPIO9) para alternar modos.");

    while (1) {
        if (is_pwm_mode)
            fade_pwm();
        k_msleep(30);
    }
}
