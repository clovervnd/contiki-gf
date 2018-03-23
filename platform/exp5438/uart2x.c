/*
 * Copyright (c) 2010, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * Yet another machine dependent MSP430X UART1 code.
 * IF2, etc. can not be used here... need to abstract to some macros
 * later.
 */

#include "contiki.h"
#include <stdlib.h>
#include <stdio.h>

#include "sys/energest.h"
#include "dev/uart2.h"
#include "dev/watchdog.h"
#include "lib/ringbuf.h"
#include "dev/leds.h"
#include "isr_compat.h"

static int (*uart2_input_handler)(unsigned char c);

static volatile uint8_t transmitting;

/*---------------------------------------------------------------------------*/
uint8_t
uart2_active(void)
{
  return (UCA2STAT & UCBUSY) | transmitting;
}
/*---------------------------------------------------------------------------*/
void
uart2_set_input(int (*input)(unsigned char c))
{
  uart2_input_handler = input;
}
/*---------------------------------------------------------------------------*/
void
uart2_writeb(unsigned char c)
{
  watchdog_periodic();
  /* Loop until the transmission buffer is available. */
  while((UCA2STAT & UCBUSY));

  /* Transmit the data. */
  UCA2TXBUF = c;
}
/*---------------------------------------------------------------------------*/
#if ! NETSTACK_CONF_WITH_IPV4 /* If NETSTACK_CONF_WITH_IPV4 is defined, putchar() is defined by the SLIP driver */
#endif /* ! NETSTACK_CONF_WITH_IPV4 */
/*---------------------------------------------------------------------------*/
/**
 * Initalize the RS232 port.
 *
 */
void
uart2_init(unsigned long ubr)
{
  /* RS232 */
  UCA2CTL1 |= UCSWRST;            /* Hold peripheral in reset state */
  UCA2CTL1 |= UCSSEL_2;           /* CLK = SMCLK */

  /* UCA2BR0 = 0x45;                 /\* 8MHz/115200 = 69 = 0x45 *\/ */
  UCA2BR0 = ubr & 0xff; //0x45; /* tested... */
  /* UCA2BR0 = 9; */
  UCA2BR1 = ubr >> 8;
  UCA2MCTL = UCBRS_3;             /* Modulation UCBRSx = 3 */
  P9DIR &= ~0x20;                 /* P3.5 = USCI_A0 RXD as input */
  P9DIR |= 0x10;                  /* P3.4 = USCI_A0 TXD as output */
  P9SEL |= 0x30;                  /* P3.4,5 = USCI_A0 TXD/RXD */

  /*UCA2CTL1 &= ~UCSWRST;*/       /* Initialize USCI state machine */

  transmitting = 0;

  /* XXX Clear pending interrupts before enable */
  UCA2IE &= ~UCRXIFG;
  UCA2IE &= ~UCTXIFG;

  UCA2CTL1 &= ~UCSWRST;                   /* Initialize USCI state machine **before** enabling interrupts */
  UCA2IE |= UCRXIE;                        /* Enable UCA1 RX interrupt */
}
/*---------------------------------------------------------------------------*/
ISR(USCI_A2, uart2_rx_interrupt)
{
  uint8_t c;
  printf("uart2x ISR\n");
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  /*leds_toggle(LEDS_ALL);*/
  if(UCA2IV == 2) {
    if(UCA2STAT & UCRXERR) {
      c = UCA2RXBUF;   /* Clear error flags by forcing a dummy read. */
    } else {
      c = UCA2RXBUF;
      //      printf("uart input is detected\n");
      if(uart2_input_handler != NULL) {
        if(uart2_input_handler(c)) {
          LPM4_EXIT;
        }
      }
    }
  }
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
