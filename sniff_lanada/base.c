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

/* static uint8_t sensing_flag; */
/* static uint8_t off_flag; */

/* --------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "base station");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  /* char* recv_data = (char *)packetbuf_dataptr(); */
  /* if(recv_data[0] == "s" && recv_data[1] == "e" && recv_data[2] == "n" && */
  /*    recv_data[3] == "s" && recv_data[4] == "i" && recv_data[5] == "n" && */
  /*    recv_data[6] == "g") { */
  /*   sensing_flag = 1; */
  /* } */
  /* else if(recv_data[0] == "o" && recv_data[1] == "f" && recv_data[2] == "f") */
  /*   { */
  /*     off_flag = 1; */
  /*   } */

  printf("DATA received from %d.%d: %s\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
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

  etimer_set(&et, CLOCK_SECOND);

  while(1) {
    linkaddr_t addr;
    /* Delay 2-4 seconds */
    /* etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4)); */
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {
      /* addr.u8[0] = 0; */
      /* addr.u8[1] = 1; */
      /* if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) { */
      /* 	packetbuf_copyfrom("Hello", 6); */
      /* 	broadcast_send(&broadcast); */
      /* 	printf("broadcast message sent\n"); */
      /* } */
    }
    if(ev == serial_line_event_message) {
      char * input = (char *)data;
      if(input[0]=='s' && input[1]=='e' && input[2]=='n' &&
	 input[3]=='s' && input[4]=='i' && input[5]=='n' &&
	 input[6]=='g') {
	packetbuf_clear();
	packetbuf_copyfrom((char *)data,8);	
	addr.u8[0] = 0;
	addr.u8[1] = 0;
	broadcast_send(&broadcast);
      }
      else if(input[0]=='o' && input[1]=='f' && input[2]=='f') {
	packetbuf_clear();
	packetbuf_copyfrom((char *)data,4);
	addr.u8[0] = 0;
	addr.u8[1] = 0;
	broadcast_send(&broadcast);
      }
      printf("UART input %s\n",(char *)data);
      /* Send command to sensors */
    }
    /* if(sensing_flag == 1) { // Do sensing */
    /*   sensing_flag = 0; */
    /*   	printf("GPIO pin High\n"); */
    /* 	MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN); */
    /* 	MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN); */
    /* 	MSP430_GPIOEN_PORT(OUT) |= BV(MSP430_GPIOEN_PIN); */
    /* 	printf("wait for a second\n"); */
    /* 	/\* Delay 1 second for power on *\/ */
    /* 	etimer_set(&et, CLOCK_SECOND); */
    /* 	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); */
    /* 	printf("put start to uart(#2)\n"); */
    /* 	putchar2('s'); // -> using different uart(#2) */
    /* 	putchar2('t');  */
    /* 	putchar2('a');  */
    /* 	putchar2('r');  */
    /* 	putchar2('t');  */
    /* } */
    /* if(off_flag == 1) { // Turn off the sensor */
    /*   off_flag = 0; */
    /*   MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN); */
    /*   MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN); */
    /*   MSP430_GPIOEN_PORT(OUT) &= ~BV(MSP430_GPIOEN_PIN); */
    /*   printf("GPIO pin Low\n"); */
    /* } */
    etimer_set(&et, CLOCK_SECOND/1000);
    /* PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
