#include <Servo.h>
#include <ESP8266WiFi.h>

#include "Application.hpp"

constexpr int SERVO_MOTOR_PIN = 14; // GPIO14 -> PIN D5

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

void ConnectWifi()
{
    WiFi.begin("Arduino_A20e070", "p%98765");

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        //Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
}

void PrintAndLedLoop()
{
    if (lastMillis < millis())
    {
        digitalWrite(LED_BUILTIN, state);
        state = 1 - state;
        if (count == 5)
            servoMotor.write(random(0, 5) * (180.0F / 5.0F));

        Serial.printf("t=%d, ", lastMillis);
        
        lastMillis += 1000;
        if (5 <= count)
        {
            count = 0;
            Serial.print("\n");
        }
        else
        {
            count++;
        }
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println();

    ConnectWifi();

    servoMotor.attach(SERVO_MOTOR_PIN);
    pinMode(LED_BUILTIN, OUTPUT);
    application.Init();
}

void loop()
{
    WiFiClient client = server.available();
    Serial.print("Client: ");
    Serial.print(client.status());
    Serial.println("");
    delay(300);

    if (client)
    {                             // If a new client connects,
        Serial.println("New Client.");          // print a message out in the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;
        while (client.connected() && currentTime - previousTime <= TIMEOUT_TIME) // loop while the client's connected
        {
            currentTime = millis();         
            if (client.available()) // if there's bytes to read from the client,
            {
                char c = client.read(); // read a byte, then
                Serial.write(c); // print it out the serial monitor
                httpRequestHeader += c;
                if (c == '\n') // if the byte is a newline character
                {
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();
                        
                        // turns the GPIOs on and off
                        if (httpRequestHeader.indexOf("GET /5/on") >= 0)
                        {
                            Serial.println("GPIO 5 on");
                            output5State = "on";
                            // digitalWrite(output5, HIGH);
                            servoMotor.write(0.0F);
                        }
                        else if (httpRequestHeader.indexOf("GET /5/off") >= 0)
                        {
                            Serial.println("GPIO 5 off");
                            output5State = "off";
                            // digitalWrite(output5, LOW);
                            servoMotor.write(45.0F);
                        }
                        else if (httpRequestHeader.indexOf("GET /4/on") >= 0)
                        {
                            Serial.println("GPIO 4 on");
                            output4State = "on";
                            // digitalWrite(output4, HIGH);
                            servoMotor.write(90.0F);
                        }
                        else if (httpRequestHeader.indexOf("GET /4/off") >= 0)
                        {
                            Serial.println("GPIO 4 off");
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
                        if (output5State=="off")
                        {
                            client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
                        } 
                        
                        // Display current state, and ON/OFF buttons for GPIO 4  
                        client.println("<p>GPIO 4 - State " + output4State + "</p>");
                        // If the output4State is off, it displays the ON button       
                        if (output4State=="off")
                        {
                            client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }
                        client.println("</body></html>");
                        
                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else // if you got a newline, then clear currentLine
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r') // if you got anything else but a carriage return character,
                {
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the httpRequestHeader variable
        httpRequestHeader = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }

    application.Update();
}
