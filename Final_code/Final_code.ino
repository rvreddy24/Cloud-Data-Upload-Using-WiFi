#include <WiFi.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>

#define DHTPIN 14          // Pin where the DHT11 data pin is connected
#define DHTTYPE DHT11      // DHT 11

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
const char* ssid = "Raj Varshith Reddy";         // Your Wi-Fi SSID
const char* password = "rvreddy@16"; // Your Wi-Fi Password
const char* serverUrl = "http://172.20.10.2:8888/post-data"; // Replace with your Flask server's IP

void setup() {
    Serial.begin(115200);
    dht.begin();  // Start the DHT sensor
    connectToWiFi(); // Connect to Wi-Fi
    configTime(0, 0, "pool.ntp.org"); // Set the timezone to UTC, adjust if necessary
}

void loop() {
    float temperature = dht.readTemperature(); // Read temperature
    float humidity = dht.readHumidity();       // Read humidity

    // Check if any reads failed and exit early
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Format data into JSON
    String jsonData = createJsonData(temperature, humidity);

    // Print the JSON data for debugging
    Serial.println(jsonData);

    // Send the data to the server
    sendDataToServer(jsonData);

    delay(10000); // Delay between readings (10 seconds)
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

String createJsonData(float temperature, float humidity) {
    StaticJsonDocument<200> doc;
    doc["team_number"] = "7"; // Your team number
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;

    // Get current time in seconds since epoch
    time_t now = time(nullptr);
    doc["timestamp"] = now; // This will give you a 10-digit timestamp

    String jsonData;
    serializeJson(doc, jsonData);
    return jsonData;
}

void sendDataToServer(String jsonData) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl); // Specify destination for HTTP request
        http.addHeader("Content-Type", "application/json"); // Specify content-type header as JSON

        // Send the request
        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode > 0) {
            String response = http.getString(); // Get response payload
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
            Serial.println("Response: " + response);
        } else {
            Serial.printf("Error in HTTP request: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end(); // Free resources
    } else {
        Serial.println("WiFi Disconnected");
    }
}
