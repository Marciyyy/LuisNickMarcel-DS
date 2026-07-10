#include "master_mqtt.h"
#include "ampel_control.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>

// MQTT flags:
mqtt_client_t *mqtt_client;
uint8_t mqtt_connected = 0;
static uint32_t mqtt_last_publish = 0;

uint8_t idle = 0;
char current_topic[64];

// Direkt initialisieren, sonst instant idle zustand:
uint32_t lastHeartbeatA;
uint32_t lastHeartbeatB;
uint32_t lastHeartbeatC;

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_sub_request_cb(void *arg, err_t result);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_pub_request_cb_HeartbeatMaster(void *arg, err_t result);

void MQTT_Init_Client(void)
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

  client_info.client_id = "Master";
  client_info.client_user = NULL;
  client_info.client_pass = NULL;
  client_info.keep_alive = 60;

  mqtt_client_connect(mqtt_client, &broker_ip, 1883, mqtt_connection_cb, NULL, &client_info);
}

// Function, wenn Client publishen und subcriben kann:
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  if (status == MQTT_CONNECT_ACCEPTED)
  {
    mqtt_connected = 1;

    // Um die folgenden zwei lines code erweitert:
    mqtt_set_inpub_callback(client,mqtt_incoming_publish_cb,mqtt_incoming_data_cb,NULL);

    mqtt_subscribe(client, "Heart/#", 0, mqtt_sub_request_cb, NULL);
    mqtt_subscribe(client, "Input/+/Sensor", 0, mqtt_sub_request_cb, NULL);
    mqtt_subscribe(client, "Input/+/Button", 0, mqtt_sub_request_cb, NULL);
  }
  else
  {
    mqtt_connected = 0;
  }
}

void mqtt_pub_request_cb(void *arg, err_t result)
{
  /* result == ERR_OK bedeutet: Publish wurde an lwIP/TCP uebergeben */
}

// Neue funktionen mit Subscriber:
static void mqtt_sub_request_cb(void *arg, err_t result)
{
  // if (result == ERR_OK)
  // {

    /* Subscribe erfolgreich */
	  // printf("\nSubscribe erfolgreich\n");
  // }
  // else
  // {
    /* Subscribe fehlgeschlagen */
  // }

	// Für Debug subscriber:

  if (result == ERR_OK)
      {
          mqtt_publish(mqtt_client,"Debug/Sub","sub_ok", strlen("sub_ok"), 0, 0, mqtt_pub_request_cb, NULL);
      }
      else
      {
          mqtt_publish(mqtt_client,"Debug/Sub","sub_failed", strlen("sub_failed"), 0, 0, mqtt_pub_request_cb, NULL);
      }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  /* Diese Funktion wird aufgerufen, sobald eine neue Nachricht beginnt.
     Hier kennt man Topic und Gesamtlänge der message */
	strncpy(current_topic, topic, sizeof(current_topic)-1); // Damit kein buffer overflow passiert, wird strncpy genutzt
	current_topic[sizeof(current_topic)-1] = '\0';			// Fügt ein \0 ein, damit string ordnungsgemaess endet

	// Fuer Debug subscriber:
	mqtt_publish(mqtt_client,"Debug/Topic",topic,strlen(topic),0,0,mqtt_pub_request_cb,NULL);

	// Ticks der Heartbeats werden gespeichert:
	// Da inhalt der heartbeats egal ist wird das in der incoming_publish gemacht und nicht in der incoming_data:
	if(strcmp(topic, "Heart/A") == 0)	// strcmp vergleicht zwei strings, wenn sie gleich sind =0, ungleich != 0
	{
		lastHeartbeatA = HAL_GetTick();
	}
	else if(strcmp(topic, "Heart/B") == 0)
	{
		lastHeartbeatB = HAL_GetTick();
	}
	else if(strcmp(topic, "Heart/C") == 0)
	{
		lastHeartbeatC = HAL_GetTick();
	}
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
  /* Diese Funktion enthält die eigentlichen Nutzdaten der MQTT-Nachricht. */

	// Master verarbeitet sensor und button inputs und packt sie in die Liste:

	uint8_t request = get_request_from_topic(current_topic);

	if (request == 0)
	{
		return; // kein Request-Topic oder fehler
	}

	if (len >= 4 && strncmp((const char *)data, "true", 4) == 0)
	{
		// Fuer Debug Subscriber:
		char msg[32];
		snprintf(msg, sizeof(msg), "request=%d", request);
		mqtt_publish(mqtt_client, "Debug/Request", msg, strlen(msg), 0, 0, mqtt_pub_request_cb, NULL);

		process_request(request);
	}
	else if (len >= 5 && strncmp((const char *)data, "false", 5) == 0)
	{
		// Aktuell nichts tun.
		// false: kein neuer Request.
	}
}

// Versende eigenen Heartbeat
void MQTT_Custom_Heartbeat(void)
{
	if (mqtt_connected && (HAL_GetTick() - mqtt_last_publish > 500))
	{
		mqtt_last_publish = HAL_GetTick();
		mqtt_publish(mqtt_client, "Heart/Master", "true", strlen("true"), 0, 0, mqtt_pub_request_cb_HeartbeatMaster, NULL);
	}
}

// wird ausgefuehrt, nachdem man seinen eigenen Heartbeat veröffentlicht hat:
static void mqtt_pub_request_cb_HeartbeatMaster(void *arg, err_t result)
{
  /* result == ERR_OK bedeutet: Publish wurde an lwIP/TCP übergeben */
}

// Empfange Heartbeats und "verarbeite" sie:
void MQTT_Monitor_Heartbeats(void)
{
	uint32_t now = HAL_GetTick();

	if ( (now - lastHeartbeatA 		> HEARTBEAT_TIMEOUT_MS) ||
	     (now - lastHeartbeatB      > HEARTBEAT_TIMEOUT_MS) ||
	     (now - lastHeartbeatC      > HEARTBEAT_TIMEOUT_MS) )
	{
		idle = 1;
	}
	else
	{
		idle = 0;
	}
}
