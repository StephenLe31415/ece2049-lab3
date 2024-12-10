#include "functions.h"

// Globals
volatile unsigned int in_temp = 0;
volatile unsigned int scroll = 0;
volatile unsigned int test_int = 0;
volatile long unsigned int global_counter = 0; // Start at 00:00:00
volatile float temperatureDegC = 0;
volatile float temperatureDegF = 0;
volatile float degC_per_bit = 0;
volatile unsigned int adc_month = 1; // Start on Jan
volatile unsigned int adc_date = 1; // Start on day 1
volatile unsigned int adc_hour = 0;
volatile unsigned int adc_min = 0;
volatile unsigned int adc_sec = 0;
volatile unsigned int bits30, bits85;
volatile state mode;
volatile int display_toggle = 0;
volatile int conversion_toggle = 0;
volatile int increment_flag = 0;
volatile int key_toggle = 0;
#pragma vector=TIMER2_A0_VECTOR //What does this do? No one knows...
__interrupt void timer_a2() {
  global_counter++;
  display_toggle = 0;
  conversion_toggle = 0;
  increment_flag = 1;
  key_toggle = 1;
  // ADC Converstion Stuff
  ADC12CTL0 &= ~ADC12SC; // clear the start bit
  ADC12CTL0 |= ADC12SC + ADC12ENC; // Sampling and conversion start
  // Single conversion (single channel)
  while (ADC12CTL1 & ADC12BUSY) // Poll busy bit waiting for conversion to complete
    __no_operation();
  in_temp = ADC12MEM0;
  scroll = ADC12MEM1; // Set store the slider value in ADC12MEM1 in slider

  temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
  temperatureDegF = temperatureDegC * 9.0/5.0 + 32.0; // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
  // Graphics_clearDisplay(&g_sContext); // Clear the display
  // Graphics_flushBuffer(&g_sContext);
}

