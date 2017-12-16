#include "config.h"
#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "ESP8266HTTPUpdateServer.h"
#include "PubSubClient.h"

#include "GarageDoor.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient client(espClient);
GarageDoor door1(D6,D5,D1);
GarageDoor door2(D8,D7,D2);

String create_webpage_html()
{
  String html = "<html><h1>Door 1</h2><p>";
  html += door1.is_closed() ? "closed" : "open";
  html += "</p>";
  html += "<form action=\"/door1/open\" method=\"post\" target=\"dummyframe\">";
  html += "<button type=\"submit\">Open</button>";
  html += "</form>";
  html += "<form action=\"/door1/close\" method=\"post\" target=\"dummyframe\">";
  html += "<button type=\"submit\">Close</button>";
  html += "</form>";
  html += "<h1>Door 2</h2><p>";
  html += door2.is_closed() ? "closed" : "open";
  html += "</p>";
  html += "<form action=\"/door2/open\" method=\"post\" target=\"dummyframe\">";
  html += "<button type=\"submit\">Open</button>";
  html += "</form>";
  html += "<form action=\"/door2/close\" method=\"post\" target=\"dummyframe\">";
  html += "<button type=\"submit\">Close</button>";
  html += "</form>";
  html += "<iframe width=\"0\" height=\"0\" border=\"0\" name=\"dummyframe\" id=\"dummyframe\"></iframe>";
  html += "</html>";
  return html;
}

void setup_web_server()
{
  server.on("/", HTTP_GET, [](){
    server.send(200, "text/html", create_webpage_html());
  });
  server.on("/door1/open", HTTP_POST, [](){
    door1.open();
    server.send(200,"text/html", "ok");
  });
  server.on("/door2/open", HTTP_POST, [](){
    door2.open();
    server.send(200,"text/html", "ok");
  });
  server.on("/door1/close", HTTP_POST, [](){
    door1.close();
    server.send(200,"text/html", "ok");
  });
  server.on("/door2/close", HTTP_POST, [](){
    door2.close();
    server.send(200,"text/html", "ok");
  });
  server.begin();
}

void send_garage_status() {
  client.publish(DOOR_1_STATUS_TOPIC, door1.is_closed() ? "closed" : "open", true);
  client.publish(DOOR_2_STATUS_TOPIC, door2.is_closed() ? "closed" : "open", true);
}

void handle_mqtt_message(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (strcmp(topic,DOOR_1_CONTROL_TOPIC) == 0){
    if (strcmp(message,"OPEN") == 0) {
      door1.open();
    }
    else if (strcmp(message,"CLOSE") == 0) {
      door1.close();
    }
  }
  else if (strcmp(topic, DOOR_2_CONTROL_TOPIC) == 0) {
    if (strcmp(message, "OPEN") == 0) {
      door2.open();
    }
    else if (strcmp(message, "CLOSE") == 0) {
      door2.close();
    }
  }
  else if (strcmp(topic, STATUS_REQUEST_TOPIC) == 0) {
    send_garage_status();
  }
}

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("garage")) {
      Serial.println("connected");
      client.subscribe(DOOR_2_CONTROL_TOPIC);
      client.subscribe(DOOR_1_CONTROL_TOPIC);
      client.subscribe(STATUS_REQUEST_TOPIC);
      send_garage_status();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // Setup serial output
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  setup_web_server();
  httpUpdater.setup(&server);

  Serial.println("Server started");
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  // Setup MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(handle_mqtt_message);
  reconnect_mqtt();

  door1.on_change(send_garage_status);
  door2.on_change(send_garage_status);
}

void reboot() // Restarts program from beginning but does not reset the peripherals and registers
{
  Serial.print("rebooting");
  ESP.reset(); 
}

void loop()
{
  server.handleClient();
  door1.update();
  door2.update();
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}