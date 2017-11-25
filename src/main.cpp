/********************************************/
/*                 Imports                  */
/********************************************/
#include <Arduino.h>

// Accelerometer
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

// Wifi, MQTT and NTP Support
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUDP.h>
#include <NTPClient.h>

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

#define MQTT_MAX_PACKET_SIZE 255 // override size

/********************************************/
/*                 Globals                  */
/********************************************/
// Accelerometer MMA8451
Adafruit_MMA8451 mma = Adafruit_MMA8451();

// FFT
arduinoFFT FFT = arduinoFFT();

// Accelerometer threshold
float energy_thresh = 15.0f;

WiFiClient espClient;
PubSubClient client(espClient);

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "au.pool.ntp.org", 3600, 60000);

// Misc Values (Filtering)
double vReal[Nbins];
double vImag[Nbins];
float delayTime;
float accl_mag;
int lastTime = 0;

FilterOnePole filterX(LOWPASS, FILTER_FREQUENCY);
FilterOnePole filterY(LOWPASS, FILTER_FREQUENCY);
FilterOnePole filterZ(LOWPASS, FILTER_FREQUENCY);

// Misc Values (WiFi & MQTT)
int status = WL_IDLE_STATUS;
int msgCount = 0, msgReceived = 0;
bool realtime = false;
char payload[512];
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

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup_ntp()
{
    timeClient.begin();
    Serial.println("ntp [Connected]");
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(mqtt_server_client_id, mqtt_server_user, mqtt_server_pass))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("test/pub", "hello world");
            // ... and resubscribe
            client.subscribe(mqtt_thing_topic_sub);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup_accl()
{
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

    setup_wifi();
    setup_ntp();

    client.setServer(mqtt_server, mqtt_server_port);
    client.setCallback(callback);

    setup_accl();

    delayTime = 1 / SAMPLES_PER_SECOND * 1000; //in millis

    delay(2000);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // NTP Update
    timeClient.update();

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
        StaticJsonBuffer<180> jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();

        root["timestamp"] = timeClient.getEpochTime();
        root["accl_mag"] = accl_mag;

        JsonArray &accl_fft_data = root.createNestedArray("accl_fft");
        for (int ii = 2; ii <= Nbins - 1; ii += 2)
        {
            accl_fft_data.add(vReal[ii]);
        }

        String json_output;
        root.printTo(json_output);

        // Construct payload item
        json_output.toCharArray(payload, 180);

        if (client.connected())
        {
            client.publish("accelerometer_out", payload);
            Serial.print("Publish message: ");
            Serial.println(json_output);
        }
    }
}
