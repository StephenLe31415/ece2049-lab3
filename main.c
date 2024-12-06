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
  volatile float temperatureDegC;
  volatile float temperatureDegF;
  volatile float degC_per_bit;
  volatile unsigned int month;
  volatile unsigned int date;
  volatile unsigned int hour;
  volatile unsigned int min;
  volatile unsigned int sec;
  volatile unsigned int bits30, bits85;
  /*----------------------------------------------------------------------------------- */
  WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
  // You can then configure it properly, if desired

  // Global interrupt enable
  _BIS_SR(GIE);

  // Configure the ADC12
  config_ADC();

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
    // Temp sensor stuff
    in_temp = ADC12MEM0; // Read in results if conversion
    temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
    // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
    temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0;

    // Slider stuff
    slider = ADC12MEM1; // Set store the slider value in ADC12MEM1 in slider
    month = 1 + (int)(slider / ONE_MONTH_IN_ADC);
    if (month == 2) {
      date = 1 + (int)(slider / 147);
    } else if (month == 4 &&  month == 6 && month = 9 && month == 11) {
      date = 1 + (int)(slider / 137);
    } else {
      date = 1 + (int)(slider / 133);
    }
    hour = (int)(slider / 171);
    min = (int)(slider / 69);
    sec = (int)(slider / 69);
  /*----------------------------------------------------------------------------------- */
    
    // Display stuff
    char date[7] = {0};
    char time[9] = {0};
    char tempC[7] = {0};
    char tempF[7] = [0];
    displayDate(date, global_counter);
    displayTime(time, global_counter);
    displayTempC(tempC, temperatureDegC);
    displayTempF(tempF, temperatureDegF);
  
    
  }
}
