#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MQTT.h> // https://github.com/256dpi/arduino-mqtt/

EthernetClient net;
MQTTClient client;

#define MQTT_DeviceName "ArdEthernet"
#define MQTT_Key "blurp"
#define MQTT_Secret "blurp"
#define MQTT_topic_Message  "NURDspace/#"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

int relayPins[] = {
  9, 2, 3, 4, 5, 6, 7, 8
};

int pinCount = 8;
String switchNo;

String switchTopic = "NURDspace/switch/";

void connect() {
  Serial.print("MQTT connecting...");

    while (!client.connect(MQTT_DeviceName, MQTT_Key, MQTT_Secret)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT connected!");

  client.subscribe(MQTT_topic_Message);
  // client.unsubscribe(MQTT_topic_Message);
}

void MQTT_control_run()
{
  client.loop();

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  /*
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }*/

}

void messageReceived(String &topic, String &payload) {
  //Show all incoming messages
  Serial.println("incoming: " + topic + " - " + payload);
  if (topic.startsWith(switchTopic)) {
    switchNo = topic.substring(switchTopic.length());
    for (int thisPin = 1; thisPin < pinCount+1; thisPin++) {
      //only do something if topic matches number from 1 to pinCount
      if (String(thisPin) == switchNo ) {
	int relay = relayPins[thisPin-1];
	if (payload == "on") {
          digitalWrite(relay, 1);
	}
	if (payload == "off") {
          digitalWrite(relay, 0);
	}
	if (payload == "toggle") {
          digitalWrite(relay, !digitalRead(relay));
	}
      }
    }
  }
}


void setup() {
  Ethernet.init(10);

  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(relayPins[thisPin], OUTPUT);
  }
  Serial.begin(9600);

  //start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.hardwareStatus() == EthernetW5100) {
      Serial.println("W5100 Ethernet controller detected.");
    }
    else if (Ethernet.hardwareStatus() == EthernetW5200) {
      Serial.println("W5200 Ethernet controller detected.");
    }
    else if (Ethernet.hardwareStatus() == EthernetW5500) {
      Serial.println("W5500 Ethernet controller detected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print network settings
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My DNS server: ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.print("My Gateway: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("My Subnet Mask: ");
  Serial.println(Ethernet.subnetMask());
  
  client.begin("172.19.84.1",net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  MQTT_control_run();
  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;
    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      Serial.print("My DNS server: ");
      Serial.println(Ethernet.dnsServerIP());
      Serial.print("My Gateway: ");
      Serial.println(Ethernet.gatewayIP());
      Serial.print("My Subnet Mask: ");
      Serial.println(Ethernet.subnetMask());
      break;
    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;
    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      Serial.print("My DNS server: ");
      Serial.println(Ethernet.dnsServerIP());
      Serial.print("My Gateway: ");
      Serial.println(Ethernet.gatewayIP());
      Serial.print("My Subnet Mask: ");
      Serial.println(Ethernet.subnetMask());
      break;
    default:
      //nothing happened
      break;
  }
}
