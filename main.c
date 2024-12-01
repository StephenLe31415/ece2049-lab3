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
unsigned int in_temp;
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
  volatile float temperatureDegC;
  volatile float temperatureDegF;
  volatile float degC_per_bit;
  volatile unsigned int bits30, bits85;
  /*----------------------------------------------------------------------------------- */

  WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
  // You can then configure it properly, if desired

  // Global interrupt enable
  _BIS_SR(GIE);

  /*----------------------------------------------------------------------------------- */
  REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
  // internal reference voltages to
  // ADC12_A control registers
  ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; // Internal ref = 1.5V
  ADC12CTL1 = ADC12SHP; // Enable sample timer
  // Using ADC12MEM0 to store reading
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temp sense
  // ACD12SREF_1 = internal ref = 1.5v
  __delay_cycles(100); // delay to allow Ref to settle
  ADC12CTL0 |= ADC12ENC; // Enable conversion
  // Use calibration data stored in info memory
  bits30 = CALADC12_15V_30C;
  bits85 = CALADC12_15V_85C;
  degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));
  /*----------------------------------------------------------------------------------- */

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
  /*----------------------------------------------------------------------------------- */
    ADC12CTL0 &= ~ADC12SC; // clear the start bit
    ADC12CTL0 |= ADC12SC; // Sampling and conversion start
    // Single conversion (single channel)
    // Poll busy bit waiting for conversion to complete
    while (ADC12CTL1 & ADC12BUSY)
      __no_operation();
    in_temp = ADC12MEM0; // Read in results if conversion
    // Temperature in Celsius. See the Device Descriptor Table section in the
    // System Resets, Interrupts, and Operating Modes, System Control Module
    // chapter in the device user's guide for background information on the
    // used formula.
    temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
    // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
    temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0;
  /*----------------------------------------------------------------------------------- */
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
        unsigned int day = global_counter / 86400;
        int i;
        for (i = 0; i < 12; i++) {
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

        //Yeah I'm not allocating memory by hand
        temperatureDegC = temperatureDegC * 10;
        unsigned int int_degC = (unsigned int)temperatureDegC;
        char c_tens = ((int_degC / 100) % 10) + '0';
        char c_ones = ((int_degC / 10) % 10) + '0';
        char c_tenths = (int_degC % 10) + '0';
        temperatureDegF = temperatureDegF * 10;
        unsigned char disp_c[6];
        disp_c[0] = c_tens;
        disp_c[1] = c_ones;
        disp_c[2] = '.';
        disp_c[3] = c_tenths;
        disp_c[4] = ' ';
        disp_c[5] = 'C';
        unsigned int int_degF = (unsigned int)temperatureDegF;
        char f_tens = ((int_degF / 100) % 10) + '0';
        char f_ones = ((int_degF / 10) % 10) + '0';
        char f_tenths = (int_degF % 10) + '0';
        unsigned char disp_f[6];
        disp_f[0] = f_tens;
        disp_f[1] = f_ones;
        disp_f[2] = '.';
        disp_f[3] = f_tenths;
        disp_f[4] = ' ';
        disp_f[5] = 'F';
        // Write to the display
        Graphics_drawStringCentered(&g_sContext, disp_date, 7, 48, 15, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, disp_time, 9, 48, 35, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, disp_c, 6, 48, 45, TRANSPARENT_TEXT);
        Graphics_drawStringCentered(&g_sContext, disp_f, 6, 48, 55, TRANSPARENT_TEXT);
  }
}
