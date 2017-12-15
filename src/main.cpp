/********************************************/
/*                 Imports                  */
/********************************************/
#include <Arduino.h>

// Accelerometer
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

// Wifi and NTP Support
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// MQTT
#include <PubSubClient.h>

// JSON Support
#include <ArduinoJson.h>

// FFT and Filtering
#include <Filters.h>
#include <arduinoFFT.h>

// Local Settings
#include <main.h>

// FFT and Filtering definitions
#define Nbins 32
#define FILTER_FREQUENCY 10   // filter 10Hz and higher
#define SAMPLES_PER_SECOND 64 //sample at 30Hz - Needs to be minimum 2x higher than desired filterFrequency

// MQTT Packet Size override
#ifdef MQTT_MAX_PACKET_SIZE       // if the macro MQTT_MAX_PACKET_SIZE is defined
#undef MQTT_MAX_PACKET_SIZE       // un-define it
#define MQTT_MAX_PACKET_SIZE 1024 // override size
#endif

/********************************************/
/*                 Globals                  */
/********************************************/
// Accelerometer MMA8451
Adafruit_MMA8451 mma = Adafruit_MMA8451();

// FFT
arduinoFFT FFT = arduinoFFT();

// Accelerometer threshold
float energy_thresh = 15.0f;

WiFiClient esp_wifi_client;
PubSubClient mqtt_client(esp_wifi_client);

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "au.pool.ntp.org", 3600, 60000);

// Misc Values (Filtering)
double vReal[Nbins];
double vImag[Nbins];
float delayTime;
float accl_mag;
int lastTime = 0;

// Speaker PIN
int speakerOut = 15;

FilterOnePole filterX(LOWPASS, FILTER_FREQUENCY);
FilterOnePole filterY(LOWPASS, FILTER_FREQUENCY);
FilterOnePole filterZ(LOWPASS, FILTER_FREQUENCY);

// Misc Values (WiFi & MQTT)
int msgCount = 0, msgReceived = 0;
bool realtime = false;
char rcvdPayload[512];

double totalEnergy(double array[])
{
    int i;
    double integrate = 0;
    for (i = 2; i < FILTER_FREQUENCY * 2; i += 2)
    {
        //taking basic numerical value for integration, multiplying bin frequency width by its height.
        integrate += array[i] * 1;
    }
    return (integrate);
}

float filteredMagnitude(float ax, float ay, float az)
{
    filterX.input(ax);
    filterY.input(ay);
    filterZ.input(az);
    return (sqrt(pow(filterX.output(), 2) + pow(filterY.output(), 2) + pow(filterZ.output(), 2)));
}

float normalMagnitude(float ax, float ay, float az)
{
    return (sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2)));
}

void callback(char *topic, byte *payLoad, unsigned int payloadLen)
{
    strncpy(rcvdPayload, reinterpret_cast<const char *>(payLoad), payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}

void beep(unsigned char delayms)
{
    analogWrite(speakerOut, 250); // Almost any value can be used except 0 and 255
                                  // experiment to get the best tone
    delay(delayms);               // wait for a delayms ms
    analogWrite(speakerOut, 0);   // 0 turns it off
    delay(delayms);               // wait for a delayms ms
}

void setup_buzzer()
{
    pinMode(speakerOut, OUTPUT);
    analogWrite(speakerOut, 0);
}

void setup_wifi()
{
    unsigned long Connection_Timeout; // Variable that measures the time it takes to connect to WiFi

    Serial.println();
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    Connection_Timeout = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - Connection_Timeout < 5000) // Gives the ESP 5 seconds to connect to a WiFi
    {
        delay(100);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("Wifi connected with IP: ");
        Serial.println(WiFi.localIP());
    }
}

void setup_ntp()
{
    timeClient.begin();
    Serial.println("ntp [Connected]");
}

void reconnect()
{
    // Loop until we're reconnected
    if (!mqtt_client.connected())
    {
        if (mqtt_client.connect(mqtt_server_client_id, mqtt_server_user, mqtt_server_pass))
        {
            Serial.println("mqtt [Connected]");
            mqtt_client.subscribe(mqtt_thing_topic_sub);
        }
        else
        {
            Serial.println("mqtt [Failed]");
            delay(3000);
        }
    }
}

void setup_mqtt()
{
    mqtt_client.setServer(mqtt_server, mqtt_server_port);
    mqtt_client.setCallback(callback);
    reconnect();
}

void setup_accl()
{
    // NTP Update
    timeClient.update();

    if (!mma.begin())
    {
        Serial.println("accl [Failed]");
        while (1)
            ;
    }
    Serial.println("accl [Connected]");
    mma.setRange(MMA8451_RANGE_2_G);
}

void setup()
{
    Serial.begin(9600);
    delay(1000);

    setup_buzzer();
    setup_wifi();
    setup_ntp();
    setup_mqtt();
    setup_accl();

    delayTime = 1 / SAMPLES_PER_SECOND * 1000; //in millis

    delay(2000);
}

void loop()
{
    if (!mqtt_client.connected())
    {
        reconnect();
    }
    mqtt_client.loop();

    sensors_event_t event;

    //run it at 30 samples/s
    for (int ii = 0; ii <= Nbins; ii++)
    {
        // delay
        if (millis() < lastTime + delayTime)
        {
            delay(lastTime + delayTime - millis());
        }

        // Read the 'raw' data in 14-bit counts
        // Get a new sensor event
        mma.read();
        mma.getEvent(&event);

        // Magnitude of values
        vReal[ii] = filteredMagnitude(event.acceleration.x, event.acceleration.y, event.acceleration.z);
        vImag[ii] = 0;
        lastTime = millis();
    }

    FFT.Windowing(vReal, Nbins, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, Nbins, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, Nbins);
    accl_mag = normalMagnitude(event.acceleration.x, event.acceleration.y, event.acceleration.z);

    if (msgReceived == 1)
    {
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
        realtime = !realtime;
    }
    //since we've calculated the frequency, check the ranges we care about (1hz-8hz)
    //sum them and check if they're higher than our threshold.
    if (totalEnergy(vReal) >= energy_thresh || realtime)
    {
        // JSON buffer
        const size_t bufferSize = /*JSON_ARRAY_SIZE(15) + */ JSON_OBJECT_SIZE(2) + 20;
        DynamicJsonBuffer jsonBuffer(bufferSize);

        JsonObject &root = jsonBuffer.createObject();
        root["timestamp"] = timeClient.getEpochTime();
        root["accl_mag"] = accl_mag;

        /*
        JsonArray &accl_fft_data = root.createNestedArray("accl_fft");
        for (int ii = 2; ii <= Nbins - 1; ii += 2)
        {
            accl_fft_data.add(vReal[ii]);
        }
        */

        String json_output;
        root.printTo(json_output);
        char payload[bufferSize];

        // Construct payload item
        json_output.toCharArray(payload, bufferSize);
        snprintf(payload, bufferSize, json_output.c_str());

        if (mqtt_client.connected())
        {
            if (mqtt_client.publish(mqtt_thing_topic_pub, payload) == 1)
            {
                Serial.print("Publish message: ");
                Serial.println(payload);

                if (totalEnergy(vReal) >= energy_thresh)
                {
                    beep(200);
                }
            }
            else
            {
                Serial.print("Publish failed: ");
                Serial.print(mqtt_client.state());
                Serial.print(" : ");
                Serial.println(payload);
            }
        }
    }
}
