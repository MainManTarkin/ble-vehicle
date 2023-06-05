#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

//defines

#define vehicalLog "tankEventLog"

//enums

enum vehicalEventTypes{

    motorValChange,
    rgbValChange,
    hornValChange


};

//structs


struct vehicalEvent{

    char eventType;
    size_t eventValLen;
    u_int64_t *eventVal;


};


//global Externs

volatile extern u_int32_t rbgValue;
volatile extern u_int8_t  hornValue;
volatile extern unsigned long motorControlValue[2];
extern TaskHandle_t vehicalEventHandle;
extern bool vehicleDisconnectFlag;

void initVehicleControl();

void vehicalValUpdate(enum vehicalEventTypes eventType);

void vehicalEventLoop();