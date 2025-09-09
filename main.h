//
// Created by marko on 9/3/25.
//

#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"
enum class IR_TX_STATUS
{
    TX = 0,
    WAIT_RESP = 1,
    TOUT = 2,
};

enum class PUZZLE_STATE
{
    RUNNING = 0,
    STOPPED = 1,
};

struct status_t
{
    IR_TX_STATUS led;
    PUZZLE_STATE puzzle;
    bool led_rx;
    uint8_t led_rx_cnt;
    bool btn_pressed;
    uint32_t btn_debounce_end;
};

#endif //MAIN_H
