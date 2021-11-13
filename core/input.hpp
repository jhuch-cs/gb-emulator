#pragma once

#include "./util.hpp"

const u8 ACTION_SELECTED_INDEX = 5;
const u8 DIRECTION_SELECTED_INDEX = 4;

enum Button {
    A      = 0,
    B      = 1,
    SELECT = 2,
    START  = 3,

    RIGHT  = 4,
    LEFT   = 5,
    UP     = 6,
    DOWN   = 7,
};

class Input {
public:
    void pressButton(Button button);
    void unpressButton(Button button);
    u8 readInput();
    void writeInput(u8 byte);
private:
    u8 buttonState = 0xFF;
    bool action_selected = false;
    bool direction_selected = false;
};