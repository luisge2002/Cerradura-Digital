#define BLYNK_TEMPLATE_ID "TMPL25qKg6Zdj"
#define BLYNK_TEMPLATE_NAME "Cerradura Digital"
#define BLYNK_AUTH_TOKEN "AxVwuosmGCFsE3hh288Q084qNwZ9Famp"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include "PCF8574KeypadLite.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// =====================
// TECLADO PCF8574
// =====================
const uint8_t KEYPAD_ADDRESS = 0x27;
PCF8574KeypadLite teclado(KEYPAD_ADDRESS);

// =====================
// WIFI
// =====================
const char* ssid = "Laptop Luis Gerardo";
const char* password = "HolaMundo";unsigned long ultimoIntentoWiFi = 0;
unsigned long ultimoIntentoBlynk = 0;
const unsigned long INTERVALO_WIFI = 10000;   // cada 10 segundos
const unsigned long INTERVALO_BLYNK = 15000;  // cada 15 segundos

// =====================
// SERVIDOR LOCAL EN PC
// =====================
// Cambia esta IP por la IP real de tu PC.
// Ejemplo: 192.168.1.50 o 172.20.10.2
const char* SERVER_IP = "172.20.10.2";
const int SERVER_PORT = 5000;

// =====================
// CLAVE
// =====================
const String CLAVE_CORRECTA = "123456";
String claveIngresada = "";

// =====================
// PINES
// =====================
const byte RELAY_PIN = 14;     // D5 / GPIO14 - Relé activo en LOW
const byte BUZZER_PIN = 13;    // D7 / GPIO13 - Buzzer activo en LOW
const byte LED_KEY_PIN = 12;   // D6 / GPIO12 - LED tecla presionada

// =====================
// TIEMPOS
// =====================
const unsigned long TIEMPO_LED_TECLA = 80;
const unsigned long TIEMPO_APERTURA = 2000;

// =====================
// VARIABLES DE CONTROL
// =====================
bool abrirDesdeBlynk = false;
bool releActivo = false;

unsigned long tiempoInicioRele = 0;

// Avisos pendientes al servidor
bool avisoPuertaAbiertaPendiente = false;
bool avisoClaveIncorrectaPendiente = false;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println();
  Serial.println("Iniciando cerradura digital...");

  // =====================
  // I2C / PCF8574
  // =====================
  Wire.begin(4, 5);  // SDA = GPIO4 / D2, SCL = GPIO5 / D1

  if (!teclado.begin()) {
    Serial.println("No se detecta el PCF8574");
    Serial.println("Revisa direccion I2C, VCC, GND, SDA y SCL.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("PCF8574 detectado correctamente");

  // =====================
  // PINES DE SALIDA
  // =====================
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_KEY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);     // Relé apagado
  digitalWrite(BUZZER_PIN, HIGH);    // Buzzer apagado
  digitalWrite(LED_KEY_PIN, LOW);    // LED apagado

  // =====================
  // WIFI
  // =====================
  Serial.println("Conectando al WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  unsigned long inicioWiFi = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - inicioWiFi < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi conectado");
    Serial.print("IP del ESP8266: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("No se pudo conectar al WiFi al iniciar");
  }

  // =====================
  // BLYNK
  // =====================
  Blynk.config(BLYNK_AUTH_TOKEN);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectando a Blynk...");

    if (Blynk.connect(5000)) {
      Serial.println("Blynk conectado correctamente");
    } else {
      Serial.println("No se pudo conectar a Blynk");
    }
  }

  Serial.println("Sistema listo");
  Serial.println("Ingrese clave de 6 digitos:");
}

void loop() {
  actualizarRele();

  mantenerWiFi();
  mantenerBlynk();

  procesarAperturaBlynk();
  procesarAvisosServidor();
  procesarTeclado();
}

// =====================
// PROCESAR TECLADO
// =====================
void procesarTeclado() {
  char tecla = teclado.getKey();

  if (!tecla) {
    return;
  }

  parpadearLedTecla();

  Serial.print("Tecla: ");
  Serial.println(tecla);

  if (tecla == '*') {
    claveIngresada = "";
    Serial.println("Clave borrada");
    sonarBuzzerError();
    return;
  }

  if (tecla < '0' || tecla > '9') {
    return;
  }

  claveIngresada += tecla;

  Serial.print("Clave ingresada: ");
  Serial.println(claveIngresada);

  if (claveIngresada.length() == 6) {
    if (claveIngresada == CLAVE_CORRECTA) {
      Serial.println("Clave correcta. Abriendo puerta...");
      abrirPuerta();
    } else {
      Serial.println("Clave incorrecta");
      sonarBuzzerError();
      avisoClaveIncorrectaPendiente = true;
    }

    claveIngresada = "";
    Serial.println("Ingrese clave nuevamente:");
  }
}

