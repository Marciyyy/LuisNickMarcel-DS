#include "button.h"
#include "ampel_mqtt.h"
#include "main.h"
#include "lwip/apps/mqtt.h"
#include <string.h>

static GPIO_PinState button_input       = GPIO_PIN_RESET;
static uint32_t      button_press_time  = 0;
static uint8_t       button_pending     = 0;
static uint8_t       button_event       = 0;

int test1 = 0;
int test2 =0;
int test3 = 0;
int test4 = 0;
int test5 = 0;

static void mqtt_pub_request_cb_button(void *arg, err_t result)
{
    (void)arg; (void)result;
}

void Button_Update(void)
{
	test1 = 1;
    GPIO_PinState current = HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5);
    uint32_t now = HAL_GetTick();

    if ((current != button_input) && (current == GPIO_PIN_SET))
    {
    	test2 = 1;
        button_input      = current;
        button_press_time = now;
        button_pending    = 1;

    }

    if ((current != button_input) && (current == GPIO_PIN_RESET))
    {
    	test3 = 1;
        button_input = GPIO_PIN_RESET;
    }

    if (button_pending && (now - button_press_time >= ENTPRELLUNGS_ZEIT_MS))
    {
    	test4 = 1;

        button_event   = 1;
        button_pending = 0;
    }
}

void Button_Process(void)
{
    if (button_event)
    {
    	test5 = 1;
        button_event = 0;
        mqtt_publish(mqtt_client, "Input/A/Button", "true", strlen("true"),
                     0, 0, mqtt_pub_request_cb_button, NULL);
    }
}
