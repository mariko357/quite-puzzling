#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <cstdio>

#define PWM_PIN 2
#define SENSE_PIN 1
#define PWM_FREQ 38000   // 38 kHz
#define PULSE_COUNT 20   // 20 pulses
#define RESP_MS 2
#define PING_PERIOD_MS 25

enum class IR_TX_STATUS
{
    TX = 0,
    WAIT_RESP = 1,
    TOUT = 2,
};

struct status_t
{
    IR_TX_STATUS led;
    bool rx;
};

status_t status = {IR_TX_STATUS::TX, false};

volatile uint pulse_counter = 0;

// PWM wrap interrupt
void pwm_irq_handler() {
    pwm_clear_irq(1);
    pulse_counter++;
}

void rx_callback(uint gpio, uint32_t event_mask)
{
    if (gpio == SENSE_PIN && status.led != IR_TX_STATUS::TOUT)
    {
        status.rx = true;
    }
}

int main() {
    stdio_init_all();

    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    gpio_set_drive_strength(PWM_PIN, GPIO_DRIVE_STRENGTH_12MA); // Better set with resistor

    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint32_t clk = 125000000;
    uint32_t wrap = clk / PWM_FREQ - 1;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PWM_PIN), wrap / 2);

    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irq_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Input setup
    gpio_init(SENSE_PIN);
    gpio_set_dir(SENSE_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(SENSE_PIN, GPIO_IRQ_EDGE_FALL, true, &rx_callback);

    pwm_set_enabled(slice_num, true);
    while (true) {

        // TX burst of 20 pulses
        pulse_counter = 0;
        pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PWM_PIN), wrap / 2);
        status.led = IR_TX_STATUS::TX;
        while (pulse_counter < PULSE_COUNT) {
            tight_loop_contents();
        }

        pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PWM_PIN), 0);
        status.led = IR_TX_STATUS::WAIT_RESP;
        sleep_ms(RESP_MS);

        status.led = IR_TX_STATUS::TOUT;
        sleep_ms(PING_PERIOD_MS - RESP_MS);

        printf("%d\n", status.rx ? 1 : 0);
        status.rx = false;
    }
}
