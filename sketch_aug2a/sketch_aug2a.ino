#include <WiFi.h>
#include <CTBot.h>

// Define el pin analógico donde está conectado el MQ-2
const int mq2Pin = 34;  // Usando GPIO 4

// Define los pines para los LEDs y el buzzer
const int ledPins[] = {22, 19, 18}; // GPIO 22, 19, 18
const int buzzerPin = 23;           // GPIO 23

// Configuración WiFi y Telegram
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define TELEGRAM_BOT_TOKEN ""
int CHAT_ID = ;

CTBot myBot;

String currentAirQuality = "No disponible";
int currentSensorValue = 0;

unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de 1 segundo

void setup() {
  // Inicializa la comunicación serie
  Serial.begin(115200);

  // Configura el pin analógico como entrada
  pinMode(mq2Pin, INPUT);

  // Configura los pines de los LEDs como salida
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Configura el pin del buzzer como salida
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Conectarse a WiFi
  Serial.println("Conectando a WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");

  // Configurar el bot de Telegram
  myBot.wifiConnect(WIFI_SSID, WIFI_PASSWORD);
  myBot.setTelegramToken(TELEGRAM_BOT_TOKEN);

  // Verificar la conexión al bot
  if (myBot.testConnection()) {
    Serial.println("Bot de Telegram conectado");
  } else {
    Serial.println("Error conectando al bot de Telegram");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Lee el valor analógico del sensor
    int sensorValue = analogRead(mq2Pin);
    currentSensorValue = sensorValue;

    // Convierte el valor a voltaje (ESP32 trabaja a 3.3V)
    float voltage = sensorValue * (3.3 / 4095.0);

    // Evalúa la calidad del aire directamente
    if (sensorValue < 1000) {
      currentAirQuality = "Buena";
    } else if (sensorValue >= 1000 && sensorValue < 1900) {
      currentAirQuality = "Moderada";
    } else {
      currentAirQuality = "Peligrosa";
    }

    // Imprime los valores en el monitor serie
   // printMemoryUsage();
    Serial.print("Sensor Value: ");
    Serial.print(sensorValue);
    Serial.print(" - Voltage: ");
    Serial.print(voltage);
    Serial.print(" - Calidad del Aire: ");
    Serial.println(currentAirQuality);

    // Controla los LEDs y el buzzer según la calidad del aire
    if (currentAirQuality == "Buena") {
      digitalWrite(ledPins[0], HIGH);  // Enciende el primer LED
      digitalWrite(ledPins[1], LOW);   // Apaga el segundo LED
      digitalWrite(ledPins[2], LOW);   // Apaga el tercer LED
      digitalWrite(buzzerPin, LOW);
      noTone(buzzerPin);               // Apaga el buzzer
    } else if (currentAirQuality == "Moderada") {
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPins[0], HIGH);  // Enciende el primer LED
      digitalWrite(ledPins[1], HIGH);  // Enciende el segundo LED
      digitalWrite(ledPins[2], LOW);   // Apaga el tercer LED
      digitalWrite(buzzerPin, LOW);           // Apaga el buzzer
    } else if (currentAirQuality == "Peligrosa") {
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPins[i], HIGH);  // Enciende todos los LEDs
      }
      digitalWrite(buzzerPin, HIGH);     // Enciende el buzzer
    }
  }

  // Verificar mensajes entrantes
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    if (msg.text.equalsIgnoreCase("calidad")) {
      sendCurrentAirQuality(msg.sender.id);
      // Serial.print("Memoria libre: ");
      // Serial.print(ESP.getFreeHeap());
      // Serial.println(" bytes");
    }
  }
}

void sendAlert(String message) {
  myBot.sendMessage(CHAT_ID, message);
}

void sendCurrentAirQuality(int64_t chat_id) {
  String message = "Calidad del Aire Actual: " + currentAirQuality + " - Valor de calidad: " + String(currentSensorValue);
  myBot.sendMessage(chat_id, message);
}

// void printMemoryUsage() {
//   Serial.print("Memoria libre: ");
//   Serial.print(ESP.getFreeHeap());
//   Serial.println(" bytes");
// }