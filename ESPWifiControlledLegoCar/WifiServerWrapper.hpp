#ifndef WIFI_SERVER_WRAPPER
#define WIFI_SERVER_WRAPPER

#include <ESP8266WiFi.h>

class WifiServerWrapper
{
public:
    /// Argument: ellapsed time since attempt at connection
    using WatingForConnectionCallback = void(*)(unsigned long);

    /// Argument: http client and http request header
    using HandleHttpRequestCallback = void(*)(WiFiClient&, const String&);

    WifiServerWrapper(uint16_t port);

    bool Initialize(
        const char* networkName,
        const char* networkPass,
        WatingForConnectionCallback waitForConnection = nullptr,
        unsigned long networkConnectionTimeout = 5000);

    bool HandleConnection(HandleHttpRequestCallback handleHttpRequest, unsigned long connectionTimeout = 2000);

private:
    WiFiServer m_Server;
};

#endif // WIFI_SERVER_WRAPPER
