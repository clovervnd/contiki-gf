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
#include "dev/serial-line.h"
#include "dev/leds.h"
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
  printf("Received from %d.%d: %s\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
static uint8_t cnt;
static char BUFFER[100];
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;
  
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  etimer_set(&et, CLOCK_SECOND);

  cnt = 0;
  while(1) {
    linkaddr_t addr;
    /* Delay 2-4 seconds */
    /* etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4)); */
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {
      addr.u8[0] = 1;
      addr.u8[1] = 0;
      if(!linkaddr_cmp(&addr, &linkaddr_node_addr) && cnt<100) {
	sprintf(BUFFER, "Count:%d\n",++cnt);
	packetbuf_clear();
      	packetbuf_copyfrom(BUFFER, 100);
      	broadcast_send(&broadcast);
      	printf("Message sent from id:%d.%d, count:%d\n",linkaddr_node_addr.u8[0],
	       linkaddr_node_addr.u8[1],cnt);
      }
    }
    etimer_set(&et, CLOCK_SECOND/10);
    /* PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