void main() {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!

  _BIS_SR(GIE); // Global interrupt enable

  // Initialize the MSP430
  // configKeypad();
  // initLeds();
  // init_user_leds();
  // init_board_buttons();
  // config_ADC(degC_per_bit, bits30, bits85); // Config the ADC12

  /*************************Testing ONLY******************************** */
  //Set up Port P6.0 (the slider) to digital I/O mode
  P6SEL &= ~BIT0;
  P6DIR |= BIT0;
  P6OUT |= BIT0;
  // TODO The ADC needs to read sequentially from the temp sensor, then the slider. It needs to read in from INCH_5 to a MCTL register with a VREF of 5V
  REFCTL0 &= ~REFMSTR; // Reset REFMSTR to hand over control of
  // internal reference voltages to
  // ADC12_A control registers
  ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON | ADC12MSC; // Internal ref = 1.5V
  ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1; // Enable sample timer and set sequential mode
  // Using ADC12MEM0 to store reading
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10; // ADC i/p ch A10 = temp sense
  // Slider stored in to MCTL1 5v reference VCC -> VSS
  ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_0 + ADC12EOS;
  // ACD12SREF_1 = internal ref = 1.5v
  //ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON | ADC12MSC;
  //ADC12CTL1 = ADC12SHP;
  //ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_0;
  __delay_cycles(100); // delay to allow Ref to settle
  ADC12CTL0 |= ADC12ENC; // Enable conversion
  // Use calibration data stored in info memory (1-time setup)
  bits30 = CALADC12_15V_30C;
  bits85 = CALADC12_15V_85C;
  degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));
  /**************************************************************************** */

  configDisplay();
  init_launchpad_button();
  Graphics_clearDisplay(&g_sContext); // Clear the display
  runtimerA2(); // Start the A2 timer
  // Array for display functions.
  int display_sequence = 0;
  char disp_date[7] = {0};
  char disp_time[9] = {0};
  char disp_tempC[7] = {0};
  char disp_tempF[7] = {0};
  // Array for the Moving Average
  float val_tempC[MOVING_AVERAGE_SIZE] = {0.0};
  float val_tempF[MOVING_AVERAGE_SIZE] = {0.0};
  float sum_tempC = 0.0;
  float sum_tempF = 0.0;
  unsigned int index = 0;
  mode = DISPLAY; // Main  mode
  // TODO: Test read_launchpad_button() (weird behavior and output)
  unsigned int user_input = read_launchpad_button(); // Read the User's Push-buttons

  while (1) {
    switch(mode) {
    case DISPLAY: {
        while(user_input == 0 | user_input == 2) { // Only left button triggers
          // Display stuff
          if (display_toggle == 0 && (global_counter % 3) == 0) {
            long unsigned int temp_counter = global_counter;
            switch(display_sequence) {
            case 0: {
              Graphics_clearDisplay(&g_sContext);
              displayDate(disp_date, global_counter, adc_month, adc_date);
              Graphics_flushBuffer(&g_sContext);
              display_sequence = 1;
              user_input = 0;
              while (user_input == 0 && temp_counter == global_counter)
                user_input = read_launchpad_button();
              break;
            }
            case 1: {
              Graphics_clearDisplay(&g_sContext);
              displayTime(disp_time, global_counter, adc_hour, adc_min, adc_sec);
              Graphics_flushBuffer(&g_sContext);
              display_sequence = 2;
              user_input = 0;
              while (user_input == 0 && temp_counter == global_counter)
                user_input = read_launchpad_button();
              break;
            }
            case 2: {
              Graphics_clearDisplay(&g_sContext);
              displayTempC(disp_tempC, (sum_tempC / MOVING_AVERAGE_SIZE));
              Graphics_flushBuffer(&g_sContext);
              display_sequence = 3;
              user_input = 0;
              while (user_input == 0 && temp_counter == global_counter)
                user_input = read_launchpad_button();
              break;
            }
            case 3: {
              Graphics_clearDisplay(&g_sContext);
              displayTempF(disp_tempF, (sum_tempF / MOVING_AVERAGE_SIZE));
              Graphics_flushBuffer(&g_sContext);
              display_sequence = 0;
              user_input = 0;
              while (user_input == 0 && temp_counter == global_counter)
                user_input = read_launchpad_button();
              break;
            }
            }
            display_toggle = 1;
          }

            if (conversion_toggle == 0) {
              index = global_counter % MOVING_AVERAGE_SIZE;

              // Moving-average logic
              sum_tempC -= val_tempC[index]; // Remove the oldest readings
              val_tempC[index] = temperatureDegC;
              sum_tempC += temperatureDegC; // Add the newest readings

              sum_tempF -= val_tempF[index];
              val_tempF[index] = temperatureDegF;
              sum_tempF += temperatureDegF;
              conversion_toggle = 1;
          }
        }
        mode = EDIT;
        break;
      }

      case EDIT: {
        unsigned int num_pressed = 0;
        user_input = 0;
        while (user_input != 2) { // Only right button triggers
          long unsigned int temp_counter = global_counter;
          user_input = 0;
          
          while (user_input == 0 && key_toggle && temp_counter == global_counter)
            user_input = read_launchpad_button();
            key_toggle = 0;
          num_pressed = (num_pressed + user_input) % 5; // Wrap around "Month - Date - Hour - Min - Sec" logic
          // Traversing logic
          switch (num_pressed) {
            case 1: { //MONTH
              if (scroll >= 50 && increment_flag && adc_month != 12) {
                adc_month++;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date);
                Graphics_flushBuffer(&g_sContext);
              }
      
              if (scroll <=30 && increment_flag && adc_month != 1) {
                adc_month--;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date);
                Graphics_flushBuffer(&g_sContext);
              }
              increment_flag = 0;
              break;
            }
              
            // TODO: define the MACROS for these magic numbers: floor((4095 / # of segment) + 1) = magic number
            case 2: { // DATE
              if (scroll >= 50 && increment_flag && adc_date != 28 && adc_month!= 2) {
                adc_date++;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date); // "Month" and "Date" have been updated
                Graphics_flushBuffer(&g_sContext);                
              }

              if ((scroll >= 50) && (increment_flag) && (adc_date != 30) && (adc_month != 4) &&  (adc_month != 6) && (adc_month != 9) && (adc_month != 11)) {
                adc_date++;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date); // "Month" and "Date" have been updated
                Graphics_flushBuffer(&g_sContext);
              }

              if ((scroll >= 50) && (increment_flag) && (adc_date != 31)) {
                adc_date++;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date); // "Month" and "Date" have been updated
                Graphics_flushBuffer(&g_sContext);
              }

              if (scroll <=30 && increment_flag && adc_date != 1) {
                adc_date--;
                Graphics_clearDisplay(&g_sContext);
                displayDate(disp_date, 0, adc_month, adc_date); // "Date" has not been updated yet.
                Graphics_flushBuffer(&g_sContext);
              }
              increment_flag = 0;
              break;
            }

            case 3: { // HOUR
              if (scroll >= 50 && increment_flag && adc_hour != 23) {
                adc_hour ++;
                Graphics_clearDisplay(&g_sContext);
                displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
                Graphics_flushBuffer(&g_sContext);
              }

              if (scroll <= 30 && increment_flag && adc_hour != 0) {
                adc_hour --;
                Graphics_clearDisplay(&g_sContext);
                displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
                Graphics_flushBuffer(&g_sContext);
              }
              increment_flag = 0;
              break;
            }

            case 4: { // MIN
              if ((scroll >= 50) && increment_flag && (adc_min != 59)) {
                adc_min ++;
                Graphics_clearDisplay(&g_sContext);
                displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
                Graphics_flushBuffer(&g_sContext);
              }

              if ((scroll <= 30) && increment_flag && (adc_min != 0)) {
                adc_min --;
                Graphics_clearDisplay(&g_sContext);
                displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
                Graphics_flushBuffer(&g_sContext);
              }
              increment_flag = 0;
              break;
            }

            case 0: { // SEC
              if ((scroll >= 50) && increment_flag && (adc_sec != 59)) {
              adc_sec ++;
              Graphics_clearDisplay(&g_sContext);
              displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
              Graphics_flushBuffer(&g_sContext);
              }

              if ((scroll <= 30) && increment_flag && (adc_sec != 0)) {
                adc_sec --;
                Graphics_clearDisplay(&g_sContext);
                displayTime(disp_time, 0, adc_hour, adc_min, adc_sec); // "Min" and "Sec" have not been updated --> use the previous values stored.
                Graphics_flushBuffer(&g_sContext);
              }
              increment_flag = 0;
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