// =====================
// APERTURA DESDE BLYNK
// =====================
void procesarAperturaBlynk() {
  if (!abrirDesdeBlynk) {
    return;
  }

  abrirDesdeBlynk = false;

  Serial.println("Apertura remota desde Blynk");
  abrirPuerta();

  Blynk.virtualWrite(V0, 0);
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();

  Serial.print("Valor recibido desde Blynk V0: ");
  Serial.println(pinValue);

  if (pinValue == 1 && !releActivo) {
    abrirDesdeBlynk = true;
  }
}

// =====================
// RELÉ
// =====================
void abrirPuerta() {
  if (releActivo) {
    Serial.println("Relé ya está activo. Se ignora nueva apertura.");
    return;
  }

  Serial.println("Relé activado");
  Serial.print("Inicio: ");
  Serial.println(millis());

  digitalWrite(RELAY_PIN, LOW);   // Activa relé
  tiempoInicioRele = millis();
  releActivo = true;
}

void actualizarRele() {
  if (releActivo && millis() - tiempoInicioRele >= TIEMPO_APERTURA) {
    digitalWrite(RELAY_PIN, HIGH);   // Apaga relé
    releActivo = false;

    avisoPuertaAbiertaPendiente = true;

    Serial.println("Relé apagado");
    Serial.print("Fin: ");
    Serial.println(millis());
  }
}

// =====================
// AVISOS AL SERVIDOR LOCAL
// =====================
void procesarAvisosServidor() {
  if (releActivo) {
    return;
  }

  if (avisoPuertaAbiertaPendiente) {
    avisoPuertaAbiertaPendiente = false;
    avisarServidor("/puerta-abierta");
  }

  if (avisoClaveIncorrectaPendiente) {
    avisoClaveIncorrectaPendiente = false;
    avisarServidor("/clave-incorrecta");
  }
}

void avisarServidor(String ruta) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. No se puede avisar al servidor.");
    return;
  }

  WiFiClient httpClient;
  HTTPClient http;

  String url = "http://";
  url += SERVER_IP;
  url += ":";
  url += SERVER_PORT;
  url += ruta;

  Serial.print("Avisando al servidor: ");
  Serial.println(url);

  http.setTimeout(1500);
  http.begin(httpClient, url);

  int httpCode = http.GET();

  Serial.print("Respuesta HTTP: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    Serial.println("Servidor recibió el aviso correctamente");
  } else {
    Serial.println("Error al avisar al servidor");
  }

  http.end();
}

// =====================
// BUZZER
// =====================
void sonarBuzzerError() {
  digitalWrite(BUZZER_PIN, LOW);
  delay(120);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);

  digitalWrite(BUZZER_PIN, LOW);
  delay(120);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);

  digitalWrite(BUZZER_PIN, LOW);
  delay(400);
  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println("Pulso de buzzer finalizado");
}

// =====================
// LED DE TECLA
// =====================
void parpadearLedTecla() {
  digitalWrite(LED_KEY_PIN, HIGH);
  delay(TIEMPO_LED_TECLA);
  digitalWrite(LED_KEY_PIN, LOW);
}

void mantenerWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if (millis() - ultimoIntentoWiFi >= INTERVALO_WIFI) {
    ultimoIntentoWiFi = millis();

    Serial.println("WiFi desconectado. Intentando reconectar...");

    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
  }
}

void mantenerBlynk() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (Blynk.connected()) {
    Blynk.run();
    return;
  }

  if (millis() - ultimoIntentoBlynk >= INTERVALO_BLYNK) {
    ultimoIntentoBlynk = millis();

    Serial.println("Blynk desconectado. Intentando reconectar...");

    if (Blynk.connect(3000)) {
      Serial.println("Blynk reconectado correctamente");
    } else {
      Serial.println("No se pudo reconectar a Blynk");
    }
  }
}