
#ifndef PINGALIVE_H
#define PINGALIVE_H

#ifdef __cplusplus
#include <ESP8266WiFi.h>
extern "C"
{

#include "utility/ping.h"

inline void startPingAlive (uint32_t ipv4 = 0)
{
  start_pingalive(ipv4?: (uint32_t)WiFi.gatewayIP());
}

// to be defined by user
extern void pingFault (void);

// informative variables
extern uint16_t ping_seq_num_send;
extern uint16_t ping_seq_num_recv;

// set this to 1 to stop ping
// (will be stopped when it reads 0)
extern uint8_t ping_should_stop;

} // extern "C"
#endif // __cplusplus

#endif // PINGALIVE_H
