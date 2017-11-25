#ifndef MAIN_H

// Wifi Details
const char *ssid = "YourWifiSSID";
const char *password = "YourWifiPassword";

const String thing_id = "YourThingID";

// MQTT Details
char *mqtt_server = "m14.cloudmqtt.com";
char *mqtt_server_client_id = "esp8266-device-01";
char *mqtt_server_user = "CloudMQTTUser";
char *mqtt_server_pass = "CloudMQTTPass";
int mqtt_server_port = 18583;

char *mqtt_thing_topic_pub = "accelerometer_out";
char *mqtt_thing_topic_sub = "feedback_in";

#endif
