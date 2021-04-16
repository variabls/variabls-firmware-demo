#include <Arduino.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "FS.h"
#include <SPI.h>
#include <SD.h>
#include <AutoConnect.h>
#include <AutoConnectCredential.h>
#include <ArduinoJson.h>
#include <string.h>

#include <SparksTime.h>
#include <SparksMQTT.h>


#define TRIGGER_PIN 0
#define HOLD_TIMER 10000
#define DATA_PERIOD 1000
#define CREDENTIAL_OFFSET 0
#define PARAM_FILE "/param.json"
#define SD_CS 5

#define DEFAULT_USER_AUTH "sparksdevice"
#define DEFAULT_PASS_AUTH "default"

#if defined(ARDUINO_ARCH_ESP8266)
  #define GET_CHIPID() ((uint16_t)(ESP.getChipId() >> 32))
  #define BOARD_TYPE "ESP8266"
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266HTTPUpdate.h>
  #include <LittleFS.h>
  // #include <WiFiClient.h>
  ESP8266WebServer webServer;
#elif defined(ARDUINO_ARCH_ESP32)
  #define GET_CHIPID() ((uint16_t)(ESP.getEfuseMac() >> 32))
  #define BOARD_TYPE "ESP32"
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include <ESP32Ping.h>
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
  #include <SPIFFS.h>
  #include <Preferences.h>
  #include <nvs.h>
  #include <nvs_flash.h>
  #include <Update.h>
  WebServer webServer;
#endif


AutoConnect Portal(webServer);
AutoConnectConfig config;
AutoConnectCredential crd(CREDENTIAL_OFFSET);

String connectionOption, usernameMQTT, passwordMQTT, deviceKey;
String authUsername = "";
String authPassword = "";
String chipId;

SparksMQTT sparksMQTT;

int ledState = LOW;
const long ledInterval = 1000;
unsigned long ledtm, invtm;
unsigned long millisPrint = 0;

void rootPage()
{
  String content = "<html>"
                   "<head>"
                   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                   "</head>"
                   "<body>"
                   "<h1 style=padding-top:5px;text-align:center>Sparks.id Device Firmware</h1>"
                   "<p style=\"padding-top:5px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
                                                                                              "</body>"
                                                                                              "</html>";
  webServer.send(200, "text/html", content);
}

String deviceID(boolean types)
{
#if defined(ARDUINO_ARCH_ESP8266)
    uint64_t chipid = ESP.getChipId(); //The chip ID is essentially its MAC address(length: 6 bytes).
#elif defined(ARDUINO_ARCH_ESP32)
    uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
#endif
    char buf[23];
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(buf, 23, "%04X%08X", chip, (uint32_t)chipid); //simplified EMS ID for pairing purpose
    String chipID = "Sparks-" + String(BOARD_TYPE) + "-" + String(buf);
    if (types)
    {
        return chipID;
    }
    else
    {
        return buf;
    }
}

void getParams(AutoConnectAux &aux)
{
  AutoConnectSelect &option = aux["connectionOption"].as<AutoConnectSelect>();
  connectionOption = option.value();
  usernameMQTT = aux["username"].value;
  usernameMQTT.trim();
  passwordMQTT = aux["password"].value;
  passwordMQTT.trim();
  deviceKey = aux["devicekey"].value;
  deviceKey.trim();
  authUsername = aux["auth_username"].value;
  authUsername.trim();
  authPassword = aux["auth_password"].value;
  authPassword.trim();
}

