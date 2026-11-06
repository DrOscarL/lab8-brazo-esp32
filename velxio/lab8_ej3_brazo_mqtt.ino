/*
 * Laboratorio 8 - Ejercicio 3
 * Teleoperación de un manipulador robótico 3-GDL desde un ESP32 simulado en Velxio
 *
 * - 3 potenciómetros fijan los ángulos articulares q1 (base), q2 (hombro), q3 (codo).
 * - El ESP32 calcula la cinemática directa en DOUBLE (posición x, y, z del efector).
 * - Lleva un contador de mensajes y una marca de tiempo en LONG LONG (64 bits).
 * - Publica el estado por MQTT; la página web (GitHub Pages) mueve el brazo 3D.
 * - Muestra ángulos y posición en un LCD 20x4 I2C.
 *
 * Conexiones:
 *   Pot q1 -> GPIO34   Pot q2 -> GPIO35   Pot q3 -> GPIO32   (extremos a 3V3 y GND)
 *   LCD I2C: SDA -> GPIO21, SCL -> GPIO22, VCC -> 5V/3V3, GND -> GND (dirección 0x27)
 *   IMPORTANTE: con WiFi activo solo se pueden usar los pines del ADC1 (GPIO32-39).
 *
 * Librerías: PubSubClient (Nick O'Leary), LiquidCrystal I2C (Frank de Brabander)
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include "esp_timer.h"

// ------------------- CONFIGURACIÓN DEL GRUPO -------------------
// Cada grupo debe usar un identificador distinto (sin espacios ni tildes).
const char* GRUPO = "grupo1";
// ---------------------------------------------------------------

const char* WIFI_SSID   = "Velxio-GUEST";        // AP abierto que emula Velxio
const char* MQTT_BROKER = "broker.hivemq.com";   // broker público
const uint16_t MQTT_PORT = 1883;

// Pines de los potenciómetros (ADC1)
const int PIN_Q1 = 34;
const int PIN_Q2 = 35;
const int PIN_Q3 = 32;

// Parámetros geométricos del manipulador [m]
const double L1 = 0.10;   // altura de la base al hombro
const double L2 = 0.12;   // eslabón del brazo
const double L3 = 0.10;   // eslabón del antebrazo

// Rangos articulares [grados]
const double Q1_MIN = -90.0,  Q1_MAX = 90.0;
const double Q2_MIN =   0.0,  Q2_MAX = 180.0;
const double Q3_MIN = -135.0, Q3_MAX = 135.0;

const unsigned long PERIODO_MS = 100;   // 10 Hz

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);

char topicEstado[64];
char topicCmd[64];

unsigned long long contador = 0;   // contador de mensajes de 64 bits
unsigned long tAnterior = 0;

// Convierte la lectura del ADC (0..4095) a un ángulo en el rango [qmin, qmax]
double adcAGrados(int pin, double qmin, double qmax) {
  int raw = analogRead(pin);
  return qmin + (qmax - qmin) * (double)raw / 4095.0;
}

// Mensajes recibidos desde la página web (topic .../cmd)
void alRecibir(char* topic, byte* payload, unsigned int len) {
  char msg[21];
  unsigned int n = len < 20 ? len : 20;
  memcpy(msg, payload, n);
  msg[n] = '\0';
  Serial.printf("CMD recibido: %s\n", msg);
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print(msg);
}

void conectarWiFi() {
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.printf("\nWiFi OK, IP %s\n", WiFi.localIP().toString().c_str());
}

void conectarMQTT() {
  while (!mqtt.connected()) {
    char clientId[40];
    snprintf(clientId, sizeof(clientId), "esp32-%s-%04X", GRUPO, (unsigned)random(0x10000));
    Serial.printf("Conectando MQTT como %s ... ", clientId);
    if (mqtt.connect(clientId)) {
      Serial.println("OK");
      mqtt.subscribe(topicCmd);
    } else {
      Serial.printf("fallo (rc=%d), reintento\n", mqtt.state());
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  snprintf(topicEstado, sizeof(topicEstado), "udla/lab8/%s/brazo/estado", GRUPO);
  snprintf(topicCmd,    sizeof(topicCmd),    "udla/lab8/%s/brazo/cmd",    GRUPO);

  Serial.printf("sizeof(long long) = %u bytes, sizeof(double) = %u bytes\n",
                (unsigned)sizeof(long long), (unsigned)sizeof(double));

  conectarWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(alRecibir);
  mqtt.setBufferSize(512);
  conectarMQTT();

  lcd.clear();
  Serial.printf("Publicando en: %s\n", topicEstado);
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  unsigned long ahora = millis();
  if (ahora - tAnterior < PERIODO_MS) return;
  tAnterior = ahora;

  // 1) Lectura de ángulos articulares [grados]
  double q1 = adcAGrados(PIN_Q1, Q1_MIN, Q1_MAX);
  double q2 = adcAGrados(PIN_Q2, Q2_MIN, Q2_MAX);
  double q3 = adcAGrados(PIN_Q3, Q3_MIN, Q3_MAX);

  // 2) Cinemática directa en double
  const double DEG2RAD = M_PI / 180.0;
  double t1 = q1 * DEG2RAD, t2 = q2 * DEG2RAD, t3 = q3 * DEG2RAD;
  double r = L2 * cos(t2) + L3 * cos(t2 + t3);   // alcance horizontal
  double x = r * cos(t1);
  double y = r * sin(t1);
  double z = L1 + L2 * sin(t2) + L3 * sin(t2 + t3);

  // 3) Datos de 64 bits
  contador++;
  long long t_us = esp_timer_get_time();         // microsegundos desde el arranque (int64_t)

  // 4) Publicación MQTT en JSON.
  //    El contador y el tiempo van como texto: JavaScript solo representa enteros
  //    exactos hasta 2^53 - 1, mientras que long long llega a 2^63 - 1.
  char json[320];
  snprintf(json, sizeof(json),
           "{\"q\":[%.2f,%.2f,%.2f],\"p\":[%.10f,%.10f,%.10f],"
           "\"n\":\"%llu\",\"t_us\":\"%lld\"}",
           q1, q2, q3, x, y, z, contador, t_us);
  mqtt.publish(topicEstado, json);

  // 5) Monitor serial y LCD
  Serial.printf("#%llu q=(%.1f, %.1f, %.1f) p=(%.6f, %.6f, %.6f) m\n",
                contador, q1, q2, q3, x, y, z);

  char linea[21];
  snprintf(linea, sizeof(linea), "q %4.0f %4.0f %4.0f  ", q1, q2, q3);
  lcd.setCursor(0, 0); lcd.print(linea);
  snprintf(linea, sizeof(linea), "x%+.3f y%+.3f   ", x, y);
  lcd.setCursor(0, 1); lcd.print(linea);
  snprintf(linea, sizeof(linea), "z%+.3f m         ", z);
  lcd.setCursor(0, 2); lcd.print(linea);
}
