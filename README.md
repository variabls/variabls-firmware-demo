
![Logo](https://user-images.githubusercontent.com/29002137/115189743-2b6cd200-a111-11eb-8adf-ce4ca1f01cb3.png)

# variabls-library
Variabls Firmware Library to Connect Device to [variabls](demo.variabls.com) Platform.

Our works is an open source. Feel free to add, modify, and use it.

For full Documentation, please visit [here](https://variabls.gitbook.io/welcome/).

## quick usage
To use this library, simply download the library and paste it into your 'lib' project folder. This code below is the simple example of 'main.cpp' code to use the Variabls Library:

```c++
#include <VariablsLib.h>
#include <VariablsTask.h>

VariablsLib Variabls;
VariablsTask task;

void demoTask()
{
    if (mqttClient.connected())
    {
        float sendRandom = random(100000, 500000) / 100.00;

        VariablsMQTT.publish("demo/test", 1, sendRandom);
    }
}

void setup()
{
    Variabls.begin();
    task.setTask("testRandom", 15000L, demoTask,0);
}

void loop()
{
    Variabls.run();
    task.run();
}
```
It will push random number every 15000 ms to topic 'demo/test' forever.  


See the [more code example](https://variabls.gitbook.io/welcome/sparks-library/example-code)
For full Documentation, please visit [here](https://variabls.gitbook.io/welcome/).

## Acknowledgement
Many thanks to those who build an amazing open-source code and still maintain it up till today. The inspiration came from this amazing Author:
1. [Autoconnect](https://github.com/Hieromon/AutoConnect) by Hieromon
2. [ArduinoMQTT](https://github.com/256dpi/arduino-mqtt) by 256dpi
3. [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) by bblanchon
4. [NTPClient](https://github.com/arduino-libraries/NTPClient) by Arduino





## Contributor
Hamid Rasyid
Muhammad Sidargo
