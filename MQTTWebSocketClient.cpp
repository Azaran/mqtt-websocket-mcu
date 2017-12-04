#include "MQTTWebSocketClient.h"

//work as a singleton to be used by websocket layer message callback
MQTTWebSocketClient* MQTTWebSocketClient::instance = NULL;

//callback to handle messages coming from the websocket layer
void MQTTWebSocketClient::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG_WEBSOCKET_MQTT("[WSc] Disconnected!");
			MQTTWebSocketClient::instance->stop ();
            break;
        case WStype_CONNECTED:
            DEBUG_WEBSOCKET_MQTT("[WSc] Connected to url: %s ", payload);
			MQTTWebSocketClient::instance->_connected = true;
            break;
        case WStype_TEXT:
            DEBUG_WEBSOCKET_MQTT("[WSc] get text: %s", *payload);
            MQTTWebSocketClient::instance->putMessage (payload, length);
            break;
        case WStype_BIN:
            DEBUG_WEBSOCKET_MQTT("[WSc] get binary length: %s", length);
            //hexdump(payload, length);
            MQTTWebSocketClient::instance->putMessage (payload, length);
            break;
    }
}

//constructor
MQTTWebSocketClient::MQTTWebSocketClient (unsigned int bufferSize, unsigned long connectionTimeout) {
    useSSL = false;
    _connectionTimeout = connectionTimeout;
    AWSWebSocketClient:instance = this;
    onEvent(MQTTWebSocketClient::webSocketEvent);
    path = NULL;
    _connected = false;
    bb.init (bufferSize); //1000 bytes of circular buffer... maybe it is too big
}

//destructor
MQTTWebSocketClient::~MQTTWebSocketClient(void) {
    if (path != NULL)
        delete[] path;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char* url_encode(const char* str) {
  const char* pstr = str;
  char *buf = (char*) malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

MQTTWebSocketClient& MQTTWebSocketClient::setPath(const char * path) {
    if (this->path != NULL)
        delete[] this->path;
	int len = strlen(path) + 1;
    this->path = new char[len]();
    strcpy(this->path, path);
	return *this;
}

int MQTTWebSocketClient::connect(IPAddress ip, uint16_t port){
  return -1; // not implemented
}

int MQTTWebSocketClient::connect(const char *host, uint16_t port) {
	//make sure it is disconnect first
	  const char* protocol = "mqtt";
	  stop ();
	  char* path = this->path;
	  //just need to free path if it was generated to connect to AWS
	  bool freePath = false;
    if (useSSL == true)
		  beginSSL (host,port,path,"",protocol);
	  else
      begin(host, port, path, protocol);
	  long now = millis ();
	  while ( (millis ()-now) < _connectionTimeout) {
		  loop ();
		  if (connected () == 1) {
			  if (freePath == true)
				delete[] path;
			  return 1;
		  }
		  delay (10);
	  }
	  if (freePath == true)
      delete[] path;
	  return 0;
}

//store messages arrived by websocket layer to be consumed by mqtt layer through the read funcion
void MQTTWebSocketClient::putMessage (byte* buffer, int length) {
	bb.push (buffer,length);
}

size_t MQTTWebSocketClient::write(uint8_t b) {
	if (_connected == false)
	  return -1;
  return write (&b,1);
}

//write through websocket layer
size_t MQTTWebSocketClient::write(const uint8_t *buf, size_t size) {
  if (_connected == false)
	  return -1;
  if (sendBIN (buf,size))
	  return size;
  return 0;
}

//return with there is bytes to consume from the circular buffer (used by mqtt layer)
int MQTTWebSocketClient::available(){
  //force websocket to handle it messages
  if (_connected == false)
	  return false;
  loop ();
  return bb.getSize ();
}

//read from circular buffer (used by mqtt layer)
int MQTTWebSocketClient::read() {
	if (_connected == false)
	  return -1;
	return bb.pop ();
}

//read from circular buffer (used by mqtt layer)
int MQTTWebSocketClient::read(uint8_t *buf, size_t size) {
	if (_connected == false)
	  return -1;
	int s = size;
	if (bb.getSize()<s)
		s = bb.getSize ();

	bb.pop (buf,s);
	return s;
}

int MQTTWebSocketClient::peek() {
	return bb.peek ();
}

void MQTTWebSocketClient::flush() {
}

void MQTTWebSocketClient::stop() {
	if (_connected == true) {
		_connected = false;
		bb.clear ();
	}
	disconnect ();
}

uint8_t MQTTWebSocketClient::connected() {
return _connected;
}

MQTTWebSocketClient::operator bool() {
	return _connected;
}

bool MQTTWebSocketClient::getUseSSL () {
  return useSSL;
}

MQTTWebSocketClient& MQTTWebSocketClient::setUseSSL (bool value) {
  useSSL = value;
  return *this;
}
