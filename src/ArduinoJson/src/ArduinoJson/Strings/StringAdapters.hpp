// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2020
// MIT License

#pragma once

#include <ArduinoJson/src/ArduinoJson/Strings/ConstRamStringAdapter.hpp>
#include <ArduinoJson/src/ArduinoJson/Strings/RamStringAdapter.hpp>
#include <ArduinoJson/src/ArduinoJson/Strings/SizedRamStringAdapter.hpp>

#if ARDUINOJSON_ENABLE_STD_STRING
#include <ArduinoJson/src/ArduinoJson/Strings/StdStringAdapter.hpp>
#endif

#if ARDUINOJSON_ENABLE_ARDUINO_STRING
#include <ArduinoJson/src/ArduinoJson/Strings/ArduinoStringAdapter.hpp>
#endif

#if ARDUINOJSON_ENABLE_PROGMEM
#include <ArduinoJson/src/ArduinoJson/Strings/FlashStringAdapter.hpp>
#include <ArduinoJson/src/ArduinoJson/Strings/SizedFlashStringAdapter.hpp>
#endif
