#include <stdlib.h>
#include <msp430.h>
#include <stdint.h>
#include "peripherals.h"

// Enum for the game state
typedef enum {
  DISPLAY,
  EDIT,
} state;

// Rin Timer A2
void runtimerA2(void);

// Display functions
// Display the Date
void displayDate(char*, volatile long unsigned int, volatile unsigned int, volatile unsigned int);

// Display the Time
void displayTime(char*, volatile long unsigned int, volatile unsigned int, volatile unsigned int, volatile unsigned int);

// Display the Temp in C
void displayTempC(char*, volatile float);

// Display the Temp in F
void displayTempF(char*, volatile float);

// ADC functions
// Configure the ADC12
void config_ADC(volatile float, volatile unsigned int, volatile unsigned int);

// Convert ADC 2 Time --> Populate slider
void ADC_2_Time(volatile unsigned int);

// Convert ADC 2 Temp --> Populate in_temp
void ADC_2_Temp(volatile unsigned int);

// Buttons functions
// Init user's launchpad buttons
void init_launchpad_button();

// Read user's launchpad button
unsigned int read_launchpad_button();






// Initializes the two user LEDs
void init_user_leds();

//Sets the two user LEDs
void set_user_leds(unsigned char uled);

// Initializes the buttons for input
void init_board_buttons();

//Reads the four buttons
unsigned int read_board_buttons();
