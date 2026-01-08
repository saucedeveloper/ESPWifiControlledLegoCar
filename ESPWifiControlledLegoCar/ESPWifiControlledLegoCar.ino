#if MAIN_ESP_WIFI_CONTROLLED_LEGO_CAR

#include <Servo.h>
#include <array>
#include <ESP8266WiFi.h>

#include "Application.hpp"

constexpr int SERVO_MOTOR_PIN = D5;
constexpr int MOTOR_CONTROL_PIN = D8;

constexpr long TIMEOUT_TIME = 2000;

Application application;
Servo servoMotor;
WiFiServer server(80);

String httpRequestHeader = "";

String output5State = "off";
String output4State = "off";
auto currentTime = millis();
decltype(millis()) previousTime = 0;

decltype(millis()) lastMillis = 0;
int count = 0;
bool state = LOW;

void ConnectWifi() {
  WiFi.begin(NETWORK_NAME, PASS_TO_NETWORK);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void PrintAndLedLoop() {
  if (lastMillis < millis()) {
    digitalWrite(LED_BUILTIN, state);
    state = 1 - state;
    if (count == 5)
      servoMotor.write(random(0, 5) * (180.0F / 5.0F));

    Serial.printf("t=%d, ", lastMillis);

    lastMillis += 1000;
    if (5 <= count) {
      count = 0;
      Serial.print("\n");
    } else {
      count++;
    }
  }
}

unsigned long startTime = 0;

constexpr uint16_t PWM_SAFETY = 50;
constexpr uint16_t PWM_LOW = 500 + PWM_SAFETY;
constexpr uint16_t PWM_HIGH = 2200 - PWM_SAFETY;

void DelayStart()
{
    Serial.println("Initialized");

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

    Serial.println("Waiting 1");

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
    }

    Serial.println("Waiting 2");

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
    }
    
    Serial.println("Done waiting");

    digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
    Serial.begin(9600);
    Serial.println();

    // ConnectWifi();

    //servoMotor.attach(SERVO_MOTOR_PIN, PWM_LOW, PWM_HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN, OUTPUT);
    application.Init();

    DelayStart();
    startTime = millis();
}

void WifiLoop() {
  WiFiClient client = server.available();
  // Serial.print("Client: ");
  // Serial.print(client.status());
  // Serial.println("");
  //delay(1000);

  if (client) // If a new client connects,
  {
    //Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";  // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= TIMEOUT_TIME)  // loop while the client's connected
    {
      currentTime = millis();
      if (client.available())  // if there's bytes to read from the client,
      {
        char c = client.read();  // read a byte, then
        //write(c); // print it out the serial monitor
        httpRequestHeader += c;
        if (c == '\n')  // if the byte is a newline character
        {
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (httpRequestHeader.indexOf("GET /5/on") >= 0) {
              // Serial.println("GPIO 5 on");
              output5State = "on";
              // digitalWrite(output5, HIGH);
              servoMotor.write(0.0F);
            } else if (httpRequestHeader.indexOf("GET /5/off") >= 0) {
              // Serial.println("GPIO 5 off");
              output5State = "off";
              // digitalWrite(output5, LOW);
              servoMotor.write(45.0F);
            } else if (httpRequestHeader.indexOf("GET /4/on") >= 0) {
              // Serial.println("GPIO 4 on");
              output4State = "on";
              // digitalWrite(output4, HIGH);
              servoMotor.write(90.0F);
            } else if (httpRequestHeader.indexOf("GET /4/off") >= 0) {
              // Serial.println("GPIO 4 off");
              output4State = "off";
              // digitalWrite(output4, LOW);
              servoMotor.write(135.0F);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 5
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            // If the output5State is off, it displays the ON button
            if (output5State == "off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button
            if (output4State == "off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else  // if you got a newline, then clear currentLine
          {
            currentLine = "";
          }
        } else if (c != '\r')  // if you got anything else but a carriage return character,
        {
          currentLine += c;  // add it to the end of the currentLine
        }
      }
    }
    // Clear the httpRequestHeader variable
    httpRequestHeader = "";
    // Close the connection
    client.stop();
    // Serial.println("Client disconnected.");
    // Serial.println("");
  }
}

#define USE_STEPS 0

std::array<float, 4> CONTROL_VALUES = { /* 0.0F, 0.2F, */ 0.4F, 0.6F, 0.8F, 1.0F };
size_t controlIndex = 0;
float lastControl = 0.0F;
unsigned long lastTime = 0;

void loop() {
    constexpr float PERIOD = 20.0E3;
    constexpr float AMPLITUDE = 1;

    unsigned long time = millis();
    unsigned long localTime = time - startTime;

#if USE_STEPS
    if (lastTime + 2.0E3F < localTime)
    {
        constexpr float AMPLITUDE_FACTOR = 0.75;
        lastControl = AMPLITUDE_FACTOR * CONTROL_VALUES[controlIndex];
        Serial.printf("[%d] Writing %d: %f\n", lastTime, controlIndex, lastControl);
        
        controlIndex = (controlIndex + 1) % CONTROL_VALUES.size();
        
        int analogSignal = (int)roundf(lastControl * 255.0F);

        analogWrite(MOTOR_CONTROL_PIN, analogSignal);
        lastTime = localTime;
    }

#else
    float periodLocalTime = localTime / PERIOD;
    float referenceSignal = AMPLITUDE * (0.5F + 0.5F * sin(periodLocalTime * 2.0F * PI));
    //int pwmSignal = PWM_LOW + (int)(referenceSignal * (PWM_HIGH - PWM_LOW));

    int analogSignal = (int)roundf(referenceSignal * 255.0F);
    analogWrite(MOTOR_CONTROL_PIN, analogSignal);
    
    if (lastTime + 1.0E3F < localTime)
    {
        Serial.printf("[%d] Writing %f\n", lastTime, referenceSignal);
        lastTime = localTime;
    }
#endif
    //servoMotor.writeMicroseconds(pwmSignal);

    application.Update();
}

#endif
