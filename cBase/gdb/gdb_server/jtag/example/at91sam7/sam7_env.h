/*
 * sam7_env.h
 *
 * Copyright (C) 2005
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.
 */


//#define AT91SAM7S_EK_DEV_BOARD
#define AT91SAM7S_P64_DEV_BOARD

#ifdef AT91SAM7S_EK_DEV_BOARD
/*
 * Atmel developmet board
 */

/* Quarz frequency in range of 10MHz - 25MHz (value in Hz)*/
#define F_OSC 18432000

/* the desired operating frequency*/
#define REQ_CCLK 48000000
#define PLL_DIV 5
#define PLL_MUL 26

/*
 * - div by 5  Fin = 18,432/5 = 3,6864 
 * - Mul 25+1: Fout = 3,6864 *26 = 95,8464
 * for 96 MHz the error is 0.16%
 */
#define F_IN (F_OSC / 5)  /*= 3686400*/
#define F_OUT (F_IN * PLL_MUL) /*= 95846400*/

//#define CYCLES_PER_MIKRO_SECOND	((REQ_CCLK / 1500000 ) + 1 )
#define CYCLES_PER_MIKRO_SECOND	((REQ_CCLK / 666666 ) + 1 )

#define MCK_DIV 2

#define SCK	32768
#define MCK	(F_OUT / MCK_DIV) /*= 47923200*/

/* Leds */
#define LED3 (1<<3)	/* PA3 - pin 43 */
#define LED2 (1<<2)	/* PA2 - pin 44 */
#define LED1 (1<<1)	/* PA1 - pin 47 */
#define LED0 (1)	/* PA0 - pin 48 */

#define LED_MSK        (LED0|LED1|LED2|LED3)

#endif /*AT91SAM7S_EK_DEV_BOARD*/

#ifdef AT91SAM7S_P64_DEV_BOARD
/*
 * Olimex P64 Board
 */

/* Quarz frequency in range of 10MHz - 25MHz (value in Hz)*/
#define F_OSC 12000000

/* the desired operating frequency*/
#define REQ_CCLK 48000000 /*= 48 MHz*/
#define PLL_DIV 5
#define PLL_MUL 40

#define F_IN (F_OSC / 5)  /*= 2400 kHz ( 1 - 32 MHz)*/
#define F_OUT (F_IN * PLL_MUL) /*= 96 MHz (_00: 80 - 160 MHz)*/

//#define CYCLES_PER_MIKRO_SECOND	((REQ_CCLK / 1500000 ) + 1)
#define CYCLES_PER_MIKRO_SECOND	((REQ_CCLK / 666666) + 1)

#define MCK_DIV 2

#define SCK	32768
#define MCK	(F_OUT / MCK_DIV) /*= 48 MHz*/

/* Leds */
#define LED3 (1<<17)	/* PA17 - pin 9 */
#define LED2 (1<<18)	/* PA18 - pin 10 */

#define LED_MSK        (LED2|LED3)


#endif /*AT91SAM7S_P64_DEV_BOARD*/

