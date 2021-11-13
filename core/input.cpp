
#include "./input.hpp"

//For input, 0 is `pressed`
void Input::pressButton(Button button) {
    u8 button_index = static_cast<u8>(button);
    buttonState = clearBit(buttonState, button_index);
}

//For input, 1 is `unpressed`
void Input::unpressButton(Button button) {
    u8 button_index = static_cast<u8>(button);
    buttonState = setBit(buttonState, button_index);
}

u8 Input::readInput() {
    u8 action_buttons = getLowNibble(buttonState);
    u8 direction_buttons = getHighNibble(buttonState);

    if (!action_selected && !direction_selected) {
        return 0b11111111; //All unselected
    } else if (action_selected && direction_selected) {
        u8 to_return = 0b11001111; //both selected
        to_return = setLowNibble(to_return, action_buttons & direction_buttons); 
        return to_return;
    } else if (action_selected) {
        u8 to_return = 0b11011111; //only action selected
        to_return = setLowNibble(to_return, action_buttons);
        return to_return;
    } else { //if (direction_selected) 
        u8 to_return = 0b11101111; //only direction selected
        to_return = setLowNibble(to_return, direction_buttons);
        return to_return;
    }
}

//0 (!true) is `selected`
void Input::writeInput(u8 byte) {
    action_selected = !checkBit(byte, ACTION_SELECTED_INDEX);
    direction_selected = !checkBit(byte, DIRECTION_SELECTED_INDEX);
}