#include <ESP8266WiFi.h>
#include "WIFIID.h"

const char* password = PASSWD;
const char* ssid = NETWORKNAME;
const int port = 80;

const int relay_pin = D1;
int relay_pin_state = LOW;
int prev_state = relay_pin_state;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

String header = "";

WiFiServer server(port);

void setup()
{
	Serial.begin(115200);


  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, relay_pin_state);

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
  {
		delay(500);
		Serial.print(".........");
	}

  delay(1000);

	Serial.println();
	Serial.print("Connected, IP address: ");
	Serial.println(WiFi.localIP());

	server.begin();
} // setup

void loop()
{
	WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client...");
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";

    while(client.connected() && currentTime - previousTime <= timeoutTime)
    {
      currentTime = millis();
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            if (header.indexOf("GET /16/on") >= 0)
            {
              relay_pin_state = HIGH;
            }
            else if (header.indexOf("GET /16/off") >= 0)
            {
              relay_pin_state = LOW;
            }

            if (relay_pin_state != prev_state)
            {
              prev_state = relay_pin_state;
              digitalWrite(relay_pin, relay_pin_state);
            }

            send_client_html(client);
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
    Serial.println("Client disconnected!");
    Serial.println("");
  } // if client
} // loop

void send_client_html(WiFiClient& client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");

  // CSS to style the on/off buttons
  client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: yellowgreen; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: gray;}</style></head>");

  client.println("<body><h1></h1>");

  if (relay_pin_state == HIGH)
  {
    client.println("<p><a href=\"/16/off\"><button class=\"button button2\">Switch me<br/> OFF</button></a></p>");
  }
  else
  {
    client.println("<p><a href=\"/16/on\"><button class=\"button\">Switch me<br/>ON</button></a></p>");
  }

  client.println("</body></html>");
}
