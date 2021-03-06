
/* dds.c: PPM firmware timer/counter peripherals source code.
 * Copyright (C) 2013  Bradley Worley  <geekysuavo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* include the main header. */
#include "main.h"

/* declare the sine wave lookup table. */
const uint8_t dds_lut[256] PROGMEM = {
  0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95,
  0x98, 0x9c, 0x9f, 0xa2, 0xa5, 0xa8, 0xab, 0xae,
  0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbf, 0xc1, 0xc4,
  0xc7, 0xc9, 0xcc, 0xce, 0xd1, 0xd3, 0xd5, 0xd8,
  0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8,
  0xea, 0xec, 0xed, 0xef, 0xf0, 0xf2, 0xf3, 0xf5,
  0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfc,
  0xfd, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe,
  0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7,
  0xf6, 0xf5, 0xf3, 0xf2, 0xf0, 0xef, 0xed, 0xec,
  0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
  0xda, 0xd8, 0xd5, 0xd3, 0xd1, 0xce, 0xcc, 0xc9,
  0xc7, 0xc4, 0xc1, 0xbf, 0xbc, 0xb9, 0xb6, 0xb3,
  0xb0, 0xae, 0xab, 0xa8, 0xa5, 0xa2, 0x9f, 0x9c,
  0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83,
  0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a,
  0x67, 0x63, 0x60, 0x5d, 0x5a, 0x57, 0x54, 0x51,
  0x4f, 0x4c, 0x49, 0x46, 0x43, 0x40, 0x3e, 0x3b,
  0x38, 0x36, 0x33, 0x31, 0x2e, 0x2c, 0x2a, 0x27,
  0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17,
  0x15, 0x13, 0x12, 0x10, 0x0f, 0x0d, 0x0c, 0x0a,
  0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x03,
  0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13,
  0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23,
  0x25, 0x27, 0x2a, 0x2c, 0x2e, 0x31, 0x33, 0x36,
  0x38, 0x3b, 0x3e, 0x40, 0x43, 0x46, 0x49, 0x4c,
  0x4f, 0x51, 0x54, 0x57, 0x5a, 0x5d, 0x60, 0x63,
  0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c
};

/* dds variables. */
volatile uint16_t dds_ftw = 2048;
volatile uint16_t dds_acc = 0;
volatile uint8_t dds_val;

/* ISR (TIMER0_COMPA_vect): timer0 compare match A interrupt. */
ISR (TIMER0_COMPA_vect) {
  /* turn on an led. */
  gpio_ledb_on ();

  /* add the frequency tuning word into the accumulation value. */
  dds_acc += dds_ftw;

  /* get the value to send to the dac. */
  dds_val = pgm_read_byte (&dds_lut[MSB (dds_acc)]);

  /* write the value to the dac. */
  spi_write_dac_8 (dds_val);

  /* turn off the led. */
  gpio_ledb_off ();
}

/* dds_init: initialize timer/counter peripherals. */
void dds_init (void) {
  /* configure the timer0 prescaler mux for 4.0 us resolution. */
  TCCR0B |= ((1 << CS01) | (1 << CS00));

  /* configure the timer0 counter to clear on compare match. */
  TCCR0A |= (1 << WGM01);

  /* set the compare match registers. */
  OCR0A = 0x08;

  /* zero the timer register. */
  TCNT0 = 0x00;
  
  /* enable the timer. */
  TIMSK0 |= ((1 << TOIE0) | (1 << OCIE0A));
}

/* dds_set_frequency: sets the frequency tuning word of the dds based on
 * two bytes sent via usb from the host.
 */
void dds_set_frequency (void) {
  /* grab the dds frequency tuning word bytes. */
  int16_t b0 = usb_cdc_getchar ();
  int16_t b1 = usb_cdc_getchar ();

  /* set the frequency tuning word. */
  if (b0 != -1 && b1 != -1)
    dds_ftw = WORD (b0, b1);
}
