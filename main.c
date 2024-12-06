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
  volatile float slider;
  volatile unsigned int bits30, bits85;
  /*----------------------------------------------------------------------------------- */
  WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
  // You can then configure it properly, if desired

  // Global interrupt enable
  _BIS_SR(GIE);

  //Set Port P8.0 (the slider) to digital I/O mode
  P8OUT |= BIT0;

  /*----------------------------------------------------------------------------------- */
  // TODO The ADC needs to read sequentially from the temp sensor, then the slider. It needs to read in from INCH_5 to a MCTL register with a VREF of 5V
  REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
  // internal reference voltages to
  // ADC12_A control registers
  ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; // Internal ref = 1.5V
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1; // Enable sample timer and set sequential mode
  // Using ADC12MEM0 to store reading
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temp sense
  // Slider stored in to MCTL1 5v reference VCC -> VSS
  ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_5 + ADC12EOS;
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
    //Set store the slider value in ADC12MEM1 in slider
    slider = ADC12MEM1;
  /*----------------------------------------------------------------------------------- */
    char date[7] = {0};
    char time[9] = {0};
    char tempC[7] = {0};
    char tempF[7] = [0];
  
    // hello
    // Hello this is a second test
  }
}
