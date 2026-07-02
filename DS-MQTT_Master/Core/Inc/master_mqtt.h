#ifndef MASTER_MQTT_H
#define MASTER_MQTT_H

#include "master_config.h"
#include "lwip/apps/mqtt.h"
#include "lwip/err.h"

extern mqtt_client_t *mqtt_client;
extern uint8_t mqtt_connected;
extern uint8_t idle;
extern char current_topic[64];
extern uint32_t lastHeartbeatA;
extern uint32_t lastHeartbeatB;
extern uint32_t lastHeartbeatC;

void MQTT_Init_Client(void);
void mqtt_pub_request_cb(void *arg, err_t result);
void MQTT_Custom_Heartbeat(void);
void MQTT_Monitor_Heartbeats(void);

#endif /* MASTER_MQTT_H */
