#ifndef _MQTTWEBSOCKETCLIENT_H_
#define _MQTTWEBSOCKETCLIENT_H

#include <Stream.h>
#include <Client.h>
#include <WebSocketsClient.h>
#include "CircularByteBuffer.h"

static const int HASH_HEX_LEN2 = 64;

#ifndef DEBUG_WEBSOCKET_MQTT
#ifdef USE_SERIAL
#define DEBUG_WEBSOCKET_MQTT(...) USE_SERIAL.printf( _VA_ARGS__ )
#else
#define DEBUG_WEBSOCKET_MQTT(...)
#endif
#define NODEBUG_WEBSOCKET_MQTT
#endif

class MQTTWebSocketClient : public Client, public WebSocketsClient {
  public:

    //bufferSize defines the size of the circular byte buffer that provides the interface between messages arrived in websocket layer and byte reads from mqtt layer
    MQTTWebSocketClient (unsigned int bufferSize = 1000, unsigned long connectionTimeout = 50000);
    ~MQTTWebSocketClient();

    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);

    void putMessage (byte* buffer, int length);
    size_t write(uint8_t b);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    int read();
    int read(uint8_t *buf, size_t size);

    int peek();
    void flush();
    void stop();
    uint8_t connected() ;
    operator bool();

    bool getUseSSL ();
    MQTTWebSocketClient& setUseSSL (bool value);
    MQTTWebSocketClient& setPath(const char * path);

    //static instance of aws websocket client
    static MQTTWebSocketClient* instance;
    //keep the connection state
    bool _connected;
    //websocket callback
    static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

  private:

    //enable ssl... if your using mqtt over websockets at AWS IoT service, it must be enabled
    bool useSSL;


    //connection timeout
    unsigned long _connectionTimeout;
    char* path;

    virtual void runCbEvent(WStype_t type, uint8_t * payload, size_t length) {
      if(_cbEvent) {
        _cbEvent(type, payload, length);
      }
    }

    //circular buffer to keep incoming messages from websocket
    CircularByteBuffer bb;
};

#endif
