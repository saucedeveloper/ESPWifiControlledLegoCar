#define MAIN_TEST_CONTROL_TIMING 1

#if MAIN_TEST_CONTROL_TIMING

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

#include "Application.hpp"
#include "ControlTiming.hpp"
#include "WifiServerWrapper.hpp"

constexpr int MOTOR_CONTROL_PIN = D8;

unsigned long previousMillis;
unsigned long loopCounter = 0;
unsigned long counterIndex = 0;

ControlTiming controlTiming(ControlSetting{50, 0.1F});
WifiServerWrapper server(80);

void DelayStart()
{
    Serial.println("Initialized");

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    Serial.println("Waiting 1");

    for (int i = 0; i < 6; i++)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
    }

    Serial.println("Waiting 2");

    for (int i = 0; i < 8; i++)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(300);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(300);
    }
    
    Serial.println("Done waiting");

    digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
    Serial.begin(9600);
    previousMillis = millis();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN, OUTPUT);

    DelayStart();

    auto waitForConnection = [](unsigned long) { Serial.print("."); };
    server.Initialize(NETWORK_NAME, PASS_TO_NETWORK, waitForConnection , 8000);
}

template <typename T>
T Clamp(T value, T min, T max)
{
    return std::min(max, std::max(min, value));
}

uint8_t ledState = LOW;
std::array<uint8_t, 500> signalValues{};
size_t signalValueIndex = 0;

const char* const htmlBaseContent[] = {
    "<!DOCTYPE html><html>",
    "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">",
    "<link rel=\"icon\" href=\"data:,\">",
    "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}",
    ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;",
    "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}",
    ".button2 {background-color: #77878A;}</style></head>",
    "<body><h1>ESP8266 Web Server</h1>"
};

#define STR_LITTERAL_CHAR_COUNT(S) (sizeof(S) - 1)

#define BUTTON_PAGE_NAME_DRIVE "/drive/"
#define BUTTON_PAGE_NAME_DRIVE_N "/drive/N"

#define BUTTON_HTML_START "<p><a href=\""
#define BUTTON_HTML_MIDDLE "\"><button class=\"button\">"
#define HTTP_REQUEST_GET_TARGET ("GET " BUTTON_PAGE_NAME_DRIVE)

#define BUTTON_HTML_END "</button></a></p>"

constexpr uint8_t DRIVE_BUTTON_MAX_COUNT =
    STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_START)
    + STR_LITTERAL_CHAR_COUNT(BUTTON_PAGE_NAME_DRIVE_N)
    + STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_MIDDLE)
    + 5 /* "[100]" */
    + STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_END);

constexpr uint8_t DRIVE_SELECTION_COUNT = 5;
constexpr float DRIVE_INPUT_MAX = 0.6F;

float driveInput = 0.0F;
uint8_t driveSelection = 0;

bool SetDriveSelection(int newSelection)
{
    if (0 <= newSelection && newSelection < DRIVE_SELECTION_COUNT)
    {
        driveSelection = newSelection;
        driveInput = DRIVE_INPUT_MAX * (float)driveSelection / (DRIVE_SELECTION_COUNT - 1.0F);
        return true;
    }
    return false;
}

void HandleHttpRequest(WiFiClient& client, const String& httpRequestHeader)
{
    int indexOfGetDrive = httpRequestHeader.indexOf(HTTP_REQUEST_GET_TARGET);
    if (0 <= indexOfGetDrive)
    {
        int targetIndex = indexOfGetDrive + STR_LITTERAL_CHAR_COUNT(HTTP_REQUEST_GET_TARGET);
        if (targetIndex < httpRequestHeader.length())
        {
            char requestIndex = httpRequestHeader[targetIndex];
            int newSelection = requestIndex - '0';
            SetDriveSelection(newSelection);
        }
    }

    for (const auto& line : htmlBaseContent)
    {
        client.println(line);
    }

    // Display current state: drive input
    client.print("<p>Drive: ");
    client.print((int)std::roundf(driveInput * 100.0F));
    client.println("%</p>");

    char buffer[DRIVE_BUTTON_MAX_COUNT] = {0};
    size_t bufferIndex = 0;
    std::sprintf(&buffer[bufferIndex], "%s", BUTTON_HTML_START);
    bufferIndex += STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_START);

    for (uint8_t i = 0; i < DRIVE_SELECTION_COUNT; i++)
    {
        std::sprintf(&buffer[bufferIndex], "%s%hhu", BUTTON_PAGE_NAME_DRIVE, i);
        bufferIndex += STR_LITTERAL_CHAR_COUNT(BUTTON_PAGE_NAME_DRIVE) + 1;

        std::sprintf(&buffer[bufferIndex], "%s", BUTTON_HTML_MIDDLE);
        bufferIndex += STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_MIDDLE);

        if (i == driveSelection)
        {
            std::sprintf(&buffer[bufferIndex], "[");
            bufferIndex += 1;
        }

        float correspondingInputValue = DRIVE_INPUT_MAX * (float)i / (DRIVE_SELECTION_COUNT - 1.0F);
        uint8_t percentageValue = (int)std::roundf(correspondingInputValue * 100.0F);
        std::sprintf(&buffer[bufferIndex], "%hhu", percentageValue);
        bufferIndex += (percentageValue <= 9) ? 1 : 2;

        if (i == driveSelection)
        {
            std::sprintf(&buffer[bufferIndex], "]");
            bufferIndex += 1;
        }

        std::sprintf(&buffer[bufferIndex], "%s", BUTTON_HTML_END);
        bufferIndex += STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_END);

        for (size_t i = 0; i < bufferIndex; i++)
            client.print(buffer[i]);

        client.println();

        bufferIndex = STR_LITTERAL_CHAR_COUNT(BUTTON_HTML_START);
    }

    client.println("</body></html>");
}

void loop()
{
    unsigned long now = millis();

    server.HandleConnection(HandleHttpRequest);

    float signalValue = 0.0F;
    if (controlTiming.Tick(now, signalValue))
    {
        uint8_t analogSignal = Clamp<int>((int)std::roundf(signalValue * 255.0F), 0, 255);
        analogWrite(MOTOR_CONTROL_PIN, analogSignal);

        ledState = 1 - ledState;
        digitalWrite(LED_BUILTIN, ledState);
        signalValues[signalValueIndex] = analogSignal;
        signalValueIndex++;
        if (signalValues.size() <= signalValueIndex)
        {
            float average = (float)std::accumulate(signalValues.begin(), signalValues.end(), 0.0F) / (float)signalValues.size();
            Serial.printf("Average: %f (now: %lu)\n", average / 255.0F, now);
            signalValueIndex = 0;
        }
    }
}

#endif
