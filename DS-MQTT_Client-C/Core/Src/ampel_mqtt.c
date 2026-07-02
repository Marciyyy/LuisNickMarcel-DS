#include "ampel_mqtt.h"
#include "main.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include <string.h>

mqtt_client_t *mqtt_client;
uint8_t mqtt_connected = 0;

extern uint32_t lastHeartbeatMaster;
extern uint32_t lastHeartbeatA;
extern uint32_t lastHeartbeatB;
extern char current_topic[64];

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

void Ampel_MQTT_Init(void)
{
    ip_addr_t broker_ip;
    struct mqtt_connect_client_info_t client_info;

    IP4_ADDR(&broker_ip, 141, 37, 158, 138);

    mqtt_client = mqtt_client_new();
    if (mqtt_client == NULL)
    {
        Error_Handler();
    }

    memset(&client_info, 0, sizeof(client_info));
    client_info.client_id  = "SlaveC";
    client_info.client_user = NULL;
    client_info.client_pass = NULL;
    client_info.keep_alive  = 60;

    mqtt_client_connect(mqtt_client, &broker_ip, 1883, mqtt_connection_cb, NULL, &client_info);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        mqtt_connected = 1;
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
        mqtt_subscribe(client, "Heart/#",    0, mqtt_sub_request_cb, NULL);
        mqtt_subscribe(client, "Ampel/C/#", 0, mqtt_sub_request_cb, NULL);
    }
    else
    {
        mqtt_connected = 0;
    }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
    (void)arg; (void)result;
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    (void)arg; (void)tot_len;

    strncpy(current_topic, topic, sizeof(current_topic) - 1);
    current_topic[sizeof(current_topic) - 1] = '\0';

    if (strcmp(topic, "Heart/Master") == 0)
        lastHeartbeatMaster = HAL_GetTick();
    else if (strcmp(topic, "Heart/A") == 0)
        lastHeartbeatA = HAL_GetTick();
    else if (strcmp(topic, "Heart/B") == 0)
        lastHeartbeatB = HAL_GetTick();
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    (void)arg; (void)len; (void)flags;

    if (strcmp(current_topic, "Ampel/C/Fuss") == 0)
    {
        if (strncmp((const char *)data, "red", 3) == 0)
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, GPIO_PIN_SET);
        }
        else if (strncmp((const char *)data, "green", 5) == 0)
        {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, GPIO_PIN_RESET);
        }
    }

    if (strcmp(current_topic, "Ampel/C/Haupt") == 0)
    {
        if (strncmp((const char *)data, "red", 3) == 0)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);
        }
        else if (strncmp((const char *)data, "orange", 6) == 0)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
        }
        else if (strncmp((const char *)data, "green", 5) == 0)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);
        }
    }

    if (strcmp(current_topic, "Ampel/C/Abbiege") == 0)
    {
        if (strncmp((const char *)data, "green", 5) == 0)
        {
        	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, GPIO_PIN_SET);
        }
        else if (strncmp((const char *)data, "red", 3) == 0)
        {
        	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, GPIO_PIN_RESET);
        }
    }
}
