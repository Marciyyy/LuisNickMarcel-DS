#include "sensor.h"
#include "ampel_mqtt.h"
#include "main.h"
#include "lwip/apps/mqtt.h"
#include <string.h>

extern ADC_HandleTypeDef hadc3;

static uint8_t  sensor_dunkel           = 0;
static uint8_t  sensor_event            = 0;
static uint32_t sensor_dunkel_start     = 0;
static uint32_t sensor_last_publish     = 0;

static void mqtt_pub_request_cb_sensor(void *arg, err_t result)
{
    (void)arg; (void)result;
}

static uint32_t Sensor_Read_ADC(void)
{
    uint32_t value = 0;

    HAL_ADC_Start(&hadc3);
    if (HAL_ADC_PollForConversion(&hadc3, 8) == HAL_OK)
    {
        value = HAL_ADC_GetValue(&hadc3);
    }
    HAL_ADC_Stop(&hadc3);

    return value;
}

void Sensor_Update(void)
{
    uint32_t adc  = Sensor_Read_ADC();
    uint32_t now  = HAL_GetTick();

    if (adc < SENSOR_ADC_GRENZWERT)
    {
        if (!sensor_dunkel)
        {
            sensor_dunkel       = 1;
            sensor_dunkel_start = now;
        }
        if ((now - sensor_dunkel_start) >= SENSOR_DUNKEL_ZEIT_MS)
        {
            sensor_event = 1;
        }
    }
    else
    {
        sensor_dunkel = 0;
        sensor_event  = 0;
    }
}

void Sensor_Process(void)
{
    uint32_t now = HAL_GetTick();

    if (sensor_event && (now - sensor_last_publish >= SENSOR_PUBLISH_COOLDOWN_MS))
    {
        sensor_event        = 0;
        sensor_last_publish = now;
        mqtt_publish(mqtt_client, "Input/A/Sensor", "true", strlen("true"),
                     0, 0, mqtt_pub_request_cb_sensor, NULL);
    }
}
