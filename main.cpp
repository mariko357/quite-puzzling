#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <cstdio>
#include "pico/multicore.h"
#include "commands.h"
#include "pico/time.h"
#include "main.h"
#include "pico/rand.h"

#define LED_PWM_PIN 6
#define SENSE_PIN 14
#define BUTTON_PIN 15
#define PWM_FREQ 38000   // 38 kHz
#define PWM_PULSE_COUNT 16   // 20 pulses
#define RESP_MS 2
#define PING_PERIOD_MS 10
#define LED_DEBOUNCE_MS 300
#define BUTTON_DEBOUNCE_MS 50
#define LED_PULSE_CNT 8
#define LED_PULSE_MARGIN 2

status_t status = {IR_TX_STATUS::TX, PUZZLE_STATE::STOPPED, false, 0, false, 0};

volatile uint pwm_pulse_count = 0;

void pwm_irq_handler() {
    pwm_clear_irq(3); // TODO: make proper slice handling
    pwm_pulse_count++;
}

void gpio_isr(uint gpio, uint32_t event_mask)
{
    if (gpio == SENSE_PIN && status.led != IR_TX_STATUS::TOUT)
    {
        status.led_rx = true;
    }
    if (gpio == BUTTON_PIN)
    {
        if (event_mask & GPIO_IRQ_EDGE_FALL) status.btn_pressed = true;
        else status.btn_pressed = false;
    }
}

int main() {
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_set_function(LED_PWM_PIN, GPIO_FUNC_PWM);
    gpio_set_drive_strength(LED_PWM_PIN, GPIO_DRIVE_STRENGTH_12MA); // Better set with resistor

    uint slice_num = pwm_gpio_to_slice_num(LED_PWM_PIN);
    uint32_t clk = 125000000;
    uint32_t wrap = clk / PWM_FREQ - 1;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_PWM_PIN), wrap / 2);

    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irq_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_PWM_PIN), 0);

    gpio_init(SENSE_PIN);
    gpio_set_dir(SENSE_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(SENSE_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_isr);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_isr);

    pwm_set_enabled(slice_num, true);

    multicore_launch_core1(comms_thread);
    while (true)
    {
        if (status.puzzle == PUZZLE_STATE::STOPPED)
        {
            if (status.btn_pressed)
            {
                if (status.btn_debounce_end == 0)
                {
                    status.btn_debounce_end = to_ms_since_boot(get_absolute_time()) + BUTTON_DEBOUNCE_MS;
                }
            }
            else
            {
                status.btn_debounce_end = 0;
            }

            if (status.btn_debounce_end && (to_ms_since_boot(get_absolute_time()) > status.btn_debounce_end))
            {
                printf("%s\n", (commands::commands_pair_list[static_cast<unsigned int>(commands::COMMAND_LIST::BUTTON_PRESSED)].input));
                status.btn_debounce_end = UINT32_MAX;
            }
        }
        else if (status.puzzle == PUZZLE_STATE::RUNNING){

            for (uint8_t j = 0; j < LED_PULSE_CNT; j++)
            {
                gpio_put(PICO_DEFAULT_LED_PIN, true);
                pwm_pulse_count = 0;
                pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_PWM_PIN), wrap / 2);
                status.led = IR_TX_STATUS::TX;
                while (pwm_pulse_count < PWM_PULSE_COUNT) {
                    tight_loop_contents();
                }
                pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_PWM_PIN), 0);

                status.led = IR_TX_STATUS::TOUT;

                if (status.led_rx)
                {
                    status.led_rx_cnt += 1;
                }
                else
                {
                    status.led_rx_cnt = 0;
                }
                status.led_rx = false;
                gpio_put(PICO_DEFAULT_LED_PIN, false);
                uint8_t random_sleep = (uint8_t)get_rand_32() >> 3;
                sleep_ms(random_sleep);
            }

            if (status.led_rx_cnt >= LED_PULSE_CNT - LED_PULSE_MARGIN)
            {
                printf("%s\n", (commands::commands_pair_list[static_cast<unsigned int>(commands::COMMAND_LIST::PUZZLE_COMPLETED)].input));
            }
            status.led_rx_cnt = 0;

            sleep_ms(PING_PERIOD_MS);
        }
    }
}
