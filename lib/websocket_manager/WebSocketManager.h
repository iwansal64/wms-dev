#include <Arduino.h>
#include <WebSocketsClient.h>
#include <WiFi.h>


#define WEBSOCKET_DATA WStype_t type, uint8_t * payload, size_t length


class WebSocketManager
{
private:
char *address;
uint16_t port;
WebSocketsClient web_socket;

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
void init(char *address, uint16_t port);
void init(const char *ssid, const char *pass, char *address, uint16_t port);

/**
 * @brief Used to listen to changes or updates that web socket server sends
 * 
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

/*


Example:
*/
/**
 * @brief Used to send data through web socket connection 
 * 
 * @code
 * ws_manager.print("data:sensor-1=10");
 * ws_manager.print("data:sensor-2=50");
 * @endcode
 * 
 */
bool print(const char *data);


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
