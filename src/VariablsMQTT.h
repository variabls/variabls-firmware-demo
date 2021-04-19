#ifndef VARIABLS_MQTT_H
#define VVARIABLS_MQTT_H

#include <MQTT/src/MQTT.h>
#include <WiFiClient.h>
#include <ArduinoJson/ArduinoJson.h>
#include <NTPClient/NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <VariablsTime.h>
#include <string.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <Ping/src/ESP8266Ping.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <Ping/src/ESP32Ping.h>
#endif

WiFiClient netWifi;
MQTTClient mqttClient(512); //MQTT
String ___usernameMQTT, ___passwordMQTT, ___deviceKey, ___chipID;
const char *remote_testhost = "www.google.com";

typedef void (*mqtt_control_callback)(String &Values); //MQTT
typedef struct                                         //MQTT
{
  String name;
  int Pin;
  String topic;
  mqtt_control_callback callback;
  String value;
} callbackControl_t;
callbackControl_t control[10]; //MQTT

static int numControl; //MQTT
// int controlKey[10];
std::vector<int> controlKey;

bool confirmmsg = false; //MQTT
int queuedId;            //MQTT
unsigned long startmillis;

class VariablsMQTT
{
public:
  // VariablsMQTT();

  void updateCredential(String _usernameMQTT, String _passwordMQTT, String _deviceKey, String _chipID)
  {
    ___usernameMQTT = _usernameMQTT;
    ___passwordMQTT = _passwordMQTT;
    ___deviceKey = _deviceKey;
    ___chipID = _chipID;
  };

  void run()
  {
    mqttClient.loop();
    //handler to reconnect
    if (!mqttClient.connected())
    {
      Serial.println("[loop]: MQTT Not connected. Reconnecting...");
      connectMQTT();
    }
    else
    {
      //Heartbeat connection
      if (___deviceKey != "")
      {
        if (millis() - prev_millis_hb >= 10000L)
        {
          unsigned long skipTimes = (millis() - prev_millis_hb) / 10000L;
          prev_millis_hb += 10000L * skipTimes;
          String Topic = "heartbeat/" + ___deviceKey;
          const char *__Topic = Topic.c_str();
          Serial.println(Topic);

          if (mqttClient.publish(__Topic, "I'm vine!", true, 1))
          {
            Serial.println("[loop]: heartbeat success!");
          }
        }
        if (confirmmsg)
        {
          String topicconfirm = "confirm/" + control[queuedId].topic + "/" + ___deviceKey;
          const char *_topicconfirm = topicconfirm.c_str();
          if (mqttClient.publish(_topicconfirm, control[queuedId].value, true, 1))
          {
            Serial.print("value: ");
            Serial.println(control[queuedId].value);
            Serial.println("confirmed published");
            confirmmsg = false;
          }
        }

        // if(uxQueueMessagesWaiting(queue)!=0){
        //   // callbackControl_t controlQueued;
        //   int controlid;
        //   xQueueReceive(queue, &controlid, portMAX_DELAY);
        //   String topicconfirm = "confirm/" + control[controlid].topic + deviceKey;
        //   const char* _topicconfirm = topicconfirm.c_str();
        //   if(mqttClient.publish(_topicconfirm, dataJSON(control[controlid].Pin, control[controlid].value), true, 1)){
        //     Serial.println("confirmed published");
        //   }
        //   // publish(controlQueued.topic.c_str(), controlQueued.Pin, controlQueued.value);
        // }
      }
    }
  }

  void begin()
  {
    mqttClient.begin(mqttServer, port, netWifi);
    mqttClient.onMessage(messageReceived);
    String willtopic = "stream/lwt/test/" + ___deviceKey;
    const char * _willtopic = willtopic.c_str();
    mqttClient.setWill(_willtopic, "I'm not vine", true, 1);
    // queue = xQueueCreate(1,1024);
  }

  template <typename T>
  String dataJSON(int Pin, T value)
  {
    StaticJsonDocument<512> doc;
    JsonObject object = doc.createNestedObject("Datapoint");
    doc["Timestamp"] = localTime();
    object[String(Pin)] = value;
    char datajson[512];
    serializeJson(doc, datajson);
    return String(datajson);
  }

  template <typename T>
  bool publish(const char *topic, int Pin, T values)
  {
    // const char* _devicekey = deviceKey.c_str();
    String Topic = "stream/" + String(topic) + "/" + ___deviceKey;
    const char *_topic = Topic.c_str();

    // StaticJsonDocument<512> doc;
    // JsonObject object = doc.createNestedObject("Datapoint");
    // doc["Timestamp"] = localTime();
    // object[String(Pin)] = values;
    String msg = dataJSON(Pin, values);
    // serializeJson(doc, msg);

    if (mqttClient.publish(_topic, msg, true, 1))
    {
      Serial.println("[" + String(_topic) + "]: " + "publish success: " + msg);
      return true;
    }
  }

