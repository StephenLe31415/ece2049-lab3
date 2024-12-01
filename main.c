#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"
#include "functions.h"

volatile long unsigned int global_counter = 16416000;

#pragma vector=TIMER2_A0_VECTOR //What does this do? No one knows...
__interrupt void timer_a2() {
  global_counter++;
}

void main() {
  WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
  // You can then configure it properly, if desired

  // Global interrupt enable
  _BIS_SR(GIE);

  // Initialize the MSP430
  initLeds();
  init_user_leds();
  init_buttons();
  configDisplay();
  configKeypad();
  // Clear the display
  Graphics_clearDisplay(&g_sContext);
  //Start the A2 timer
  runtimerA2();
  while (1) {
    //You can wrap this in a function but I'm pretty sure it needs to return a pointer to an array to work properly, so you'd need to initialize the array and allocate memory first
    //Stores month abbreviations  as an array of char in an array
    const char* month_abbr[] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    //Stores length of month to be used to decrement days later
    const int month_days[] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
        unsigned int month = 0;
        unsigned int days = global_counter / 86400;
        for (int i = 0; i < 12; i++) {
          if (day <= month_days[i]) {
            month = i;
            break;
          }
          day = day - month_days[i];
        }
        char day_tens = ((day - (day % 10)) / 10) + '0';
        char day_ones = (day % 10) + '0';
        unsigned char disp_date[7];
        disp_date[0] = month_abbr[month][0];
        disp_date[1] = month_abbr[month][1];
        disp_date[2] = month_abbr[month][2];
        disp_date[3] = ' ';
        disp_date[4] = day_tens;
        disp_date[5] = day_ones;
        disp_date[6] = '\0';
        // Same as above, it needs to have a target to return the array to to be wrapped as a function
        unsigned int hours = (global_counter / 3600) % 24;
        unsigned int minutes = (global_counter / 60) % 60;
        unsigned int seconds = global_counter % 60;
        char hours_tens = ((hours - (hours % 10)) / 10) + '0';
        char hours_ones = (hours % 10) + '0';
        char minutes_tens = ((minutes - (minutes % 10)) / 10) + '0';
        char minutes_ones = (minutes % 10) + '0';
        char seconds_tens = ((seconds - (seconds % 10)) / 10) + '0';
        char seconds_ones = (seconds % 10) + '0';
        unsigned char disp_time[9];
        disp_time[0] = hours_tens;
        disp_time[1] = hours_ones;
        disp_time[2] = ':';
        disp_time[3] = minutes_tens;
        disp_time[4] = minutes_ones;
        disp_time[5] = ':';
        disp_time[6] = seconds_tens;
        disp_time[7] = seconds_ones;
        disp_time[8] = '\0';
        // Write to the display
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, disp_date, 7, 48, 15, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, disp_time, 9, 48, 35, TRANSPARENT_TEXT);
        Graphics_flushBuffer(&g_sContext);
  }
}
