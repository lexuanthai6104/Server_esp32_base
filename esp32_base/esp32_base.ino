#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SocketIOclient.h>

#define WIFI_SSID "Lxt"      
#define WIFI_PASS "12345678" 

#define SERVER_IP "192.168.137.1"  
#define SERVER_PORT 5000

SocketIOclient socketIO;
unsigned long lastReconnectAttempt = 0;

void socketIOEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case sIOtype_DISCONNECT:
            Serial.println("[IO] Mất kết nối WebSocket!");
            break;
        case sIOtype_CONNECT:
            Serial.println("[IO] Kết nối thành công!");
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            Serial.printf("[IO] Nhận tin nhắn từ server: %s\n", payload);
            break;
    }
}

void sendMessage() {
    // int temp = random(20, 22);  // ⚡ Sinh giá trị nhiệt độ ngẫu nhiên
    // int humid = random(50, 55); // ⚡ Sinh giá trị độ ẩm ngẫu nhiên

    DynamicJsonDocument doc(256);
    JsonArray array = doc.to<JsonArray>();
    array.add("message");  // Sự kiện gửi
    JsonObject data = array.createNestedObject();
    data["temp"] = temp;
    data["humid"] = humid;

    String output;
    serializeJson(doc, output);
    
    Serial.println("📤 Gửi tin nhắn: " + output);
    socketIO.sendEVENT(output);
}

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("🔹 Đang kết nối WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi đã kết nối!");
    Serial.print("📡 IP ESP32: ");
    Serial.println(WiFi.localIP());

    Serial.println("🔗 Kết nối WebSocket...");
    socketIO.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4");
    socketIO.onEvent(socketIOEvent);
}

void loop() {
    socketIO.loop();

    if (!socketIO.isConnected()) {
        if (millis() - lastReconnectAttempt > 5000) {
            Serial.println("🔄 Đang thử kết nối lại WebSocket...");
            socketIO.begin(SERVER_IP, SERVER_PORT, "/socket.io/?EIO=4");
            lastReconnectAttempt = millis();
        }
    }

    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000) {  // Gửi tin nhắn mỗi 5 giây
        lastSend = millis();
        sendMessage();
    }
}