String saveParams(AutoConnectAux &aux, PageArgument &args)
{
  AutoConnectAux &connection = *Portal.aux(Portal.where());
  getParams(connection);

  AutoConnectText &echo = aux["parameters"].as<AutoConnectText>();

#if defined(ARDUINO_ARCH_ESP8266)
  File param = LittleFS.open(PARAM_FILE, "w");
#elif defined(ARDUINO_ARCH_ESP32)
  File param = SPIFFS.open(PARAM_FILE, "w");
#endif
  connection.saveElement(param, {"connectionOption", "username", "password", "devicekey", "auth_username", "auth_password"});
  param.close();
  echo.value = "Connection Option: " + connectionOption + "<br>";
  echo.value += "Username: " + usernameMQTT + "<br>";
  echo.value += "Password: " + passwordMQTT + "<br>";
  echo.value += "Device Key: " + deviceKey + "<br><br>";
  echo.value += "Auth username: " + authUsername + "<br>";
  echo.value += "Auth password: " + authPassword + "<br><br>";

  return String();
}

String loadParams(AutoConnectAux &aux, PageArgument &args)
{
  (void)(args);
  #if defined(ARDUINO_ARCH_ESP8266)
    File param = LittleFS.open(PARAM_FILE, "r");
  #elif defined(ARDUINO_ARCH_ESP32)
    File param = SPIFFS.open(PARAM_FILE, "r");
  #endif

  if (param)
  {
    if (aux.loadElement(param))
    {
      getParams(aux);
      Serial.println(PARAM_FILE " loaded");
    }
    else
    {
      Serial.println(PARAM_FILE " failed to load");
    }
    param.close();
  }
  else
  {
    Serial.println(PARAM_FILE " open failed");
  }
  return String("");
}

const char DEVICE_PAGE[] PROGMEM = R"raw(
      [
      {
        "title" : "Credential Setting",
        "uri" : "/ConnectionSetting",
        "menu" : true,
        "element" : [
          {
            "name": "style",
            "type": "ACStyle",
            "value": "label+input,label+select{position:sticky;left:120px;width:230px!important;box-sizing:border-box;},label+button{background-color:#fffff;color:white;}"
          },
          {
            "name" : "header",
            "type" : "ACText",
            "value" : "<h2>Device Connection Setting</h2>",
            "style" : "text-align:center;color:#2f4f4f;padding:10px;"
          },
          {
            "name" : "caption",
            "type" : "ACText",
            "value" : "Connection configuration to connect this device to the Sparks platform. Please go to your account to add device key, and contact us if you need help. To apply the change, please restart your device. ",
            "style" : "text-align:center;color:#8B0000"
          },
          {
            "name" : "connectionOption",
            "type" : "ACSelect",
            "option" : ["MQTT", "Websocket", "AMQP", "HTTP" ,"Others"],
            "selected" : 0,
            "label" : "Connection Type"
          },
          {
            "name" : "username",
            "type" : "ACInput",
            "value" : "",
            "placeholder" : "Input MQTT username",
            "label" : "Username : "
          },
          {
            "name" : "password",
            "type" : "ACInput",
            "value" : "",
            "placeholder" : "Input MQTT Password",
            "label" : "Password : "
          },
          {
            "name" : "Newline",
            "type" : "ACText",
            "value" : "",
            "style" : "text-align:center;color:#8B0000"
          },
          {
            "name" : "devicekey",
            "type" : "ACInput",
            "value" : "",
            "placeholder" : "Input Device Key",
            "label" : "Device Key : "
          },
          {
            "name": "adjust_width",
            "type": "ACElement",
            "value": "<script type=\"text/javascript\">window.onload=function(){var t=document.querySelectorAll(\"input[type='text']\");for(i=0;i<t.length;i++){var e=t[i].getAttribute(\"placeholder\");e&&t[i].setAttribute(\"size\",e.length*1.2)}};</script>"
          },
          {
            "name" : "auth_header",
            "type" : "ACText",
            "value" : "<h2>Device Authentication Setting</h2>",
            "style" : "text-align:center;color:#2f4f4f;padding:10px;"
          },
          {
            "name" : "auth_caption",
            "type" : "ACText",
            "value" : "Local access point authentication setting. If you make an authentication setting change, you need to restart your device to apply setting.",
            "style" : "text-align:center;color:#8B0000"
          },
          {
            "name" : "Newline",
            "type" : "ACText",
            "value" : "",
            "style" : "text-align:center;color:#8B0000"
          },
          {
            "name" : "auth_username",
            "type" : "ACInput",
            "value" : "",
            "placeholder" : "Input authentication username",
            "label" : "Auth Username : "
          },
          {
            "name" : "auth_password",
            "type" : "ACInput",
            "value" : "",
            "placeholder" : "Input authentication Password",
            "label" : "Auth Password : "
          },
          {
            "name": "save",
            "type": "ACSubmit",
            "value": "Save",
            "uri": "/connection_save"
          },
          {
            "name": "auth_discard",
            "type": "ACSubmit",
            "value": "Discard",
            "uri": "/_ac"
          }
        ]
      },
      {
      "uri": "/connection_save",
      "title": "Connection Setting",
      "menu": false,
      "element": [
        {
          "name": "caption",
          "type": "ACText",
          "value": "<h4>Parameters saved as:</h4>",
          "style": "text-align:center;color:#2f4f4f;padding:10px;"
        },
        {
          "name": "parameters",
          "type": "ACText"
        },
        {
          "name": "restartNotice",
          "type": "ACText",
          "value": "<h4>Please restart your device to apply the change.</h4>",
          "style": "text-align:center;color:#2f4f4f;padding:10px;"
        },
        {
          "name": "clear",
          "type": "ACSubmit",
          "value": "Done",
          "uri": "/_ac"
        }
        ]
      }
    ])raw";



