
#include <stdint.h>
#include <peripheral/adc10.h>
#include "peripheral/ports.h" // Needed by adc10.h
#include "battery_monitor.h"

//BATTERY VOLTAGE SCALE
//3.3/(1024 bits/4(voltage divisor)) = 0.012890625, 6000mv * 0.012890625 ~= 78
#define BATTERY_SCALE  77.34

/*
 * Set up ADC10 to read Cerebot battery monitor AN8.
 * If you are having problems, verify jumper JP4 is on.
 */
void init_battery_monitor()
{
   // configure and enable the ADC
   // ensure the ADC is off before setting the configuration
   CloseADC10();

   // use ground as neg ref for A | use AN8 for input A
   // configure to sample AN8
   SetChanADC10( ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN8);

   // Configure the ADC
   // 16 bit integer, auto convert, auto sampling select.
   //
   // Use AVDD and AVSS for reference, disable calibration, do not scan,
   // interrupt at completion of every 2nd sample, use 2 8 word buffers,
   // alternate between muxes.
   //
   // Use internal clock, set sample time
   //
   // Do not assign channels to scan
   //
   // Set AN8 as analog input
   OpenADC10(
      ADC_FORMAT_INTG | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON,
      ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF |
           ADC_SAMPLES_PER_INT_2 | ADC_ALT_BUF_ON | ADC_ALT_INPUT_ON,
      ADC_CONV_CLK_INTERNAL_RC | ADC_SAMPLE_TIME_15,
      ENABLE_AN8_ANA,
      SKIP_SCAN_ALL);

   // Enable the ADC
   EnableADC10();

   // Wait for the first conversion to complete so there will be vaild
   // data in ADC result registers
   while ( ! mAD1GetIntFlag() );
}

/*
 * Read ADC10 and convert the counts to volts.
 */
float read_battery_voltage()
{
   uint8_t offset = 0;
   uint32_t battery = 0;

   // ADC10 is set up for dual buffers, we need to read from the inactive
   // buffer, invert the active buffer to read from the right place.
   offset = 8 * (~ReadActiveBufferADC10() & 0x01);
   battery = ReadADC10(offset);

   return (float)battery / BATTERY_SCALE;
}
