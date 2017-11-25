#ifndef MAIN_H

// Wifi Details
const char *ssid = "YourWifiSSID";
const char *password = "YourWifiPassword";

const String thing_id = "YourThingID";

// MQTT Details
const char *mqtt_server = "m14.cloudmqtt.com";
const char *mqtt_server_client_id = "esp8266-device-01";
const char *mqtt_server_user = "CloudMQTTUser";
const char *mqtt_server_pass = "CloudMQTTPass";
const int mqtt_server_port = 18583;

const char *mqtt_thing_topic_pub = "esp8266/accelerometer_out";
const char *mqtt_thing_topic_sub = "esp8266/feedback_in";

#endif