class SparksLib
{
private:
  const char *hardwareVersion = "EMS-Lite-0";
  const char *firmwareVersion = "v.0.0.1";
  const char *mqttServer = "broker.sparks.id";
  const int port = 1883;
  const int portSecure = 1883;

  // const char *mqttUser;
  // const char *mqttPass;

  void checkAuthCredential(AutoConnectAux &aux)
  {
    #if defined(ARDUINO_ARCH_ESP8266)
          File param = LittleFS.open(PARAM_FILE, "r");
    #elif defined(ARDUINO_ARCH_ESP32)
          File param = SPIFFS.open(PARAM_FILE, "r");
    #endif

  if (aux.loadElement(param))
  {
        if (aux["auth_username"].value != "")
          {
            authUsername = aux["auth_username"].value;
            authUsername.trim();
            authPassword = aux["auth_password"].value;
            authPassword.trim();
          }
        else
          {
            authUsername = DEFAULT_USER_AUTH;
            authPassword = DEFAULT_PASS_AUTH;
          }
  } 
  else{
        authUsername = DEFAULT_USER_AUTH;
        authPassword = DEFAULT_PASS_AUTH;
  }
  }

  static bool startCP(IPAddress ip)
  {
      #if defined(ARDUINO_ARCH_ESP8266)
        digitalWrite(2, HIGH);
      #elif defined(ARDUINO_ARCH_ESP32)
        digitalWrite(2, LOW);
      #endif
   
    Serial.println("C.P. started, IP:" + WiFi.localIP().toString());
    return true;
  }

  void startPortal()
  {
    if (Portal.begin())
    {
      digitalWrite(2, LOW);
      configTime(25200, 0, ntpServer);
      #if defined(ARDUINO_ARCH_ESP8266)
        if (MDNS.begin("sparksdevice"))
        {
          MDNS.addService("http", "tcp", 80);
        }
      #elif defined(ARDUINO_ARCH_ESP32)
        if (MDNS.begin("sparksdevice"))
        {
          MDNS.addService("http", "tcp", 80);
        }
      #endif
    }
  }


public:
  void begin()
  {
    // put your setup code here, to run once:
    Serial.begin(9600);
    delay(500);

    #if defined(ARDUINO_ARCH_ESP8266)
        LittleFS.begin();
        pinMode(TRIGGER_PIN, INPUT_PULLUP);
        pinMode(LED_BUILTIN, OUTPUT);
        digitalWrite(LED_BUILTIN, HIGH);

        // pinMode(19, OUTPUT);
        // digitalWrite(19, LOW);
    #elif defined(ARDUINO_ARCH_ESP32)
        SPIFFS.begin(true);

        pinMode(TRIGGER_PIN, INPUT_PULLUP);
        pinMode(2, OUTPUT);
        digitalWrite(2, LOW);

        pinMode(19, OUTPUT);
        digitalWrite(19, LOW);
    #endif
    sparksMQTT.begin();
    Portal.onDetect(startCP);

    // queue = xQueueCreate(100, 1024);
    WiFi.begin();
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      //No auth required for direct softAP Connection
      config.auth = AC_AUTH_NONE;
    }
    else
    {
      //Auth required for all portal
      config.auth = AC_AUTH_DIGEST;
      config.authScope = AC_AUTHSCOPE_PORTAL;
    }

