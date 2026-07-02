#ifndef AMPEL_MQTT_H
#define AMPEL_MQTT_H

#include "lwip/apps/mqtt.h"
#include "lwip/err.h"

extern mqtt_client_t *mqtt_client;
extern uint8_t mqtt_connected;

void Ampel_MQTT_Init(void);

#endif /* AMPEL_MQTT_H */
