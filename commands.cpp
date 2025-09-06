//
// Created by marko on 9/6/25.
//
#include "commands.h"
#include "algorithm"
#include <cstring>
#include "stdint.h"
#include  <cstdio>
#include "pico/stdlib.h"
#include "main.h"

extern status_t status;

commands::COMMAND_LIST get_command_from_string(char * input)
{
    for (auto i : commands::commands_pair_list)
    {
        if (strcmp(input, i.input) == 0) return i.command;
    }
    return commands::COMMAND_LIST::UNKNOWN;
}

void comms_thread()
{
    char buffer[256];
    buffer[sizeof(buffer) - 1] = '\0';
    uint8_t i = 0;
    while (1)
    {
        int input = getchar_timeout_us(1000);
        if (input != PICO_ERROR_TIMEOUT)
        {
            if (input == '\n' || input == '\r')
            {
                //TODO: well do something
                const auto command = get_command_from_string(buffer);
                printf("rcv cmd: %s[%u]\n", buffer, command);
                if (command == commands::COMMAND_LIST::START) status.puzzle = PUZZLE_STATE::RUNNING;
                else if (command == commands::COMMAND_LIST::STOP) status.puzzle = PUZZLE_STATE::STOPPED;
                i = 0;
                continue;
            }
            buffer[i] = input;
            buffer[i + 1] = '\0';
            i++;
            if (i >= sizeof(buffer) - 1) i = 0;
        }
    }
}