    if (Portal.load(FPSTR(DEVICE_PAGE)))
    {
      // check credentials
      AutoConnectAux aux;
      checkAuthCredential(aux);

      //configuration for autoconnect authentication
      config.username = authUsername;
      config.password = authPassword;      
      
      /*
      //please uncomment these line below to activate captive portal everytime device is restarted
      config.immediateStart = true;
      config.autoRise = true;
      */
      
      config.portalTimeout = 60000;
      config.autoReconnect = true;
      config.retainPortal = true;
      config.boundaryOffset = CREDENTIAL_OFFSET;
      config.hostName = "starsolems";
      config.title = "Sparks Device WEB UI";
      config.apid = "Sparks-Device-" + deviceID(false);
      config.psk = "";
      config.homeUri = "/";
      Portal.config(config);

      Portal.on("/ConnectionSetting", loadParams);
      Portal.on("/connection_save", saveParams);
    }
    else
    {
      Serial.println("Load Error");
    }
    startPortal();
    webServer.on("/", rootPage);

    // #if defined(ARDUINO_ARCH_ESP8266)
        timeClient.begin();
    // #endif

    // #if defined(ARDUINO_ARCH_ESP8266)
    //   ESP8266WebServer &webServer = Portal.host();

    // #elif defined(ARDUINO_ARCH_ESP32)
    //   WebServer &webServer = Portal.host();
    //   webServer.on("/", rootPage);
    // #endif

    AutoConnectAux &mqttSetting = *Portal.aux("/ConnectionSetting");
    PageArgument args;

    loadParams(mqttSetting, args);
    Serial.println("DEVICE ID : " + deviceID(true));
    Serial.println("DEVICE ID : " + deviceID(false));
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.SSID());

    sparksMQTT.connectMQTT();
    
  }

  void run()
  {
    // Trigger
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
      unsigned long tm = millis();
      while (digitalRead(TRIGGER_PIN) == LOW)
      {
        yield();
      }
      if (millis() - tm > HOLD_TIMER)
      {
        startPortal();
      }
    }

    if (millis() - millisPrint > HOLD_TIMER)
    {
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.SSID());
      Serial.println("Connection: " + connectionOption);
      Serial.println("Username: " + usernameMQTT);
      Serial.println("Password: " + passwordMQTT);
      Serial.println("Device Key: " + deviceKey);
      Serial.println("Auth username: " + authUsername);
      Serial.println("Auth password: " + authPassword);
      millisPrint = millis();
    }

    //Blink LED if WiFi not connected
    if (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - ledtm >= ledInterval)
      {
        ledtm = millis();
        if (ledState == LOW)
        {
          ledState = HIGH;
        }
        else
        {
          ledState = LOW;
        }
        digitalWrite(2, ledState);
      }
    }
    // #if defined(ARDUINO_ARCH_ESP8266)
        while(!timeClient.update()){
          timeClient.forceUpdate();
        }
        
    // #endif
        sparksMQTT.updateCredential(usernameMQTT, passwordMQTT, deviceKey, chipId);
        sparksMQTT.run();
        webServer.handleClient();
        Portal.handleRequest();
      }
};
