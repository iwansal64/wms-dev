#include <WebSocketManager.h>
#include <env.h>

void WebSocketManager::init(char *address, uint16_t port)
{
  while(WiFi.status() != WL_CONNECTED) delay(500);
  
  this->address = address;
  this->port = port;
  
  this->web_socket.setExtraHeaders(ENV_COOKIE);
  this->web_socket.begin(address, port);
  this->web_socket.onEvent(WebSocketManager::handle_data);
}

void WebSocketManager::init(const char *ssid, const char *pass, char *address, uint16_t port)
{
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED) delay(500);

  this->address = address;
  this->port = port;
  
  this->web_socket.setExtraHeaders(ENV_COOKIE);
  this->web_socket.begin(address, port);
  this->web_socket.onEvent(WebSocketManager::handle_data);
}

void WebSocketManager::listen(void (*callback)(WStype_t type, uint8_t * payload, size_t length))
{
  if(!this->web_socket.isConnected())
  {
    return;
  }

  this->web_socket.onEvent(callback);
}

void WebSocketManager::handle_data(WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;

    case WStype_CONNECTED:
      Serial.printf("Connected to: %s\n", payload);
      break;

    case WStype_TEXT:
      Serial.printf("Message from server: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.printf("Binary message received (%d bytes)\n", length);
      break;
  }
}

bool WebSocketManager::print(const char *data)
{
  if(!this->web_socket.isConnected())
  {
    return false;
  }

  return this->web_socket.sendTXT(data);
}

void WebSocketManager::wait_to_connect()
{
  if(!this->web_socket.isConnected())
  {
    Serial.print("Connecting to web socket");
    while (!this->web_socket.isConnected())
    {
      Serial.print(" .");
      delay(500);
    }
  }
}

void WebSocketManager::loop() {
  this->web_socket.loop();
}