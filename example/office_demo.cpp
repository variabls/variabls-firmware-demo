#include <SparksLib.h>
#include <SparksTask.h>

SparksLib sparks;
SparksTask task;

bool three, four, five, six = 0;


void powerConsumption()
{
    if (mqttClient.connected()){
        float powerConsumption = random(100000, 500000)/100.00;

        sparksMQTT.publish("demo/office", 1, powerConsumption);
    }
}

void waterConsumption()
{
    if (mqttClient.connected()){
        float waterConsumption = random(1, 500)/100.00;

        sparksMQTT.publish("demo/office", 2, waterConsumption);
    }
}

void voltageCurrentAir()
{
    if (mqttClient.connected()){
        float voltage = random(32800, 33500)/100.00;
        float current = random(900, 1200)/100.00;
        float air = random(3800, 5000)/100.00;


        sparksMQTT.publish("demo/office", 3, voltage);
        sparksMQTT.publish("demo/office", 4, current);
        sparksMQTT.publish("demo/office", 5, air);
    }
}

void tempHumid()
{
    if (mqttClient.connected()){
        float temp = random(2300, 2800)/100.00;
        float humidity = random(4000, 5500)/100.00;

        sparksMQTT.publish("demo/office", 6, temp);
        sparksMQTT.publish("demo/office", 7, humidity);
    }
}

void oxygen()
{
    if (mqttClient.connected()){
        float oxy = random(2000, 2300)/100.00;

        sparksMQTT.publish("demo/office", 8, oxy);
    }
}

void billing()
{
    if (mqttClient.connected()){
        float billing = random(15000000, 100000000)/100.00;

        sparksMQTT.publish("demo/office", 9, billing);
    }
}

void thTemp()
{
    if (mqttClient.connected()){
        float thTemperature = random(2800, 5000)/100.00;

        sparksMQTT.publish("demo/office", 6, thTemperature);
    }
}
void thHumid()
{
    if (mqttClient.connected()){
        float thHumidity = random(5500, 7000)/100.00;

        sparksMQTT.publish("demo/office", 7, thHumidity);
    }
}
void thOxy()
{
    if (mqttClient.connected()){
        float thOxygen = random(2300, 2700)/100.00;

        sparksMQTT.publish("demo/office", 8, thOxygen);
    }
}

void satu(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(0, LOW);
    }
    else
    {
        digitalWrite(0, HIGH);
    }
}

void dua(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(2, LOW);
    }
    else
    {
        digitalWrite(2, HIGH);
    }
}

void tiga(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(three, LOW);
    }
    else
    {
        digitalWrite(three, HIGH);
    }
}

void empat(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(four, LOW);
    }
    else
    {
        digitalWrite(four, HIGH);
    }
}

void lima(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(five, LOW);
    }
    else
    {
        digitalWrite(five, HIGH);
    }
}

void enam(String &value)
{
    int data = sparksMQTT.asInt(value);
    Serial.print("Data: ");
    Serial.println(data);
    // digitalWrite(D0, data);
    if (data == 1)
    {
        digitalWrite(six, LOW);
    }
    else
    {
        digitalWrite(six, HIGH);
    }
}

void setup()
{
    pinMode(0,OUTPUT);
    pinMode(2,OUTPUT);
    sparks.begin();

    task.setTask("power", 15000L, powerConsumption,0);
    task.setTask("water", 12000L, waterConsumption,0);
    task.setTask("VA", 2000L, voltageCurrentAir, 0);
    task.setTask("TH", 4000L, tempHumid, 0);
    task.setTask("oxy", 5000L, oxygen, 0);
    task.setTask("bill", 60000L, billing, 0);

    task.setTask("th1", 25000L, thTemp,0);
    task.setTask("th1", 23000L, thHumid,0);
    task.setTask("th1", 20000L, thOxy,0);
  
    sparksMQTT.callbackControl("room1-lamp", "demo/office", -1, satu);
    sparksMQTT.callbackControl("meetingRoom-lamp", "demo/office", -2, dua);
    sparksMQTT.callbackControl("room2-lamp", "demo/office", -3, tiga);
    sparksMQTT.callbackControl("mushalla-lamp", "demo/office", -4, empat);
    sparksMQTT.callbackControl("toilet-lamp", "demo/office", -5, lima);
    sparksMQTT.callbackControl("room3-AC", "demo/office", -6, enam);
       
}

void loop()
{
    sparks.run();
    task.run();
}