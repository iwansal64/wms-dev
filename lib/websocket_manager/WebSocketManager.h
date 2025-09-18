#pragma once

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFi.h>


#define WEBSOCKET_DATA WStype_t type, uint8_t * payload, size_t length


class WebSocketManager
{
private:
const char *address;
uint16_t port;
WebSocketsClient web_socket;
String payload;

public:

//? ---------- FUNCTIONS ---------- ?//

/**
 * @brief Used to connect to {address} in {port} port.
 * @note This function should be called before sending or listening to any data 
 * @note This function can also be called with WiFi connection first
 * 
 * @code
 * init("ws.example.com", 80);
 * init("SSID", "PASS", "192.168.1.1", 80);
 * @endcode
 * 
 */
bool init(const char *address, uint16_t port);
bool init(const char *ssid, const char *pass, const char *address, uint16_t port);

/**
 * @brief Used to listen to changes or updates that web socket server sends
 * 
 * @param callback this function will be called whenever there's data from web socket server
 * 
 * @code
 * void listener(WEBSOCKET_DATA) {
 *   Serial.print("Payload:");
 *   Serial.println(payload);
 * }
 * 
 * void setup() {
 *  ws_manager.listen(listener);
 * }
 * @endcode
 */
void listen(void (*callback)(WStype_t type, uint8_t * payload, size_t length));


/**
 * @brief Used to prepare data through web socket connection 
 * 
 * @param data all sorts of things that can be convereted into a string
 * 
 * 
 * @code
 * uint8_t sensor_1 = 10;
 * ws_manager.put("sensor-1=");
 * ws_manager.put(sensor_1_value);
 * ws_manager.launch();
 * @endcode
 * 
 */
template <typename T>
bool put(const T& data);


/**
 * @brief Used to launch the message to the sky!!.. Oh, I mean web socket server!
 * 
 * @code
 * uint8_t sensor_1 = 10;
 * ws_manager.put("sensor-1=");
 * ws_manager.put(sensor_1_value);
 * ws_manager.launch();
 * @endcode
 */
bool launch();


/**
 * @brief Used to automatically reconnect to the web socket server when disconnected
 * @note can be called in a loop()
 * 
 * @code
 * void loop() {
 *   auto_reconnect(); // This will block program when disconnected to the web socket server
 * }
 * @endcode
 */
void wait_to_connect();

/**
 * @brief Used to make web socket client works.
 * @note Should be called in a loop()
 * 
 * 
 * @code
 * void loop() {
 *   loop(); // This will block program when disconnected to the web socket server
 * }
 * @endcode
 * 
 */
void loop();

static void handle_data(WStype_t type, uint8_t * payload, size_t length);
};
