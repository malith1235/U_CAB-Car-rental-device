#define TINY_GSM_RX_BUFFER 256

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Wire.h>

// I2C address of the slave device
#define SLAVE_ADDRESS 0x08

// Define custom I2C pins
#define I2C_SDA 18
#define I2C_SCL 19

int number_rec = 0;
int prev_num = 0;

const char FIREBASE_HOST[] = "u-cab-d8f61-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "TgxYqS3ns7BaGNkuaTCrZEESpIOzwdyIc7IQ0sCL";
const String FIREBASE_PATH_rem_dist = "Version1/suziki/remaining distance";
const String FIREBASE_PATH_set_dist = "Version1/suziki/setDistanceKm";
const String FIREBASE_PATH_set_flag = "Version1/suziki/setflag";
const String FIREBASE_PATH_mil_on_set = "Version1/suziki/milage on set";
const String FIREBASE_PATH_rem_disp = "Version1/suziki/disp rem";
const String FIREBASE_PATH_long = "Version1/suziki/longitude";
const String FIREBASE_PATH_lat = "Version1/suziki/latitude";
const String FIREBASE_PATH_manual_control = "Version1/suziki/manual control";
const int SSL_PORT = 443;

const char* ssid = "Dialog 4G 783";
const char* password = "8e93Ab07";

WiFiClientSecure wifiClient;
HttpClient http_client = HttpClient(wifiClient, FIREBASE_HOST, SSL_PORT);

TinyGPSPlus gps;
HardwareSerial MySerial(1);

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("Device serial initialize");

  MySerial.begin(9600, SERIAL_8N1, 22, 23);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  wifiClient.setInsecure();                       // Disable SSL certificate verification
  http_client.setHttpResponseTimeout(10 * 1000);  // 10 secs timeout

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void loop() {
  delay(500);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  http_client.connect(FIREBASE_HOST, SSL_PORT);

  while (true) {
    if (!http_client.connected()) {
      Serial.println();
      http_client.stop();  // Shutdown
      Serial.println("HTTP not connect");
      break;
    } else {
      main_loop();
      gps_1();
    }
  }
}

void PostToFirebase(const char* method, const String& path, const String& data, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive();  // Currently, this is needed for HTTPS

  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;
  Serial.print("POST:");
  Serial.println(url);
  Serial.print("Data:");
  Serial.println(data);

  String contentType = "application/json";
  http->put(url, contentType, data);

  statusCode = http->responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  if (!http->connected()) {
    Serial.println();
    http->stop();  // Shutdown
    Serial.println("HTTP POST disconnected");
  }
}

String GetFirebase(const char* method, const String& path, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive();  // Currently, this is needed for HTTPS

  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;

  http->get(url);

  response = http->responseBody();

  if (!http->connected()) {
    Serial.println();
    http->stop();  // Shutdown
    Serial.println("HTTP POST disconnected");
  }

  return response;
}

int getIntValueFromJson(String response, const char* key) {
  int value = 0;
  const char* jsonString = response.c_str();

  // Construct the search key with quotes
  char searchKey[50];
  snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);

  // Find the position of the key in the JSON string
  const char* keyPosition = strstr(jsonString, searchKey);
  if (keyPosition != nullptr) {
    // Find the colon following the key
    const char* colonPosition = strchr(keyPosition, ':');
    if (colonPosition != nullptr) {
      // Convert the substring after the colon to an integer
      value = atoi(colonPosition + 1);
    }
  }

  return value;
}

int extractValue(String response, const char* key) {
  const char* json = response.c_str();
  const char* found = strstr(json, key);

  if (found == NULL) {
    return -1;
  }

  found += strlen(key);

  int flagValue = 0;
  while (*found >= '0' && *found <= '9') {
    flagValue = (flagValue * 10) + (*found - '0');
    found++;
  }

  return flagValue;
}

int extractValue_2(String input, String key) {
  int keyPos = input.indexOf(key);
  if (keyPos == -1) {
    return -1;
  }

  int valueStart = input.indexOf("\\\"", keyPos + key.length());
  if (valueStart == -1) {
    return -1;
  }
  valueStart += 2;

  int valueEnd = input.indexOf("\\\"", valueStart);
  if (valueEnd == -1) {
    return -1;
  }

  String valueStr = input.substring(valueStart, valueEnd);
  int value = valueStr.toInt();

  return value;
}

