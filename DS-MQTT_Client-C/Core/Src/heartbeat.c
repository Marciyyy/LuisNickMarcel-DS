#include "heartbeat.h"
#include "ampel_mqtt.h"
#include "main.h"
#include "lwip/apps/mqtt.h"
#include <string.h>

uint8_t idle = 0;

uint32_t lastHeartbeatMaster;
uint32_t lastHeartbeatA;
uint32_t lastHeartbeatB;

static uint32_t mqtt_last_publish = 0;

static void mqtt_pub_request_cb_heartbeat(void *arg, err_t result)
{
    (void)arg; (void)result;
}

void Heartbeat_Send(void)
{
    if (mqtt_connected && (HAL_GetTick() - mqtt_last_publish > 500))
    {
        mqtt_last_publish = HAL_GetTick();
        mqtt_publish(mqtt_client, "Heart/C", "true", strlen("true"), 0, 0,
                     mqtt_pub_request_cb_heartbeat, NULL);
    }
}

void Heartbeat_Monitor(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - lastHeartbeatMaster > HEARTBEAT_TIMEOUT_MS) ||
        (now - lastHeartbeatA      > HEARTBEAT_TIMEOUT_MS) ||
        (now - lastHeartbeatB      > HEARTBEAT_TIMEOUT_MS))
    {
        idle = 1;
    }
    else
    {
        idle = 0;
    }
}
