#include <SparksLib.h>
#include <SparksTask.h>

SparksLib sparks;
SparksTask task;

void demoTask()
{
    if (mqttClient.connected())
    {
        float powerConsumption = random(100000, 500000) / 100.00;

        sparksMQTT.publish("demo/office", 1, powerConsumption);
    }
}

void setup()
{
    sparks.begin();
    task.setTask("power", 15000L, demoTask,0);
}

void loop()
{
    sparks.run();
    task.run();
}