#include <WebSocketManager.h>
#include <env.h>

void WebSocketManager::init(char *address, uint16_t port)
{
  if(WiFi.status() != WL_CONNECTED)
  {

  #ifdef SHOW_WARN
  Serial.println("[WebSocket] WiFi is not connected");
  Serial.println("[WebSocket] Waiting connection to WiFi");
  #endif

    while(WiFi.status() != WL_CONNECTED)
    {
      #ifdef SHOW_WARN
      Serial.print(" .");
      #endif
    
      delay(500);
    }
  };
  
  this->address = address;
  this->port = port;
  
  #ifdef SHOW_DEBUG
  Serial.print("[WebSocket] Connecting to: ");
  Serial.print(address);
  Serial.print(":");
  Serial.println(port);
  #endif

  this->web_socket.setExtraHeaders(ENV_COOKIE);
  this->web_socket.begin(address, port);
  this->web_socket.onEvent(WebSocketManager::handle_data);
}

void WebSocketManager::init(const char *ssid, const char *pass, char *address, uint16_t port)
{
  WiFi.begin(ssid, pass);

  if(WiFi.status() != WL_CONNECTED) {
    
    #ifdef SHOW_WARN
    Serial.println("[WebSocket] WiFi is not connected");
    Serial.println("[WebSocket] Waiting connection to WiFi");
    #endif

    while(WiFi.status() != WL_CONNECTED)
    {
      #ifdef SHOW_WARN
      Serial.print(" .");
      #endif
    
      delay(500);
    }
  };

  this->address = address;
  this->port = port;
  
  #ifdef SHOW_DEBUG
  Serial.print("[WebSocket] Connecting to: ");
  Serial.print(address);
  Serial.print(":");
  Serial.println(port);
  #endif
  
  this->web_socket.setExtraHeaders(ENV_COOKIE);
  this->web_socket.begin(address, port);
  this->web_socket.onEvent(WebSocketManager::handle_data);
}

void WebSocketManager::listen(void (*callback)(WStype_t type, uint8_t * payload, size_t length))
{
  if(!this->web_socket.isConnected())
  {
    #ifdef SHOW_WARN
    Serial.println("[WebSocket] WebSocket is not connected!");
    #endif

    return;
  }

  #ifdef SHOW_INFO
  Serial.println("[WebSocket] Successfully listening to Web Socket server changes!");
  #endif

  this->web_socket.onEvent(callback);
}

void WebSocketManager::handle_data(WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      #ifdef SHOW_INFO
      Serial.println("[WebSocket] Disconnected from server");
      #endif
      break;

    case WStype_CONNECTED:
      #ifdef SHOW_INFO
      Serial.printf("[WebSocket] Connected to: %s\n", payload);
      #endif
      break;

    case WStype_TEXT:
      #ifdef SHOW_INFO
      Serial.printf("[WebSocket] Message from server: %s\n", payload);
      #endif
      break;

    case WStype_BIN:
      #ifdef SHOW_INFO
      Serial.printf("[WebSocket] Binary message received (%d bytes)\n", length);
      #endif
      break;
  }
}

template bool WebSocketManager::put<int>(const int& data);
template bool WebSocketManager::put<uint8_t>(const uint8_t& data);
template bool WebSocketManager::put<float>(const float& data);
template bool WebSocketManager::put<char>(const char& data);
template bool WebSocketManager::put<String>(const String& data);

template <typename T>
bool WebSocketManager::put(const T& data)
{
  if (!this->web_socket.isConnected())
  {
    #ifdef SHOW_WARN
    Serial.println("[WebSocket] WebSocket is not connected!");
    #endif
    return false;
  }

  this->payload += String(data);
  return true;
}

bool WebSocketManager::launch() {
  bool result = this->web_socket.sendTXT(this->payload);
  
  #ifdef SHOW_WARN
  if(result) {
    Serial.println("[WebSocket] Successfully send data!");
  }
  else {
    Serial.println("[WebSocket] There's an error when trying to send data!");
  }
  #endif
  
  this->payload = "";
  return result;
}

void WebSocketManager::wait_to_connect()
{
  if(!this->web_socket.isConnected())
  {
    #ifdef SHOW_INFO
    Serial.print("[WebSocket] Connecting to web socket");
    #endif
    while (!this->web_socket.isConnected())
    {
      #ifdef SHOW_INFO
      Serial.print(" .");
      #endif
      delay(500);
    }
  }
}

void WebSocketManager::loop() {
  this->web_socket.loop();
}