// This example will use Kalman.h file. Please move Kalman.h file to 'src' folder

#include <SparksLib.h>
#include <SparksTask.h>
//#include <Kalman.h>

SparksLib sparks;
SparksTask task;

//define Kalman Filter Function
//Kalman myFilter(0.125,32,32,0);
double measurement, filteredMeasurement;

//handler for relay
// D1 for relay1, D2 for relay2, D3 for relay3, D4 for relay4
void relay1(String &value)
{
    bool data = sparksMQTT.asBoolean(value);
    Serial.print("Data: ");
    Serial.println(data);
    if (data == false)
    {
        digitalWrite(D1, HIGH);
    }
    else
    {
        digitalWrite(D1, LOW);
    }
}

void relay2(String &value)
{
    bool data = sparksMQTT.asBoolean(value);
    Serial.print("Data: ");
    Serial.println(data);
    if (data == false)
    {
        digitalWrite(D2, HIGH);
    }
    else
    {
        digitalWrite(D2, LOW);
    }
}


//building function to read sensor value and calibrate with datasheet
void readSensor(){
    int sensorValue = analogRead(A0);

    float outvoltage = sensorValue * (3.3 / 1023.0);
    sparksMQTT.publish("demo/windturbine",1, outvoltage);

    float Level = (6*5/3.3)*outvoltage;//The level of wind speed is proportional to the output voltage.
    sparksMQTT.publish("demo/windturbine",2,Level);

    //filteredMeasurement = myFilter.getFilteredValue(Level); //applying Kalman Filter
    //sparksMQTT.publish("demo/windturbine",3,filteredMeasurement);
  
    Serial.println("=======Reading Wind Speed=========");
    Serial.print("outvoltage = ");
    Serial.print(outvoltage);
    Serial.println("V");

    Serial.print("Wind Speed: ");
    Serial.print(Level);
    Serial.println(" m/s");

    Serial.print("Filtered Measurement: ");
    //Serial.print(filteredMeasurement);
    Serial.println(" m/s");
    Serial.println("=======Finished Reading=========");
    Serial.println();
}

void setup()
{
    pinMode(A0,INPUT);
    pinMode(D1,OUTPUT);
    pinMode(D2,OUTPUT);
    pinMode(D3,OUTPUT);
    pinMode(D4,OUTPUT);

    sparks.begin();

    //setting up repetitive task in given time interval
    task.setTask("readSensor", 1000L, readSensor, 0);

    sparksMQTT.callbackControl("relay1", "demo/windturbine", -3, relay1);
    sparksMQTT.callbackControl("relay2", "demo/windturbine", -4, relay2);
}

void loop()
{

    sparks.run();
    task.run();
    delay(10);
}
