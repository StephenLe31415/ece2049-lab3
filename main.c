#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"
#include "functions.h"
/*----------------------------------------------------------------------------------- */
// Temperature Sensor Calibration = Reading at 30 degrees C is stored at addr 1A1Ah
// See end of datasheet for TLV table memory mapping
#define CALADC12_15V_30C *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration = Reading at 85 degrees C is stored at addr 1A1Ch
//See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C *((unsigned int *)0x1A1C)
#define ONE_MONTH_IN_ADC 342 // 12 segments from 0 --> 2045

volatile unsigned int in_temp;
volatile unsigned int slider;
/*----------------------------------------------------------------------------------- */
volatile long unsigned int global_counter = 16416000;

#pragma vector=TIMER2_A0_VECTOR //What does this do? No one knows...
__interrupt void timer_a2() {
  global_counter++;
  Graphics_clearDisplay(&g_sContext); // Clear the display
  Graphics_flushBuffer(&g_sContext);
}

void main() {
  /*----------------------------------------------------------------------------------- */
  volatile float temperatureDegC = 0;
  volatile float temperatureDegF = 0;
  volatile float degC_per_bit;
  volatile unsigned int adc_month = 1;
  volatile unsigned int adc_date = 0;
  volatile unsigned int adc_hour = 0;
  volatile unsigned int adc_min = 0;
  volatile unsigned int adc_sec = 0;
  volatile unsigned int bits30, bits85;
  volatile state mode;
  /*----------------------------------------------------------------------------------- */
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!

  _BIS_SR(GIE); // Global interrupt enable

  // Initialize the MSP430
  config_ADC(); // Config the ADC12
  initLeds();
  init_user_leds();
  init_buttons();
  configDisplay();
  configKeypad();
  Graphics_clearDisplay(&g_sContext); // Clear the display
  runtimerA2(); // Start the A2 timer
  // Array for display functions.
  char date[7] = {0};
  char time[9] = {0};
  char tempC[7] = {0};
  char tempF[7] = [0];
  mode = DISPLAY; // Main  mode
  user_input = read_launchpad_button(); // Read the User's Push-buttons

  while (1) {
    switch(mode) {
      case DISPLAY: {
        while(user_input != 1) { // Left button
          ADC_2_Temp(); // ADC Conversion stuff
          temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
          temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0; // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
          // Display stuff
          displayDate(date, global_counter, adc_month, adc_date);
          displayTime(time, global_counter, adc_hour, adc_min, adc_sec);
          displayTempC(tempC, temperatureDegC);
          displayTempF(tempF, temperatureDegF);
        }

        mode = EDIT;
        break;
      }

      case EDIT: {
        int num_pressed = 0;
        while (user_input != 2) { // Right button
          ADC_2_Time(); // ADC Conversion stuff
          num_pressed += (read_launchpad_button() % 5); // Wrap around to "Month" logic

          switch (num_pressed) {
            case 0: { //MONTH
              adc_month = 1 + (unsigned int)(slider / ONE_MONTH_IN_ADC);
              displayDate(date, 0, adc_month, adc_date); // "Date" has not been updated yet.
              break;    
            }

            case 1: { // DATE
              if (adc_month == 2) {
              adc_date = 1 + (unsigned int)(slider / 147);
              } else if (adc_month == 4 &&  adc_month == 6 && adc_month = 9 && adc_month == 11) {
                date = 1 + (unsigned int)(slider / 137);
              } else {
                adc_date = 1 + (unsigned int)(slider / 133);
              } 
              displayDate(date, 0, adc_month, adc_date); // "Month" and "Date" have been updated
              break;   
            }

            case 2: { // HOUR
              adc_hour = (unsigned int)(slider / 171);    
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
              break;
            }

            case 3: { // MIN
              adc_min = (unsigned int)(slider / 69);
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // "Hour" has been updated. "Sec" has not been updated
              break;
            }

            case 4: { // SEC
              adc_sec = (unsigned int)(slider / 69);
              displayTime(time, 0, adc_hour, adc_min, adc_sec); // Every param has been updated.
              break;    
            }
          } // End of switch num_pressed
        } // End of while loop

        mode = DISPLAY;
        break;
      } // End of case EDIT
    }  // End of switch mode
  } // End of while(1)
} // End of main()
