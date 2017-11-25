#ifndef MAIN_H

// Wifi Details
const char *ssid = "duck";
const char *password = "A45hftP90Y";

const String thing_id = "thing-01";

// MQTT Details
char *mqtt_server = "m14.cloudmqtt.com";
char *mqtt_server_client_id = "esp8266-device-01";
char *mqtt_server_user = "xrtdjyzz";
char *mqtt_server_pass = "np5yUl4nBEBh";
int mqtt_server_port = 18583;

char *mqtt_thing_topic_pub = "accelerometer_out";
char *mqtt_thing_topic_sub = "feedback_in";

#endif
