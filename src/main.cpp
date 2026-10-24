#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // Jika perlu parsing JSON, atau gunakan String manual
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <arduinoFFT.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define ONE_WIRE_BUS 4

#define SAMPLES 64
#define SAMPLING_FREQUENCY 500 

// Konfigurasi WiFi Virtual & MQTT Broker
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic = "pertamina/ru2_dumai/motor_01/telemetry";

WiFiClient espClient;
PubSubClient client(espClient);

double vReal[SAMPLES];
double vImag[SAMPLES];

Adafruit_MPU6050 mpu;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

void setupWiFi() {
  Serial.print("[WiFi] Menghubungkan ke Wokwi-GUEST");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Terhubung! IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("[MQTT] Menghubungkan ke Broker HiveMQ...");
    String clientId = "ESP32S3_CMS_Client_" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" Terhubung!");
    } else {
      Serial.print(" Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!mpu.begin()) {
    Serial.println("[-] MPU6050 Gagal!");
    while (1) delay(10);
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  sensors.begin();

  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // 1. Baca Suhu & MPU6050
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float simulatedFreq = map(abs(a.acceleration.x) * 10, 0, 160, 10, 100);

  // 2. Generate Sinyal & FFT
  for (int i = 0; i < SAMPLES; i++) {
    double t = (double)i / SAMPLING_FREQUENCY;
    vReal[i] = sin(2 * PI * simulatedFreq * t) * 5.0; 
    vImag[i] = 0.0;
    delayMicroseconds(1000000 / SAMPLING_FREQUENCY);
  }

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  double peakFrequency = FFT.majorPeak();

  // 3. Edge-AI Decision Rules (Prediksi Kondisi Mesin)
  String healthStatus = "HEALTHY";
  if (tempC > 60.0 || peakFrequency > 70.0) {
    healthStatus = "CRITICAL_ANOMALY";
  } else if (tempC > 40.0 || peakFrequency > 45.0) {
    healthStatus = "WARNING_UNBALANCE";
  }

  // 4. Konstruksi Payload JSON
  String payload = "{";
  payload += "\"device_id\":\"ESP32S3_MOTOR_01\",";
  payload += "\"temperature\":" + String(tempC, 2) + ",";
  payload += "\"dominant_freq_hz\":" + String(peakFrequency, 2) + ",";
  payload += "\"status\":\"" + healthStatus + "\"";
  payload += "}";

  // 5. Publish ke MQTT Broker
  Serial.println("[MQTT PUBLISH] " + payload);
  client.publish(mqtt_topic, payload.c_str());

  delay(2000);
}
