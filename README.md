# Industrial IoT & Edge-AI Condition Monitoring System (CMS)

![ESP32-S3](https://img.shields.io/badge/Board-ESP32--S3-blue?style=for-the-badge&logo=espressif)
![Protocol](https://img.shields.io/badge/Protocol-MQTT-green?style=for-the-badge&logo=mqtt)
![DSP](https://img.shields.io/badge/DSP-FFT_Analysis-orange?style=for-the-badge)
![Status](https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge)

> **Target Implementation:** Predictive Maintenance Motor Induksi 3-Fasa — PT Pertamina Patra Niaga (RU II Dumai).

---

## 📌 Ringkasan Proyek
Sistem pemantauan kondisi berbasis *Edge Computing* untuk mendeteksi anomali getaran dan suhu pada motor induksi industri secara *real-time*. Menggunakan pemrosesan sinyal digital di tingkat *Edge Controller* (ESP32-S3) untuk meminimalkan beban trafik jaringan dan latensi deteksi anomali.

## 🛠️ Spesifikasi Hardware & Software
- **Microcontroller:** ESP32-S3 DevKit
- **Vibration Sensor:** MPU6050 Accelerometer/Gyroscope (I2C: SDA Pin 8, SCL Pin 9)
- **Temperature Sensor:** DS18B20 Industrial Probe (OneWire: DQ Pin 4)
- **DSP Algorithm:** Fast Fourier Transform (FFT) 500 Hz Sampling Rate
- **Telemetry Protocol:** Industrial MQTT (Port 1883) dengan Payload JSON
- **Simulation Environment:** Wokwi Simulator & MQTTX Client

## 📊 Telemetry Payload Standard
```json
{
  "device_id": "ESP32S3_MOTOR_01",
  "temperature": 42.50,
  "dominant_freq_hz": 23.50,
  "status": "HEALTHY"
}
