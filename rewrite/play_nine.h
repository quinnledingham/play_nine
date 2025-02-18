enum Play_Nine_Input {
  PNI_CARD_0,
  PNI_CARD_1,
  PNI_CARD_2,
  PNI_CARD_3,
  PNI_CARD_4,
  PNI_CARD_5,
  PNI_CARD_6,
  PNI_CARD_7,

  PNI_PICKUP_PILE,
  PNI_DISCARD_PILE,
  PNI_PASS,

  PNI_COUNT
};

enum Input_Modes {
  INPUT_MODE_MENU,
  INPUT_MODE_ONSCREEN_MENU,
  INPUT_MODE_IN_GAME,

  INPUT_MODE_COUNT
};

/*
MENU:
Controller: Navigate menu with joystick/DPAD, pop up onscreen keyboard while active
Keyboard: Navigate with arrow keys, typing
Mouse: Click on each button, go to keyboard for typing

ON_SCREEN MENU:
Controller: Button Prompts
Keyboard: Button Prompts (Navigate not with array keys)
Mouse: Clickable Buttons

IN_GAME:
Controller: Used joystick/DPAD to select cards -> button prompt for pass
Keyboard: Use individual keys for each input -> button prompt for pass
Mouse: Click on the card or show button for pass
*/