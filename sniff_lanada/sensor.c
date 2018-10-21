/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain1 the above copyright
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

#define SINGLEHOP_MODE	TEST_SINGLE_HOP

static uint8_t sensing_flag;
static uint8_t off_flag;

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Event-driven sensor");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
unicast_recv(struct unicast_conn *c, const linkaddr_t *from)
{
  char* recv_data = (char *)packetbuf_dataptr();
  if(recv_data[0] == 's' && recv_data[1] == 'e' && recv_data[2] == 'n' &&
     recv_data[3] == 's' && recv_data[4] == 'i' && recv_data[5] == 'n' &&
     recv_data[6] == 'g') {
    if(sensing_flag != 2) {
      sensing_flag = 1;
    }
  }
  else if(recv_data[0] == 'o' && recv_data[1] == 'f' && recv_data[2] == 'f')
    {
      off_flag = 1;
    }

  printf("unicast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static const struct unicast_callbacks unicast_call = {unicast_recv};
static struct unicast_conn unicast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;
  static char BUFFER[100];
  static char myid[15];
  static char* temp;
  static uint8_t receiving_uart=0;
  static uint8_t index=0;
  static int count=0;
  static uint8_t my_id=0;


  PROCESS_EXITHANDLER(unicast_close(&unicast);)

  PROCESS_BEGIN();

  unicast_open(&unicast, 146, &unicast_call);

  etimer_set(&et, CLOCK_SECOND);
  my_id = linkaddr_node_addr.u8[0];

  while(1) {
    linkaddr_t addr;
    /* Delay 2-4 seconds */
    /* etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4)); */
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER) {
      if(receiving_uart == 2) {
	receiving_uart = 0;
	etimer_set(&et, CLOCK_SECOND * 10 * my_id + random_rand() % CLOCK_SECOND); // Random backoff
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	packetbuf_clear();
	packetbuf_copyfrom(BUFFER, 100);
#if	SINGLEHOP_MODE
	addr.u8[0] = 30;
	addr.u8[1] = 0;
	unicast_send(&unicast, &addr);
#else
	addr.u8[0] = 24;
	addr.u8[1] = 0;
	/* packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,&addr); */
	unicast_send(&unicast, &addr);

	etimer_set(&et, CLOCK_SECOND + random_rand() % CLOCK_SECOND); // Random backoff
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	packetbuf_clear();
	packetbuf_copyfrom(BUFFER, 100);
	addr.u8[0] = 27;
	addr.u8[1] = 0;
	/* packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,&addr); */
	unicast_send(&unicast, &addr);
#endif

	off_flag = 0;
	MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(OUT) &= ~BV(MSP430_GPIOEN_PIN);
	printf("GPIO pin Low\n");

      }
    }
    if(ev == serial_line_event_message) {
      printf("UART input %s\n",(char *)data);
      temp=(char *)data;
      if(temp[0]=='S' && temp[1]=='k' && temp[2]=='e' && temp[3]=='w') {
	memset(BUFFER,0,100);
	index = 0;
      	receiving_uart = 1;
      	sprintf(myid,"My ID:%2d ",my_id);
      	memcpy(BUFFER,myid,strlen(myid));
      	index += strlen(myid);
      	memcpy(BUFFER+index,(char *)data,strlen(data));
      	index += strlen(data);
      	//	BUFFER[0]='X';
      }
      else if((temp[0]=='D' && temp[1]=='a' && temp[2]=='m' && temp[3]=='a') ||
	      (temp[0]=='N' && temp[1]=='o' && temp[2]=='n')) {
      	memcpy(BUFFER+index,(char *)data,strlen(data));
      	index += strlen(data);
      	receiving_uart = 2;
	sensing_flag = 0;
      }
      else {
      	memcpy(BUFFER+index,(char *)data,strlen(data));
      	index += strlen(data);
      	//	receiving_uart++;
      	//	BUFFER[1]='Y';
      }
      /* Send it to the base station */


      /* unicast_send(&uc, &addr);  */
    }
    if(sensing_flag == 1) { // Do sensing
        sensing_flag = 2;
      	printf("GPIO pin High\n");
	MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN);
	MSP430_GPIOEN_PORT(OUT) |= BV(MSP430_GPIOEN_PIN);
	printf("wait for a second\n");
	/* Delay 1 second for power on */
	etimer_set(&et, CLOCK_SECOND);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	printf("put start to uart(#2)\n");
	putchar2('s'); // -> using different uart(#2)
	putchar2('t'); 
	putchar2('a'); 
	putchar2('r'); 
	putchar2('t'); 
    }
    if(off_flag == 1) { // Turn off the sensor
      off_flag = 0;
      MSP430_GPIOEN_PORT(SEL) &= ~BV(MSP430_GPIOEN_PIN);
      MSP430_GPIOEN_PORT(DIR) |= BV(MSP430_GPIOEN_PIN);
      MSP430_GPIOEN_PORT(OUT) &= ~BV(MSP430_GPIOEN_PIN);
      printf("GPIO pin Low\n");
    }
    etimer_set(&et, CLOCK_SECOND/1000);
    /* PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
