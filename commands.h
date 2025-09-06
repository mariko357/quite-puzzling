//
// Created by marko on 9/6/25.
//

#ifndef COMMANDS_H
#define COMMANDS_H

namespace commands
{
    inline constexpr const char* START_DETECTION_COMMAND   = "START";
    inline constexpr const char* STOP_DETECTION_COMMAND    = "STOP";
    inline constexpr const char* BUTTON_PRESSED_REPORT     = "BUTTON_PRESSED";
    inline constexpr const char* PUZZLE_COMPLETED_REPORT   = "PUZZLE_COMPLETED";

    enum class COMMAND_LIST
    {
        START,
        STOP,
        BUTTON_PRESSED,
        PUZZLE_COMPLETED,
        UNKNOWN
    };

    struct command_pair_t
    {
        COMMAND_LIST command;
        const char * input;
    };

    inline command_pair_t commands_pair_list[] = {
        {COMMAND_LIST::START, START_DETECTION_COMMAND},
        {COMMAND_LIST::STOP, STOP_DETECTION_COMMAND},
        {COMMAND_LIST::BUTTON_PRESSED, BUTTON_PRESSED_REPORT},
        {COMMAND_LIST::PUZZLE_COMPLETED, PUZZLE_COMPLETED_REPORT},
    };
}

commands::COMMAND_LIST get_command_from_string(char * input);
void comms_thread();
#endif //COMMANDS_H
