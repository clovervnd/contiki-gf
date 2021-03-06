/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Testing the broadcast layer in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"
#include "dev/serial-line.h"
#include "platform/exp5438/uart2-putchar.h"

#include <stdio.h>
#define MSP430_GPIOEN_PORT(type)		P9##type
#define MSP430_GPIOEN_PIN		6
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  char* msg = (char *)packetbuf_dataptr();
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
  if(msg[0] == 'i') {
    printf("input i from radio\n");
	MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(OUT) |= BV(MSP430_GPIOEN_PIN);
	printf("GPIO pin High\n");
  }
  else if(msg[0] == 's') {
    printf("input s from radio\n");
	putchar2('s'); // -> using different uart(#2)
	putchar2('t'); 
	putchar2('a'); 
	putchar2('r'); 
	putchar2('t'); 
  }
  
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

	broadcast_open(&broadcast, 129, &broadcast_call);

	//	packetbuf_copyfrom("Hello", 6);
	//	printf("broadcast message sending\n");
	//	broadcast_send(&broadcast);
	//	printf("broadcast message sent\n");
	//	printf("Wait until input 'i'\n");
	printf("Powered on\n");
	etimer_set(&et, CLOCK_SECOND*5);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	etimer_set(&et, CLOCK_SECOND);
	
  while(1) {

    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {
      /* do nothing */
    }
    if(ev == serial_line_event_message) {
      /* leds_toggle(LEDS_SERIAL_IN); */
      /* printf("DATA received\n"); */
      /* printf("input uart:%s\n",(char *)data); */
      /* if(((char *)data)[0]=='i') { */
      /* 	/\* sensor start signal *\/ */
      /* 	printf("Wait until input 's'\n"); */
      /* } */
      /* else if(((char *)data)[0]=='s') { */
      /* } */
     	printf("DATA received %s\n",(char *)data);
	MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(OUT) &= ~BV(MSP430_GPIOEN_PIN);
	printf("GPIO pin Low\n");
	//	packetbuf_copyfrom(data,strlen(data));
	//	broadcast_send(&broadcast);
    }
    etimer_set(&et, CLOCK_SECOND);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
