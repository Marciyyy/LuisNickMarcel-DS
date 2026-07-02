#include "ampel_output.h"
#include "master_mqtt.h"
#include <string.h>

/* MQTT Topics */
static const char *led_topics[LED_COUNT] = {
	"Ampel/A/Haupt",
	"Ampel/A/Abbiege",
	"Ampel/B/Haupt",
	"Ampel/B/Abbiege",
	"Ampel/C/Haupt",
	"Ampel/C/Abbiege",
	"Ampel/A/Fuss",
	"Ampel/C/Fuss"
};

// Initialisiere es mit State Unknown, damit beim aller ersten durchlauf alle LEDs verschickt werden
LedState last_led_state[LED_COUNT] = {LED_STATE_UNKNOWN};

// Und als zweites backup die publish retry queue; Falls ein publish fehlschlaegt:
static RetryItem retry_queue[RETRY_QUEUE_SIZE];
static uint8_t retry_count = 0;
static uint32_t last_retry_time = 0;

static LedState color_to_state(const char *color);

// Hilfs funktion fuer publish_led()
static LedState color_to_state(const char *color)
{
    if (strcmp(color, "red") == 0)
        return LED_STATE_RED;

    if (strcmp(color, "orange") == 0)
        return LED_STATE_ORANGE;

    if (strcmp(color, "green") == 0)
        return LED_STATE_GREEN;

    return LED_STATE_UNKNOWN;
}

void publish_led(uint8_t led, const char *color)
{

	// Speichere die zu schickende farbe im aktuellen LED_State fuer diese Ampel:
	LedState new_state = color_to_state(color);

	// Vergleiche ob es eine veraenderung des LED states gab:
	if (last_led_state[led] == new_state)
	{
		return; // Zustand unveraendert, deshlab kein erneutes publish um den "publish" speicher zu schonen
	}

	// Publishen der neuen states:
	err_t err = mqtt_publish(mqtt_client, led_topics[led], color, strlen(color), 0, 0, mqtt_pub_request_cb, NULL);

	// Wenn publish geglueckt, dann speichere den aktuellen LED state der aktuellen Ampel in last_led_state fuer die naechste abfrage
	if (err == ERR_OK)
	{
		last_led_state[led] = new_state;
	}
	else // Wenn publish nicht funktioniert, dann den publish versuch in die retry_queue geben
	{
		if (retry_count < RETRY_QUEUE_SIZE)	// Test ob in der queue noch platz ist
		{
			retry_queue[retry_count].led = led;
			retry_queue[retry_count].state = new_state;
			retry_queue[retry_count].color = color;
			retry_count++;
		}
	}

}

void process_publish_retries(void)
{
    uint32_t now = HAL_GetTick();

    // Check ob retry liste leer ist:
    if (retry_count == 0)
    {
        return;
    }
    // Check ob Rety Interval abgelaufen ist:
    else if (now - last_retry_time < RETRY_INTERVAL_MS)
	{
        return;
    }

    last_retry_time = now;

    RetryItem item = retry_queue[0];

    err_t err = mqtt_publish(mqtt_client, led_topics[item.led], item.color, strlen(item.color), 0, 0, mqtt_pub_request_cb, NULL);

    if (err == ERR_OK)
    {
        // Wenn publish dieses mal geklappt, dann den state der LED in last_led_state speichern
        last_led_state[item.led] = item.state;

        // retry queue aufruecken lassen:
        for (uint8_t i = 0; i < retry_count - 1; i++)
        {
            retry_queue[i] = retry_queue[i + 1];
        }
        retry_count--;

	// Falls err != ERR_OK wuerde die LED mit ihrer color in der rety queue bleiben und es wuerde nochmal nach 10 ms (RETRY-INERVAL) probiert werden
    }
}
