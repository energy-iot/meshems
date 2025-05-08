#include <mqtt_client.h>
#include <console.h>

#if ENABLE_MQTT
// Create WiFi and MQTT client instances
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MQTT connection status
bool mqtt_connected = false;
#endif

/**
 * @brief Initialize WiFi and MQTT connections
 */
void setup_mqtt() {
#if ENABLE_MQTT
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;
    WiFi.begin(ssid, password);
    
    // Wait for connection (with timeout)
    int wifi_timeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_timeout < 20) {
        delay(500);
        Serial.print(".");
        wifi_timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
        
        // Configure MQTT connection
        const char* mqtt_server = MQTT_SERVER;
        mqttClient.setServer(mqtt_server, MQTT_PORT);
        
        // Try to connect to MQTT broker
        mqtt_reconnect();
    } else {
        Serial.println("WiFi connection failed!");
    }
#else
    Serial.println("MQTT functionality is disabled");
#endif
}

/**
 * @brief Reconnect to MQTT broker if connection is lost
 */
void mqtt_reconnect() {
#if ENABLE_MQTT
    // Loop until we're reconnected
    int retry_count = 0;
    while (!mqttClient.connected() && retry_count < 3) {
        Serial.print("Attempting MQTT connection...");
        
        // Attempt to connect (with or without credentials)
        bool connect_result;
        
        // If username and password are provided, use them
        const char* client_id = MQTT_CLIENT_ID;
        const char* username = MQTT_USERNAME;
        const char* password = MQTT_PASSWORD;
        
        if (strlen(username) > 0) {
            connect_result = mqttClient.connect(client_id, username, password);
        } else {
            // Connect without credentials
            connect_result = mqttClient.connect(client_id);
        }
        
        if (connect_result) {
            Serial.println("connected");
            mqtt_connected = true;
            
            // Once connected, publish an announcement
            mqttClient.publish("ems/status", "EMS device connected");
            
            // Subscribe to command topics if needed
            // mqttClient.subscribe("ems/commands");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 2 seconds");
            mqtt_connected = false;
            delay(2000);
            retry_count++;
        }
    }
#endif
}

/**
 * @brief Main MQTT loop to maintain connection and process messages
 */
void loop_mqtt() {
#if ENABLE_MQTT
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Reconnecting...");
        const char* ssid = WIFI_SSID;
        const char* password = WIFI_PASSWORD;
        WiFi.begin(ssid, password);
    }
    
    // Check MQTT connection
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
        mqtt_reconnect();
    }
    
    // Process MQTT messages
    if (mqtt_connected) {
        mqttClient.loop();
    }
#endif
}

/**
 * @brief Publish temperature data to MQTT
 * 
 * @param temperature The temperature value to publish
 * @return true if published successfully, false otherwise
 */
bool mqtt_publish_temperature(float temperature) {
#if ENABLE_MQTT
    if (!mqtt_connected) {
        return false;
    }
    
    // Convert float to string with 1 decimal place
    char temp_str[10];
    dtostrf(temperature, 4, 1, temp_str);
    
    // Publish to temperature topic
    const char* temp_topic = MQTT_TOPIC_TEMPERATURE;
    bool result = mqttClient.publish(temp_topic, temp_str);
    
    if (result) {
        Serial.print("Published temperature: ");
        Serial.println(temp_str);
    } else {
        Serial.println("Failed to publish temperature");
    }
    
    return result;
#else
    // Just print to serial when MQTT is disabled
    Serial.print("Temperature: ");
    Serial.println(temperature);
    return true;
#endif
}

/**
 * @brief Publish humidity data to MQTT
 * 
 * @param humidity The humidity value to publish
 * @return true if published successfully, false otherwise
 */
bool mqtt_publish_humidity(float humidity) {
#if ENABLE_MQTT
    if (!mqtt_connected) {
        return false;
    }
    
    // Convert float to string with 1 decimal place
    char humid_str[10];
    dtostrf(humidity, 4, 1, humid_str);
    
    // Publish to humidity topic
    const char* humid_topic = MQTT_TOPIC_HUMIDITY;
    bool result = mqttClient.publish(humid_topic, humid_str);
    
    if (result) {
        Serial.print("Published humidity: ");
        Serial.println(humid_str);
    } else {
        Serial.println("Failed to publish humidity");
    }
    
    return result;
#else
    // Just print to serial when MQTT is disabled
    Serial.print("Humidity: ");
    Serial.println(humidity);
    return true;
#endif
}
