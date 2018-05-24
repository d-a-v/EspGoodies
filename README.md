
esp8266/Arduino goodies
-----------------------

* PingAlive  
  Pings the gateway every 5 seconds, and calls `pingFault()` after 5 minutes of unsuccessful pings
  * define your own `void pingFault (void)`
  * call `startPingAlive()` from `setup()` after wifi STA is connected.
  * status can be checked by reading `ping_seq_num_send` and `ping_seq_num_recv` values
  * setting `ping_should_stop` to 1 will stop ping (effectively stopped when it is reset to 0)

* NetDump (lwip2)  
  Packet sniffer library to help study network issues, check examples

* accurate TZ and DST available to your ESP with https://github.com/nayarsystems/posix_tz_db
  example: `configTZ(TZ_Asia_Shanghai);`

* `wifi_on()` / `wifi_off()` helpers

* uart loopback enabler

* hard reset checker (can't soft-reset after serial programming checker)
