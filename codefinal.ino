#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Wi-Fi credentials
const char* ssid = "TEST_WIFI";
const char* password = "12345678";

// Firebase credentials
#define API_KEY "AIzaSyBl-0kLLVg55sg2Jc5b_GjMc-5YiuQ51eo"
#define DATABASE_URL "https://html-2d983-default-rtdb.firebaseio.com/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Relay pins and state
const int relayPins[8] = {2, 4, 5, 18, 19, 21, 22, 23};
bool relayState[8] = {false, false, false, false, false, false, false, false};

// Connect to Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  int wifi_retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_retry++;
    if (wifi_retry > 30) {
      Serial.println("\n❌ Wi-Fi connection failed!");
      ESP.restart();
    }
  }
  Serial.println("\n✅ Wi-Fi connected: " + WiFi.localIP().toString());
}

// Connect to Firebase
void connectToFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = "c.soumya150@gmail.com";
  auth.user.password = "soumya123";

  fbdo.setBSSLBufferSize(4096, 1024); // Optional: increases SSL buffer
  Firebase.reconnectWiFi(true);

  Firebase.begin(&config, &auth);

  Serial.print("⏳ Waiting for Firebase connection");
  int retryCount = 0;
  const int maxRetries = 20;
  while (!Firebase.ready() && retryCount++ < maxRetries) {
    delay(500);
    Serial.print(".");
  }

  if (Firebase.ready()) {
    Serial.println("\n✅ Firebase initialized successfully");
    Serial.print("📛 Firebase UID: ");
    Serial.println(auth.token.uid.c_str());
  } else {
    Serial.println("\n❌ Firebase initialization timed out");
    ESP.restart();
  }
}

// Initialize relay pins
void initializeRelays() {
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Relays OFF (assuming LOW is ON)
  }
  Serial.println("✅ Relays initialized");
}

// Check relay state from Firebase
void checkRelay(int index, const String &path) {
  if (Firebase.RTDB.getBool(&fbdo, path)) {
    bool state = fbdo.boolData();
    if (relayState[index] != state) {
      relayState[index] = state;
      digitalWrite(relayPins[index], state ? LOW : HIGH);
      Serial.println("Relay " + String(index + 1) + " turned " + (state ? "ON" : "OFF"));
    }
  } else {
    Serial.println("❌ Failed to read " + path + ": " + fbdo.errorReason());
  }
}

// Loop through all relays
void loopRelays() {
  for (int i = 0; i < 8; i++) {
    checkRelay(i, "/devices/relay" + String(i + 1));
  }
}

// Main setup
void setup() {
  Serial.begin(115200);
  connectToWiFi();
  connectToFirebase();
  initializeRelays();
}

// Main loop
void loop() {
  loopRelays();
  delay(1000);
}
