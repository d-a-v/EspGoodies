
// pingAlive tester
// pingAlive is only about defining pingFault() and calling startPingAlive()
// this demo setups the minimum power requirement
// and installs a telnet server for testing
// and a webserver:
// The webpage self-refreshes every 20 seconds
// and show the maximum time it really takes to refresh
//
// released to public domain

#include <PingAlive.h>
#include <time.h> // time() ctime()

#define SSID "ssid"
#define PSK "psk"
#define DISPLAY_MS 10000
#define HTTP_REFRESH_SEC 20

WiFiServer hello80(80);
WiFiServer hello23(23);

unsigned long nextDisplayMs;
unsigned long maxms = 0;
time_t lastms = 0;

void pingFault ()
{
  Serial.println("gateway not responding to ping");
}

void setup() {
  Serial.begin(115200);
  Serial.println("pingAlive demo");

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP, 10); // remove ",10" for older cores
  WiFi.begin(SSID, PSK);
  Serial.println("Connecting to " SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }

  Serial.println("connected");
  Serial.print("IP address = ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway IP address = ");
  Serial.println(WiFi.gatewayIP());

  nextDisplayMs = millis() + DISPLAY_MS;

  // this line only is needed to start ping originating
  // from ESP to the gatgeway, forcing WiFi to wake up every 5 secs
  // (hardcoded in pingAlive source - should it be configurable?)
  startPingAlive();

  hello.begin();
  hello80.begin();

  configTime(0, 0, "pool.ntp.org"); // UTC
};

void loop()
{
  // user code here



  // all code below is testing and is not necessary for pingAlive to ping

  // telnet info
  if (hello23.hasClient())
  {
    time_t now = time(nullptr);
    hello23.available().println(String("UTC") + ctime(&now));
  }

  // http info
  if (hello80.hasClient())
  {
    time_t now80 = time(nullptr);
    unsigned long nowms = millis();
    unsigned long deltams = nowms - lastms;
    if (lastms && maxms < deltams)
      maxms = deltams;
    lastms = nowms;

    hello80.available().printf(R"EOF(
<meta http-equiv="refresh" content="%d"><pre>
date (UTC): %sdelta:      %d ms
delta-max:  %d ms
            (should not be more than (ping)%d + (refresh)%d = %d ms)

gateway ping stats: %d sent - %d received

will be refreshed in <span id="countdown">%d</span> seconds
<script type="text/javascript">
var seconds = %d;
function countdown()
{
  document.getElementById("countdown").innerHTML = seconds;
  window.setTimeout("countdown()", 1000);
  seconds = seconds - 1;
}
countdown();
</script>
)EOF",
                               HTTP_REFRESH_SEC,
                               ctime(&now80),
                               (int)deltams,
                               (int)maxms, PING_DELAY, HTTP_REFRESH_SEC * 1000, HTTP_REFRESH_SEC * 1000 + PING_DELAY,
                               ping_seq_num_send, ping_seq_num_recv,
                               HTTP_REFRESH_SEC,
                               HTTP_REFRESH_SEC);
  }

  // serial info
  if (millis() > nextDisplayMs)
  {
    nextDisplayMs += DISPLAY_MS;
    Serial.printf("counters: send=%5d  -  recv=%5d\n",
                  ping_seq_num_send,
                  ping_seq_num_recv);
  }
}