  template <typename D>
  const char *VariablsData(int Pin, D values)
  {
    StaticJsonDocument<512> data;
    data[String(Pin)] = values;
    char _data[512];
    serializeJson(data, _data);
    return _data;
  }

  bool callbackControl(String name, String substopic, int Pin, mqtt_control_callback c)
  {
    int freeControl;
    if (numControl < 0)
    {
      for (int i = 0; i < MAX_CONTROL; i++)
      {
        // memset(&control[i], 0, sizeof(control));
        memset(&control, 0, sizeof(control));
        numControl = 0;
      }
    }

    freeControl = findFreeSlot();
    if (freeControl < 0)
    {
      return -1;
    }

    control[freeControl].name = name;
    control[freeControl].topic = substopic;
    control[freeControl].Pin = Pin;
    control[freeControl].callback = c;

    numControl++;

    return freeControl;
  }

  bool connectMQTT()
  {
    int Errstat;
    if (checkNetwork(&Errstat))
    {
      if (___usernameMQTT || ___passwordMQTT != "")
      {
        const char *__usernameMQTT = ___usernameMQTT.c_str();
        Serial.println(__usernameMQTT);
        // String deviceid_ = deviceID(true);
        const char *__deviceid = ___chipID.c_str();
        const char *__passwordMQTT = ___passwordMQTT.c_str();
        Serial.println(__passwordMQTT);
        Serial.println(__deviceid);
        if (mqttClient.connect(__deviceid, __usernameMQTT, __passwordMQTT))
        {
          Serial.println("[connect MQTT]: Connect to broker success!");
          String substopic = "emit/*/" + ___deviceKey;
          String substopic2 = "emit/*/*/" + ___deviceKey;
          String substopic3 = "emit/*/*/*/" + ___deviceKey;
          String substopic4 = "emit/*/*/*/*" + ___deviceKey;
          String substopic5 = "emit/*/*/*/*/*/" + ___deviceKey;
          const char *_substopic = substopic.c_str();
          const char *_substopic2 = substopic2.c_str();
          const char *_substopic3 = substopic3.c_str();
          const char *_substopic4 = substopic4.c_str();
          const char *_substopic5 = substopic5.c_str();

          mqttClient.subscribe(_substopic);
          mqttClient.subscribe(_substopic2);
          mqttClient.subscribe(_substopic3);
          mqttClient.subscribe(_substopic4);
          mqttClient.subscribe(_substopic5);

          if (numControl > 0)
          {
            for (int i = 0; i < numControl; i++)
            {
              String topiclaststate = "laststate/" + control[i].topic + "/" + ___deviceKey;
              const char *_topiclaststate = topiclaststate.c_str();
              mqttClient.publish(_topiclaststate, String(control[i].Pin));
            }
          }
          return true;
        }
        else
        {
          Serial.println("[connect MQTT]: Connect to broker failed. Please check the credential");
          return false;
        }
      }
      else
      {
        Serial.println("[connect MQTT]:Credential username and/or password empty");
        return false;
      }
    }
    // else {
    //   Serial.println("Network failed!");
    // }
  }

  bool filter_by_name(String name, int size, int &controlId)
  {
    // bool found = false;
    for (int i = 0; i < size; i++)
    {
      if (name == control[i].name)
      {
        controlId = i;
        return true;
      }
    }
    return false;
  }

