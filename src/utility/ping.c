	
// based on lwIP-contrib ping.c version STABLE-2_0_1_RELEASE
// http://git.savannah.gnu.org/cgit/lwip/lwip-contrib.git/plain/apps/ping/ping.c?h=STABLE-2_0_1_RELEASE

#include <lwip/arch.h>
#include "PingAlive.h"

/**
 * @file
 * Ping sender module
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * (heavily simplified)
 */

/** 
 * This is an example of a "ping" sender (with raw API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include "lwip/opt.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "ping.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/inet_chksum.h"
#include "lwip/prot/ip4.h"

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/* ping variables */

uint16_t ping_seq_num_send;
uint16_t ping_seq_num_recv;
uint8_t ping_should_stop;

static ip_addr_t ping_target;
static struct raw_pcb *ping_pcb;
//static u32_t ping_time;

/** Prepare a echo ICMP request */
static void ping_prepare_echo (struct icmp_echo_hdr *iecho)
{
  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = lwip_htons(++ping_seq_num_send);

  iecho->chksum = inet_chksum(iecho, sizeof(struct icmp_echo_hdr));
}

/* Ping using the raw ip */
static u8_t
ping_recv(void* arg, struct raw_pcb* pcb, struct pbuf* p, const ip_addr_t* addr)
{
  struct icmp_echo_hdr* iecho;
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(addr);
  LWIP_ASSERT("p != NULL", p != NULL);

  if (p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr)))
  {
    iecho = (struct icmp_echo_hdr*)((long)p->payload + PBUF_IP_HLEN);
    if (iecho->id == PING_ID)
    {
      ping_seq_num_recv = lwip_ntohs(iecho->seqno);
      pbuf_free(p);
      return 1; /* eat the packet */
    }
  }

  return 0; /* don't eat the packet */
}

static void
ping_send(struct raw_pcb *raw)
{
  struct pbuf *p;
  
  if (((u16_t)(ping_seq_num_send - ping_seq_num_recv)) > PING_MAX_LOST)
    PING_FAULT();

  p = pbuf_alloc(PBUF_IP, sizeof(struct icmp_echo_hdr), PBUF_RAM);
  if (!p)
    return;

  if ((p->len == p->tot_len) && (p->next == NULL))
  {
    ping_prepare_echo((struct icmp_echo_hdr*)p->payload);
    raw_sendto(raw, p, &ping_target);
    //ping_time = sys_now();
  }
  pbuf_free(p);
}

static void ping_clock (void* arg)
{
  struct raw_pcb* pcb = (struct raw_pcb*)arg;
  
  if (ping_should_stop)
  {
    ping_should_stop = 0;
    ping_seq_num_send = ping_seq_num_recv = 0;
    return;
  }
    
  ping_send(pcb);
  sys_timeout(PING_DELAY, ping_clock, pcb);
}

int ping_init(const ip_addr_t* ping_addr)
{
  ip_addr_copy(ping_target, *ping_addr);
  if ((ping_pcb = raw_new(IP_PROTO_ICMP)))
  {
    raw_recv(ping_pcb, ping_recv, NULL);
    if (raw_bind(ping_pcb, IP_ADDR_ANY) == ERR_OK)
    {
      sys_timeout(PING_DELAY, ping_clock, ping_pcb);
      return 1;
    }
  }
  return 0;
}

#endif /* LWIP_RAW */
