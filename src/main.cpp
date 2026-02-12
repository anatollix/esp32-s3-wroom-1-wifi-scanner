#include <Arduino.h>
#include <WiFi.h>

// Built-in LED pin for ESP32-S3-WROOM-1 DevKitC
#define LED_PIN 48  // RGB LED on GPIO48

// WiFi scan interval (in milliseconds)
#define SCAN_INTERVAL 15000  // Scan every 15 seconds

// Variables
unsigned long lastScanTime = 0;
int scanCount = 0;

// Function to get encryption type as string
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2-PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-Enterprise";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3-PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3-PSK";
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI-PSK";
    default:
      return "Unknown";
  }
}

// Function to get signal strength description
String getSignalStrength(int rssi) {
  if (rssi >= -50) {
    return "Excellent";
  } else if (rssi >= -60) {
    return "Good";
  } else if (rssi >= -70) {
    return "Fair";
  } else if (rssi >= -80) {
    return "Weak";
  } else {
    return "Very Weak";
  }
}

// Function to scan and display WiFi networks
void scanWiFiNetworks() {
  Serial.println("\n========================================");
  Serial.println("       WiFi Network Scanner");
  Serial.println("========================================");
  Serial.println("Scanning for networks...");

  // LED blue during scan
  neopixelWrite(LED_PIN, 0, 0, 40);

  // Start scan (set to true for async scan)
  int networkCount = WiFi.scanNetworks(false, true);

  // LED off after scan
  neopixelWrite(LED_PIN, 0, 0, 0);

  Serial.println("Scan complete!");
  Serial.println("========================================\n");

  if (networkCount == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("Found %d network(s):\n\n", networkCount);

    // Sort by signal strength (RSSI)
    // Create array of indices
    int indices[networkCount];
    for (int i = 0; i < networkCount; i++) {
      indices[i] = i;
    }

    // Bubble sort by RSSI (strongest first)
    for (int i = 0; i < networkCount - 1; i++) {
      for (int j = 0; j < networkCount - i - 1; j++) {
        if (WiFi.RSSI(indices[j]) < WiFi.RSSI(indices[j + 1])) {
          int temp = indices[j];
          indices[j] = indices[j + 1];
          indices[j + 1] = temp;
        }
      }
    }

    // Display sorted networks
    for (int i = 0; i < networkCount; i++) {
      int idx = indices[i];

      Serial.printf("%2d: ", i + 1);
      Serial.println(WiFi.SSID(idx));
      Serial.printf("    BSSID: %s\n", WiFi.BSSIDstr(idx).c_str());
      Serial.printf("    Signal: %d dBm (%s)\n", WiFi.RSSI(idx), getSignalStrength(WiFi.RSSI(idx)).c_str());
      Serial.printf("    Channel: %d\n", WiFi.channel(idx));
      Serial.printf("    Security: %s\n", getEncryptionType(WiFi.encryptionType(idx)).c_str());
      Serial.println();
    }
  }

  // Memory info
  Serial.println("========================================");
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("Scan #%d complete\n", ++scanCount);
  Serial.println("========================================");

  // Clean up
  WiFi.scanDelete();
}

void setup() {
  // Initialize USB Serial
  Serial.begin(115200);

  // Wait for serial connection
  delay(5000);

  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("   ESP32-S3 WiFi Scanner");
  Serial.println("========================================");
  Serial.println("Board: ESP32-S3-WROOM-1");
  Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.println("========================================\n");

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("WiFi initialized in Station mode.");
  Serial.println("Ready to scan!\n");

  // Perform first scan immediately
  scanWiFiNetworks();
  lastScanTime = millis();
}

void loop() {
  // Check if it's time for another scan
  unsigned long currentTime = millis();

  if (currentTime - lastScanTime >= SCAN_INTERVAL) {
    scanWiFiNetworks();
    lastScanTime = currentTime;
  }

  // Blink LED slowly while waiting
  static unsigned long lastBlink = 0;
  if (currentTime - lastBlink >= 500) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastBlink = currentTime;
  }

  // Yield to prevent watchdog issues
  yield();
}
