
#include <WiFi.h>
#include <ESP_Mail_Client.h>

// WiFi
const char* ssid = "Galaxy M34 5G C2C6";
const char* password = "sskwifi123";

// Email SMTP
#define SMTP_HOST       "smtp.gmail.com"
#define SMTP_PORT       465
#define AUTHOR_EMAIL    "230701288@rajalakshmi.edu.in"
#define AUTHOR_PASSWORD "stjmvdmvrpztnvan"
#define RECIPIENT_EMAIL "230701288@rajalakshmi.edu.in"

// Pins
#define TRIG_PIN   2
#define ECHO_PIN   4
#define RELAY_PIN  19

SMTPSession smtp;
Session_Config config;
SMTP_Message message;
bool alertSent = false;

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long d = pulseIn(ECHO_PIN, HIGH, 30000);
  return d == 0 ? -1 : d * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  

  digitalWrite(RELAY_PIN, LOW);


  WiFi.begin(ssid, password);
  Serial.print("WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" connected!");

  // SMTP
  config.server.host_name = SMTP_HOST;
  config.server.port      = SMTP_PORT;
  config.login.email      = AUTHOR_EMAIL;
  config.login.password   = AUTHOR_PASSWORD;
  config.login.user_domain= "";

  message.sender.name    = "ESP32 Alert";
  message.sender.email   = AUTHOR_EMAIL;
  message.subject        = "⚠️ Low Water Level";
  message.addRecipient("User", RECIPIENT_EMAIL);
  message.text.content   = "Distance >20cm; motor off.";
}

void loop() {
  if (alertSent) return;

  float dist = measureDistance();
  if (dist < 0) {
    Serial.println("No echo");
  } else {
    Serial.printf("Dist: %.1f cm\n", dist);
    if (dist > 20.0) {
      Serial.println("Triggering alert");
      digitalWrite(RELAY_PIN, HIGH);
      delay(1000);

      if (!smtp.connected()) {
        if (!smtp.connect(&config)) {
          Serial.println("❌ SMTP reconnect failed");
          return;
        }
      }

      // send synchronously
      if (MailClient.sendMail(&smtp, &message)) {
        Serial.println("Email sent");
      } else {
        Serial.printf("Email err: %s\n", smtp.errorReason().c_str());
      }
      smtp.closeSession();
      alertSent = true;
    }
  }
  delay(1000);
}
