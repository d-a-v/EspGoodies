
#ifndef __PING_H
#define __PING_H

// simple C api for ping-alive

/////////////////////
// user configurable

#define PING_MAX_FAILED_MN  5    // minutes
#define PING_DELAY          5000 // milliseconds
extern void pingFault (void);    // to de defined by user

/////////////////////

#include <lwip/ip_addr.h>

// informative variables
extern uint16_t ping_seq_num_send;
extern uint16_t ping_seq_num_recv;

// set this to 1 to stop ping (will be stopped when it reads 0)
extern uint8_t ping_should_stop;

/////////////////////
// internal config

#define PING_ID             0x8266
#define PING_MAX_LOST       (((PING_MAX_FAILED_MN) * 60000) / (PING_DELAY))
#define PING_FAULT()        do { pingFault(); } while (0)

int ping_init (const ip_addr_t* ping_addr);

inline int start_pingalive (uint32_t ipv4)
{
    ip_addr_t addr;
    ip_addr_set_ip4_u32(&addr, ipv4);
    return ping_init(&addr);
}

#endif // __PING_H
