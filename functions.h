#include <stdlib.h>
#include <msp430.h>
#include <stdint.h>

// Enum for the game state
typedef enum {
  DISPLAY,
  EDIT,
} state;

// Rin Timer A2
void runtimerA2(void);

// Display functions
// Display the Date
void displayDate(char*, volatile long unsigned int, unsigned int, unsigned int);

// Display the Time
void displayTime(char*, volatile long unsigned int, unsigned int, unsigned int, unsigned int);

// Display the Temp in C
void displayTempC(char*, float);

// Display the Temp in F
void displayTempF(char*, float);

// ADC functions
// Configure the ADC12
void config_ADC(void);

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
void init_buttons();

//Reads the four buttons
unsigned int read_buttons();

//Turn on the buzzer
void buzzer_on(int ticks);