  static bool filter_by_topic(String topic, int size, std::vector<int> &controlKey)
  {
    // bool found = false;
    for (int i = 0; i < size; i++)
    {
      if (topic == control[i].topic)
      {
        controlKey.push_back(i);

        // return true;
      }
    }
    Serial.print("controlKey: ");
    for (int i = 0; i < controlKey.size(); i++)
    {
      Serial.print(controlKey[i]);
      Serial.print(", ");
    }
    Serial.println("");

    if (controlKey.empty())
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  static bool filter_by_topic_n_pin(String topic, int pin, int size, int controlId)
  {
    // bool found = false;
    for (int i = 0; i < size; i++)
    {
      if (pin == control[i].Pin && topic == control[i].topic)
      {
        controlId = i;
        return true;
      }
    }
    return false;
  }

  static bool filter_by_topic(String topic, int size, int &controlId)
  {
    // bool found = false;
    for (int i = 0; i < size; i++)
    {
      if (topic == control[i].topic)
      {
        controlId = i;
        return true;
      }
    }
    return false;
  }

  bool asBoolean(String value)
  {
    // std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    // std::istringstream is(value);
    bool b = false;
    if (value == "true")
    {
      b = true;
    }
    else if (value == "false")
    {
      b = false;
    }
    return b;
  }

  int asInt(String value)
  {
    return atoi(value.c_str());
  }

private:
  static void messageReceived(String &Variablstopic, String &payload)
  {
    Serial.println("incoming: " + Variablstopic + " - " + payload);
    startmillis = millis();

    char _topic[Variablstopic.length() + 1];
    Variablstopic.toCharArray(_topic, Variablstopic.length() + 1);
    char *strings[10];
    memset(strings, 0, sizeof(strings));

    char *ptr = NULL;
    int index = 0;
    ptr = strtok(_topic, "/");
    while (ptr != NULL)
    {
      strings[index] = ptr;
      index++;
      ptr = strtok(NULL, "/");
    }

    String Topic = String(strings[1]);

    if (index > 3)
    {
      for (int i = 2; i < index - 1; i++)
      {
        Topic += "/";
        Topic += String(strings[i]);
      }
    }

    Serial.println(index);
    Serial.println(Topic);

    static int controlId;

    if (numControl > 0)
    {
      if (filter_by_topic(Topic, numControl, controlKey))
      {
        // String value = "test value";
        // const char * _payload = payload.c_str();

        for (int i = 0; i < controlKey.size(); i++)
        {
          StaticJsonDocument<512> buff;
          deserializeJson(buff, payload);
          JsonObject object = buff.as<JsonObject>();
          String data = object["Datapoint"][String(control[i].Pin)];
          String dat = object["Datapoint"];
          buff.clear();
          object.clear();

          deserializeJson(buff, dat);
          object = buff.as<JsonObject>();

          for (JsonPair keyValue : object)
          {
            String datpin = keyValue.key().c_str();
            if (datpin == String(control[i].Pin))
            {
              Serial.println("masuk");
              control[i].value = payload;
              if (control[i].value != "<nil>" && data != "<nil>")
              {
                confirmmsg = true;
              }
              queuedId = i;
              Serial.print("Callback Data: ");
              Serial.println(data);

              (*(mqtt_control_callback)control[i].callback)(data);
              // buff.clear();
            }
          }

          // StaticJsonDocument<512> DP;
          // deserializeJson(buff, object);
          // JsonObject dat = buff.as<JsonObject>();

          // for (JsonPair keyValue : dat)
          //   {
          //     Serial.print("Key: ");
          //     Serial.println(keyValue.key().c_str());
          //     Serial.print("Value: ");
          //     Serial.println(keyValue.value().containsKey(payload));
          //   }
        }
        // commit
        unsigned long endmillis = millis();
        Serial.println("Filtering Time elapsed: " + String(endmillis - startmillis));
        Serial.println(payload);

        unsigned long Pendmillis = millis();
        Serial.println("Parsing Time elapsed: " + String(Pendmillis - startmillis));

        // xQueueSend(queue, &controlId, portMAX_DELAY);
        Serial.println("---------------------");
      }
      else
      {
        Serial.println("Error loading data");
      }
      controlKey.clear();
    }
    // Note: Do not use the client in the callback to publish, subscribe or
    // unsubscribe as it may cause deadlocks when other things arrive while
    // sending and receiving acknowledgments. Instead, change a global variable,
    // or push to a queue and handle it in the loop after calling `client.loop()`.
  }

  bool checkNetwork(int *Err)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      *Err = -10;
      Serial.println("[check network]: Wifi not connected");
      return false;
    }
    else
    {
      if (!Ping.ping(remote_testhost, 2))
      {
        Serial.println("[check network]: There is no internet!");
        *Err = -11;
        return false;
      }
      else
      {
        Serial.println("[check network]: Network ready");
        return true;
      }
    }
  }

  // typedef struct {
  //   int Pin;
  //   const char* topics;
  // }subs_topic_t;

  // subs_topic_t substopic[10];

  // template <typename D>

  int findFreeSlot()
  {
    //no slot available
    if (numControl >= MAX_CONTROL)
    {
      return -1;
    }

    for (int i = 0; i < 10; i++)
    {
      if (control[i].name == NULL)
      {
        return i;
      }
    }

    return -1;
  }

  // const char* data(int value){
  //   char str[2 + 8 * sizeof(value)];
  //   return itoa(value, str, 10);
  // }

  // const char* data(unsigned int value){
  //   char str[1 + 8 * sizeof(value)];
  //   return utoa(value, str, 10);
  // }

  // const char* data(long value){
  //   char str[2 + 8 * sizeof(value)];
  //   return ltoa(value, str, 10);
  // }

  // const char* data(unsigned long value){
  //   char str[1 + 8 * sizeof(value)];
  //   return ultoa(value, str, 10);
  // }

  // const char* data(float value){
  //   char str[33];
  //   return dtostrf(value, 4, 3, str);
  // }

  // const char* data(double value){
  //   char str[33];
  //   return dtostrf(value, 5, 7, str);
  // }

  // const char* data(String value){
  //   // char str[33];
  //   return value.c_str();
  // }

  unsigned long prev_millis_hb = 0;

  const char *mqttServer = "broker.Variabls.id";
  const int port = 1883;
  const int portSecure = 1883;

  const static int MAX_CONTROL = 10;
};

#endif //Variabls_MQTT_H