void main_loop() {
  Serial.println("loop begin");
  int manual_flag = extractValue(GetFirebase("PATCH", FIREBASE_PATH_manual_control, &http_client), "\"flag\":\"");
  Serial.print("manual flag:");
  Serial.println(manual_flag);

  if (manual_flag == 1) {
    int flag = extractValue(GetFirebase("PATCH", FIREBASE_PATH_set_flag, &http_client), "\"flag\":\"");

    if (flag == 1) {
      String Data = "{";
      Data += "\"dist\":" + String(getIntValueFromJson(GetFirebase("PATCH", FIREBASE_PATH_rem_disp, &http_client), "dist")) + "";

      Data += "}";

      PostToFirebase("PATCH", FIREBASE_PATH_rem_dist, Data, &http_client);

      int set_dist = extractValue_2(GetFirebase("PATCH", FIREBASE_PATH_set_dist, &http_client), "dist");
      int rem_dist = getIntValueFromJson(GetFirebase("PATCH", FIREBASE_PATH_rem_dist, &http_client), "dist");
      int new_rem = set_dist + rem_dist;
      Data = "{";
      Data += "\"dist\":" + String(new_rem) + "";

      Data += "}";

      PostToFirebase("PATCH", FIREBASE_PATH_rem_dist, Data, &http_client);
      PostToFirebase("PATCH", FIREBASE_PATH_rem_disp, Data, &http_client);
      flag = 0;
      Data = "{\"flag\":\"" + String(flag) + "\"}";

      PostToFirebase("PATCH", FIREBASE_PATH_set_flag, Data, &http_client);

      read_mil();

      Data = "{";
      Data += "\"dist\":" + String(number_rec) + "";
      Data += "}";

      PostToFirebase("PATCH", FIREBASE_PATH_mil_on_set, Data, &http_client);
      digitalWrite(2, HIGH);
    }


    int odo_at_set = getIntValueFromJson(GetFirebase("PATCH", FIREBASE_PATH_mil_on_set, &http_client), "dist");

    read_mil();
    if (true) {
      int dist_travelled = number_rec - odo_at_set;
      int rem_dist = getIntValueFromJson(GetFirebase("PATCH", FIREBASE_PATH_rem_dist, &http_client), "dist");
      int display_rem = rem_dist - dist_travelled;

      if (display_rem >= 0) {
        String Data = "{";
        Data += "\"dist\":" + String(display_rem) + "";
        Data += "}";

        PostToFirebase("PATCH", FIREBASE_PATH_rem_disp, Data, &http_client);
      }

      if (display_rem <= 0) {
        digitalWrite(2, LOW);
      } else {
        digitalWrite(2, HIGH);
      }
      prev_num = number_rec;
    }

  }

  else {
    digitalWrite(2, LOW);
  }
}

void read_mil() {
  // Request data from slave device
  Wire.requestFrom(SLAVE_ADDRESS, 5);

  // Wait until data is available
  while (Wire.available() == 0)
    ;

  // Read the received bytes from the slave
  String receivedData = "";
  while (Wire.available()) {
    char c = Wire.read();
    receivedData += c;
  }

  // Convert received data string to integer
  number_rec = receivedData.toInt();

  // Print received data (now integer) to the serial monitor
  Serial.print("Received data (as integer): ");
  Serial.println(number_rec);
}

void gps_1() {
  while (MySerial.available() > 0) {
    gps.encode(MySerial.read());
    if (gps.location.isUpdated()) {
      Serial.print("Latitude= ");
      Serial.print(gps.location.lat(), 6);
      String Data = "{";
      Data += "\"latitude\":" + String(gps.location.lat(), 6) + "";
      Data += "}";

      PostToFirebase("PATCH", FIREBASE_PATH_lat, Data, &http_client);
      Serial.print(" Longitude= ");
      Serial.println(gps.location.lng(), 6);

      Data = "{";
      Data += "\"longitude\":" + String(gps.location.lng(), 6) + "";
      Data += "}";

      PostToFirebase("PATCH", FIREBASE_PATH_long, Data, &http_client);
    }
  }
}
